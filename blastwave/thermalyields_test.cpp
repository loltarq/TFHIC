#include "include/io_utils.h"
#include "include/blastwave_utils.h"
#include "include/io_thermal.h"

//\usepackage[numbers]{natbib}
//\bibliographystyle{plain}
//\bibliography{bibliografia.bib}

void compareHepData_asTGraphs()
{
  std::vector<std::string> particles = {"pi^{+}", "K^{+}", "p"};
  std::vector<int> graph_offsets = {0, 10, 20}; // TGraph indices in HEP file
  std::vector<std::string> ensembles = {"CE"};
  std::vector<double> k_factors = {1.6, 3., 6.};
  std::vector<string> k_string = {"1.6","3","6"};

  TFile *hepFile = new TFile("data/HEPData-ins1222333-v1-root.root");
  TFile *thermalyields = new TFile("out/spectra_yields_CE_k1.6_k3_k6_gs_1303.0737_tgraph.root");


  for (int j = 0; j < 3; j++)
  {
    double k_factor = k_factors[j];

    std::string out_filename = Form("out/compare_%s_gs_k%s_graph.root", ensembles[0].c_str(), k_string[j].c_str());
    TFile *outFile = new TFile(out_filename.c_str(), "RECREATE");

    for (int cent = 0; cent < 10; cent++)
    {
      for (int i = 0; i < 3; ++i) // for pi, K, p
      {

        std::string pname = particles[i];

        int graph_idx = graph_offsets[i] + cent;
        std::string tableName = Form("Table %d", graph_idx + 1); // Tables start from 1

        TDirectory* dir = (TDirectory*) hepFile->Get(tableName.c_str());
        if (!dir)
        {
          cout << "\nERROR: dir not found\n";
          return;
        }

        TGraphAsymmErrors* gr = (TGraphAsymmErrors*)dir->Get("Graph1D_y1");
        if (!gr) 
        {
          cout << "\nERROR: graph not found\n";
          return;
        }

        int pdg = 0;
        if (pname == "p")
          pdg = 2212;
        else if (pname == "K^{+}")
          pdg = 321;
        else
          pdg = 211;

        std::string grmodelpath = Form("k_%s/cent_%i/h_%s_pdg%i", k_string[j].c_str(), cent, pname.c_str(), pdg); 

        std::cerr << "grModel path: " << grmodelpath << "\n";
        TGraph* grModel = (TGraph*)thermalyields->Get(grmodelpath.c_str());
        if (!grModel)
        {
          std::cerr << "grModel not found\n";
        }
        //true->computing pt*dn/dpt !!! necessary for correct normalization to exp yield!

        //for (int i = 0; i < grModel->GetN(); i++)
        //{
        //  double pT = grModel->GetX()[i];
        //  double y = grModel->GetY()[i];

        //  grModel->SetPoint(i, pT, y/(2*TMath::Pi())); //rescale model by 1/2pipt
        //}

        //rescale model by 1/2pi
        grModel->Scale(1./(TMath::Pi()*2.));

        // Create new canvas for comparison
        std::string compName = Form("Comparison_%s_gs_k%s_cent%d_%s", 
                                    ensembles[0].c_str(), k_string[j].c_str(), cent, pname.c_str());
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
  }

  hepFile->Close();
  delete hepFile;
}
