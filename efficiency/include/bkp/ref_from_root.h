// include/ref_from_root.h
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <tuple>   
#include <numeric> 

#include <TFile.h>
#include <TObject.h>
#include <TH1.h>
#include <TGraph.h>
#include <TTree.h>
#include <TEfficiency.h>

struct RefData {
  // Use ONE of the two forms:
  // - Binned form:
  std::vector<double> edges; // size nbins+1
  std::vector<double> val;   // size nbins
  // - Pointwise form:
  std::vector<double> x;     // pt values
  std::vector<double> y;     // eps values

  bool isBinned()     const { return !edges.empty() && !val.empty(); }
  bool isPointwise()  const { return !x.empty() && !y.empty(); }
  void clear() { edges.clear(); val.clear(); x.clear(); y.clear(); }
};

// Optional branch name mapping for TTrees
struct BranchNames {
  // Either set (low, high, eps) for binned rows
  std::string low, high, eps;
  // Or set (pt, eps) for pointwise rows
  std::string pt;
};

inline bool load_ref_from_TH1(TH1* h, RefData& out) {
  if (!h) return false;
  const int nb = h->GetNbinsX();
  out.edges.resize(nb+1);
  out.val.resize(nb);
  for (int b=1; b<=nb; ++b) {
    out.edges[b-1] = h->GetXaxis()->GetBinLowEdge(b);
    out.val[b-1]   = h->GetBinContent(b);
  }
  out.edges[nb] = h->GetXaxis()->GetBinUpEdge(nb);
  return true;
}

inline bool load_ref_from_TGraph(TGraph* g, RefData& out, bool useSpline=true) {
  if (!g) return false;
  const int n = g->GetN();
  out.x.resize(n); out.y.resize(n);
  double xi, yi;
  for (int i=0;i<n;++i) {
    g->GetPoint(i, xi, yi);
    out.x[i]=xi; out.y[i]=yi;
  }
  // ensure ascending in x
  std::vector<size_t> idx(n);
  std::iota(idx.begin(), idx.end(), 0);
  std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b){ return out.x[a] < out.x[b]; });
  std::vector<double> X(n), Y(n);
  for (int i=0;i<n;++i){ X[i]=out.x[idx[i]]; Y[i]=out.y[idx[i]]; }
  out.x.swap(X); out.y.swap(Y);
  return true;
}

inline bool load_ref_from_TEfficiency(TEfficiency* e, RefData& out) {
  if (!e) return false;
  const TH1* hTot = e->GetTotalHistogram();
  if (!hTot) return false;
  const int nb = hTot->GetNbinsX();
  out.edges.resize(nb+1);
  out.val.resize(nb);
  for (int b=1; b<=nb; ++b) {
    out.edges[b-1] = hTot->GetXaxis()->GetBinLowEdge(b);
    out.val[b-1]   = e->GetEfficiency(b); // central estimate
  }
  out.edges[nb] = hTot->GetXaxis()->GetBinUpEdge(nb);
  return true;
}

// Basic auto-detection of branch names if BranchNames is empty.
inline BranchNames guess_branches(TTree* t) {
  BranchNames bn;
  auto has = [&](const char* name){ return t->GetBranch(name)!=nullptr; };
  // binned form candidates
  const char* lows [] = {"low","pt_low","ptLow","ptmin","ptMin","pT_low","pTmin","bin_low"};
  const char* highs[] = {"high","pt_high","ptHigh","pthigh","ptmax","ptMax","pT_high","pTmax","bin_high"};
  const char* epss [] = {"eps","epsilon","eff","efficiency","Eff","Efficiency"};
  // pointwise candidates
  const char* pts  [] = {"pt","pT","mom","momentum","PT"};

  for (auto s: lows)  if (has(s)) { bn.low=s; break; }
  for (auto s: highs) if (has(s)) { bn.high=s; break; }
  for (auto s: epss)  if (has(s)) { bn.eps=s; break; }
  if (!bn.low.empty() && !bn.high.empty() && !bn.eps.empty()) return bn;

  for (auto s: pts)   if (has(s)) { bn.pt=s; break; }
  for (auto s: epss)  if (has(s)) { bn.eps=s; break; }
  return bn;
}

inline bool load_ref_from_TTree(TTree* t, RefData& out, BranchNames bn = {}) {
  if (!t) return false;
  if (bn.low.empty() && bn.high.empty() && bn.pt.empty()) {
    bn = guess_branches(t);
  }
  out.clear();
  if (!bn.low.empty() && !bn.high.empty() && !bn.eps.empty()) {
    // binned rows
    double lo=0, hi=0, e=0;
    t->SetBranchAddress(bn.low.c_str(),  &lo);
    t->SetBranchAddress(bn.high.c_str(), &hi);
    t->SetBranchAddress(bn.eps.c_str(),  &e);
    const Long64_t N = t->GetEntries();
    std::vector<std::tuple<double,double,double>> rows; rows.reserve(N);
    for (Long64_t i=0;i<N;++i){
      t->GetEntry(i);
      rows.emplace_back(lo,hi,e);
    }
    std::sort(rows.begin(), rows.end());
    out.edges.clear(); out.val.clear();
    for (size_t i=0;i<rows.size();++i){
      auto [l,h,v] = rows[i];
      if (out.edges.empty()) out.edges.push_back(l);
      out.edges.push_back(h);
      out.val.push_back(v);
    }
    return !out.val.empty();
  }
  if (!bn.pt.empty() && !bn.eps.empty()) {
    // pointwise rows
    double pt=0, e=0;
    t->SetBranchAddress(bn.pt.c_str(),  &pt);
    t->SetBranchAddress(bn.eps.c_str(), &e);
    const Long64_t N = t->GetEntries();
    out.x.resize(N); out.y.resize(N);
    for (Long64_t i=0;i<N;++i){ t->GetEntry(i); out.x[i]=pt; out.y[i]=e; }
    // sort by pt
    std::vector<size_t> idx(N);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](size_t a,size_t b){ return out.x[a] < out.x[b]; });
    std::vector<double> X(N), Y(N);
    for (size_t i=0;i<idx.size();++i){ X[i]=out.x[idx[i]]; Y[i]=out.y[idx[i]]; }
    out.x.swap(X); out.y.swap(Y);
    return !out.x.empty();
  }
  return false;
}

inline bool load_ref_from_root_obj(const std::string& file, const std::string& objpath,
                                   RefData& out, BranchNames treeBranches = {}) {
  TFile f(file.c_str(), "READ");
  if (f.IsZombie()) throw std::runtime_error("Cannot open ROOT file: " + file);
  TObject* obj = f.Get(objpath.c_str());
  if (!obj) throw std::runtime_error("Object not found: " + objpath);

  if (auto* h = dynamic_cast<TH1*>(obj))       return load_ref_from_TH1(h, out);
  if (auto* g = dynamic_cast<TGraph*>(obj))    return load_ref_from_TGraph(g, out);
  if (auto* e = dynamic_cast<TEfficiency*>(obj)) return load_ref_from_TEfficiency(e, out);
  if (auto* t = dynamic_cast<TTree*>(obj))     return load_ref_from_TTree(t, out, treeBranches);

  throw std::runtime_error(std::string("Unsupported object class: ") + obj->ClassName());
}
