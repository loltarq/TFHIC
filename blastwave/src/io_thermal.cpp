#include "io_thermal.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

#if __has_include(<nlohmann/json.hpp>)
  #include <nlohmann/json.hpp>
  using json = nlohmann::json;
#else
  // Fallback: vendor the single-header locally if needed:
  #include "third_party/json.hpp"
  using json = nlohmann::json;
#endif

// ---------- internal helpers ----------

static bool load_json_file(const std::string& path, json& j){
  std::ifstream in(path);
  if(!in){ std::cerr << "[io_thermal] cannot open JSON: " << path << "\n"; return false; }
  try { in >> j; } catch(const std::exception& e){
    std::cerr << "[io_thermal] JSON parse error: " << e.what() << "\n"; return false;
  }
  return true;
}

static int pick_bin_closest(const json& bins,
                             const std::string& wantMode,
                             double wantK, double wantNch, double wantV)
{
  const int n = (int)bins.size();
  int best = 0; double bestScore = 1e300;

  for(int i=0;i<n;i++){
    const json& b = bins[i];
    double score = 0.0;

    if(!wantMode.empty() && b.contains("mode")){
      const std::string bm = b["mode"].get<std::string>();
      if(bm != wantMode){ score = 1e12; } // hard reject different mode
    }

    if(std::isfinite(wantK)   && b.contains("k"))
      score += std::abs(b["k"].get<double>() - wantK) * 100.0;

    if(std::isfinite(wantNch) && b.contains("Nch"))
      score += std::abs(b["Nch"].get<double>() - wantNch);

    if(std::isfinite(wantV)   && b.contains("dVdy"))
      score += std::abs(b["dVdy"].get<double>() - wantV) * 0.1;

    if(score < bestScore){ bestScore = score; best = i; }
  }
  return best;
}

static const char* key_for(ThermalYieldKind k){
  return (k==ThermalYieldKind::Total) ? "dNdy_total" : "dNdy_primary";
}

// ---------- public API ----------

bool load_yields_from_json(const ThermalSelection& sel,
                           std::map<int,double>& outYields,
                           int* used_bin,
                           std::string* out_desc)
{
  json j;
  if(!load_json_file(sel.json_path, j)) return false;

  outYields.clear();
  if(used_bin) *used_bin = -1;
  if(out_desc) out_desc->clear();

  // New schema with "bins"
  if(j.contains("bins") && j["bins"].is_array() && !j["bins"].empty()){
    const json& bins = j["bins"];
    int idx = sel.bin_index;

    if(idx < 0){
      // closest-match selection
      const std::string mode = sel.mode;
      const double k    = sel.k;
      const double Nch  = sel.Nch;
      const double V    = sel.dVdy;
      idx = pick_bin_closest(bins, mode, k, Nch, V);
    } else {
      if(idx >= (int)bins.size()) idx = (int)bins.size()-1;
    }

    const json& bin = bins[idx];
    const char* key = key_for(sel.kind);

    if(!bin.contains(key)){
      std::cerr << "[io_thermal] selected bin has no \"" << key << "\" map\n";
      return false;
    }
    for(auto it = bin[key].begin(); it != bin[key].end(); ++it){
      int pdg = std::stoi(it.key());      // keys are strings in the file
      double val = it.value().get<double>();
      outYields[pdg] = val;
    }

    if(used_bin) *used_bin = idx;
    if(out_desc){
      std::ostringstream os;
      os << "mode=" << (bin.contains("mode")? bin["mode"].get<std::string>() : "?")
         << " k="    << (bin.contains("k")?    bin["k"].get<double>() : NAN)
         << " Nch="  << (bin.contains("Nch")?  bin["Nch"].get<double>() : NAN)
         << " dVdy=" << (bin.contains("dVdy")? bin["dVdy"].get<double>() : NAN);
      *out_desc = os.str();
    }
    return true;
  }

  // Legacy flat schema fallback: {"yields_dNdy_total":{...}} or {"yields_dNdy_primary":{...}}
  const char* legacy = (sel.kind==ThermalYieldKind::Total) ? "yields_dNdy_total"
                                                           : "yields_dNdy_primary";
  if(j.contains(legacy) && j[legacy].is_object()){
    for(auto it = j[legacy].begin(); it != j[legacy].end(); ++it){
      int pdg = std::stoi(it.key());
      double val = it.value().get<double>();
      outYields[pdg] = val;
    }
    if(out_desc) *out_desc = "legacy flat schema";
    return true;
  }

  std::cerr << "[io_thermal] Unrecognized JSON schema in " << sel.json_path << "\n";
  return false;
}

// ---- CLI parsing for blastwave driver ----

static bool has_flag(int argc, char** argv, const char* name){
  for(int i=1;i<argc;i++) if(std::string(argv[i])==name) return true;
  return false;
}
static const char* take_next_or_die(int& i, int argc, char** argv, const char* flag){
  if(i+1>=argc){ std::cerr << "Missing value after " << flag << "\n"; std::exit(2); }
  return argv[++i];
}

void print_thermal_cli_help(){
  std::cerr <<
"Thermal JSON options:\n"
"  --thermal-json PATH           (required) path to yields JSON (export_dndy_json)\n"
"  --thermal-kind total|primordial   [default: total]\n"
"  --thermal-bin INDEX           (0-based). If omitted, auto-selects the closest bin via params below.\n"
"  --thermal-mode gs|vanilla     (optional) constrain selection by mode.\n"
"  --thermal-k    <double>       (optional) match k value.\n"
"  --thermal-nch  <double>       (optional, mode=gs) match Nch.\n"
"  --thermal-dvdy <double>       (optional, mode=vanilla) match dV/dy.\n";
}

ThermalSelection parse_thermal_cli(int& argc, char**& argv){
  ThermalSelection sel;
  sel.kind = ThermalYieldKind::Total; // default per your request

  // First pass: collect values; second pass could erase flags from argv if desired.
  for(int i=1;i<argc;i++){
    std::string a = argv[i];
    if(a=="--thermal-json"){
      sel.json_path = take_next_or_die(i, argc, argv, "--thermal-json");
    } else if(a=="--thermal-kind"){
      std::string v = take_next_or_die(i, argc, argv, "--thermal-kind");
      if(v=="total") sel.kind = ThermalYieldKind::Total;
      else if(v=="primordial") sel.kind = ThermalYieldKind::Primordial;
      else { std::cerr << "Unknown --thermal-kind " << v << "\n"; std::exit(2); }
    } else if(a=="--thermal-bin"){
      sel.bin_index = std::atoi(take_next_or_die(i, argc, argv, "--thermal-bin"));
    } else if(a=="--thermal-mode"){
      sel.mode = take_next_or_die(i, argc, argv, "--thermal-mode");
    } else if(a=="--thermal-k"){
      sel.k = std::atof(take_next_or_die(i, argc, argv, "--thermal-k"));
    } else if(a=="--thermal-nch"){
      sel.Nch = std::atof(take_next_or_die(i, argc, argv, "--thermal-nch"));
    } else if(a=="--thermal-dvdy"){
      sel.dVdy = std::atof(take_next_or_die(i, argc, argv, "--thermal-dvdy"));
    }
  }

  if(sel.json_path.empty()){
    std::cerr << "ERROR: --thermal-json is required.\n";
    print_thermal_cli_help();
    std::exit(2);
  }
  return sel;
}
