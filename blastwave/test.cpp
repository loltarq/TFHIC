#include "include/io_utils.h"
#include "include/blastwave_utils.h"

//\usepackage[numbers]{natbib}
//\bibliographystyle{plain}
//\bibliography{bibliografia.bib}

void test() 
{
  //HBT pion interferometry - experimental results....

  double Vc = 8875.68; //fm^3
  double tau_f = 10.; //fm/c --- c=1
  double delta_y = 1.;

  double R = TMath::Power(Vc/(TMath::Pi() * tau_f * delta_y), 0.5);
  std::cout << "\nVc: " << Vc << " fm^3\n";
  std::cout << "R: " << R << " fm\n";

  TCanvas *c = new TCanvas("Centrality 0", "Centrality 0", 1200, 400);
  c->Divide(3,1);
  //TLegend *leg = new TLegend(0.6, 0.7, 0.88, 0.88);
  //leg->SetBorderSize(0);

  auto infos = get_integration_info();

  int j = 0;
  for (auto item : infos[0])
  {
    auto hPt = computePtSpectrum(item, R);
    hPt->SetLineColor(item.hadron.color);
    hPt->SetLineWidth(2);

    c->cd(j+1);
    j++;
    TString drawOpt = "HIST";
    hPt->Draw(drawOpt);
    //leg->AddEntry(hPt, item.hadron.name.c_str(), "l");
  }

  //leg->Draw();

  //auto hPt = computePtSpectrum(item, R);
  //hPt->Draw("HIST");

}