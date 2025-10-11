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
#include <cmath>

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
  std::array<double,3> w_baryon   {0.0, 0.0, 1.0}; // (pi,K,p)
  std::array<double,3> w_smeson   {0.0, 1.0, 0.0};
  std::array<double,3> w_meson    {1.0, 0.0, 0.0};
  bool use_beta_gamma = true; // if false, identity (no mapping)

  // numeric integration grid for folding in bins
  int fold_grid = 400;

  // ---- γ-mapping exponents (category-level) ----
  double gamma_baryon = 1.0;
  double gamma_smeson = 1.0;
  double gamma_meson  = 1.0; // π-like

  // ---- family plateau caps (learned from π/K/p or set by CLI) ----
  bool   cap_plateau = true;
  double cap_baryon  = 1.0;  // use proton plateau
  double cap_smeson  = 1.0;  // use kaon plateau
  double cap_meson   = 1.0;  // use pion plateau

  // ---- Survival handling (generic, ref→target) ----
  // If true, automatically undo ref decays (divide by P_surv(ref)) and apply target survival (multiply by P_surv(target))
  bool   survival_auto = true;
  double L_m           = 4.0;     // effective path length to TOF [m]

  // legacy kaon-only knobs kept for backward-compat (ignored if survival_auto=true)
  bool   correct_kaon_decay = false;
  double cTauK_m = 3.713;   // K± cτ [m]

  // behavior of binned lookup outside support
  bool   zero_outside = true;

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

static inline double survival_prob(double pt, double mGeV, double L_m, double ctau_m) {
  if (ctau_m <= 0.0) return 1.0;              // stable
  if (pt <= 0.0)     return 0.0;
  const double bg = pt / mGeV;                // mid-rapidity approx
  return std::exp(- L_m / (bg * ctau_m));
}

// category gamma/cap pickers
static inline double gamma_for_cat(const std::array<double,3>& w, const ExtrapConfig& cfg){
  if (w[2] > w[1] && w[2] > w[0]) return cfg.gamma_baryon;
  if (w[1] > w[0] && w[1] > w[2]) return cfg.gamma_smeson;
  return cfg.gamma_meson;
}
static inline double cap_for_cat(const std::array<double,3>& w, const ExtrapConfig& cfg){
  if (w[2] > w[1] && w[2] > w[0]) return cfg.cap_baryon;
  if (w[1] > w[0] && w[1] > w[2]) return cfg.cap_smeson;
  return cfg.cap_meson;
}

// γ-aware pT mapping: pt_ref = pt_target * (m_ref/m_target)^γ
static inline double map_pt_gamma(double pt, double m_ref, double m_tar, double gamma_exp){
  return pt * std::pow(m_ref / m_tar, gamma_exp);
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
    if (!d || std::fabs(pt) < 1e-12) return 0.0;
  
    // masses (fallbacks in case undefined in catalog)
    auto* pi_def = cat_.get(211);  auto* k_def  = cat_.get(321);  auto* p_def  = cat_.get(2212);
    const double m_pi = pi_def ? pi_def->mass_GeV : 0.13957039;
    const double m_K  = k_def  ? k_def->mass_GeV  : 0.49367;
    const double m_p  = p_def  ? p_def->mass_GeV  : 0.93827208816;
    const double mh   = d->mass_GeV;
  
    const auto w = weights_for(*d);
    const double gamma_exp = gamma_for_cat(w, cfg_);
    const double cap   = cap_for_cat(w, cfg_);
  
    // 1) build "base" detector efficiency from refs (undo ref decays if enabled)
    double e_base = 0.0;

    if (!pi_.empty()){
      const double pt_pi = map_pt_gamma(pt, m_pi, mh, gamma_exp);
      double e_pi = pi_.eval(pt_pi);
      if (cfg_.survival_auto) {
        const double Ppi = survival_prob(pt_pi, m_pi, cfg_.L_m, ctau_m_from_catalog(211));
        if (Ppi > 0) e_pi = std::min(1.0, e_pi / Ppi); else e_pi = 0.0;
      }
      e_base += w[0] * e_pi;
    }
  
    if (!K_.empty()){
      const double pt_K = map_pt_gamma(pt, m_K, mh, gamma_exp);
      double e_K = K_.eval(pt_K);
      if (cfg_.survival_auto || cfg_.correct_kaon_decay){
        const double PK = survival_prob(pt_K, m_K, cfg_.L_m, ctau_m_from_catalog(321));
        if (PK > 0) e_K = std::min(1.0, e_K / PK); else e_K = 0.0;
      }
      e_base += w[1] * e_K;
    }
  
    if (!p_.empty()){
      const double pt_p = map_pt_gamma(pt, m_p, mh, gamma_exp);
      double e_p = p_.eval(pt_p); // proton is stable (no undo)
      e_base += w[2] * e_p;
    }

    // 2) cap by family plateau (detector-side), then apply target survival if any
    if (cfg_.cap_plateau) e_base = std::min(e_base, cap);

    double e = e_base;
    if (cfg_.survival_auto){
      const double Ph = survival_prob(pt, mh, cfg_.L_m, ctau_m_from_catalog(pdg));
      e *= Ph; // target survival
    }

    // clip to [0,1]
    if (e < 0.0) e = 0.0; else if (e > 1.0) e = 1.0;
    return e;
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
    const auto w = weights_for(*d);
    const double gamma_exp = gamma_for_cat(w, cfg_);
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
      const double pt_pi = map_pt_gamma(xm, m_pi, mh, gamma_exp);
      double e_pi = (!pi_bins_.empty()) ? bin_value(pi_bins_, pt_pi) : 0.0;
      if (cfg_.survival_auto){
        const double Ppi = survival_prob(pt_pi, m_pi, cfg_.L_m, ctau_m_from_catalog(211));
        if (Ppi > 0) e_pi = std::min(1.0, e_pi / Ppi); else e_pi = 0.0;
      }
      
      const double pt_K = map_pt_gamma(xm, m_K, mh, gamma_exp);
      double e_K = (!K_bins_.empty()) ? bin_value(K_bins_, pt_K) : 0.0;
      if (cfg_.survival_auto || cfg_.correct_kaon_decay){
        const double PK = survival_prob(pt_K, m_K, cfg_.L_m, ctau_m_from_catalog(321));
        if (PK > 0) e_K = std::min(1.0, e_K / PK); else e_K = 0.0;
      }
      
      const double pt_p = map_pt_gamma(xm, m_p, mh, gamma_exp);
      double e_p = (!p_bins_.empty()) ? bin_value(p_bins_, pt_p) : 0.0;
      
      double e_base = w[0]*e_pi + w[1]*e_K + w[2]*e_p;
      if (cfg_.cap_plateau) e_base = std::min(e_base, cap_for_cat(w, cfg_));
      
      double e = e_base;
      if (cfg_.survival_auto){
        const double Ph = survival_prob(xm, mh, cfg_.L_m, ctau_m_from_catalog(pdg));
        e *= Ph;
      }
      num += e * Sm * (x1-x0);
      den += Sm * (x1-x0);
    }
    return (den>0.0) ? clamp01(num/den) : 0.0;
  }

private:
  const HadronCatalog& cat_;
  LogitInterp pi_, K_, p_;
  BinnedCurve pi_bins_, K_bins_, p_bins_;
  ExtrapConfig cfg_;

  //static double mass_pi() { return 0.13957039; }
  //static double mass_K()  { return 0.49367; }
  //static double mass_p()  { return 0.93827208816; }

  static double map_pt(double pt, double m_ref, double m_h) {
    return pt * (m_ref / m_h); // βγ mapping
  }

  double bin_value(const BinnedCurve& bc, double x) const{
    size_t b = bc.find_bin(x);
    if (b >= bc.nbins()) {
      return cfg_.zero_outside ? 0.0 : (x < bc.edges.front() ? bc.val.front() : bc.val.back());
    }
    return bc.val[b];
  }

    // get PDG lifetimes*c (cτ in meters)
  double ctau_m_from_catalog(int pdg) const {
    if (const auto* d = cat_.get(pdg)) return d->ctau_m;
    return 0.0; // treat missing as stable
  }

  std::array<double,3> weights_for(const HadronDef& d) const {
    // Simple token-based mapping; extend as needed
    // may simplify with "BARYON" - "MESON" or "STABLE" - "UNSTABLE"
    const std::string& t = d.token;
    if      (t=="P" || t=="LAMBDA" || t=="XI" || t=="OMEGA") return cfg_.w_baryon;
    else if (t=="K" || t=="KS" || t=="PHI")                  return cfg_.w_smeson;
    else                                                     return cfg_.w_meson; // π-like
  }
};

} // namespace tfhic
