// blastwave/apps/blastwave_thermal.cpp
//
// Minimal-flag app: build pT spectra normalized to thermal dN/dy.
// Required: --thermal-json
// Optional: --primordial (default total), --k 3,6 (default: ALL ks found in JSON),
//           --cent N (default: auto per k), --species PDG,... (default: ALL common species),
//           --pt min,max,nbins (default: 0,10,400), --out out.root (default: derived from JSON),
//           --bw-csv FILE (blastwave parameters per centrality per particle species),
//           --bw-path (csv file path).
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>

#include <TFile.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TParameter.h>
#include <TSystem.h>

#include "hadron_list.h"
#include "io_utils.h"
#include "blastwave_utils.h"
#include <filesystem>
#include <cctype> 



namespace fs = std::filesystem;

#if __has_include(<nlohmann/json.hpp>)
  #include <nlohmann/json.hpp>
  using json = nlohmann::json;
#else
  #include "third_party/json.hpp"
  using json = nlohmann::json;
#endif

// ------------ small utils ------------
static inline std::string trim(std::string s) {
  auto notspace_front = [](unsigned char c){ return !std::isspace(c); };
  auto notspace_back  = [](unsigned char c){ return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace_front));
  s.erase(std::find_if(s.rbegin(), s.rend(), notspace_back).base(), s.end());
  return s;
}
static std::vector<std::string> split(const std::string& s, char delim){
  std::vector<std::string> out; std::stringstream ss(s); std::string item;
  while (std::getline(ss, item, delim)) if(!item.empty()) out.push_back(item);
  return out;
}
static std::vector<int> parse_int_list(const std::string& s){
  std::vector<int> v; for (auto& x : split(s, ',')) v.push_back(std::stoi(x)); return v;
}
static std::vector<double> parse_double_list(const std::string& s){
  std::vector<double> v; for (auto& x : split(s, ',')) v.push_back(std::stod(x)); return v;
}
// default folder for thermal JSONs when only a filename is given
static const std::string kDefaultThermalDir = "../data";
static std::string normalize_thermal_json_path(const std::string& in) {
  if (in.empty()) return in;
  fs::path p(in);
  // If user passed an absolute path or included a directory, keep as-is
  if (p.is_absolute() || p.has_parent_path()) return in;
  // Else, try ../data/<filename>; if it exists, use it; otherwise keep original
  fs::path candidate = fs::path(kDefaultThermalDir) / p;
  return fs::exists(candidate) ? candidate.string() : in;
}
static const std::string kDefaultOutDir = "../out";
static std::string derive_out_name(const std::string& jsonPath){
  // spectra_<basename>.root under ../out/
  auto pos = jsonPath.find_last_of("/\\");
  std::string base = (pos==std::string::npos) ? jsonPath : jsonPath.substr(pos+1);
  auto dot = base.find_last_of('.');
  if (dot != std::string::npos) base = base.substr(0, dot);
  fs::create_directories(kDefaultOutDir);              // ensure ../out exists
  return (fs::path(kDefaultOutDir) / ("spectra_" + base + ".root")).string();
}
static std::string normalize_out_path(const std::string& out) {
  if (out.empty()) return out;
  fs::path p(out);
  // if user gave an absolute path or included a directory, keep it
  if (p.is_absolute() || p.has_parent_path()) {
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
    return p.string();
  }
  // bare filename -> place it under ../out/
  fs::create_directories(kDefaultOutDir);
  return (fs::path(kDefaultOutDir) / p.filename()).string();
}
static int print_help(const char* prog,
                      const std::string& bw_csv,
                      const std::string& bw_path)
{
  std::cerr
    << "Usage:  " << prog << "\n"
    << "  --thermal-json FILE          path or bare filename; if no path, looks in ../data/\n"
    << "  OR\n"
    << "  --yields-csv FILE            use experimental yields from CSV (instead of thermal JSON)\n"
    << "Options:\n"
    << "  --primordial                 (thermal) use JSON primordial yields (default: total)\n"
    << "  --mode gammaS|vanilla        (thermal) select JSON bins by mode (default: gammaS)\n"
    << "  --k k1[,k2,...]              (thermal) restrict to these k values (default: all in JSON)\n"
    << "  --cent N                     take first N centralities per k (default: auto)\n"
    << "  --species PDG[,PDG,...]      restrict to these PDGs (default: all common)\n"
    << "  --pt min,max,nbins           pT grid (default: 0,10,400). Use --timesPt for dN/dpT.\n"
    << "  --out FILE.root              path or bare filename; if no path, outputs in ../out/\n"
    << "  --bw-csv FILE.csv            BW params csv file (default: " << bw_csv << ")\n"
    << "  --bw-path  DIR               base path for the BW csv file (default: " << bw_path << ")\n"
    << "  --timesPt                    returns spectra as dN/dPt instead of (1/Pt)dN/dPt\n"
    << "  --clampR                     num stability: clamp fireball radius instead of forcing subluminal beta in blastwave calculation routine\n"
    << "  --tgraph                     store spectra as TGraph(s) instead of THist(s)\n"
    << "  --help                       show this help\n"
    << "  --verbose                    run with verbose output\n";
  return 0;
}

// ---- JSON helpers ----
static bool load_json(const std::string& path, json& j){
  std::ifstream in(path);
  if(!in){ std::cerr << "[blastwave_thermal] cannot open JSON: " << path << "\n"; return false; }
  try { in >> j; } catch(const std::exception& e){ std::cerr << "[blastwave_thermal] JSON parse error: " << e.what() << "\n"; return false; }
  return true;
}
static std::set<double> unique_k(const json& j, const std::string& mode){
  std::set<double> ks;
  if(!j.contains("bins") || !j["bins"].is_array()) return ks;
  for (const auto& b : j["bins"]) {
    if (!mode.empty() && b.contains("mode") && b["mode"].get<std::string>()!=mode) continue;
    if (b.contains("k")) ks.insert(b["k"].get<double>());
  }
  return ks;
}
struct JBin { int idx; double Nch; };
static std::vector<JBin> bins_for_k_descNch(const json& j, double k, const std::string& mode){
  std::vector<JBin> v;
  if(!j.contains("bins") || !j["bins"].is_array()) return v;
  for (int i=0;i<(int)j["bins"].size();++i){
    const auto& b = j["bins"][i];
    if (!mode.empty() && b.contains("mode") && b["mode"].get<std::string>()!=mode) continue;
    if (!b.contains("k") || !b.contains("Nch")) continue;
    if (std::abs(b["k"].get<double>() - k) > 1e-12) continue;
    v.push_back({i, b["Nch"].get<double>()});
  }
  std::sort(v.begin(), v.end(), [](const JBin&a, const JBin&b){ return a.Nch > b.Nch; });
  return v;
}
static std::map<int,double> yield_map(const json& j, int bin_idx, bool total){
  std::map<int,double> m;
  const char* key = total ? "dNdy_total" : "dNdy_primary";
  const auto& b = j["bins"][bin_idx];
  if (!b.contains(key) || !b[key].is_object()) return m;
  for (auto it = b[key].begin(); it != b[key].end(); ++it){
    int pdg = std::stoi(it.key());
    double val = it.value().get<double>();
    m[pdg] = val;
  }
  return m;
}
struct YieldCSV {
  // centrality_class -> (pdg -> yield)
  std::map<int, std::map<int,double>> byCentPdg;
  // set of PDGs present in the CSV
  std::set<int> pdgs;
};

static std::string lower_copy(std::string s){
  for(char& c: s) c = std::tolower((unsigned char)c);
  return s;
}

// CLI flag for reading yields from csv file rather than thermal model
static bool read_yields_csv(const std::string& path, YieldCSV& out, bool verbose){
  std::ifstream in(path);
  if (!in) { std::cerr << "[blastwave_thermal] ERROR: cannot open yields CSV: " << path << "\n"; return false; }

  std::string header;
  if(!std::getline(in, header)){ std::cerr << "[blastwave_thermal] ERROR: empty CSV: " << path << "\n"; return false; }
  auto cols = split(header, ',');
  for (auto& c : cols) c = lower_copy(trim(c));

  auto find_col = [&](const char* key)->int{
    std::string k = lower_copy(key);
    for (size_t i=0;i<cols.size();++i) if (cols[i]==k) return (int)i;
    return -1;
  };
  int ci = find_col("centrality_class");
  int pi = find_col("pdg");
  int yi = find_col("yield");
  if (ci<0 || pi<0 || yi<0){
    std::cerr << "[blastwave_thermal] ERROR: CSV must contain columns: centrality_class,pdg,yield\n";
    return false;
  }

  std::string line; size_t n=0;
  while (std::getline(in, line)) {
    if (trim(line).empty()) continue;
    auto cells = split(line, ',');
    if ((int)cells.size() <= std::max({ci,pi,yi})) {
      std::cerr << "[blastwave_thermal] WARNING: skipping short line: " << line << "\n";
      continue;
    }
    int cent  = std::stoi(trim(cells[ci]));
    int pdg   = std::stoi(trim(cells[pi]));
    double y  = std::stod(trim(cells[yi]));
    out.byCentPdg[cent][pdg] = y;
    out.pdgs.insert(pdg);
    ++n;
  }
  if (verbose) std::cerr << "[blastwave_thermal] loaded " << n << " yield rows from " << path << "\n";
  return true;
}

// ---- Spectrum ----
static TH1D* make_spectrum(const HadronIntegrationInfo& info, double ptmin, double ptmax, int nbins, bool timesPt, bool verbose, bool clampR){
  HadronIntegrationInfo tmp = info;
  return computePtSpectrum(tmp, ptmin, ptmax, /*timesPt=*/timesPt, /*clampR=*/clampR, /*rmax=*/0.0, /*nBins=*/nbins, verbose);
}
static TGraph* make_spectrum_tGraph(const HadronIntegrationInfo& info, double ptmin, double ptmax, int nbins, bool timesPt, bool verbose, bool clampR){
  HadronIntegrationInfo tmp = info;

  // tgraph evaluates point per point, need some margin from 0
  if (ptmin == 0)
    ptmin = 0.00001;

  return computePtSpectrum_tGraph(tmp, ptmin, ptmax, /*timesPt=*/timesPt, /*clampR=*/clampR, /*rmax=*/0.0, /*nBins=*/nbins, verbose);
}

int main(int argc, char** argv){
  // Minimal knobs
  std::string thermal_json;          // optional: thermal source
  bool total = true;                 // TOTAL vs primordial
  std::string mode = "gammaS";
  std::vector<double> klist;         // empty -> infer from JSON
  int centN = 0;                     // 0 -> take all available
  std::vector<int> species;          // empty -> infer per source
  std::string out_root;              // empty -> derived from source
  bool verbose = false;

  // BW + alternative yield source
  std::string bw_csv   = "bw_data_1303.0737.csv";       // BW parameters (CSV), under bw_path
  std::string bw_path  = "../data";                     // base folder for CSVs
  std::string yields_csv = "";                          // experimental yields CSV (optional)

  if (argc == 1) return print_help(argv[0], bw_csv, bw_path);

  // pT grid & knobs
  double ptmin=0.0, ptmax=10.0; int nbins=400;
  bool timesPt=false, tgraph=false, clampR=false;

  // Parse args
  for (int i=1;i<argc;i++){
    std::string a = argv[i];
    auto next = [&](const char* flag){ if (i+1>=argc){ std::cerr<<"Missing value after "<<flag<<"\n"; std::exit(2);} return std::string(argv[++i]); };
    if (a=="--thermal-json") thermal_json = next("--thermal-json");
    else if (a=="--yields-csv") yields_csv = next("--yields-csv");
    else if (a=="--primordial") total = false;
    else if (a=="--mode") mode = next("--mode");
    else if (a=="--k") klist = parse_double_list(next("--k"));
    else if (a=="--cent") centN = std::stoi(next("--cent"));
    else if (a=="--species") species = parse_int_list(next("--species"));
    else if (a=="--out") out_root = next("--out");
    else if (a=="--verbose") verbose = true;
    else if (a=="--pt") { auto v = parse_double_list(next("--pt")); if(v.size()==3){ ptmin=v[0]; ptmax=v[1]; nbins=(int)v[2]; } }
    else if (a=="--timesPt") timesPt = true;
    else if (a=="--bw-csv") bw_csv = next("--bw-csv");
    else if (a=="--bw-path")  bw_path  = next("--bw-path");
    else if (a=="--tgraph")  tgraph  = true;
    else if (a=="--clampR")  clampR  = true;
    else if (a=="--help" || a=="-h") { print_help(argv[0], bw_csv, bw_path); return 0; }
  }

  thermal_json = normalize_thermal_json_path(thermal_json);

  // must have at least one yield source
  if (thermal_json.empty() && yields_csv.empty()){
    std::cerr << "[blastwave_thermal] ERROR: provide either --thermal-json or --yields-csv\n\n";
    return 2;
  }

  // ---------- CSV experimental yields MODE ----------
  if (!yields_csv.empty()){
    // read yields CSV
    YieldCSV Y;
    const std::string ypath = (fs::path(bw_path) / yields_csv).string();
    if (!read_yields_csv(ypath, Y, verbose)) return 2;

    // species selection (CSV-driven if none given)
    std::vector<int> species_use = species;
    if (species_use.empty())
      species_use.assign(Y.pdgs.begin(), Y.pdgs.end());

    // BW params from CSV (species-aware: ALL / tokens)
    const std::string bwpath = (fs::path(bw_path) / bw_csv).string();
    if (verbose) std::cerr << "[blastwave_thermal] loading BW params from " << bwpath << "\n";
    auto all = get_integration_info_from_csv(bwpath, species_use, verbose);
    if (all.empty()){ std::cerr << "[blastwave_thermal] No centralities from BW CSV\n"; return 2; }

    // centrality count
    int Cmax = (int)all.size();
    int C    = (centN>0) ? std::min(centN, Cmax) : Cmax;

    // output path
    if (out_root.empty()) out_root = derive_out_name("csv_yields"); // ../out/spectra_csv_yields.root
    else out_root = normalize_out_path(out_root);

    // ROOT out
    TFile* fout = TFile::Open(out_root.c_str(), "RECREATE");
    if(!fout || fout->IsZombie()){ std::cerr << "Cannot create output ROOT: " << out_root << "\n"; return 2; }

    // metadata
    TParameter<int>("nbins", nbins).Write("nbins");
    TParameter<double>("ptmin", ptmin).Write("ptmin");
    TParameter<double>("ptmax", ptmax).Write("ptmax");
    TParameter<int>("timesPt", (int)timesPt).Write("timesPt");
    TNamed("yield_source", "csv").Write("yield_source");
    TNamed("yields_csv", ypath.c_str()).Write("yields_csv");
    TNamed("bw_csv", bwpath.c_str()).Write("bw_csv");

    // optional k wrapper (k-independent CSV yields)
    fout->mkdir("k_1"); fout->cd("k_1");

    // loop centralities
    for (int c = 0; c < C; ++c){
      int centClass = all[c].empty() ? c : all[c][0].centrality_class;
      auto itCent = Y.byCentPdg.find(centClass);
      if (itCent == Y.byCentPdg.end()){
        if (verbose) std::cerr << "[blastwave_thermal] no CSV yields for cent class " << centClass << "\n";
        continue;
      }
      const auto& ymap = itCent->second;

      std::ostringstream cdir; cdir << "cent_" << c;
      gDirectory->mkdir(cdir.str().c_str());
      gDirectory->cd(cdir.str().c_str());

      for (const auto& infoOrig : all[c]){
        int pdg = infoOrig.hadron.pdg;
        auto yit = ymap.find(pdg);
        if (yit == ymap.end()) {
          if (verbose) std::cerr << "[blastwave_thermal] missing CSV yield for PDG " << pdg
                                 << " at cent class " << centClass << "\n";
          continue;
        }

        HadronIntegrationInfo info = infoOrig;
        info.yield = yit->second;

        TH1D* h=nullptr; TGraph* hg=nullptr;
        if (tgraph) hg = make_spectrum_tGraph(info, ptmin, ptmax, nbins, timesPt, verbose, clampR);
        else        h  = make_spectrum(info,      ptmin, ptmax, nbins, timesPt, verbose, clampR);
        if (!h && !hg) continue;

        std::ostringstream hname; hname << "h_" << info.hadron.name << "_pdg" << pdg;
        if (hg) { hg->SetName(hname.str().c_str());
                  hg->SetTitle(Form("%s (pdg=%d);p_{T} [GeV/c];%s",
                                    info.hadron.name.c_str(), pdg,
                                    timesPt?"dN/dp_{T}":"(1/p_{T}) dN/dp_{T}"));
                  hg->Write(); delete hg; }
        else    { h->SetName(hname.str().c_str());
                  h->SetTitle(Form("%s (pdg=%d);p_{T} [GeV/c];%s",
                                    info.hadron.name.c_str(), pdg,
                                    timesPt?"dN/dp_{T}":"(1/p_{T}) dN/dp_{T}"));
                  h->Write(); delete h; }
      }
      gDirectory->cd("..");
    }
    fout->cd(); fout->Write(); fout->Close();
    std::cout << "[blastwave_thermal] Wrote spectra to " << out_root << "\n";
    return 0;
  }

  // ---------- Thermal JSON MODE ----------
  // JSON is required in this branch
  if (verbose) std::cerr << "[blastwave_thermal] reading JSON " << thermal_json << "\n";
  json j; if (!load_json(thermal_json, j)) return 2;

  // klist (infer if empty)
  if (klist.empty()){
    auto ks = unique_k(j, mode);
    klist.assign(ks.begin(), ks.end());
    if (klist.empty()){
      std::cerr << "[blastwave_thermal] No k values found in JSON for mode="<<mode<<"\n";
      return 2;
    }
  }

  // species inference (from JSON) if not provided
  std::vector<int> species_use;
  if (species.empty()){
    std::set<int> json_pdgs_union;
    for (double k : klist){
      auto v = bins_for_k_descNch(j, k, mode);
      int take = (centN>0) ? std::min<int>(centN, (int)v.size()) : (int)v.size();
      for (int i=0;i<take;i++){
        auto m = yield_map(j, v[i].idx, total);
        for (auto& kv : m) json_pdgs_union.insert(kv.first);
      }
    }
    species_use.assign(json_pdgs_union.begin(), json_pdgs_union.end());
  } else {
    species_use = species;
  }
  if (species_use.empty()){
    std::cerr << "[blastwave_thermal] No species to process (none from --species and none in JSON selection)\n";
    return 2;
  }

  // BW params from CSV (species-aware)
  const std::string bwpath = (fs::path(bw_path) / bw_csv).string();
  if (verbose) std::cerr << "[blastwave_thermal] loading BW params from " << bwpath << "\n";
  auto all = get_integration_info_from_csv(bwpath, species_use, verbose);
  if (verbose) std::cerr << "[blastwave_thermal] centralities="<<all.size()
                         << " species/cent="<<(all.empty()?0:all[0].size())<<"\n";
  if (all.empty()){ std::cerr << "[blastwave_thermal] No centralities from BW CSV\n"; return 2; }

  // output
  if (out_root.empty()) out_root = derive_out_name(thermal_json);
  else out_root = normalize_out_path(out_root);

  TFile* fout = TFile::Open(out_root.c_str(), "RECREATE");
  if(!fout || fout->IsZombie()){ std::cerr << "Cannot create output ROOT: " << out_root << "\n"; return 2; }

  TParameter<int>("nbins", nbins).Write("nbins");
  TParameter<double>("ptmin", ptmin).Write("ptmin");
  TParameter<double>("ptmax", ptmax).Write("ptmax");
  TParameter<int>("timesPt", (int)timesPt).Write("timesPt");
  TNamed("thermal_json", thermal_json.c_str()).Write("thermal_json");
  TNamed("thermal_kind", total ? "total":"primordial").Write("thermal_kind");
  TNamed("thermal_mode", mode.c_str()).Write("thermal_mode");
  TNamed("bw_csv", bwpath.c_str()).Write("bw_csv");

  // loop over k and centralities from JSON
  for (double k : klist){
    auto vec = bins_for_k_descNch(j, k, mode);
    if (vec.empty()){ std::cerr << "[blastwave_thermal] No bins for k="<<k<<"\n"; continue; }

    int Cmax = std::min<int>((int)vec.size(), (int)all.size());
    int C = (centN>0) ? std::min(centN, Cmax) : Cmax;

    std::ostringstream kdir; kdir << "k_" << k;
    fout->mkdir(kdir.str().c_str());
    fout->cd(kdir.str().c_str());

    for (int c = 0; c < C; ++c){
      int bin_idx = vec[c].idx;
      auto ymap = yield_map(j, bin_idx, total);

      std::ostringstream cdir; cdir << "cent_" << c;
      gDirectory->mkdir(cdir.str().c_str());
      gDirectory->cd(cdir.str().c_str());

      for (const auto& infoOrig : all[c]){
        int pdg = infoOrig.hadron.pdg;
        if (std::find(species_use.begin(), species_use.end(), pdg)==species_use.end()) continue;

        auto itY = ymap.find(pdg);
        if (itY == ymap.end()) continue;

        HadronIntegrationInfo info = infoOrig;
        if (verbose) std::cerr << "  [k="<<k<<"] cent "<<c<<": computing PDG "<<pdg<<" ("<<info.hadron.name<<")\n";
        info.yield = itY->second;

        TH1D* h=nullptr; TGraph* hg=nullptr;
        if (tgraph) hg = make_spectrum_tGraph(info, ptmin, ptmax, nbins, timesPt, verbose, clampR);
        else        h  = make_spectrum(info,      ptmin, ptmax, nbins, timesPt, verbose, clampR);
        if (!h && !hg) continue;

        std::ostringstream hname; hname << "h_" << info.hadron.name << "_pdg" << pdg;
        if (hg) { hg->SetName(hname.str().c_str());
                  hg->SetTitle(Form("%s (pdg=%d);p_{T} [GeV/c];%s",
                                    info.hadron.name.c_str(), pdg,
                                    timesPt?"dN/dp_{T}":"(1/p_{T}) dN/dp_{T}"));
                  hg->Write(); delete hg; }
        else    { h->SetName(hname.str().c_str());
                  h->SetTitle(Form("%s (pdg=%d);p_{T} [GeV/c];%s",
                                    info.hadron.name.c_str(), pdg,
                                    timesPt?"dN/dp_{T}":"(1/p_{T}) dN/dp_{T}"));
                  h->Write(); delete h; }
      }
      gDirectory->cd("..");
    }
    fout->cd();
  }

  fout->Write();
  fout->Close();
  std::cout << "[blastwave_thermal] Wrote spectra to " << out_root << "\n";
  return 0;
}
