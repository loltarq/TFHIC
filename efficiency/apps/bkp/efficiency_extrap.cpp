// apps/efficiency_extrap.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include <TFile.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TGraph.h>
#include <TParameter.h>
#include <TObjString.h>

#include "../include/eff_extrap.h"
#include "../include/bw_adapter.h"
#include "../include/ref_from_root.h"
#include "../../common/hadron_catalog.h"

using namespace tfhic;

// ---------- CSV reader kept for backwards-compat ----------
struct CsvRef { std::vector<double> edges,val,x,y; };
static std::string trim(const std::string& s){ const char* ws=" \t\r\n"; auto i=s.find_first_not_of(ws); if(i==std::string::npos) return ""; auto j=s.find_last_not_of(ws); return s.substr(i,j-i+1); }
static void split(const std::string& s, char sep, std::vector<std::string>& out){ out.clear(); std::stringstream ss(s); std::string x; while (std::getline(ss,x,sep)) if(!x.empty()) out.push_back(x); }
static bool read_eps_csv(const std::string& path, CsvRef& out){
  std::ifstream in(path); if(!in) return false;
  std::string line; bool header=false, binned=false;
  while (std::getline(in,line)){
    if (trim(line).empty()) continue;
    if (!header){
      header=true;
      if (line.find("low")!=std::string::npos && line.find("high")!=std::string::npos) { binned=true; }
      else if (line.find("pt")!=std::string::npos && line.find("eps")!=std::string::npos) { binned=false; }
      else { binned=true; }
      in.clear(); in.seekg(0); continue;
    }
    std::vector<std::string> c; split(line, ',', c); if (c.size()<2) continue;
    try{
      if (binned && c.size()>=3){
        double lo=std::stod(trim(c[0])), hi=std::stod(trim(c[1])), v=std::stod(trim(c[2]));
        if (out.edges.empty()) out.edges.push_back(lo);
        out.edges.push_back(hi); out.val.push_back(v);
      } else {
        out.x.push_back(std::stod(trim(c[0]))); out.y.push_back(std::stod(trim(c[1])));
      }
    }catch(...){}
  }
  return true;
}
// ---------------------------------------------------------

static void usage(const char* prog){
  std::cerr <<
  "Usage: " << prog << " \\\n"
  "  --bw-root FILE.root --obj PATH                  # hadron spectrum (TH1 or TGraph)\n"
  "  --eps-pi CSV|ROOT --eps-K CSV|ROOT --eps-p CSV|ROOT\n"
  "     CSV  = path/to/file.csv\n"
  "     ROOT = file.root#path/in/file [#tree-branches pt=pt,eps=epsilon | low=pt_low,high=pt_high,eps=eff]\n"
  "  [--gamma-baryon auto|VAL] [--gamma-smeson auto|VAL] [--gamma-meson VAL]\n"
  "  [--cap-plateau on|off]\n"
  "  [--survival auto|off] [--flight-L LENGTH]       # apply general unstable corrections by Psurv, replaces/extends --kaon-decay-corr and --kaon-decay-l\n"
  "  [--kaon-decay-corr]                             # LEGACY: if using kaon as reference for stable particles, correct for its decay\n"
  "  [--kaon-decay-l LENGTH]                         # LEGACY: if correcting for kaon decay, set travel length\n"
  "  [--pt-bins a,b,n]                               # only needed for TGraph spectra w/o metadata\n"
  "  [--out JSON] [--out-root ROOT]\n";
}

// Parse ROOT spec like "file.root#dir/object[#tree-branches ...]"
struct RootSpec { std::string file, obj, branches; bool isRoot=false; };
static RootSpec parse_root_spec(const std::string& s){
  RootSpec rs;
  auto hash = s.find('#');
  if (hash == std::string::npos) { rs.isRoot=false; rs.file=s; return rs; } // CSV path or just file
  rs.isRoot = true;
  rs.file = s.substr(0, hash);
  auto rest = s.substr(hash+1);
  auto hash2 = rest.find('#');
  if (hash2 == std::string::npos) { rs.obj = rest; }
  else { rs.obj = rest.substr(0, hash2); rs.branches = rest.substr(hash2+1); }
  return rs;
}
static BranchNames parse_branches(const std::string& s){
  BranchNames bn;
  if (s.empty()) return bn;
  std::vector<std::string> kv; split(s, ',', kv);
  for (auto &pair : kv){
    auto eq = pair.find('=');
    if (eq==std::string::npos) continue;
    auto k = trim(pair.substr(0,eq));
    auto v = trim(pair.substr(eq+1));
    if (k=="low")  bn.low=v;
    else if (k=="high") bn.high=v;
    else if (k=="eps") bn.eps=v;
    else if (k=="pt")  bn.pt=v;
  }
  return bn;
}

int main(int argc, char** argv){
  std::string bwroot, obj, out_json="efficiency_out.json", out_root;
  std::string pi_in, K_in, p_in;
  int nbins_override=0; double ptmin=0, ptmax=0;
  std::string gamma_b_str="auto", gamma_s_str="auto";
  double gamma_m = 1.0;
  std::string survival_mode="auto";
  bool cap_plateau = true;
  double L_eff = 4.0;
  // keep legacy:
  bool  kaon_corr = false;
  double kaon_L = 4.0;


  for (int i=1;i<argc;++i){
    std::string a = argv[i];
    auto need = [&](const char* flag){ if (i+1>=argc) { usage(argv[0]); exit(2);} return std::string(argv[++i]); };
    if (a=="--bw-root") bwroot = need(a.c_str());
    else if (a=="--obj") obj = need(a.c_str());
    else if (a=="--eps-pi") pi_in = need(a.c_str());
    else if (a=="--eps-K" || a=="--eps-k") K_in = need(a.c_str());
    else if (a=="--eps-p") p_in = need(a.c_str());
    else if (a=="--pt-bins"){
      auto s = need(a.c_str()); std::vector<std::string> c; split(s, ',', c);
      if (c.size()!=3){ std::cerr << "--pt-bins a,b,n\n"; return 2; }
      ptmin = std::stod(c[0]); ptmax = std::stod(c[1]); nbins_override = std::stoi(c[2]);
    }
    else if (a=="--out") out_json = need(a.c_str());
    else if (a=="--out-root") out_root = need(a.c_str());
    else if (a=="--gamma-baryon") gamma_b_str = need(a.c_str());
    else if (a=="--gamma-smeson") gamma_s_str = need(a.c_str());
    else if (a=="--gamma-meson")  gamma_m = std::stod(need(a.c_str()));
    else if (a=="--cap-plateau")  { std::string v=need(a.c_str()); cap_plateau = (v!="off" && v!="0" && v!="false" && v!="FALSE" && v!="OFF"); }
    else if (a=="--survival")     survival_mode = need(a.c_str()); // "auto" or "off"
    else if (a=="--flight-L")     L_eff = std::stod(need(a.c_str()));
    // legacy (honor if given)
    else if (a=="--kaon-decay-corr") kaon_corr = true;
    else if (a=="--kaon-decay-l")
    {
      std::string c = need(a.c_str());
      kaon_L = std::stod(c);
    } 
    else if (a=="--help" || a=="-h"){ usage(argv[0]); return 0; }
  }

  if (bwroot.empty() || obj.empty() || pi_in.empty() || K_in.empty() || p_in.empty()){
    usage(argv[0]); return 2;
  }

  // Load hadron catalog
  HadronCatalog cat; std::string err;
  if (!cat.load("common/data/hadrons.json", &err) && !cat.load("../../common/data/hadrons.json", &err)){
    std::cerr << "Failed to load hadron catalog: " << err << "\n"; return 2;
  }

  // Make spectrum callable from BW ROOT
  TFile f(bwroot.c_str(), "READ");
  if (f.IsZombie()){ std::cerr << "Cannot open BW file: " << bwroot << "\n"; return 2; }
  std::function<double(double)> Sh;
  if (auto* h = dynamic_cast<TH1*>(f.Get(obj.c_str()))){
    Sh = makeSpectrumFromTH1(h);
    if (nbins_override==0){
      nbins_override = h->GetNbinsX();
      ptmin = h->GetXaxis()->GetXmin();
      ptmax = h->GetXaxis()->GetXmax();
    }
  } else if (auto* g = dynamic_cast<TGraph*>(f.Get(obj.c_str()))){
    Sh = makeSpectrumFromTGraph(g);
    if (nbins_override==0){
      // try metadata
      auto* pnb  = dynamic_cast<TParameter<int>*>(f.Get("nbins"));
      auto* pmin = dynamic_cast<TParameter<double>*>(f.Get("ptmin"));
      auto* pmax = dynamic_cast<TParameter<double>*>(f.Get("ptmax"));
      if (pnb && pmin && pmax){ nbins_override=pnb->GetVal(); ptmin=pmin->GetVal(); ptmax=pmax->GetVal(); }
    }
    if (nbins_override==0){ std::cerr << "For TGraph spectra, supply --pt-bins a,b,n.\n"; return 2; }
  } else {
    std::cerr << "Spectrum object not found: " << obj << "\n"; return 2;
  }

  // Build pT edges
  std::vector<double> edges(nbins_override+1);
  const double binw = (ptmax-ptmin)/nbins_override;
  for (int i=0;i<=nbins_override;++i) edges[i] = ptmin + i*binw;

  // ---------- Load reference efficiencies for pi/K/p (CSV or ROOT) ----------
  auto load_ref_any = [](const std::string& spec) -> RefData {
    RefData rd;
    auto rs = parse_root_spec(spec);
    if (rs.isRoot) {
      rd.clear();
      if (!load_ref_from_root_obj(rs.file, rs.obj, rd, parse_branches(rs.branches)))
        throw std::runtime_error("Failed to load ROOT ref: "+spec);
      return rd;
    } else {
      // CSV
      CsvRef c; if (!read_eps_csv(spec, c)) throw std::runtime_error("Failed CSV: "+spec);
      if (!c.edges.empty()){ rd.edges=c.edges; rd.val=c.val; }
      else { rd.x=c.x; rd.y=c.y; }
      return rd;
    }
  };

  RefData Rpi = load_ref_any(pi_in);
  RefData RK  = load_ref_any(K_in);
  RefData Rp  = load_ref_any(p_in);

  auto ref_plateau = [](const RefData& r){
    // robust median of top ~20% in pT
    std::vector<std::pair<double,double>> pts; pts.reserve(r.isPointwise()?r.x.size():r.val.size());
    if (r.isPointwise()){
      for (size_t i=0;i<r.x.size();++i) pts.emplace_back(r.x[i], r.y[i]);
    } else {
      for (size_t b=0;b<r.val.size();++b){
        double lo=r.edges[b], hi=r.edges[b+1], xc=0.5*(lo+hi);
        pts.emplace_back(xc, r.val[b]);
      }
    }
    std::sort(pts.begin(), pts.end(), [](auto& a, auto& b){ return a.first < b.first; });
    size_t n = pts.size(); size_t k0 = (n*4)/5; if (k0>=n) k0=n? n-1:0;
    std::vector<double> top; for (size_t i=k0;i<n;++i) top.push_back(pts[i].second);
    std::sort(top.begin(), top.end());
    return top.empty()? 1.0 : top[top.size()/2]; // median
  };
  
  auto ref_pt50 = [&](const RefData& r, double plateau){
    const double half = 0.5*plateau;
    if (r.isPointwise()){
      for (size_t i=1;i<r.x.size();++i){
        double x0=r.x[i-1], y0=r.y[i-1], x1=r.x[i], y1=r.y[i];
        if ((y0<=half && y1>=half) || (y0>=half && y1<=half)){
          const double t = (half - y0)/std::max(1e-12, (y1-y0));
          return x0 + t*(x1-x0);
        }
      }
      return r.x.empty()? 0.0 : r.x.back();
    } else {
      for (size_t b=0;b<r.val.size();++b){
        if (r.val[b] >= half){
          double lo=r.edges[b], hi=r.edges[b+1];
          return 0.5*(lo+hi);
        }
      }
      return r.edges.empty()? 0.0 : r.edges.back();
    }
  };

  double plat_pi = ref_plateau(Rpi);
  double plat_K  = ref_plateau(RK);
  double plat_p  = ref_plateau(Rp);
  double pt50_pi = ref_pt50(Rpi, plat_pi);
  double pt50_K  = ref_pt50(RK,  plat_K);
  double pt50_p  = ref_pt50(Rp,  plat_p);

  // Build extrap cfg
  ExtrapConfig cfg;
  cfg.cap_plateau = cap_plateau;
  cfg.cap_meson   = plat_pi;
  cfg.cap_smeson  = plat_K;
  cfg.cap_baryon  = plat_p;

  cfg.survival_auto     = (survival_mode!="off");
  cfg.L_m               = (survival_mode!="off") ? L_eff : kaon_L; // prefer new knob; keep legacy if only that is given
  cfg.correct_kaon_decay= kaon_corr; // legacy; harmless if survival_auto=true

  cfg.gamma_meson  = gamma_m;
  auto* pi_def = cat.get(211);
  auto* k_def  = cat.get(321);
  auto* p_def  = cat.get(2212);
  const double m_pi = pi_def ? pi_def->mass_GeV : 0.13957039;
  const double m_K  = k_def  ? k_def->mass_GeV  : 0.49367;
  const double m_p  = p_def  ? p_def->mass_GeV  : 0.93827208816;
  if (gamma_b_str=="auto" && pt50_pi>0 && plat_pi>0 && pt50_p>0){
    cfg.gamma_baryon = std::clamp(std::log(pt50_p/pt50_pi)/std::log(m_p/m_pi), 0.5, 1.5);
  } else if (gamma_b_str!="auto") {
    cfg.gamma_baryon = std::stod(gamma_b_str);
  }
  if (gamma_s_str=="auto" && pt50_pi>0 && plat_pi>0 && pt50_K>0){
    cfg.gamma_smeson = std::clamp(std::log(pt50_K/pt50_pi)/std::log(m_K/m_pi), 0.5, 1.5);
  } else if (gamma_s_str!="auto") {
    cfg.gamma_smeson = std::stod(gamma_s_str);
  }

  //pass to extrapolator
  EffExtrapolator X = [&](){
    if (Rpi.isPointwise() && RK.isPointwise() && Rp.isPointwise()) {
      auto toL = [](const RefData& r){ tfhic::LogitInterp L; L.x=r.x; L.glog.resize(r.y.size()); for (size_t i=0;i<r.y.size();++i) L.glog[i]=tfhic::logit(r.y[i]); return L; };
      return EffExtrapolator(cat, toL(Rpi), toL(RK), toL(Rp), cfg);
    } else if (Rpi.isBinned() && RK.isBinned() && Rp.isBinned()) {
      auto toB = [](const RefData& r){ tfhic::BinnedCurve B; B.edges=r.edges; B.val=r.val; return B; };
      return EffExtrapolator(cat, toB(Rpi), toB(RK), toB(Rp), cfg);
    } else {
      throw std::runtime_error("Reference inputs must be all pointwise or all binned.");
    }
  }();

  // Infer PDG from spectrum object name
  int pdg=0; if (auto pos = obj.rfind("pdg"); pos!=std::string::npos) { try { pdg = std::stoi(obj.substr(pos+3)); } catch(...){} }
  if (!cat.has(pdg)) { std::cerr << "WARNING: PDG not inferred from object path; defaulting to proton (2212)\n"; pdg=2212; }

  // Compute bin-averaged epsilon
  std::vector<double> eps(nbins_override);
  const bool use_interpolant = Rpi.isPointwise();
  for (int b=0;b<nbins_override;++b){
    double a=edges[b], bnd=edges[b+1];
    eps[b] = use_interpolant ? X.epsilon_bin_weighted(pdg, a, bnd, Sh)
                             : X.epsilon_bin_mixture (pdg, a, bnd, Sh);
  }

  // JSON output
  if (!out_json.empty()){
    std::ofstream out(out_json);
    out << "{\n  \"pdg\": " << pdg << ",\n  \"obj\": \"" << obj << "\",\n  \"pt_edges\": [";
    for (size_t i=0;i<edges.size();++i){ if(i) out<<","; out<<edges[i]; }
    out << "],\n  \"epsilon_bin\": [";
    for (size_t i=0;i<eps.size();++i){ if(i) out<<","; out<<eps[i]; }
    out << "]\n}\n";
    std::cout << "[efficiency_extrap] wrote " << out_json << "\n";
  }

  // ROOT output (recommended)
  if (!out_root.empty()){
    TFile fout(out_root.c_str(), "RECREATE");
    TH1D h("epsilon_bin","Bin-avg efficiency; p_{T} (GeV/c); #varepsilon", nbins_override, edges.data());
    for (int b=1;b<=nbins_override;++b) h.SetBinContent(b, eps[b-1]);
    h.Write();
    TParameter<int>("pdg", pdg).Write("pdg");
    TObjString(obj.c_str()).Write("source_object");
    TObjString(use_interpolant?"interpolant":"binned_mixture").Write("mode");
    fout.Close();
    std::cout << "[efficiency_extrap] wrote " << out_root << "\n";
  }

  return 0;
}