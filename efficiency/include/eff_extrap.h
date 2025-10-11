// efficiency/eff_extrap.h
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Minimal dependency
#include "hadron_catalog.h"  

namespace tfhic {

// ---------- Utilities
inline double clamp01(double x) {
  if (x < 0.0) return 0.0;
  if (x > 1.0) return 1.0;
  return x;
}
inline double logit(double e) {
  const double eps = 1e-9;
  double t = std::min(std::max(e, eps), 1.0 - eps);
  return std::log(t / (1.0 - t));
}
inline double inv_logit(double g) {
  if (g >= 40) return 1.0;     // avoid overflow
  if (g <= -40) return 0.0;
  return 1.0 / (1.0 + std::exp(-g));
}

// ---------- Interpolator for reference efficiency curves
struct LogitInterp {
  // knots in pT (GeV/c), ascending; vals in (0,1)
  std::vector<double> x;
  std::vector<double> glog; // logit(y)
  bool empty() const { return x.empty(); }

  double eval(double xv) const {
    if (x.empty()) return 0.0;
    if (xv <= x.front()) return inv_logit(glog.front());
    if (xv >= x.back())  return inv_logit(glog.back());
    auto it = std::upper_bound(x.begin(), x.end(), xv);
    size_t j = size_t(it - x.begin());
    size_t i = j - 1;
    double t = (xv - x[i]) / (x[j] - x[i]);
    double g = glog[i] + t * (glog[j] - glog[i]);
    return inv_logit(g);
  }
};

// ---------- Binned reference efficiencies (no fit needed)
struct BinnedCurve {
  // bin edges in pT (GeV/c), size = nbins+1, ascending
  std::vector<double> edges;
  // values per bin, size = nbins
  std::vector<double> val;
  bool empty() const { return edges.size() < 2 || val.empty(); }
  size_t nbins() const { return val.size(); }

  // find bin index for x (half-open [edge[i], edge[i+1]))
  // returns nbins() if out of range
  size_t find_bin(double xv) const {
    if (xv < edges.front() || xv >= edges.back()) return nbins();
    auto it = std::upper_bound(edges.begin(), edges.end(), xv);
    size_t j = size_t(it - edges.begin());
    return j - 1;
  }
};

// ---------- Extrapolator
struct ExtrapConfig {
  // default weights by family: baryon->p, strange meson->K, other meson->pi
  // You can override per hadron if desired.
  std::array<double,3> w_baryon   {0.0, 0.0, 1.0}; // (pi,K,p)
  std::array<double,3> w_smeson   {0.0, 1.0, 0.0};
  std::array<double,3> w_meson    {1.0, 0.0, 0.0};
  bool use_beta_gamma = true; // if false, identity (no mapping)
  // numeric integration grid for folding in bins
  int fold_grid = 400;

  // correction for stable particles when extrapolating from kaons -> ε / P_survival
  bool correct_kaon_decay;
  double L_m;              // effective path length (m), radius to TOF outer border
  double cTauK_m = 3.713;        // K± cτ in meters

  ExtrapConfig()
  {
    L_m = 3.99; // m
    correct_kaon_decay = false;
  }
  ExtrapConfig(bool kaon_correct, double L)
  {
    correct_kaon_decay = kaon_correct;
    L_m = L;
  }
};

static inline double survival(double pt, double m, double L, double cTau){
  double bg = (pt>0? pt/m : 0.0);                 // mid-rapidity approx
  if (bg<=0) return 0.0;
  return std::exp(-L / (bg * cTau));
}


class EffExtrapolator {
public:
  EffExtrapolator(const HadronCatalog& catalog,
                  LogitInterp pi, LogitInterp K, LogitInterp p,
                  ExtrapConfig cfg = {})
    : cat_(catalog), pi_(std::move(pi)), K_(std::move(K)), p_(std::move(p)), cfg_(cfg)
  {}

  EffExtrapolator(const HadronCatalog& catalog,
                  BinnedCurve pi_b, BinnedCurve K_b, BinnedCurve p_b,
                  ExtrapConfig cfg = {})
    : cat_(catalog), pi_bins_(std::move(pi_b)), K_bins_(std::move(K_b)), p_bins_(std::move(p_b)), cfg_(cfg)
  {}

  // Pointwise epsilon_h(pT) using interpolants (if provided)
  double epsilon_pointwise(int pdg, double pt) const {
    const auto* d = cat_.get(pdg);
    if (!d) return 0.0;
    auto w = weights_for(*d);
    if (std::fabs(pt) < 1e-12) return 0.0;
    const double mh = d->mass_GeV;
    auto* pi_def = cat_.get(211);
    auto* k_def  = cat_.get(321);
    auto* p_def  = cat_.get(2212);
    const double m_pi = pi_def ? pi_def->mass_GeV : 0.13957039;
    const double m_K  = k_def  ? k_def->mass_GeV  : 0.49367;
    const double m_p  = p_def  ? p_def->mass_GeV  : 0.93827208816;

    double e = 0.0;

    if (!pi_.empty()) e += w[0] * pi_.eval(map_pt(pt, m_pi, mh));

    if (!K_.empty()){
      double ptK = map_pt(pt, m_K, mh);
      double eK  = K_.eval(ptK);
      if (cfg_.correct_kaon_decay) {
        double Psurv = survival(ptK, m_K, cfg_.L_m, cfg_.cTauK_m);
        if (Psurv > 0) eK = std::min(1.0, eK / Psurv);
        else           eK = 0.0;
      }
      e += w[1] * eK;
    }

    if (!p_.empty())  e += w[2] * p_.eval (map_pt(pt, m_p,  mh));

    return clamp01(e);
  }

  // Bin-averaged epsilon using BW weight S(pt) (callable), via interpolants
  double epsilon_bin_weighted(int pdg, double pt_min, double pt_max,
                              const std::function<double(double)>& S) const {
    const int N = std::max(cfg_.fold_grid, 50);
    double num=0.0, den=0.0;
    for (int i=0;i<N;++i){
      double t0 = double(i)/N, t1 = double(i+1)/N;
      double x0 = pt_min + (pt_max-pt_min)*t0;
      double x1 = pt_min + (pt_max-pt_min)*t1;
      double xm = 0.5*(x0+x1);
      double Sm = S(xm);
      double em = epsilon_pointwise(pdg, xm);
      num += em * Sm * (x1-x0);
      den += Sm * (x1-x0);
    }
    return (den>0.0) ? clamp01(num/den) : 0.0;
  }

  // Fit-free discrete mixture using binned refs
  double epsilon_bin_mixture(int pdg, double pt_min, double pt_max,
                             const std::function<double(double)>& S) const {
    const auto* d = cat_.get(pdg);
    if (!d) return 0.0;
    auto w = weights_for(*d);
    const double mh = d->mass_GeV;
    auto* pi_def = cat_.get(211);
    auto* k_def  = cat_.get(321);
    auto* p_def  = cat_.get(2212);
    const double m_pi = pi_def ? pi_def->mass_GeV : 0.13957039;
    const double m_K  = k_def  ? k_def->mass_GeV  : 0.49367;
    const double m_p  = p_def  ? p_def->mass_GeV  : 0.93827208816;

    double num=0.0, den=0.0;
    const int N = std::max(cfg_.fold_grid, 200);
    for (int k=0;k<N;++k){
      double t0 = double(k)/N, t1 = double(k+1)/N;
      double x0 = pt_min + (pt_max-pt_min)*t0;
      double x1 = pt_min + (pt_max-pt_min)*t1;
      double xm = 0.5*(x0+x1);
      double Sm = S(xm);
      // Map to each reference and pick its bin's value
      double e_pi = (!pi_bins_.empty()) ? bin_value(pi_bins_, map_pt(xm, m_pi, mh)) : 0.0;
      double e_K = 0.0;
      if (!K_bins_.empty()){
        double ptK = map_pt(xm, m_K, mh);
        e_K = bin_value(K_bins_, ptK);
        if (cfg_.correct_kaon_decay) {
          double Psurv = survival(ptK, m_K, cfg_.L_m, cfg_.cTauK_m);
          if (Psurv > 0) e_K = std::min(1.0, e_K / Psurv);
          else           e_K = 0.0;
        }
      }
      double e_p  = (!p_bins_.empty())  ? bin_value(p_bins_,  map_pt(xm, m_p,  mh)) : 0.0;
      double e = w[0]*e_pi + w[1]*e_K + w[2]*e_p;
      num += e * Sm * (x1-x0);
      den += Sm * (x1-x0);
    }
    return (den>0.0) ? clamp01(num/den) : 0.0;
  }

private:
  const HadronCatalog& cat_;
  LogitInterp pi_, K_, p_;           // either these...
  BinnedCurve pi_bins_, K_bins_, p_bins_; // ...or these
  ExtrapConfig cfg_;

  //static double mass_pi() { return 0.13957039; }
  //static double mass_K()  { return 0.49367; }
  //static double mass_p()  { return 0.93827208816; }

  static double map_pt(double pt, double m_ref, double m_h) {
    return pt * (m_ref / m_h); // βγ mapping
  }

  static double bin_value(const BinnedCurve& bc, double x){
    size_t b = bc.find_bin(x);
    //if (b >= bc.nbins()) {
    //  // out of range: clamp to nearest edge bin if within coverage; else 0
    //  if (x < bc.edges.front()) return bc.val.front();
    //  if (x >= bc.edges.back()) return bc.val.back();
    //  return 0.0;
    //}
    if (b >= bc.nbins()) {
      if (x < bc.edges.front()) return bc.val.front();
      if (x >= bc.edges.back()) return bc.val.back();
      return 0.0;
    }
    
    return bc.val[b];
  }

  std::array<double,3> weights_for(const HadronDef& d) const {
    // Simple token-based mapping; extend as needed
    const std::string& t = d.token;
    if      (t=="P" || t=="LAMBDA" || t=="XI" || t=="OMEGA") return cfg_.w_baryon;
    else if (t=="K" || t=="KS" || t=="PHI")                  return cfg_.w_smeson;
    else                                                     return cfg_.w_meson; // π-like
  }
};

} // namespace tfhic
