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

    std::vector<string> filename[4];
    std::vector<double> k = {1.,1.6,3.};

    for (auto i : k)
    {
        filename[0].push_back("../out/piRatios_CE_scan_k"+dtos(i)+"_toGCE.dat");
        filename[1].push_back("../out/piRatios_SCE_scan_k"+dtos(i)+"_toGCE.dat");
        filename[2].push_back("../out/piRatios_gs_CE_scan_k"+dtos(i)+"_toGCE.dat");
        filename[3].push_back("../out/piRatios_gs_SCE_scan_k"+dtos(i)+"_toGCE.dat");
    }


    //for (auto i : k)
    //{

        //filenames[1].push_back("../out/piRatios_SCE_scan_k"+dtos(i)+"_toGCE.dat");
    //}

    std::vector<string> format = {
        "%lg %*lg %lg",
        "%lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %lg",
        "%lg %*lg %*lg %*lg %*lg %*lg %*lg %lg"
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
            TMultiGraph * mg = new TMultiGraph();
            TGraph *g[3];
            for (int k=0; k<3; k++)
            {
                g[k] = new TGraph(filename[i][k].c_str(),format[j].c_str());
                //g[k] -> SetLineWidth(3);
                g[k] -> SetLineStyle(k);
                g[k] -> SetLineColor(k+2);
                mg -> Add(g[k]);
            }
            mg->Draw("AC*");
            mg->SetTitle(canvastitles[j].c_str());
            mg->GetXaxis()->SetTitle("dNpi/dy");
            mg->GetYaxis()->SetTitle("Ratio to GCE limit");
            //mg->GetXaxis()->SetLimits(0,4000);
            //gPad->SetLogx();
            //gPad->SetGrid();
            gPad->Modified();
            //gPad->Update();
        }
        canvas[i]->SaveAs((cname+cinx+ext1).c_str());
        canvas[i]->SaveAs((cname+cinx+ext2).c_str());
    }
}
    