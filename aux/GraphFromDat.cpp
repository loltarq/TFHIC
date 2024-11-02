//#include "TString.h"
//#include "TFile.h"
//#include "TTree.h"
//#include "TCanvas.h"


string dtos(double d) {
    std::stringstream ss;
    ss << d;

    return ss.str();
}

void GraphFromDat()
{
    std::vector<string> canvastitles{"K/pi_toGCE","Xi/pi_toGCE","phi/pi_toGCE","p/pi_toGCE","Omega/pi_toGCE","La/pi_toGCE"};
    //std::vector<string> canvastitles1{"K/pi","Xi/pi","phi/pi","p/pi","Omega/pi","La/pi"};

    std::vector<string> filename[2];
    std::vector<double> kscan = {1.,1.6,3.};

    for (auto i : kscan)
    {
        //filename[0].push_back("../out/piRatios_CE_scan_k"+dtos(i)+"_toGCE.dat");
        //filename[1].push_back("../out/piRatios_SCE_scan_k"+dtos(i)+"_toGCE.dat");
        filename[0].push_back("../out/piRatios_gs_CE_scan_k"+dtos(i)+"_toGCE.dat");
        filename[1].push_back("../out/piRatios_gs_SCE_scan_k"+dtos(i)+"_toGCE.dat");
    }

    std::vector<string> format = {
        "%lg %*lg %lg",
        "%lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %*lg %lg"
        };

    std::vector<string> formatgs = {
        "%lg %*lg  %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %*lg %*lg %*lg %*lg %lg"
        };


    TCanvas * canvas[2];

    for (int i=0; i<2; i++)
    {
        std::string cname = "canvas", ext1 = ".root", ext2 = ".png";
        std::string cinx = to_string(i);

        canvas[i] = new TCanvas((cname+cinx).c_str(), (cname+cinx).c_str());
        canvas[i] -> Divide(3,2);

        for (int j=0; j<6; j++)
        {
            canvas[i] -> cd(j+1);
            auto legend = new TLegend(0.55, 0.2, 0.9, 0.4);
            TMultiGraph * mg = new TMultiGraph();
            TGraph *g[3];
            for (int k=0; k<3; k++)
            {
                g[k] = new TGraph(filename[i][k].c_str(),formatgs[j].c_str());
                //g[k] -> SetLineWidth(3);
                g[k] -> SetLineStyle(k);
                g[k] -> SetLineColor(k+2);
                std::string legendentry = "Ratio @ k = ", kstring = dtos(kscan[k]);
                legend -> AddEntry(g[k], (legendentry + kstring).c_str(), "l");
                mg -> Add(g[k]);
            }
            mg->Draw("AC");
            TLine *l = new TLine(0,1,2000,1);
            l->SetLineStyle(10);
            l->Draw();
            mg->SetTitle(canvastitles[j].c_str());
            mg->GetXaxis()->SetTitle("dNch/deta");
            mg->GetYaxis()->SetTitle("Ratio to GCE limit");
            mg->GetXaxis()->SetLimits(0.1,5000);
            gPad->SetLogx();

            legend -> SetHeader((i==0 ? "Gs. Canonical Ensemble" : "Gs. S-canonical ensemble"));
            //legend -> SetX1NDC(0.6);
            //legend -> SetX2NDC(1.);
            legend -> Draw();
            //gPad->BuildLegend();
            //mg->GetXaxis()->SetRangeUser(0.01,4000);
            //gPad->SetGrid();
            gPad->Modified();
            //gPad->Update();
        }
        canvas[i]->SaveAs((cname+cinx+ext1).c_str());
        canvas[i]->SaveAs((cname+cinx+ext2).c_str());
    }
}
    