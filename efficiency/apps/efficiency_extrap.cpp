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
#include "../include/eff_fit.h"
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
  "  [--kaon-decay-corr]                             # if using kaon as reference for stable particles, correct for its decay\n"
  "  [--kaon-decay-l LENGTH]                         # if correcting for kaon decay, set travel length\n"
  "  [--pt-bins a,b,n]                               # only needed for TGraph spectra w/o metadata\n"
  "  [--out JSON] [--out-root ROOT]\n"
  "  [--fit-refs] [--fit-model logistic/turnon] [--fit-range MIN,MAX]   # forces logistic or turnon fit to input eff curves in the specified range";
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

// Make a TH1D from RefData (works for both pointwise and binned)
static TH1D* th1_from_ref(const RefData& r, const char* name) {
  if (r.isBinned()) {
    const int nb = (int)r.val.size();
    TH1D* h = new TH1D(name, "", nb, r.edges.data());
    for (int i=1;i<=nb;++i){ h->SetBinContent(i, r.val[i-1]); }
    return h;
  } else { // pointwise: build edges from midpoints
    const int n = (int)r.x.size();
    if (n<2) throw std::runtime_error("th1_from_ref: not enough points");
    std::vector<double> edges(n+1);
    edges[0] = r.x[0] - 0.5*(r.x[1]-r.x[0]);
    for (int i=1;i<n;++i) edges[i] = 0.5*(r.x[i-1]+r.x[i]);
    edges[n] = r.x[n-1] + 0.5*(r.x[n-1]-r.x[n-2]);
    TH1D* h = new TH1D(name, "", n, edges.data());
    for (int i=1;i<=n;++i){ h->SetBinContent(i, r.y[i-1]); }
    return h;
  }
}

// Resample TF1 into a dense pointwise RefData over [xmin,xmax]
static RefData ref_from_TF1(TF1* f, double xmin, double xmax, int N=400) {
  RefData r;                 // pointwise RefData
  r.x.resize(N); r.y.resize(N);
  for (int i=0;i<N;++i) {
    const double x = xmin + (xmax - xmin) * (i + 0.5)/N;
    double v = f->Eval(x); if (v<0) v=0; if (v>1) v=1;
    r.x[i]=x; r.y[i]=v;
  }
  return r;
}

int main(int argc, char** argv){
  std::string bwroot, obj, out_json="efficiency_out.json", out_root;
  std::string pi_in, K_in, p_in;
  int nbins_override=0; double ptmin=0, ptmax=0;
  bool correct_for_kaon_decay = false;
  double kaon_path_length = 3.99;
  bool fit_refs = false;
  std::string fit_model = "logistic";     // or "turnon"
  double fit_xmin = -1, fit_xmax = -1;

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
    else if (a=="--kaon-decay-corr") correct_for_kaon_decay = true;
    else if (a=="--kaon-decay-l")
    {
      std::string c = need(a.c_str());
      kaon_path_length = std::stod(c);
    }
    else if (a=="--fit-refs") fit_refs = true;
    else if (a=="--fit-model") fit_model = argv[++i];           // logistic | turnon
    else if (a=="--fit-range")
    { 
      auto s=std::string(argv[++i]); // "xmin,xmax"
      auto c = s.find(','); fit_xmin = std::stod(s.substr(0,c));
      fit_xmax = std::stod(s.substr(c+1));
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

  // --- Optional smooth fit of the reference ε(pT) curves ---
  TF1 *f_pi = nullptr, *f_K = nullptr, *f_p = nullptr;

  if (fit_refs) {
    // Keep originals to extract ranges before we overwrite
    const RefData Rpi0 = Rpi, RK0 = RK, Rp0 = Rp;

    // Build temporary histograms from RefData
    TH1D* h_pi = th1_from_ref(Rpi0, "eff_pi_ref");
    TH1D* h_K  = th1_from_ref(RK0 , "eff_K_ref");
    TH1D* h_p  = th1_from_ref(Rp0 , "eff_p_ref");

    // Choose model
    EffModel model = (fit_model=="logistic") ? EffModel::Logistic : EffModel::LogisticTurnOn;

    // Fit to get smooth TF1s (ROOT-native fits)
    f_pi = fit_efficiency(h_pi, model, fit_xmin, fit_xmax, /*verbose=*/false);
    f_K  = fit_efficiency(h_K , model, fit_xmin, fit_xmax, /*verbose=*/false);
    f_p  = fit_efficiency(h_p , model, fit_xmin, fit_xmax, /*verbose=*/false);

    // Resample the fits densely back into pointwise RefData (so isPointwise()==true)
    auto bounds = [&](const RefData& r)->std::pair<double,double>{
      if (r.isBinned()) return { r.edges.front(), r.edges.back() };
      else              return { r.x.front(),     r.x.back()     };
    };
    auto [xmin_pi,xmax_pi] = bounds(Rpi0);
    auto [xmin_K ,xmax_K ] = bounds(RK0 );
    auto [xmin_p ,xmax_p ] = bounds(Rp0 );

    Rpi = ref_from_TF1(f_pi, xmin_pi, xmax_pi);
    RK  = ref_from_TF1(f_K , xmin_K , xmax_K );
    Rp  = ref_from_TF1(f_p , xmin_p , xmax_p );

  }

  // Choose extrapolator mode
  ExtrapConfig cfg(correct_for_kaon_decay, kaon_path_length);
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
    if (f_pi) { f_pi->SetName("fit_pi"); fout.cd(); f_pi->Write(); }
    if (f_K ) { f_K ->SetName("fit_K");  fout.cd(); f_K ->Write(); }
    if (f_p ) { f_p ->SetName("fit_p");  fout.cd(); f_p ->Write(); }
    fout.Close();
    std::cout << "[efficiency_extrap] wrote " << out_root << "\n";
  }

  return 0;
}