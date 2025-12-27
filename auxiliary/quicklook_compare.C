#include <TFile.h>
#include <TKey.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TString.h>
#include <TCanvas.h>
#include <TStyle.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

struct CentralityBin {
  std::string key;
  double nch = 0.0;
  std::map<std::string, TGraph*> species;
};

using SpeciesMap = std::map<std::string, TGraph*>;
using NchMap = std::map<std::string, CentralityBin>;
using SystemMap = std::map<std::string, NchMap>;
using KMap = std::map<double, SystemMap>;
using FileMap = std::map<std::string, KMap>;

void extend_range(TGraph* g, double& xmin, double& xmax, double& ymin, double& ymax) {
  if (!g) return;
  const int n = g->GetN();
  for (int i = 0; i < n; ++i) {
    double x = 0.0, y = 0.0;
    g->GetPoint(i, x, y);
    if (!std::isfinite(x) || !std::isfinite(y)) continue;
    xmin = std::min(xmin, x);
    xmax = std::max(xmax, x);
    ymin = std::min(ymin, y);
    ymax = std::max(ymax, y);
  }
}

void apply_graph_limits(TGraph* g, double xmin, double xmax, double ymin, double ymax, bool logy) {
  if (!g) return;
  double xpad = (xmax - xmin) * 0.03;
  if (xpad <= 0) xpad = 0.1;
  double ypad = (ymax - ymin) * 0.08;
  if (ypad <= 0) ypad = (logy ? ymin * 0.5 : 0.1);

  if (logy) {
    ymin = (ymin > 0) ? ymin * 0.7 : 1e-9;
    ymax = ymax * 1.3;
  } else {
    ymin -= ypad;
    ymax += ypad;
  }

  g->GetXaxis()->SetLimits(xmin - xpad, xmax + xpad);
  g->SetMinimum(ymin);
  g->SetMaximum(ymax);
}

std::string basename_noext(const std::string& path) {
  auto pos = path.find_last_of("/\\");
  std::string base = (pos == std::string::npos) ? path : path.substr(pos + 1);
  auto dot = base.find_last_of('.');
  return (dot == std::string::npos) ? base : base.substr(0, dot);
}

std::string extract_species(const TGraph* g) {
  if (!g) return "unknown";
  std::string name = g->GetName() ? g->GetName() : "";
  if (name.rfind("g_", 0) == 0) {
    auto pos = name.find("_pdg");
    if (pos != std::string::npos && pos > 2) {
      return name.substr(2, pos - 2);
    }
  }
  std::string title = g->GetTitle() ? g->GetTitle() : "";
  auto pos = title.find(" (");
  if (pos != std::string::npos && pos > 0) return title.substr(0, pos);
  return name.empty() ? "species" : name;
}

double parse_nch_from_key(const std::string& key) {
  std::string k = key;
  auto pos = k.find("nch_");
  if (pos != std::string::npos) k = k.substr(pos + 4);
  try { return std::stod(k); } catch (...) { return 0.0; }
}

std::string roman(int i) {
  static const char* romans[] = {"I","II","III","IV","V","VI","VII","VIII","IX","X"};
  if (i >= 1 && i <= 10) return romans[i-1];
  return std::to_string(i);
}

std::string sanitize(const std::string& in) {
  std::string out;
  out.reserve(in.size());
  for (unsigned char c : in) {
    if (std::isalnum(c)) out.push_back(c);
    else out.push_back('_');
  }
  return out;
}

std::string format_k(double k) {
  std::ostringstream ss;
  ss << std::fixed << std::setprecision(2) << k;
  std::string s = ss.str();
  std::replace(s.begin(), s.end(), '.', 'p');
  return s;
}

std::vector<std::string> list_root_files(const std::string& dir) {
  std::vector<std::string> files;
  TSystemDirectory d("out_dir", dir.c_str());
  TList* list = d.GetListOfFiles();
  if (!list) return files;
  TIter it(list);
  while (auto* f = dynamic_cast<TSystemFile*>(it())) {
    if (f->IsDirectory()) continue;
    TString name = f->GetName();
    if (!name.EndsWith(".root")) continue;
    files.push_back(dir + "/" + name.Data());
  }
  std::sort(files.begin(), files.end());
  return files;
}

void load_file(const std::string& path, FileMap& data) {
  TFile f(path.c_str(), "READ");
  if (f.IsZombie()) {
    std::cerr << "Cannot open " << path << "\n";
    return;
  }
  std::string label = basename_noext(path);

  TIter keyIter(f.GetListOfKeys());
  while (TKey* key = dynamic_cast<TKey*>(keyIter())) {
    TString kname = key->GetName();
    if (!kname.BeginsWith("spec_k")) continue;
    TString kstr = kname;
    kstr.ReplaceAll("spec_k", "");
    double kval = kstr.Atof();
    auto* dspec = dynamic_cast<TDirectory*>(f.Get(kname));
    if (!dspec) continue;

    TIter sysIter(dspec->GetListOfKeys());
    while (TKey* skey = dynamic_cast<TKey*>(sysIter())) {
      auto* dsys = dynamic_cast<TDirectory*>(dspec->Get(skey->GetName()));
      if (!dsys) continue;
      std::string system = dsys->GetName();

      TIter nchIter(dsys->GetListOfKeys());
      while (TKey* nkey = dynamic_cast<TKey*>(nchIter())) {
        auto* dnch = dynamic_cast<TDirectory*>(dsys->Get(nkey->GetName()));
        if (!dnch) continue;
        std::string nch_key = dnch->GetName();

        CentralityBin& bin = data[label][kval][system][nch_key];
        bin.key = nch_key;
        bin.nch = parse_nch_from_key(nch_key);

        TIter gIter(dnch->GetListOfKeys());
        while (TKey* gkey = dynamic_cast<TKey*>(gIter())) {
          TObject* obj = dnch->Get(gkey->GetName());
          auto* g = dynamic_cast<TGraph*>(obj);
          if (!g) continue;
          TGraph* gc = dynamic_cast<TGraph*>(g->Clone());
          if (!gc) continue;
          std::string species = extract_species(gc);
          bin.species[species] = gc;
        }
      }
    }
  }
}

std::vector<CentralityBin> sorted_bins(const NchMap& bins) {
  std::vector<CentralityBin> out;
  out.reserve(bins.size());
  for (const auto& kv : bins) out.push_back(kv.second);
  std::sort(out.begin(), out.end(), [](const CentralityBin& a, const CentralityBin& b){
    return a.nch > b.nch;
  });
  return out;
}

} // namespace

void quicklook_compare(const char* out_dir = "../blastwave/out",
                       const char* pdf_set1 = "auxiliary/quicklook_set1.pdf",
                       const char* pdf_set2 = "auxiliary/quicklook_set2.pdf",
                       const char* root_set1 = "auxiliary/quicklook_set1.root",
                       const char* root_set2 = "auxiliary/quicklook_set2.root",
                       bool logy = true)
{
  gStyle->SetOptStat(0);

  const std::string outDir(out_dir);
  auto roots = list_root_files(outDir);
  if (roots.empty()) {
    std::cerr << "No ROOT files found in " << outDir << "\n";
    return;
  }

  FileMap data;
  for (const auto& path : roots) load_file(path, data);

  // -------- Set 1: per prediction, per k, per centrality (species overlay)
  {
    TFile froot(root_set1, "RECREATE");
    TCanvas c("c_set1", "set1", 900, 700);
    if (logy) c.SetLogy();
    c.SetLeftMargin(0.12);
    c.SetRightMargin(0.05);
    c.SetBottomMargin(0.12);
    c.SetTopMargin(0.08);
    c.Print(TString::Format("%s[", pdf_set1));

    for (const auto& fkv : data) {
      const std::string& label = fkv.first;
      for (const auto& kkv : fkv.second) {
        double kval = kkv.first;
        for (const auto& skv : kkv.second) {
          const std::string& system = skv.first;
          auto bins = sorted_bins(skv.second);
          int cent_idx = 1;
          for (const auto& bin : bins) {
            c.Clear();
            if (logy) c.SetLogy();
            TLegend leg(0.62, 0.62, 0.88, 0.88);
            leg.SetBorderSize(0);
            leg.SetFillStyle(0);

            std::vector<TGraph*> graphs;
            int color = 1;
            for (const auto& sp : bin.species) {
              TGraph* g = sp.second;
              if (!g) continue;
              int col = 1 + (color % 9);
              g->SetLineColor(col);
              g->SetMarkerColor(col);
              g->SetMarkerStyle(20 + (color % 5));
              graphs.push_back(g);
              leg.AddEntry(g, sp.first.c_str(), "l");
              color++;
            }
            if (!graphs.empty()) {
              double xmin = 1e9, xmax = -1e9, ymin = 1e99, ymax = -1e99;
              for (auto* g : graphs) extend_range(g, xmin, xmax, ymin, ymax);
              graphs[0]->SetTitle(Form("%s | k=%.2f | %s | cent %s; p_{T} [GeV/c]; dN/dp_{T}",
                                       label.c_str(), kval, system.c_str(), roman(cent_idx).c_str()));
              apply_graph_limits(graphs[0], xmin, xmax, ymin, ymax, logy);
              graphs[0]->Draw("AL");
              for (size_t i = 1; i < graphs.size(); ++i) graphs[i]->Draw("L SAME");
              leg.Draw();
            }
            c.Print(pdf_set1);
            if (froot.IsOpen() && !graphs.empty()) {
              std::string cname = "set1_" + sanitize(label) + "_k" + format_k(kval)
                                  + "_" + sanitize(system) + "_cent" + roman(cent_idx);
              c.SetName(cname.c_str());
              froot.cd();
              c.Write();
            }
            cent_idx++;
          }
        }
      }
    }
    c.Print(TString::Format("%s]", pdf_set1));
    froot.Close();
  }

  // -------- Set 2: per species, per k (prediction overlay)
  {
    TFile froot(root_set2, "RECREATE");
    TCanvas c("c_set2", "set2", 900, 700);
    if (logy) c.SetLogy();
    c.SetLeftMargin(0.12);
    c.SetRightMargin(0.05);
    c.SetBottomMargin(0.12);
    c.SetTopMargin(0.08);
    c.Print(TString::Format("%s[", pdf_set2));

    // Gather all k/system/nch/species keys present across files.
    for (const auto& fkv : data) {
      (void)fkv;
    }

    // Use first file as a structural reference for systems/Nch/species keys.
    const auto& refFile = data.begin()->second;
    for (const auto& kkv : refFile) {
      double kval = kkv.first;
      for (const auto& skv : kkv.second) {
        const std::string& system = skv.first;
        auto bins = sorted_bins(skv.second);
        int cent_idx = 1;
        for (const auto& bin : bins) {
          for (const auto& sp : bin.species) {
            c.Clear();
            if (logy) c.SetLogy();
            TLegend leg(0.62, 0.62, 0.88, 0.88);
            leg.SetBorderSize(0);
            leg.SetFillStyle(0);

            std::vector<TGraph*> graphs;
            std::vector<std::string> labels;
            int color = 1;
            for (const auto& f2 : data) {
              const std::string& label = f2.first;
              auto fk = f2.second.find(kval);
              if (fk == f2.second.end()) continue;
              auto fs = fk->second.find(system);
              if (fs == fk->second.end()) continue;
              auto fn = fs->second.find(bin.key);
              if (fn == fs->second.end()) continue;
              auto fg = fn->second.species.find(sp.first);
              if (fg == fn->second.species.end()) continue;
              TGraph* g = fg->second;
              if (!g) continue;
              int col = 1 + (color % 9);
              g->SetLineColor(col);
              g->SetMarkerColor(col);
              g->SetMarkerStyle(20 + (color % 5));
              graphs.push_back(g);
              labels.push_back(label);
              color++;
            }
            if (!graphs.empty()) {
              double xmin = 1e9, xmax = -1e9, ymin = 1e99, ymax = -1e99;
              for (auto* g : graphs) extend_range(g, xmin, xmax, ymin, ymax);
              graphs[0]->SetTitle(Form("%s | k=%.2f | %s | cent %s; p_{T} [GeV/c]; dN/dp_{T}",
                                       sp.first.c_str(), kval, system.c_str(), roman(cent_idx).c_str()));
              apply_graph_limits(graphs[0], xmin, xmax, ymin, ymax, logy);
              graphs[0]->Draw("AL");
              for (size_t i = 1; i < graphs.size(); ++i) graphs[i]->Draw("L SAME");
              for (size_t i = 0; i < graphs.size(); ++i) {
                leg.AddEntry(graphs[i], labels[i].c_str(), "l");
              }
              leg.Draw();
            }
            c.Print(pdf_set2);
            if (froot.IsOpen() && !graphs.empty()) {
              std::string cname = "set2_" + sanitize(sp.first) + "_k" + format_k(kval)
                                  + "_" + sanitize(system) + "_cent" + roman(cent_idx);
              c.SetName(cname.c_str());
              froot.cd();
              c.Write();
            }
          }
          cent_idx++;
        }
      }
    }
    c.Print(TString::Format("%s]", pdf_set2));
    froot.Close();
  }
}
