#pragma once
#include <TF1.h>
#include <TH1.h>
#include <TEfficiency.h>
#include <TGraphAsymmErrors.h>
#include <algorithm>
#include <vector>
#include <string>
#include <stdexcept>

// ---------- Utilities ----------
inline std::vector<double> hist_x(const TH1* h){
  std::vector<double> x; x.reserve(h->GetNbinsX());
  for (int b=1;b<=h->GetNbinsX();++b) x.push_back(h->GetXaxis()->GetBinCenter(b));
  return x;
}
inline std::vector<double> hist_y(const TH1* h){
  std::vector<double> y; y.reserve(h->GetNbinsX());
  for (int b=1;b<=h->GetNbinsX();++b) y.push_back(h->GetBinContent(b));
  return y;
}
inline TGraphAsymmErrors* graph_from_TH1(const TH1* h){
  auto *g = new TGraphAsymmErrors(h->GetNbinsX());
  for (int b=1;b<=h->GetNbinsX();++b){
    const double xc = h->GetXaxis()->GetBinCenter(b);
    const double xl = h->GetXaxis()->GetBinLowEdge(b);
    const double xr = h->GetXaxis()->GetBinUpEdge(b);
    g->SetPoint(b-1, xc, h->GetBinContent(b));
    g->SetPointError(b-1, xc-xl, xr-xc, h->GetBinError(b), h->GetBinError(b));
  }
  return g;
}
inline TGraphAsymmErrors* graph_from_TEff(const TEfficiency* te){
  // central values + asymm (binomial) errors
  return te->CreateGraph();
}

// ---------- Models ----------
// M1: pure logistic (3 params): A / (1 + exp(-(x - x0)/k))
inline TF1* make_model_logistic(const char* name, double xmin, double xmax){
  auto* f = new TF1(name, "[0] / (1.0 + exp(-(x-[1])/[2]))", xmin, xmax);
  f->SetParNames("A","x0","k");
  f->SetParLimits(0, 0.0, 1.5);     // plateau cap
  f->SetParLimits(2, 1e-3, 5.0);    // slope width
  f->SetNpx(2000);
  return f;
}

// M2: logistic * low-pT turn-on (5 params):
// A * (1 - exp(-pow(x/[3],[4]))) / (1 + exp(-(x-[1])/[2]))
inline TF1* make_model_logistic_turnon(const char* name, double xmin, double xmax){
  auto* f = new TF1(name,
    "[0]*(1.0 - exp(-pow(x/[3],[4]))) / (1.0 + exp(-(x-[1])/[2]))",
    xmin, xmax);
  f->SetParNames("A","x0","k","p0","n");
  f->SetParLimits(0, 0.0, 1.5);     // plateau
  f->SetParLimits(2, 1e-3, 5.0);    // slope width
  f->SetParLimits(3, 1e-3, 1.0);    // low-pT scale
  f->SetParLimits(4, 1.0,  8.0);    // exponent
  f->SetNpx(2000);
  return f;
}

// ---------- Seeding ----------
inline void seed_logistic(TF1* f, const std::vector<double>& x, const std::vector<double>& y){
  if (x.empty()) return;
  const double Aguess = std::min(1.0, *std::max_element(y.begin(), y.end())*1.05);
  // crude x0: first x where y>0.5*A
  double x0 = x.front();
  for (size_t i=0;i<x.size();++i){ if (y[i] > 0.5*Aguess){ x0 = x[i]; break; } }
  // crude k from 10-90 width: w ≈ 4.394*k
  double p10x=x.front(), p90x=x.back();
  for (size_t i=0;i<x.size();++i){ if (y[i] > 0.1*Aguess){ p10x=x[i]; break; } }
  for (size_t i=0;i<x.size();++i){ if (y[i] > 0.9*Aguess){ p90x=x[i]; break; } }
  const double k = std::max(1e-2, (p90x - p10x)/4.394);
  f->SetParameters(Aguess, x0, k);
}
inline void seed_logistic_turnon(TF1* f, const std::vector<double>& x, const std::vector<double>& y){
  seed_logistic(f, x, y);            // sets A,x0,k
  f->SetParameter(3, 0.20);          // p0 ~ 0.2 GeV/c
  f->SetParameter(4, 2.0);           // n  ~ 2
}

// ---------- Fit wrappers ----------
enum class EffModel { Logistic, LogisticTurnOn };

inline TF1* fit_efficiency(const TH1* href_or_ratio,
                           EffModel model,
                           double xmin=-1, double xmax=-1,
                           bool verbose=false)
{
  if (!href_or_ratio) throw std::runtime_error("fit_efficiency: null hist");
  const double xlo = (xmin<0 ? href_or_ratio->GetXaxis()->GetXmin() : xmin);
  const double xhi = (xmax<0 ? href_or_ratio->GetXaxis()->GetXmax() : xmax);

  std::vector<double> x = hist_x(href_or_ratio);
  std::vector<double> y = hist_y(href_or_ratio);

  TF1* f = (model==EffModel::Logistic)
           ? make_model_logistic("eps_logistic", xlo, xhi)
           : make_model_logistic_turnon("eps_logistic_turnon", xlo, xhi);

  if (model==EffModel::Logistic)       seed_logistic(f, x, y);
  else                                 seed_logistic_turnon(f, x, y);

  auto* g = graph_from_TH1(href_or_ratio);
  g->Fit(f, verbose ? "R" : "RQ");     // quiet by default

  return f; // caller owns; f->Eval(pT) is your smooth ε(pT)
}

inline TF1* fit_efficiency(const TEfficiency* te,
                           EffModel model,
                           double xmin=-1, double xmax=-1,
                           bool verbose=false)
{
  if (!te) throw std::runtime_error("fit_efficiency: null TEfficiency");
  auto* g = te->CreateGraph(); // TGraphAsymmErrors with binomial errors
  double xlo,xhi,ylo,yhi; g->GetPoint(0,xlo,ylo); g->GetPoint(g->GetN()-1,xhi,yhi);
  if (xmin>=0) xlo=xmin; if (xmax>=0) xhi=xmax;

  TF1* f = (model==EffModel::Logistic)
           ? make_model_logistic("eps_logistic", xlo, xhi)
           : make_model_logistic_turnon("eps_logistic_turnon", xlo, xhi);

  // simple seeds from graph points
  std::vector<double> xs(g->GetN()), ys(g->GetN());
  for (int i=0;i<g->GetN();++i) g->GetPoint(i, xs[i], ys[i]);
  if (model==EffModel::Logistic)       seed_logistic(f, xs, ys);
  else                                 seed_logistic_turnon(f, xs, ys);

  g->Fit(f, verbose ? "R" : "RQ");
  return f;
}
