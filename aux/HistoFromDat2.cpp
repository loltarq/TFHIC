//#include "TString.h"
//#include "TFile.h"
//#include "TTree.h"
//#include "TCanvas.h"

void HistoFromDat2()
{

    std::vector<string> filenames;
    std::vector<int> lol = {1,3,6};

    for (auto i : lol)
    {
        filenames.push_back("../out/piRatios_CEscan"+std::to_string(i)+".dat");
    }

    std::vector<string> format = {
        "%lg %*lg %lg",
        "%lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %*lg %lg"
        };

    TCanvas *c1 = new TCanvas("c1");
 
    c1->Divide(3,2);

    TMultiGraph* mg[6];
    
    for (int i=0; i<6; i++)
    {
        c1->cd(i);
        mg[i] = new TMultiGraph();
        
        for (int j=0; j<filenames.size(); j++)
        {
            TGraph *g = new TGraph(filenames[j].c_str(), format[i].c_str());
            //vg.push_back(*g);
            mg[i]->Add(g);

        }
        //g1->SetTitle("stuff;stuff;stuff");
        //g1->Draw("AC");
 
        c1->SetLogx();
        mg[i]->Draw("AC");
        gPad->Update();
    }

    c1->SaveAs("out.png");
}