//#include "TString.h"
//#include "TFile.h"
//#include "TTree.h"
//#include "TCanvas.h"

void HistoFromDat()
{

    std::vector<string> filenames;
    for (int i=0; i<4; i++)
    {
        filenames.push_back("../out/CEscan"+std::to_string(i)+".dat");
    }

    TCanvas *c1 = new TCanvas("c1");
    c1->SetLogx();
    c1->cd();

    TMultiGraph *mg = new TMultiGraph();
    //std::vector<TGraph> vg;
    for (int i=0; i<filenames.size(); i++)
    {
        TGraph *g = new TGraph(filenames[i].c_str());
        //vg.push_back(*g);
        mg->Add(g);
    }
    //g1->SetTitle("stuff;stuff;stuff");
    //g1->Draw("AC");

    mg->Draw("AC");

    
    
}