#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TMath.h>
#include <iostream>
#include <vector>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include <iomanip>
#include <TH1.h>

// Integration granularity
const int n_steps = 10000;
const int n_points = 2000;


// Struct to hold particle info
struct Hadron
{
    std::string name;
    double mass;     // in GeV
    int color;       // no QCD shenanigans, just ROOT plotting
};

// Struct to hold integration data for each hadron
struct HadronIntegrationInfo
{
    Hadron hadron;
    double yield; // to normalize integration in pT
    double pT_min, pT_max;

    // integration parameters
    double beta_t;
    double Tkin;
    double n_profile;
    
    double R_max = 1.0;

    // uncertainties
    double beta_t_unc[2];
    double Tkin_t_unc[2];
    double n_profile_unc[2];
};

// List of hadrons: pion, kaon, proton
Hadron pion = {"pi^{+}", 0.13957039, kRed + 1};
Hadron kaon = {"K^{+}",   0.49367, kBlue + 1};
Hadron proton = {"p", 0.93827208816,  kGreen + 2};

std::vector<std::vector<HadronIntegrationInfo>> HIFs;

void read_yield_and_integrationInfo()
{
    // Open the ROOT file
    TFile *file1 = TFile::Open("data_TableIV.root", "READ");
    if (!file1 || file1->IsZombie()) {
        std::cerr << "Error opening ROOT file!" << std::endl;
        return;
    }

    // Access the TTree stored in the ROOT file
    TTree *tree = (TTree*)file1->Get("tree");
    if (!tree) {
        std::cerr << "Error accessing TTree in the ROOT file!" << std::endl;
        return;
    }

    //file1->Close();

    double beta_t[3];
    double Tkin[3];
    double n_profile[3];
    double chisquare_dof;

    int centrality_class;
    std::string centrality;

    // tree->Print();
    
    tree->SetBranchAddress("Class", &centrality_class);
    //tree->SetBranchAddress("Centrality", &centrality);
    tree->SetBranchAddress("beta_t", &beta_t[0]);
    tree->SetBranchAddress("beta_t_btb", &beta_t[1]);
    tree->SetBranchAddress("beta_t_sys", &beta_t[2]);
    tree->SetBranchAddress("Tkin", &Tkin[0]);
    tree->SetBranchAddress("Tkin_btb", &Tkin[1]);
    tree->SetBranchAddress("Tkin_sys", &Tkin[2]);
    tree->SetBranchAddress("n_profile", &n_profile[0]);
    tree->SetBranchAddress("n_profile_btb", &n_profile[1]);
    tree->SetBranchAddress("n_profile_sys", &n_profile[2]);
    tree->SetBranchAddress("chi^2/dof", &chisquare_dof);


    TFile * file2 = new TFile("HEPData-ins1222333-v1-Table_31.root");
    if (!file2 || file2->IsZombie()) 
    {
        std::cerr << "Error opening ROOT file!" << std::endl;
        return;
    }

    TDirectory *td = (TDirectory*)file2->Get("Table 31");
    if (!td)
    {
        std::cerr << "Failed to find the 'table' folder!" << std::endl;
        file1->Close();
        return;
    }

    std::vector<std::vector<double>> yields;
    yields.resize(3);

    for (int i = 2; i <= 7; i+=2)
    { 
        TString hist_name = TString::Format("Hist1D_y%d", i);

        // Get the histogram
        TH1F* hist = (TH1F*)td->Get(hist_name);
        if (hist)
        {
            for (int bin = 1; bin <= hist->GetNbinsX(); ++bin)
            {
                //double x_value = hist->GetBinCenter(bin);  // Bin center
                double y_value = hist->GetBinContent(bin);  // Bin content (value)
                yields[i/2-1].push_back(y_value);
            }
        }   
        else 
        {
            std::cerr << "Histogram " << hist_name << " not found!" << std::endl;
        }
    }


    for (int i = 0; i < 10; i++)
    {

        tree->GetEntry(i);

        std::vector<HadronIntegrationInfo> hif = 
        {
            {pion, yields[0][i], 0., 3., beta_t[0], Tkin[0], n_profile[0]},
            {kaon, yields[1][i], 0., 3., beta_t[0], Tkin[0], n_profile[0]},
            {proton, yields[2][i], 0., 3., beta_t[0], Tkin[0], n_profile[0]}
        };

        HIFs.push_back(hif);
    }

    file1->Close();
    file2->Close();
}

// Blast-wave integrand (thermal+flow), GeV units
double BlastWaveIntegrand(double pT, HadronIntegrationInfo info, double R_max = 1.0) 
{
    double mass = info.hadron.mass;
    double beta_t = info.beta_t;
    double Tkin = info.Tkin;
    double n_profile = info.n_profile;

    double beta_s = (n_profile + 1) * beta_t;           // compute beta_s from beta_t
    double mT = TMath::Sqrt(pT * pT + mass * mass);     // transverse mass
    double result = 0.0;

    // (crude) integration over r
    for (int i = 0; i < n_steps; ++i)
    {
        double r = R_max * i / n_steps;
        double beta = beta_s * TMath::Power(r / R_max, n_profile);
        std::cout << r << " " << beta << "\n";
        double rho = TMath::ATanH(beta);
        std::cout << rho << std::endl;

        double arg1 = (pT * TMath::SinH(rho)) / Tkin;
        double arg2 = (mT * TMath::CosH(rho)) / Tkin;

        result += r * mT * TMath::BesselI0(arg1) * TMath::BesselK1(arg2);

    }

    return result * (R_max / n_steps);  // riemann sum
}

void blastwave_dNdpT_dev()
{
    TFile *file = TFile::Open("output.root", "RECREATE");
    
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    TCanvas* c1 = new TCanvas("c1", "Blast-Wave pT Spectra", 900, 700);
    TMultiGraph* mg = new TMultiGraph();
    TLegend* legend = new TLegend(0.65, 0.70, 0.88, 0.88);
    
    read_yield_and_integrationInfo();

    int i;

    std::cout << "\nWhich centrality class? (0-9, 10 for all)\n";
    std::cin >> i;
    std::cout << "\n";

    if (i < 0)
        return;
    else if (i > HIFs.size() - 1)
    {
        for (const auto& HIF : HIFs)
        {
            for (const auto& h : HIF)
            {
                TGraphErrors* graph = new TGraphErrors(n_points);
                double norm = 0.0;
                double pT_min = h.pT_min;
                double pT_max = h.pT_max;
                double mass = h.hadron.mass;

                std::cout << "\nIntegrating over pT for hadron " << h.hadron.name << ", between " << pT_min << " and " << pT_max << " GeV\n";
                // Compute spectrum shape and normalization
                for (int i = 0; i < n_points; ++i)
                {
                    double pT = pT_min + (pT_max - pT_min) * i / (n_points - 1);

                    double dNdpT = pT * BlastWaveIntegrand(pT, h); // Jacobian included
                    graph->SetPoint(i, pT, dNdpT);
                    norm += dNdpT * (pT_max - pT_min) / n_points;       // compute the area of the curve in pt. Will be used to normalize the curve w

                    // progress check
                    if (i % (n_points / 100) == 0)
                    {
                        int progress = static_cast<int>(100.0 * i / n_points);
                        std::cout << "\r[" << std::setw(3) << progress << "%] " << std::flush;
                    }
                }
                std::cout << "\n\n";

                // Normalize spectrum to thermal yield
                for (int i = 0; i < n_points; ++i)
                {
                    double pT, dNdpT;
                    graph->GetPoint(i, pT, dNdpT);
                    graph->SetPoint(i, pT, dNdpT * h.yield / norm);     // normalize the model's output so that the integral of dNdpT returns the expected yield
                }

                graph->SetLineColor(h.hadron.color);
                graph->SetLineWidth(2);
                graph->SetTitle(h.hadron.name.c_str());
                mg->Add(graph);
                legend->AddEntry(graph, h.hadron.name.c_str(), "l");
            }

            mg->SetTitle("Blast-Wave Model: p_{T} Spectra for pions, kaons and protons; p_{T} [GeV/c]; dN/dp_{T}");
            mg->Draw("AL");
            legend->Draw();
            c1->SetLogy();
            c1->Update();

            file->Write();
        }
    }
    else
    {
        for (const auto& h : HIFs[i])
        {
            TGraph* graph = new TGraph(n_points);
            double norm = 0.0;
            double pT_min = h.pT_min;
            double pT_max = h.pT_max;
            double mass = h.hadron.mass;

            std::cout << "\nIntegrating over pT for hadron " << h.hadron.name << ", between " << pT_min << " and " << pT_max << " GeV\n";
            // Compute spectrum shape and normalization
            for (int i = 0; i < n_points; ++i)
            {
                double pT = pT_min + (pT_max - pT_min) * i / (n_points - 1);
                double dNdpT = pT * BlastWaveIntegrand(pT, h); // Jacobian included
                graph->SetPoint(i, pT, dNdpT);
                norm += dNdpT * (pT_max - pT_min) / n_points;       // compute the area of the curve in pt. Will be used to normalize the curve w

                // progress check
                if (i % (n_points / 100) == 0)
                {
                    int progress = static_cast<int>(100.0 * i / n_points);
                    std::cout << "\r[" << std::setw(3) << progress << "%] " << std::flush;
                }
            }
            std::cout << "\n\n";

            // Normalize spectrum to thermal yield
            for (int i = 0; i < n_points; ++i)
            {
                double pT, dNdpT;
                graph->GetPoint(i, pT, dNdpT);
                graph->SetPoint(i, pT, dNdpT * h.yield / norm);     // normalize the model's output so that the integral of dNdpT returns the expected yield
            }

            graph->SetLineColor(h.hadron.color);
            graph->SetLineWidth(2);
            graph->SetTitle(h.hadron.name.c_str());
            mg->Add(graph);
            legend->AddEntry(graph, h.hadron.name.c_str(), "l");
        }

        mg->SetTitle("Blast-Wave Model: p_{T} Spectra for pions, kaons and protons; p_{T} [GeV/c]; dN/dp_{T}");
        mg->Draw("AL");
        legend->Draw();
        c1->SetLogy();
        c1->Update();

        file->Write();
    }

    file->Close();
}