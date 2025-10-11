// include/bw_adapter.h
#pragma once
#include <functional>
#include <memory>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <TFile.h>
#include <TH1.h>
#include <TGraph.h>

// ---------- TH1 ----------
inline std::function<double(double)>
makeSpectrumFromTH1(TH1* hin, bool clamp=true) {
  if (!hin) throw std::runtime_error("makeSpectrumFromTH1: null TH1*");
  // Clone and manage with shared_ptr so the lambda is copyable
  std::shared_ptr<TH1> h(static_cast<TH1*>(hin->Clone()));
  h->SetDirectory(nullptr);
  const double xmin = h->GetXaxis()->GetXmin();
  const double xmax = h->GetXaxis()->GetXmax();

  return [h, xmin, xmax, clamp](double pt) -> double {
    if (clamp) {
      if (pt <= xmin) return h->GetBinContent(1);
      if (pt >= xmax) return h->GetBinContent(h->GetNbinsX());
    } else {
      if (pt < xmin || pt > xmax) return 0.0;
    }
    double y = h->Interpolate(pt);
    return (y > 0 ? y : 0.0);
  };
}

inline std::function<double(double)>
makeSpectrumFromFile_TH1(const std::string& file, const std::string& objpath) {
  TFile f(file.c_str(), "READ");
  if (f.IsZombie()) throw std::runtime_error("Cannot open ROOT file: " + file);
  TH1* h = nullptr;
  f.GetObject(objpath.c_str(), h);
  if (!h) throw std::runtime_error("TH1 not found at: " + objpath);
  auto S = makeSpectrumFromTH1(h);
  return S; // cloned & owned via shared_ptr; file can close
}

// ---------- TGraph ----------
inline std::function<double(double)>
makeSpectrumFromTGraph(TGraph* gin, bool spline=true, bool clamp=true) {
  if (!gin) throw std::runtime_error("makeSpectrumFromTGraph: null TGraph*");
  std::shared_ptr<TGraph> g(static_cast<TGraph*>(gin->Clone()));
  double xmin, ymin, xmax, ymax; g->ComputeRange(xmin, ymin, xmax, ymax);
  const std::string opt = spline ? "S" : "L"; // spline or linear

  return [g, xmin, xmax, opt, clamp](double pt) -> double {
    if (clamp) {
      if (pt <= xmin) return g->Eval(xmin, nullptr, opt.c_str());
      if (pt >= xmax) return g->Eval(xmax, nullptr, opt.c_str());
    } else {
      if (pt < xmin || pt > xmax) return 0.0;
    }
    double y = g->Eval(pt, nullptr, opt.c_str());
    return (y > 0 ? y : 0.0);
  };
}

inline std::function<double(double)>
makeSpectrumFromFile_TGraph(const std::string& file, const std::string& objpath) {
  TFile f(file.c_str(), "READ");
  if (f.IsZombie()) throw std::runtime_error("Cannot open ROOT file: " + file);
  TGraph* g = nullptr;
  f.GetObject(objpath.c_str(), g);
  if (!g) throw std::runtime_error("TGraph not found at: " + objpath);
  auto S = makeSpectrumFromTGraph(g);
  return S; // cloned & owned via shared_ptr; file can close
}