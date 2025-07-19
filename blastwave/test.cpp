#include "include/io_utils.h"
#include "include/blastwave_utils.h"

//\usepackage[numbers]{natbib}
//\bibliographystyle{plain}
//\bibliography{bibliografia.bib}

//note: get_integration_info returns a vector of size 10, 1 index for each centrality class. 
//computePtSpectrum_tgraphs(info,pTmin = 0.,pTmax = 5.0,timesPt = false,clampR = false, rmax=0, graphpoints = 1000)
//computePtSpectrum_tgraphs(info,pTmin = 0.,pTmax = 5.0,timesPt = false,clampR = false, rmax=0, nbins = 200)


void histo()
{
  std::vector<string> ensembles = {"GCE", "CE", "SCE"};
  std::vector<double> k_factors = {1, 1.6, 3};

  auto integration_infos = get_integration_info();

  //compute with all thermal model scans, on all centralities
  for (const auto& ensemble : ensembles)
  {

    std::cout << "\nUsing ensemble: " << ensemble << "\n";

    for (const auto& k_factor : k_factors)
    {
      std::cout << "k factor: " << k_factor << "\n";

      std::ostringstream k_str;
      k_str << std::fixed << std::setprecision(1) << k_factor;

      // 📌 Write these canvas to its own ROOT file
      std::ostringstream fname;
      fname << "out/" << ensemble << "_k" << k_str.str() << ".root";

      TFile *outfile = new TFile(fname.str().c_str(), "RECREATE");

      for (int centrality = 0; centrality < 10; centrality++)
      {
        std::cout << "Integrating for centrality class: " << centrality << "\n";

        std::ostringstream cname;
        cname << ensemble << "_k" << k_str.str() << "_centr" << std::to_string(centrality);

        TCanvas *c = new TCanvas(cname.str().c_str(), cname.str().c_str(), 1200, 400);
        c->Divide(3,1);

        int j = 0;
        for (auto item : integration_infos[centrality])
        {
          auto hPt = computePtSpectrum(item, 0.001, 150., true, false, 0, 10000);

          //std::string hname = item.hadron.name + std::to_string(centrality);
          hPt->SetName(Form("%s_%s", item.hadron.name.c_str(), std::to_string(centrality).c_str()));
          hPt->SetLineColor(item.hadron.color);
          hPt->SetLineWidth(2);

          c->cd(j+1);
          j++;
          TString drawOpt = "HIST";
          hPt->Draw(drawOpt);
          
          outfile->cd();
          hPt->Write();
        }

        outfile->cd();
        c->Write();
        delete c;


      }

      outfile->Close();
      delete outfile;

      std::cout << "All canvases saved in " << fname.str() << std::endl;
    }
  }
}

void compareHepData()
{
  std::vector<std::string> particles = {"pi", "K", "p"};
  std::vector<int> graph_offsets = {0, 10, 20}; // TGraph indices in HEP file
  std::vector<std::string> ensembles = {"GCE", "CE", "SCE"};
  std::vector<double> k_factors = {1, 1.6, 3};

  //std::vector<double> k_factors = {1.6};
  //std::vector<std::string> ensembles = {"CE"};

  TFile *hepFile = new TFile("data/HEPData-ins1222333-v1-root.root");
  auto integration_infos = get_integration_info();

  for (const auto& ensemble : ensembles)
  {
    for (const auto& k_factor : k_factors)
    {
      std::ostringstream k_str;
      k_str << std::fixed << std::setprecision(1) << k_factor;

      std::ostringstream fname;
      fname << "out/" << ensemble << "_k" << k_str.str() << ".root";

      TFile *modelFile = new TFile(fname.str().c_str());

      if (!modelFile)
      {
        std::cout << "\nmodel file not found \n";
        return;
      }

      std::string out_filename = Form("out/compare_%s_k%s.root", ensemble.c_str(), k_str.str().c_str());
      TFile *outFile = new TFile(out_filename.c_str(), "RECREATE");

      for (int cent = 0; cent < 10; cent++)
      {
        TCanvas* c_model = (TCanvas*) modelFile->Get(Form("%s_k%s_centr%s", ensemble.c_str(), k_str.str().c_str(), std::to_string(cent).c_str()));
        if (!c_model) continue;
        
        TH1D* h[3] = {0, 0, 0};

        for (int i = 0; i < 3; ++i) // for pi, K, p
        {

          std::string pname = particles[i];

          int graph_idx = graph_offsets[i] + cent;
          std::string tableName = Form("Table %d", graph_idx + 1); // Tables start from 1

          h[i] = (TH1D*)modelFile->Get(Form("%s_%s", integration_infos[cent][i].hadron.name.c_str(), std::to_string(cent).c_str()));

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

          // Extract the model histogram
          TPad* pad = (TPad*)c_model->GetPad(i+1);
          if (!pad) {
              std::cout << "Pad " << i+1 << " not found!\n";
              continue;
          }

          pad->cd();

          // Create new canvas for comparison
          std::string compName = Form("Comparison_%s_k%s_cent%d_%s", 
                                     ensemble.c_str(), k_str.str().c_str(), cent, pname.c_str());
          TCanvas* cComp = new TCanvas(compName.c_str(), compName.c_str(), 600, 500);

          // Convert TH1D to same units as HEP data
          auto hClone = (TH1D*) h[i]->Clone();
          hClone->Draw();
          for (int b = 1; b <= hClone->GetNbinsX(); ++b)
          {
            double pT = hClone->GetBinCenter(b);
            double content = hClone->GetBinContent(b);
            hClone->SetBinContent(b, content / (2 * TMath::Pi() * pT));
          }

          hClone->SetLineColor(kMagenta);
          hClone->Draw("HIST");
          //h[i]->SetLineColor(kBlue);
          //h[i]->Draw("HIST");

          gr->SetMarkerStyle(20);
          gr->SetMarkerColor(kRed);
          gr->Draw("P SAME");

          auto legend = new TLegend(0.55, 0.75, 0.88, 0.88);
          legend->AddEntry(hClone, "Model", "l");
          //legend->AddEntry(h[i], "Model", "l");
          legend->AddEntry(gr, "HEPData", "p");
          legend->Draw();

          cComp->Write();
          delete cComp;
        }
      }

      modelFile->Close();
      delete modelFile;

      outFile->Close();
      delete outFile;
    }
  }

  hepFile->Close();
  delete hepFile;
}

void compareHepData_asTGraphs()
{
  std::vector<std::string> particles = {"pi", "K", "p"};
  std::vector<int> graph_offsets = {0, 10, 20}; // TGraph indices in HEP file
  std::vector<std::string> ensembles = {"GCE", "CE", "SCE"};
  std::vector<double> k_factors = {1, 1.6, 3};

  //std::vector<double> k_factors = {1.6};
  //std::vector<std::string> ensembles = {"CE"};

  TFile *hepFile = new TFile("data/HEPData-ins1222333-v1-root.root");
  auto integration_infos = get_integration_info();

  for (const auto& ensemble : ensembles)
  {
    for (const auto& k_factor : k_factors)
    {
      std::ostringstream k_str;
      k_str << std::fixed << std::setprecision(1) << k_factor;

      std::string out_filename = Form("out/compare_%s_k%s_graph.root", ensemble.c_str(), k_str.str().c_str());
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

          TGraph* grModel = computePtSpectrum_tGraph(integration_infos[cent][i], 0.001, 150., true, false, 0., 10000); 
          //true->computing pt*dn/dpt !!! necessary for correct normalization to exp yield!

          for (int i = 0; i < grModel->GetN(); i++)
          {
            double pT = grModel->GetX()[i];
            double y = grModel->GetY()[i];

            grModel->SetPoint(i, pT, y/(2*TMath::Pi()*pT)); //rescale model by 1/2pipt
          }

          //rescale model by 1/2pi
          //grModel->Scale(1./(TMath::Pi()*2.));

          // Create new canvas for comparison
          std::string compName = Form("Comparison_%s_k%s_cent%d_%s", 
                                     ensemble.c_str(), k_str.str().c_str(), cent, pname.c_str());
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
  }

  hepFile->Close();
  delete hepFile;
}
