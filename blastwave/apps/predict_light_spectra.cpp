// blastwave/apps/predict_light_spectra.cpp
//
// Extrapolate blast-wave parameters vs multiplicity and build pT spectra
// for light-flavor species in small collision systems (e.g. p-O, O-O, Ne-Ne).
// Steps:
//   1) read Pb-Pb BW parameters and corresponding dNch/deta map
//   2) fit beta_t, Tkin and n as functions of dNch/deta
//   3) evaluate those fits at target multiplicities for each system
//   4) fetch thermal yields at the same multiplicities (interpolated from JSON)
//   5) generate blast-wave spectra normalized to the thermal yields
//   6) dump ROOT objects (fits + spectra) and a quick-look PDF (one canvas per species)
//
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <limits>
#include <vector>
#include <utility>

#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TNamed.h>
#include <TParameter.h>
#include <TF1.h>
#include <TStyle.h>
#include <TMath.h>
#include <TString.h>
#include <TLatex.h>

#include "blastwave_utils.h"
#include "hadron_list.h"
#include "../../common/hadron_catalog.h"
#include "runtime_paths.h"

namespace fs = std::filesystem;

#if __has_include(<nlohmann/json.hpp>)
  #include <nlohmann/json.hpp>
  using json = nlohmann::json;
#else
  #include "third_party/json.hpp"
  using json = nlohmann::json;
#endif

// ---------- small string helpers ----------
static inline std::string trim(std::string s) {
  auto issp = [](unsigned char c){ return std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char c){ return !issp(c); }));
  s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char c){ return !issp(c); }).base(), s.end());
  return s;
}
static std::vector<std::string> split(const std::string& s, char delim){
  std::vector<std::string> out; std::stringstream ss(s); std::string item;
  while (std::getline(ss, item, delim)) if(!item.empty()) out.push_back(item);
  return out;
}
static std::string lower(std::string s){ for (auto& c: s) c = std::tolower((unsigned char)c); return s; }
static std::string upper(std::string s){ for (auto& c: s) c = std::toupper((unsigned char)c); return s; }

// ---------- parsing helpers ----------
static std::vector<int> parse_int_list(const std::string& s){
  std::vector<int> v; for (auto& x : split(s, ',')) v.push_back(std::stoi(x)); return v;
}
static std::vector<double> parse_double_list(const std::string& s){
  std::vector<double> v; for (auto& x : split(s, ',')) v.push_back(std::stod(x)); return v;
}

// ---------- physics containers ----------
struct BWPoint {
  int cent = -1;
  double nch = std::numeric_limits<double>::quiet_NaN();
  double dvdy = std::numeric_limits<double>::quiet_NaN();
  double beta = 0, beta_stat = 0, beta_sys = 0;
  double Tkin = 0, Tkin_stat = 0, Tkin_sys = 0;
  double nprof = 0, nprof_stat = 0, nprof_sys = 0;
};

struct YieldBin {
  double nch = 0;
  double dvdy = 0;
  std::map<int,double> yields;
};
struct YieldTable {
  double k = 0;
  std::vector<YieldBin> bins; // sorted later by nch
};

struct SystemTargets {
  std::string name;
  std::vector<double> nch_values;
};

// ---------- IO helpers ----------
static std::vector<double> read_nch_map(const std::string& path){
  std::vector<double> out;
  if (path.empty()) return out;
  std::ifstream in(path);
  if(!in){
    std::cerr << "[predict] WARNING: cannot open Nch map: " << path << " (will rely on CSV Nch column if present)\n";
    return out;
  }
  std::string line;
  while (std::getline(in, line)){
    line = trim(line);
    if (line.empty() || line[0]=='#') continue;
    try { out.push_back(std::stod(line)); }
    catch(...){ /* ignore malformed lines */ }
  }
  return out;
}

static bool approx_match(double a, double b, double tol=1e-6){
  return std::abs(a-b) < tol;
}

static std::vector<BWPoint> read_bw_csv(const std::string& path,
                                        const std::vector<double>& nch_map,
                                        const std::string& particle_filter,
                                        bool verbose)
{
  std::vector<BWPoint> out;
  std::ifstream in(path);
  if (!in) {
    std::cerr << "[predict] ERROR: cannot open BW CSV: " << path << "\n";
    return out;
  }

  std::string header;
  if(!std::getline(in, header)){
    std::cerr << "[predict] ERROR: empty BW CSV\n";
    return out;
  }
  auto cols = split(header, ',');
  for (auto& c : cols) c = lower(trim(c));
  auto colidx = [&](const char* key)->int{
    std::string k = lower(key);
    for (size_t i=0;i<cols.size();++i) if(cols[i]==k) return (int)i;
    return -1;
  };
  int ci = colidx("centrality_class");
  int pi = colidx("particle");
  int nci= colidx("nch");
  int bi = colidx("beta_t"), bu1=colidx("beta_t_unc1"), bu2=colidx("beta_t_unc2");
  int ti = colidx("Tkin"),   tu1=colidx("Tkin_unc1"),   tu2=colidx("Tkin_unc2");
  int ni = colidx("n_profile"), nu1=colidx("n_profile_unc1"), nu2=colidx("n_profile_unc2");
  int vi = colidx("dvdy");
  if (vi < 0) vi = colidx("dv/dy");
  if(ci<0 || bi<0 || ti<0 || ni<0){
    std::cerr << "[predict] ERROR: BW CSV missing required columns\n";
    return out;
  }
  if (nci<0 && nch_map.empty()){
    std::cerr << "[predict] ERROR: BW CSV has no Nch column and no --nch-map was provided\n";
    return out;
  }

  std::string line; size_t nrows=0;
  std::string want = upper(trim(particle_filter));
  bool warned_missing_nch=false;
  while (std::getline(in, line)){
    if (trim(line).empty()) continue;
    auto cells = split(line, ',');
    while (cells.size() < cols.size()) cells.push_back("");
    if (pi>=0){
      std::string ptk = upper(trim(cells[pi]));
      if (ptk != want && want != "ALL") continue;
    }
    BWPoint p{};
    try { p.cent = std::stoi(trim(cells[ci])); }
    catch(...){ continue; }
    auto asDouble = [&](int idx)->double{
      if (idx<0 || idx >= (int)cells.size()) return 0.0;
      const auto s = trim(cells[idx]); if (s.empty()) return 0.0;
      return std::stod(s);
    };
    p.beta      = asDouble(bi);
    p.beta_stat = asDouble(bu1);
    p.beta_sys  = asDouble(bu2);
    p.Tkin      = asDouble(ti);
    p.Tkin_stat = asDouble(tu1);
    p.Tkin_sys  = asDouble(tu2);
    p.nprof     = asDouble(ni);
    p.nprof_stat= asDouble(nu1);
    p.nprof_sys = asDouble(nu2);
    if (vi >= 0) p.dvdy = asDouble(vi);

    if (nci>=0){
      p.nch = asDouble(nci);
    } else if (p.cent-1 < (int)nch_map.size() && p.cent>0){
      p.nch = nch_map[p.cent-1];
    } else {
      p.nch = std::numeric_limits<double>::quiet_NaN();
    }
    if (!std::isfinite(p.nch)){
      if (!warned_missing_nch && verbose){
        std::cerr << "[predict] WARNING: missing Nch for some rows (centrality map or column)\n";
        warned_missing_nch = true;
      }
      continue;
    }
    out.push_back(p); ++nrows;
  }

  std::sort(out.begin(), out.end(), [](const BWPoint&a,const BWPoint&b){ return a.nch>b.nch; });
  if(verbose) std::cerr << "[predict] loaded " << nrows << " BW rows (kept " << out.size() << ")\n";
  return out;
}

// ---------- thermal yields ----------
static std::string normalize_mode(const std::string& m){
  std::string ml = lower(m);
  if (ml=="gs") return std::string("gammas");
  if (ml.rfind("gamma",0)==0) return std::string("gammas");
  return ml;
}

static std::vector<YieldTable> load_yield_tables(const std::string& jsonPath,
                                                 const std::string& mode,
                                                 const std::vector<double>& klist,
                                                 bool useTotal,
                                                 bool verbose)
{
  std::vector<YieldTable> out;
  json j;
  std::ifstream in(jsonPath);
  if(!in){ std::cerr << "[predict] ERROR: cannot open thermal JSON: " << jsonPath << "\n"; return out; }
  try { in >> j; } catch(const std::exception& e){ std::cerr << "[predict] JSON parse error: " << e.what() << "\n"; return out; }
  if(!j.contains("bins") || !j["bins"].is_array()){
    std::cerr << "[predict] ERROR: thermal JSON has no 'bins' array\n";
    return out;
  }

  std::set<double> kset(klist.begin(), klist.end());
  auto add_to = [&](double k, const YieldBin& b){
    auto it = std::find_if(out.begin(), out.end(), [&](const YieldTable& t){ return approx_match(t.k, k, 1e-6); });
    if (it == out.end()){
      YieldTable t; t.k = k; t.bins.push_back(b); out.push_back(std::move(t));
    } else {
      it->bins.push_back(b);
    }
  };

  std::string modeLower = normalize_mode(mode);
  for (const auto& b : j["bins"]){
    if (!b.contains("k")) continue;
    if (!modeLower.empty() && b.contains("mode")){
      std::string bm = normalize_mode(b["mode"].get<std::string>());
      if (bm != modeLower) continue;
    }
    double k = b["k"].get<double>();
    if (!kset.empty() && kset.count(k)==0) continue;
    std::string key = useTotal ? "dNdy_total" : "dNdy_primary";
    if (!b.contains(key)) continue;

    YieldBin yb;
    if (modeLower == "vanilla") {
      if (!b.contains("dVdy")) continue;
      yb.dvdy = b["dVdy"].get<double>();
    } else {
      if (!b.contains("Nch")) continue;
      yb.nch = b["Nch"].get<double>();
    }
    for (auto it = b[key].begin(); it!=b[key].end(); ++it){
      int pdg = std::stoi(it.key());
      yb.yields[pdg] = it.value().get<double>();
    }
    add_to(k, yb);
  }

  for (auto& t : out){
    if (modeLower == "vanilla") {
      std::sort(t.bins.begin(), t.bins.end(), [](const YieldBin&a,const YieldBin&b){ return a.dvdy < b.dvdy; });
      if(verbose) std::cerr << "[predict] k="<<t.k<<" bins="<<t.bins.size()
                             << " dVdy in ["<< (t.bins.empty()?0:t.bins.front().dvdy)
                             << ", "<< (t.bins.empty()?0:t.bins.back().dvdy) << "]\n";
    } else {
      std::sort(t.bins.begin(), t.bins.end(), [](const YieldBin&a,const YieldBin&b){ return a.nch < b.nch; });
      if(verbose) std::cerr << "[predict] k="<<t.k<<" bins="<<t.bins.size()
                             << " Nch in ["<< (t.bins.empty()?0:t.bins.front().nch)
                             << ", "<< (t.bins.empty()?0:t.bins.back().nch) << "]\n";
    }
  }

  return out;
}

static std::map<int,double> interpolate_yields(const YieldTable& tab, double target, bool use_dvdy){
  std::map<int,double> out;
  if (tab.bins.empty()) return out;
  auto getx = [&](const YieldBin& b){ return use_dvdy ? b.dvdy : b.nch; };
  if (target <= getx(tab.bins.front())) return tab.bins.front().yields;
  if (target >= getx(tab.bins.back()))  return tab.bins.back().yields;

  auto upperIt = std::upper_bound(tab.bins.begin(), tab.bins.end(), target,
    [&](double val, const YieldBin& b){ return val < getx(b); });
  if (upperIt == tab.bins.begin()) return upperIt->yields;
  if (upperIt == tab.bins.end())   return tab.bins.back().yields;
  auto lowerIt = upperIt - 1;

  double x1 = getx(*lowerIt), x2 = getx(*upperIt);
  double t = (target - x1) / (x2 - x1);

  std::set<int> pdgs;
  for (auto& kv : lowerIt->yields) pdgs.insert(kv.first);
  for (auto& kv : upperIt->yields) pdgs.insert(kv.first);

  for (int pdg : pdgs){
    double y1 = lowerIt->yields.count(pdg) ? lowerIt->yields.at(pdg) : 0.0;
    double y2 = upperIt->yields.count(pdg) ? upperIt->yields.at(pdg) : 0.0;
    out[pdg] = y1 + t*(y2 - y1);
  }
  return out;
}

static std::vector<std::pair<double,double>> build_nch_dvdy_map(const std::vector<BWPoint>& pts){
  std::vector<std::pair<double,double>> out;
  for (const auto& p : pts){
    if (!std::isfinite(p.nch) || !std::isfinite(p.dvdy)) continue;
    out.emplace_back(p.nch, p.dvdy);
  }
  std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.first < b.first; });
  return out;
}

static double map_nch_to_dvdy(const std::vector<std::pair<double,double>>& map, double nch){
  if (map.size() < 2) return std::numeric_limits<double>::quiet_NaN();
  if (nch <= map.front().first){
    auto [x1,y1] = map[0];
    auto [x2,y2] = map[1];
    double t = (nch - x1) / (x2 - x1);
    return y1 + t*(y2 - y1);
  }
  if (nch >= map.back().first){
    auto [x1,y1] = map[map.size()-2];
    auto [x2,y2] = map[map.size()-1];
    double t = (nch - x1) / (x2 - x1);
    return y1 + t*(y2 - y1);
  }
  auto upperIt = std::upper_bound(map.begin(), map.end(), nch,
    [](double val, const auto& p){ return val < p.first; });
  auto lowerIt = upperIt - 1;
  double x1 = lowerIt->first, y1 = lowerIt->second;
  double x2 = upperIt->first, y2 = upperIt->second;
  double t = (nch - x1) / (x2 - x1);
  return y1 + t*(y2 - y1);
}

// ---------- fitting helpers ----------
static TGraphErrors* make_graph(const std::vector<BWPoint>& pts,
                                double BWPoint::* value,
                                double BWPoint::* stat,
                                double BWPoint::* sys,
                                const char* name)
{
  std::vector<double> x, y, ex, ey;
  x.reserve(pts.size()); y.reserve(pts.size()); ex.assign(pts.size(), 0.0); ey.reserve(pts.size());
  for (const auto& p : pts){
    if (!std::isfinite(p.nch)) continue;
    x.push_back(p.nch);
    y.push_back(p.*value);
    double err = std::hypot(p.*stat, p.*sys);
    ey.push_back(err);
  }
  auto* g = new TGraphErrors((int)x.size(), x.data(), y.data(), ex.data(), ey.data());
  g->SetName(name);
  return g;
}

static TF1* fit_graph(TGraphErrors* g, const std::string& formula,
                      const std::string& name, double xmin, double xmax, bool verbose,
                      int* fitStatusOut = nullptr,
                      const std::vector<double>& initPars = {})
{
  auto* f = new TF1(name.c_str(), formula.c_str(), xmin, xmax);
  if (!initPars.empty()){
    const int npar = f->GetNpar();
    for (int i=0; i<std::min<int>(npar, initPars.size()); ++i){
      f->SetParameter(i, initPars[i]);
    }
  } else if (g->GetN()>=2){
    double y0 = g->GetY()[0];
    double y1 = g->GetY()[g->GetN()-1];
    if (f->GetNpar()>0) f->SetParameter(0, y0);
    if (f->GetNpar()>1) f->SetParameter(1, y0 - y1);
    if (f->GetNpar()>2) f->SetParameter(2, std::max(50.0, xmin));
  }
  int status = g->Fit(f, "Q"); // quiet
  if (status!=0 && verbose) {
    std::cerr << "[predict] WARNING: fit " << name << " returned status " << status << "\n";
  }
   if (fitStatusOut) *fitStatusOut = status;
  return f;
}

// ---------- systems parser ----------
static std::vector<SystemTargets> parse_systems(const std::string& spec){
  std::vector<SystemTargets> out;
  for (auto blk : split(spec, ';')){
    auto pos = blk.find(':');
    if (pos == std::string::npos) continue;
    SystemTargets s;
    s.name = trim(blk.substr(0, pos));
    s.nch_values = parse_double_list(blk.substr(pos+1));
    if (!s.name.empty() && !s.nch_values.empty()) out.push_back(std::move(s));
  }
  return out;
}

static void print_help(const char* prog){
  std::cerr <<
"Usage: " << prog << " --thermal-json FILE --systems \"OO:60,120;NeNe:150,220\" [options]\n"
"Required:\n"
"  --thermal-json PATH            Thermal-FIST yields JSON (gammaS scan recommended)\n"
"  --systems SPEC                 Semicolon-separated list: NAME:n1,n2;NAME2:n3,... (e.g. OO:60,120;NeNe:150,220)\n"
"Options:\n"
"  --bw-csv FILE                  Pb-Pb blast-wave fit CSV (default: bw_data_1910.07678.csv in data-dir; if it contains an Nch column, no map is needed)\n"
"  --nch-map FILE                 dNch/deta list for centralities (used only if CSV lacks Nch; default: Nch_PbPb_pp_1910.07678_ALICE.txt in conf-dir)\n"
"  --mode gammaS|vanilla          Thermal JSON mode filter (default: gammaS)\n"
"  --k values                     Comma-separated k list for yields (default: all k in JSON)\n"
"  --species PDG,...              Species PDG list (default: pi+/-, K+/-, p/-, deuteron)\n"
"  --pt min,max,nbins             pT grid and normalization range (default: 0,10,400)\n"
"  --fit-range min,max            Fit range in dNch/deta (default: 10,2000)\n"
"  --fit-beta FORM                TF1 formula for beta_t(Nch)   (default: [0]*pow(x,[1])/(pow(x,[1]) + pow([2],[1]))\n"
"  --fit-T FORM                   TF1 formula for Tkin(Nch)     (default: [0] + [1]*log(x)\n"
"  --fit-n FORM                   TF1 formula for n_profile(Nch)(default: [0] + ([1]-[0])/(1 + x/[2])\n"
"  --fit-beta-pars a,b,c          Initial parameters for beta fit (comma-separated)\n"
"  --fit-T-pars a,b,c             Initial parameters for T fit (comma-separated)\n"
"  --fit-n-pars a,b,c             Initial parameters for n fit (comma-separated)\n"
"  --out ROOTFILE                 Output ROOT (default: out-dir/predict_light_spectra.root)\n"
"  --pdf FILE                     Quick-look PDF (default: out-dir/predict_light_spectra.pdf)\n"
"Notes:\n"
"  * vanilla mode expects dVdy in the BW CSV; yields are interpolated vs dVdy using a linear Nch->dVdy map.\n"
"  --data-dir PATH                Base data dir for bare filenames\n"
"  --conf-dir PATH                Base config dir for bare filenames\n"
"  --out-dir PATH                 Base output dir for bare filenames\n"
"  --no-pdf                       Skip PDF overlay\n"
"  --primordial                   Use primordial yields (default: total)\n"
"  --timesPt                      Return dN/dpT instead of (1/pT)dN/dpT\n"
"  --clampR                       Numerical stability: clamp r instead of beta\n"
"  --verbose                      Verbose logging\n"
"Env overrides: TFHIC_DATA, TFHIC_CONF, TFHIC_OUT.\n";
}

// ---------- main ----------
int main(int argc, char** argv){
  if (argc==1){ print_help(argv[0]); return 0; }

  // defaults
  std::string bw_csv = "bw_data_1910.07678.csv";
  std::string nch_map = "Nch_PbPb_pp_1910.07678_ALICE.txt"; // optional if CSV has Nch column
  std::string thermal_json;
  std::string thermal_mode = "gammaS";
  std::vector<double> klist; // empty -> all k found in JSON
  std::string systems_spec;
  std::vector<int> species = {211,-211,321,-321,2212,-2212,1000010020};
  double ptmin=0.0, ptmax=10.0; int npt=400;
  double fitmin=10.0, fitmax=2000.0;
  std::string f_beta = "[0]*pow(x,[1])/(pow(x,[1]) + pow([2],[1]))";
  std::string f_T    = "[0] + [1]*log(x)";
  std::string f_n    = "[0] + ([1]-[0])/(1 + x/[2])";
  std::vector<double> f_beta_pars, f_T_pars, f_n_pars;
  std::string out_root = "predict_light_spectra.root";
  std::string out_pdf  = "predict_light_spectra.pdf";
  bool make_pdf = true;
  bool primordial = false;
  bool timesPt = false;
  bool clampR = false;
  bool verbose = false;
  std::string dataDirFlag, confDirFlag, outDirFlag;

  auto next = [&](int& i, const char* flag)->std::string{
    if (i+1>=argc){ std::cerr << "Missing value after " << flag << "\n"; std::exit(2); }
    return std::string(argv[++i]);
  };

  for (int i=1;i<argc;i++){
    std::string a = argv[i];
    if (a=="--thermal-json") thermal_json = next(i,a.c_str());
    else if (a=="--systems") systems_spec = next(i,a.c_str());
    else if (a=="--bw-csv")  bw_csv = next(i,a.c_str());
    else if (a=="--nch-map") nch_map = next(i,a.c_str());
    else if (a=="--mode")    thermal_mode = next(i,a.c_str());
    else if (a=="--k")       klist = parse_double_list(next(i,a.c_str()));
    else if (a=="--species") species = parse_int_list(next(i,a.c_str()));
    else if (a=="--pt")      { auto v=parse_double_list(next(i,a.c_str())); if(v.size()==3){ ptmin=v[0]; ptmax=v[1]; npt=(int)v[2]; } }
    else if (a=="--fit-range"){ auto v=parse_double_list(next(i,a.c_str())); if(v.size()==2){ fitmin=v[0]; fitmax=v[1]; } }
    else if (a=="--fit-beta") f_beta = next(i,a.c_str());
    else if (a=="--fit-T")    f_T    = next(i,a.c_str());
    else if (a=="--fit-n")    f_n    = next(i,a.c_str());
    else if (a=="--fit-beta-pars") f_beta_pars = parse_double_list(next(i,a.c_str()));
    else if (a=="--fit-T-pars")    f_T_pars    = parse_double_list(next(i,a.c_str()));
    else if (a=="--fit-n-pars")    f_n_pars    = parse_double_list(next(i,a.c_str()));
    else if (a=="--out")      out_root = next(i,a.c_str());
    else if (a=="--pdf")      { out_pdf = next(i,a.c_str()); make_pdf=true; }
    else if (a=="--data-dir") dataDirFlag = next(i,a.c_str());
    else if (a=="--conf-dir") confDirFlag = next(i,a.c_str());
    else if (a=="--out-dir")  outDirFlag  = next(i,a.c_str());
    else if (a=="--no-pdf")   make_pdf=false;
    else if (a=="--primordial") primordial=true;
    else if (a=="--timesPt")  timesPt=true;
    else if (a=="--clampR")   clampR=true;
    else if (a=="--verbose")  verbose=true;
    else if (a=="--help" || a=="-h"){ print_help(argv[0]); return 0; }
  }

  if (thermal_json.empty()){
    std::cerr << "[predict] ERROR: --thermal-json is required\n";
    print_help(argv[0]); return 2;
  }
  if (systems_spec.empty()){
    std::cerr << "[predict] ERROR: --systems is required (e.g. OO:60,120;NeNe:150,220)\n";
    print_help(argv[0]); return 2;
  }

  auto paths = resolve_runtime_paths(argv[0], dataDirFlag, confDirFlag, outDirFlag, "blastwave/out");

  fs::path bw_csv_path = resolve_data_path(paths, bw_csv);
  fs::path nch_map_path = resolve_conf_path(paths, nch_map);
  fs::path thermal_json_path = resolve_data_path(paths, thermal_json);
  fs::path out_root_path = resolve_out_path(paths, out_root);
  fs::path out_pdf_path = resolve_out_path(paths, out_pdf);

  // load inputs
  if (!bw_csv_path.empty() && !fs::exists(bw_csv_path)) {
    std::cerr << "[predict] ERROR: BW CSV not found: " << bw_csv_path << "\n"
              << "Searched data dirs:\n" << describe_data_search(paths);
    return 2;
  }
  if (!thermal_json_path.empty() && !fs::exists(thermal_json_path)) {
    std::cerr << "[predict] ERROR: thermal JSON not found: " << thermal_json_path << "\n"
              << "Searched data dirs:\n" << describe_data_search(paths);
    return 2;
  }
  if (!nch_map.empty() && !fs::exists(nch_map_path)) {
    std::cerr << "[predict] WARNING: Nch map not found: " << nch_map_path << "\n"
              << "Searched conf dirs:\n" << describe_conf_search(paths)
              << "(will rely on CSV Nch column if present)\n";
    nch_map_path.clear();
  }
  auto nchVals = read_nch_map(nch_map_path.string());

  auto bwPts = read_bw_csv(bw_csv_path.string(), nchVals, "ALL", verbose);
  if (bwPts.empty()){
    std::cerr << "[predict] ERROR: no BW rows loaded. Abort.\n"; return 2;
  }

  const bool vanilla_mode = (normalize_mode(thermal_mode) == "vanilla");
  std::vector<std::pair<double,double>> nch_dvdy_map;
  if (vanilla_mode) {
    nch_dvdy_map = build_nch_dvdy_map(bwPts);
    if (nch_dvdy_map.size() < 2) {
      std::cerr << "[predict] ERROR: vanilla mode requires dVdy in BW CSV for Nch->dVdy mapping.\n";
      return 2;
    }
  }

  auto systems = parse_systems(systems_spec);
  if (systems.empty()){
    std::cerr << "[predict] ERROR: could not parse --systems string\n"; return 2;
  }

  auto yieldTables = load_yield_tables(thermal_json_path.string(), thermal_mode, klist, !primordial, verbose);
  if (yieldTables.empty()){
    std::cerr << "[predict] ERROR: no yields found in JSON for requested mode/k\n"; return 2;
  }

  HadronCatalog catalog;
  std::string err;
  const std::string hadrons_path = resolve_common_data_path(paths, "hadrons.json").string();
  if (!catalog.load(hadrons_path, &err)){
    std::cerr << "[predict] ERROR: cannot load hadron catalog: " << err << "\n"
              << "Searched common data dirs:\n" << describe_common_data_search(paths);
    return 2;
  }

  // graphs + fits
  auto g_beta = make_graph(bwPts, &BWPoint::beta, &BWPoint::beta_stat, &BWPoint::beta_sys, "g_beta_vs_nch");
  auto g_T    = make_graph(bwPts, &BWPoint::Tkin, &BWPoint::Tkin_stat, &BWPoint::Tkin_sys, "g_T_vs_nch");
  auto g_n    = make_graph(bwPts, &BWPoint::nprof,&BWPoint::nprof_stat,&BWPoint::nprof_sys,"g_n_vs_nch");

  // Axis titles
  g_beta->SetTitle("#LT#beta_{T}#GT vs dN_{ch}/d#eta;dN_{ch}/d#eta;#LT#beta_{T}#GT");
  g_T->SetTitle("T_{kin} vs dN_{ch}/d#eta;dN_{ch}/d#eta;T_{kin} [GeV]");
  g_n->SetTitle("n_{profile} vs dN_{ch}/d#eta;dN_{ch}/d#eta;n_{profile}");

  int status_beta=0, status_T=0, status_n=0;
  auto fbeta = fit_graph(g_beta, f_beta, "f_beta_vs_nch", fitmin, fitmax, verbose, &status_beta, f_beta_pars);
  auto fT    = fit_graph(g_T,    f_T,    "f_T_vs_nch",    fitmin, fitmax, verbose, &status_T,    f_T_pars);
  auto fn    = fit_graph(g_n,    f_n,    "f_n_vs_nch",    fitmin, fitmax, verbose, &status_n,    f_n_pars);

  // prepare output ROOT
  fs::path outPath(out_root_path);
  if (outPath.has_parent_path()) fs::create_directories(outPath.parent_path());
  const std::string out_root_s = out_root_path.string();
  TFile* fout = TFile::Open(out_root_s.c_str(), "RECREATE");
  if (!fout || fout->IsZombie()){ std::cerr << "[predict] ERROR: cannot create ROOT file " << out_root_s << "\n"; return 2; }

  const double chi2_beta = fbeta->GetChisquare();
  const double chi2_T    = fT->GetChisquare();
  const double chi2_n    = fn->GetChisquare();
  const double ndf_beta  = fbeta->GetNDF();
  const double ndf_T     = fT->GetNDF();
  const double ndf_n     = fn->GetNDF();
  const double p_beta    = (ndf_beta>0) ? TMath::Prob(chi2_beta, ndf_beta) : std::numeric_limits<double>::quiet_NaN();
  const double p_T       = (ndf_T>0)    ? TMath::Prob(chi2_T,    ndf_T)    : std::numeric_limits<double>::quiet_NaN();
  const double p_n       = (ndf_n>0)    ? TMath::Prob(chi2_n,    ndf_n)    : std::numeric_limits<double>::quiet_NaN();

  // Fit canvases with legends
  auto make_fit_canvas = [&](const char* cname, TGraphErrors* g, TF1* f, double chi2, double ndf, double pval, int status, const std::string& formula){
    TCanvas* c = new TCanvas(cname, cname, 800, 600);
    g->SetMarkerStyle(20);
    g->SetMarkerColor(kBlack);
    g->SetLineColor(kBlack);
    g->Draw("AP");
    f->SetLineColor(kRed+1);
    f->SetLineWidth(2);
    f->Draw("SAME");
    TLegend* leg = new TLegend(0.55, 0.68, 0.88, 0.88);
    leg->SetBorderSize(0); leg->SetFillStyle(0);
    leg->AddEntry(g, "BW points", "lep");
    std::ostringstream ss;
    double redChi2 = (ndf>0) ? chi2/ndf : chi2;
    ss << "fit (#chi^{2}/ndf=" << std::fixed << std::setprecision(2) << redChi2
       << ", p=" << std::setprecision(2) << std::scientific << pval
       << ", status=" << std::defaultfloat << status << ")";
    leg->AddEntry(f, ss.str().c_str(), "l");
    // Formula and parameters as text-only legend lines
    std::string form = formula;
    if (form.size() > 70) form = form.substr(0,67) + "...";
    leg->AddEntry((TObject*)nullptr, ("f(x) = " + form).c_str(), "");

    std::ostringstream ps;
    ps << "pars:";
    const int npar = f->GetNpar();
    for (int i=0;i<npar;i++){
      ps << " p" << i << "=" << std::fixed << std::setprecision(3) << f->GetParameter(i);
      if (i != npar-1) ps << ",";
      if ((int)ps.str().size() > 80 && i != npar-1){ ps << "..."; break; }
    }
    leg->AddEntry((TObject*)nullptr, ps.str().c_str(), "");
    leg->Draw();
    return c;
  };

  TCanvas* c_fit_beta = make_fit_canvas("c_fit_beta", g_beta, fbeta, chi2_beta, ndf_beta, p_beta, status_beta, f_beta);
  TCanvas* c_fit_T    = make_fit_canvas("c_fit_T",    g_T,    fT,    chi2_T,    ndf_T,    p_T,    status_T,    f_T);
  TCanvas* c_fit_n    = make_fit_canvas("c_fit_n",    g_n,    fn,    chi2_n,    ndf_n,    p_n,    status_n,    f_n);

  fout->WriteObject(g_beta, g_beta->GetName());
  fout->WriteObject(g_T,    g_T->GetName());
  fout->WriteObject(g_n,    g_n->GetName());
  fbeta->Write();
  fT->Write();
  fn->Write();
  c_fit_beta->Write();
  c_fit_T->Write();
  c_fit_n->Write();
  TParameter<int>("npt", npt).Write("npt");
  TParameter<double>("ptmin", ptmin).Write("ptmin");
  TParameter<double>("ptmax", ptmax).Write("ptmax");
  const std::string thermal_json_s = thermal_json_path.string();
  TNamed("thermal_json", thermal_json_s.c_str()).Write();
  TNamed("thermal_mode", thermal_mode.c_str()).Write();
  const std::string bw_csv_s = bw_csv_path.string();
  TNamed("bw_csv", bw_csv_s.c_str()).Write();
  TNamed("fit_formula_beta", f_beta.c_str()).Write();
  TNamed("fit_formula_T", f_T.c_str()).Write();
  TNamed("fit_formula_n", f_n.c_str()).Write();
  TParameter<double>("chi2_beta", chi2_beta).Write("chi2_beta");
  TParameter<double>("chi2_T", chi2_T).Write("chi2_T");
  TParameter<double>("chi2_n", chi2_n).Write("chi2_n");
  TParameter<double>("ndf_beta", ndf_beta).Write("ndf_beta");
  TParameter<double>("ndf_T", ndf_T).Write("ndf_T");
  TParameter<double>("ndf_n", ndf_n).Write("ndf_n");
  TParameter<double>("pval_beta", p_beta).Write("pval_beta");
  TParameter<double>("pval_T", p_T).Write("pval_T");
  TParameter<double>("pval_n", p_n).Write("pval_n");
  TParameter<int>("fit_status_beta", status_beta).Write("fit_status_beta");
  TParameter<int>("fit_status_T", status_T).Write("fit_status_T");
  TParameter<int>("fit_status_n", status_n).Write("fit_status_n");
  std::cout << "[predict] pT grid for normalization: [" << ptmin << ", " << ptmax
            << "] with " << npt << " points\n";

  // containers for plotting
  struct SpectraEntry {
    TGraph* g;
    std::string system;
    double k;
    double nch;
    std::string name;
  };
  std::map<int, std::vector<SpectraEntry>> spectra_by_species;

  // compute spectra
  int centrality_tag = 0;
  for (const auto& sys : systems){
    for (double nch : sys.nch_values){
      double target_yield = nch;
      double dvdy = std::numeric_limits<double>::quiet_NaN();
      if (vanilla_mode) {
        dvdy = map_nch_to_dvdy(nch_dvdy_map, nch);
        if (!std::isfinite(dvdy)) {
          std::cerr << "[predict] WARNING: cannot map Nch=" << nch << " to dVdy; skipping.\n";
          continue;
        }
        target_yield = dvdy;
      }
      double beta = fbeta->Eval(nch);
      double T    = fT->Eval(nch);
      double nprof= fn->Eval(nch);
      if (verbose){
        std::cerr << "[predict] " << sys.name << " Nch="<<nch;
        if (vanilla_mode) std::cerr << " dVdy="<<dvdy;
        std::cerr << " -> beta_t="<<beta<<" Tkin="<<T<<" n="<<nprof<<"\n";
      }
      for (const auto& tab : yieldTables){
        auto yields = interpolate_yields(tab, target_yield, vanilla_mode);
        if (yields.empty()){
          std::cerr << "[predict] WARNING: no yields for k="<<tab.k<<" at "
                    << (vanilla_mode ? "dVdy=" : "Nch=") << target_yield << "\n";
          continue;
        }

        // directory in ROOT
        std::ostringstream dir; dir << "spec_k" << tab.k << "/" << sys.name << "/nch_" << std::round(nch*10)/10.0;
        fout->mkdir(dir.str().c_str());
        fout->cd(dir.str().c_str());

        for (int pdg : species){
          auto itY = yields.find(pdg);
          if (itY == yields.end()) continue;

          HadronIntegrationInfo info{};
          info.hadron = make_hadron_from_catalog(pdg, catalog);
          info.beta_t = beta;
          info.Tkin   = T;
          info.n_profile = nprof;
          info.yield  = itY->second;
          info.centrality_class = centrality_tag;

          int npoints = npt;
          double use_ptmin = ptmin;
          if (!timesPt && ptmin==0.0) use_ptmin = 1e-5; // avoid divide-by-zero

          TGraph* gr = computePtSpectrum_tGraph(info, use_ptmin, ptmax, timesPt, clampR, /*rmax=*/0.0, /*intPoints=*/npoints, verbose);
          if (!gr) continue;
          std::ostringstream gname; gname << "g_" << info.hadron.name << "_pdg" << pdg
                                          << "_k" << tab.k << "_" << sys.name << "_Nch" << nch;
          gr->SetName(gname.str().c_str());
          gr->SetTitle(Form("%s (%s, k=%.2f, Nch=%.1f);p_{T} [GeV/c];%s",
                            info.hadron.name.c_str(), sys.name.c_str(), tab.k, nch,
                            timesPt ? "dN/dp_{T}" : "(1/p_{T}) dN/dp_{T}"));
          gr->Write();
          spectra_by_species[pdg].push_back({(TGraph*)gr->Clone(), sys.name, tab.k, nch, info.hadron.name}); // for plotting; file owns the original
          delete gr; // ROOT file keeps its copy
        }
        fout->cd();
      }
      ++centrality_tag;
    }
  }

  fout->Write();
  fout->Close();
  std::cout << "[predict] Wrote ROOT output to " << out_root_s << "\n";

  // quick-look PDF (one canvas per species/system/k to reduce clutter)
  if (make_pdf && !spectra_by_species.empty()){
    fs::path pdfPath(out_pdf_path);
    if (pdfPath.has_parent_path()) fs::create_directories(pdfPath.parent_path());
    gStyle->SetOptStat(0);
    TCanvas c("c","c",900,700);
    const std::string out_pdf_s = out_pdf_path.string();
    c.Print((out_pdf_s+"[").c_str()); // open

    for (auto& kv : spectra_by_species){
      int pdg = kv.first;
      auto& vec = kv.second;
      // group by system then k to keep pages readable
      std::map<std::string, std::map<double, std::vector<SpectraEntry>>> grouped;
      for (const auto& e : vec) grouped[e.system][e.k].push_back(e);

      for (auto& sysKV : grouped){
        const std::string& sysName = sysKV.first;
        for (auto& kKV : sysKV.second){
          double kVal = kKV.first;
          auto& entries = kKV.second;

          int colorIdx = 1;
          TMultiGraph mg;
          TLegend leg(0.60,0.68,0.88,0.88);
          leg.SetBorderSize(0); leg.SetFillStyle(0);

          for (const auto& e : entries){
            TGraph* g = e.g;
            int color = colorIdx % 9 + 1;
            g->SetLineColor(color);
            g->SetMarkerColor(color);
            mg.Add(g, "L");
            std::ostringstream lab;
            lab << e.name << " | " << sysName << ", k=" << kVal << ", Nch=" << std::fixed << std::setprecision(1) << e.nch;
            leg.AddEntry(g, lab.str().c_str(), "l");
            ++colorIdx;
          }

          mg.Draw("AL");
          mg.GetXaxis()->SetTitle("p_{T} [GeV/c]");
          mg.GetYaxis()->SetTitle(timesPt ? "dN/dp_{T}" : "(1/p_{T}) dN/dp_{T}");
          std::string speciesName = entries.empty() ? Form("PDG %d", pdg) : entries.front().name;
          std::string title = Form("%s (PDG %d) spectra - %s, k=%.2f", speciesName.c_str(), pdg, sysName.c_str(), kVal);
          mg.SetTitle(title.c_str());
          // Explicit title overlay (TPave-like) to ensure visibility in PDF
          TLatex lat;
          lat.SetNDC();
          lat.SetTextSize(0.04);
          lat.DrawLatex(0.12, 0.94, title.c_str());
          leg.Draw();
          c.Print(out_pdf_s.c_str());
        }
      }
    }
    c.Print((out_pdf_s+"]").c_str()); // close
    std::cout << "[predict] Wrote quick-look PDF to " << out_pdf_s << "\n";
  }

  return 0;
}
