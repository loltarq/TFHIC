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


double blastwave_integrand(double *x, double *par) {

    double r = x[0];
    double pT = par[0];
    double mass = par[1];
    double Tkin = par[2];
    double beta_t = par[3];
    double beta_s = (n + 1) * beta_t;
    double n = par[4];
    double Rmax = par[5];
    
    double mT = sqrt(pT*pT + mass*mass);
    double beta = beta_s * pow(r / Rmax, n);
    double rho = atanh(beta);

    //to do: implement check for beta < 1

    double I0 = TMath::BesselI0(pT * sinh(rho) / Tkin);
    double K1 = TMath::BesselK1(mT * cosh(rho) / Tkin);

    double integrand_result = r * mT * I0 * K1;

    return integrand_result;

}

TF1* f = new TF1("blastwave integrand", blastwave_integrand);

double blastwave_integral(double pTmin, double pTmax, double dpT, double Rmax, double mass, double beta_t, double Tkin, double n_profile)
{
    f -> SetParameters(0.0, mass, Tkin, beta_t, n, Rmax);

    for (double pT = pTmin, pT <= pTmax, pT += dpT)
    {
        f -> SetParameter(0, pT);
        double integral = f -> Integral(0.0, Rmax, 1e-8);
    }
}

// Blast-wave integrand (thermal+flow), GeV units
double BlastWaveIntegrand(double pT, double mass, double beta_t = 0.3, double Tkin = 0.9, double n_profile = 1.0, double R_max = 1.0) 
{
    double beta_s = (n_profile + 1) * beta_t;           // compute beta_s from beta_t
    double mT = TMath::Sqrt(pT * pT + mass * mass);     // transverse mass
    double result = 0.0;

    // (crude) integration over r
    for (int i = 0; i < n_steps; ++i)
    {
        double r = R_max * i / n_steps;
        double beta = beta_s * TMath::Power(r / R_max, n_profile);
        double rho = TMath::ATanH(beta);

        double arg1 = (pT * TMath::SinH(rho)) / Tkin;
        double arg2 = (mT * TMath::CosH(rho)) / Tkin;

        result += r * mT * TMath::BesselI0(arg1) * TMath::BesselK1(arg2);

    }

    return result * (R_max / n_steps);  // riemann sum
}

void blastwave_dNdpT()
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

                    // including uncertainties
                    //double dNdpT_beta_err = pT * BlastWaveIntegrand(pT, mass, beta_t + h.beta_t_unc[1] + h.beta_t_unc[0], Tkin, n_profile);

                    //double dNdpT_Tkin_err = pT * BlastWaveIntegrand(pT, mass, beta_t, Tkin + h.Tkin_t_unc[1] + h.Tkin_t_unc[0], n_profile);

                    //double dNdpT_n_err = pT * BlastWaveIntegrand(pT, mass, beta_t, Tkin, n_profile + h.n_profile_unc[1] + h.n_profile_unc[0]);


                    double dNdpT = pT * BlastWaveIntegrand(pT, mass); // Jacobian included
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
                double dNdpT = pT * BlastWaveIntegrand(pT, mass); // Jacobian included
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