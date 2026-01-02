#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphAsymmErrors.h>
#include <TLegend.h>
#include <TMath.h>
#include <TSystem.h>
#include <TString.h>

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../../blastwave/include/blastwave_utils.h"
#include "../../blastwave/include/io_utils.h"
#include "../../common/hadron_catalog.h"

namespace fs = std::filesystem;

static fs::path repo_root_from_file() {
  fs::path cur = fs::path(__FILE__).parent_path();
  for (int i = 0; i < 6; ++i) {
    if (fs::exists(cur / "common" / "data" / "hadrons.json")) return cur;
    if (fs::exists(cur / "CMakeLists.txt")) return cur;
    if (!cur.has_parent_path()) break;
    cur = cur.parent_path();
  }
  return fs::path(__FILE__).parent_path();
}

static bool load_tfhic_library(const fs::path& repo_root) {
  const std::vector<fs::path> candidates = {
    repo_root / "build/lib/libTFHIC.so",
    repo_root / "../build/lib/libTFHIC.so",
    "/opt/tfhic/install/lib/libTFHIC.so",
    repo_root / "build/lib/libTFHIC.dylib",
    "libTFHIC.so",

  };
  for (const auto& lib : candidates) {
    if (lib.is_absolute() && !fs::exists(lib))
    {
      std::cout << "looking for libTFHIC.so in " << lib.c_str() << "\n"; 
      continue;
    } 
    if (gSystem->Load(lib.string().c_str()) >= 0)
    {
      std::cout << "found!" << "\n";
      return true;
    } 
  }
  return false;
}

static std::vector<double> read_yield_hist(TDirectory* dir, const char* name) {
  std::vector<double> out;
  if (!dir) return out;
  auto* h = dynamic_cast<TH1*>(dir->Get(name));
  if (!h) return out;
  for (int i = 1; i <= h->GetNbinsX(); ++i) out.push_back(h->GetBinContent(i));
  return out;
}

void compareHepData_asTGraphs() {
  const fs::path repo_root = repo_root_from_file();
  if (!load_tfhic_library(repo_root)) {
    std::cerr << "ERROR: could not load libTFHIC.so (build or install it first).\n";
    return;
  }

  const fs::path data_dir = fs::path(__FILE__).parent_path();
  const fs::path hep_path = data_dir / "HEPData-ins1222333-v1-root.root";
  const fs::path bw_csv = repo_root / "blastwave" / "data" / "bw_data_1303.0737.csv";
  const fs::path hadrons_json = repo_root / "common" / "data" / "hadrons.json";
  const fs::path out_dir = data_dir / "out";
  fs::create_directories(out_dir);

  std::vector<std::string> particles = {"pi", "K", "p"};
  std::vector<int> pdgs = {211, 321, 2212};
  std::vector<int> graph_offsets = {0, 10, 20}; // TGraph indices in HEP file

  TFile* hepFile = TFile::Open(hep_path.string().c_str(), "READ");
  if (!hepFile || hepFile->IsZombie()) {
    std::cerr << "ERROR: cannot open HEPData file: " << hep_path << "\n";
    return;
  }

  HadronCatalog catalog;
  std::string err;
  if (!catalog.load(hadrons_json.string(), &err)) {
    std::cerr << "ERROR: cannot load hadron catalog: " << err << "\n";
    return;
  }

  auto integration_infos = get_integration_info_from_csv(bw_csv.string(), catalog, pdgs, false);
  if (integration_infos.empty()) {
    std::cerr << "ERROR: cannot load blast-wave CSV: " << bw_csv << "\n";
    return;
  }

  TDirectory* yields_dir = dynamic_cast<TDirectory*>(hepFile->Get("Table 31"));
  if (!yields_dir) {
    std::cerr << "ERROR: cannot find Table 31 in HEPData file.\n";
    return;
  }

  std::vector<std::vector<double>> yields;
  yields.resize(3);
  yields[0] = read_yield_hist(yields_dir, "Hist1D_y2");
  yields[1] = read_yield_hist(yields_dir, "Hist1D_y4");
  yields[2] = read_yield_hist(yields_dir, "Hist1D_y6");

  for (size_t i = 0; i < yields.size(); ++i) {
    if (yields[i].empty()) {
      std::cerr << "ERROR: missing yields for species index " << i << "\n";
      return;
    }
  }

  const int ncent = std::min<int>(integration_infos.size(), yields[0].size());

  std::string out_filename = (out_dir / "compare_hepdata_blastwave.root").string();
  TFile* outFile = TFile::Open(out_filename.c_str(), "RECREATE");
  if (!outFile || outFile->IsZombie()) {
    std::cerr << "ERROR: cannot create output file: " << out_filename << "\n";
    return;
  }

  for (int cent = 0; cent < ncent; cent++) {
    if (integration_infos[cent].size() < pdgs.size()) {
      std::cerr << "ERROR: missing blast-wave params for centrality index " << cent << "\n";
      continue;
    }
    for (int i = 0; i < 3; ++i) { // pi, K, p
      std::string pname = particles[i];

      int graph_idx = graph_offsets[i] + cent;
      std::string tableName = Form("Table %d", graph_idx + 1); // Tables start from 1

      TDirectory* dir = dynamic_cast<TDirectory*>(hepFile->Get(tableName.c_str()));
      if (!dir) {
        std::cerr << "ERROR: dir not found for " << tableName << "\n";
        return;
      }

      TGraphAsymmErrors* gr = dynamic_cast<TGraphAsymmErrors*>(dir->Get("Graph1D_y1"));
      if (!gr) {
        std::cerr << "ERROR: graph not found in " << tableName << "\n";
        return;
      }

      HadronIntegrationInfo info = integration_infos[cent][i];
      info.yield = yields[i][cent];
      const double pt_min = gr->GetX()[0];
      const double pt_max = gr->GetX()[gr->GetN() - 1];
      TGraph* grModel = computePtSpectrum_tGraph(info, pt_min, pt_max, true, false, 0., 1000);
      // timesPt=true -> pT dN/dpT; rescale to (1/2pi pT) dN/dpT

      for (int j = 0; j < grModel->GetN(); j++) {
        double pT = grModel->GetX()[j];
        double y = grModel->GetY()[j];
        grModel->SetPoint(j, pT, pT > 0.0 ? y / (2 * TMath::Pi() * pT) : 0.0);
      }

      std::string compName = Form("Comparison_cent%d_%s", cent, pname.c_str());
      TCanvas* cComp = new TCanvas(compName.c_str(), compName.c_str(), 600, 500);

      gr->SetMarkerStyle(20);
      gr->SetMarkerColor(kRed);
      gr->SetLineColor(kRed);

      grModel->SetLineColor(kBlue);
      grModel->SetLineWidth(2);

      gr->Draw("AP");
      grModel->Draw("L SAME");

      auto leg = new TLegend(0.55, 0.75, 0.88, 0.88);
      leg->AddEntry(gr, "Experiment", "p");
      leg->AddEntry(grModel, "Model", "l");
      leg->Draw();

      cComp->Write();
      delete cComp;
    }
  }

  outFile->Close();
  delete outFile;

  hepFile->Close();
  delete hepFile;
}
