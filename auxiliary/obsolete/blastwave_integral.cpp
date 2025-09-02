#include <cmath>
#include <TMath.h>
#include <TH1D.h>
#include <TFile.h>
#include <TTree.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>

// Compute the blast-wave pT spectrum (unnormalized)
double computeBlastWavePtSpectrum(double pT, double mass, double T_kin, double beta_T, double n_profile, double R = 1.0, int n_steps = 10000)
{
    double mT = std::sqrt(pT * pT + mass * mass);
    double dr = R / n_steps;
    double result = 0.0;

    for (int i = 0; i < n_steps; ++i)
    {
        double r = (i + 0.5) * dr;
        double beta_r = beta_T * std::pow(r / R, n_profile);
        double rho = std::atanh(beta_r);

        double arg0 = (pT * std::sinh(rho)) / T_kin;
        double arg1 = (mT * std::cosh(rho)) / T_kin;

        double integrand = r * mT * TMath::BesselI0(arg0) * TMath::BesselK1(arg1);
        result += integrand;
    }

    result *= dr;
    return result;
}

// Data structure to hold hadron properties
struct Hadron {
    std::string name;
    double mass; // GeV
    int color;   // for ROOT plotting
};

// Blast-wave parameter structure with symmetric uncertainties
struct BlastWaveParams {
    double T_kin;
    double beta_T;
    double n_profile;
    double R_max = 1.0;

    // Symmetric uncertainties
    double T_kin_unc = 0.0;
    double beta_T_unc = 0.0;
    double n_profile_unc = 0.0;
};

// Generate a ROOT histogram of the pT spectrum
TH1D* generatePtSpectrum(const Hadron& hadron, const BlastWaveParams& params,
                         double pT_min, double pT_max, int n_bins, double yield_norm)
{
    std::string hist_name = "h_" + hadron.name;
    TH1D* h = new TH1D(hist_name.c_str(), (hadron.name + " pT Spectrum").c_str(),
                       n_bins, pT_min, pT_max);

    for (int i = 1; i <= n_bins; ++i)
    {
        double pT = h->GetBinCenter(i);
        double val = computeBlastWavePtSpectrum(pT, hadron.mass,
                                                params.T_kin, params.beta_T, params.n_profile,
                                                params.R_max);
        h->SetBinContent(i, val);
    }

    // Normalize to yield
    double integral = h->Integral("width");
    if (integral > 0)
        h->Scale(yield_norm / integral);

    h->SetLineColor(hadron.color);
    h->SetLineWidth(2);
    return h;
}

// Generate upper/lower uncertainty envelopes
std::pair<TH1D*, TH1D*> generatePtSpectrumEnvelope(const Hadron& hadron, const BlastWaveParams& params,
                                                   double pT_min, double pT_max, int n_bins, double yield_norm)
{
    BlastWaveParams p_low = params;
    p_low.T_kin -= params.T_kin_unc;
    p_low.beta_T -= params.beta_T_unc;
    p_low.n_profile -= params.n_profile_unc;

    BlastWaveParams p_high = params;
    p_high.T_kin += params.T_kin_unc;
    p_high.beta_T += params.beta_T_unc;
    p_high.n_profile += params.n_profile_unc;

    TH1D* h_low = generatePtSpectrum(hadron, p_low, pT_min, pT_max, n_bins, yield_norm);
    TH1D* h_high = generatePtSpectrum(hadron, p_high, pT_min, pT_max, n_bins, yield_norm);

    h_low->SetLineStyle(2);
    h_high->SetLineStyle(2);
    return {h_low, h_high};
}

// Load parameters and yields from ROOT files
struct CentralityData {
    BlastWaveParams params;
    double yield;
};

std::map<int, std::map<std::string, CentralityData>> loadParametersAndYields(const std::string& paramFile, const std::string& yieldFile) {
    std::map<int, std::map<std::string, CentralityData>> dataset;

    TFile* f_params = TFile::Open(paramFile.c_str());
    TTree* tree = (TTree*)f_params->Get("tree");

    int Class;
    double beta_t, Tkin, n_prof;
    double beta_unc, Tkin_unc, n_prof_unc;

    tree->SetBranchAddress("Class", &Class);
    tree->SetBranchAddress("beta_t", &beta_t);
    tree->SetBranchAddress("beta_t_sys", &beta_unc);
    tree->SetBranchAddress("Tkin", &Tkin);
    tree->SetBranchAddress("Tkin_sys", &Tkin_unc);
    tree->SetBranchAddress("n_profile", &n_prof);
    tree->SetBranchAddress("n_profile_sys", &n_prof_unc);

    Long64_t nentries = tree->GetEntries();
    for (Long64_t i = 0; i < nentries; ++i) {
        tree->GetEntry(i);
        for (const std::string& hadron : {"pi+", "K+", "p"}) {
            CentralityData cd;
            cd.params.T_kin = Tkin;
            cd.params.beta_T = beta_t;
            cd.params.n_profile = n_prof;
            cd.params.T_kin_unc = Tkin_unc;
            cd.params.beta_T_unc = beta_unc;
            cd.params.n_profile_unc = n_prof_unc;
            dataset[Class][hadron] = cd;
        }
    }
    f_params->Close();

    TFile* f_yield = TFile::Open(yieldFile.c_str());
    TDirectory* dir = (TDirectory*)f_yield->Get("Table 31");

    for (int c = 2; c <= 7; c += 2) {
        for (const std::pair<std::string, std::string>& entry : {{"pi+", "Hist1D_y1"}, {"K+", "Hist1D_y2"}, {"p", "Hist1D_y3"}}) {
            TH1* h = (TH1*)dir->Get(entry.second.c_str());
            if (h) dataset[c][entry.first].yield = h->GetBinContent(1);
        }
    }
    f_yield->Close();

    return dataset;
}

// Main driver to plot spectra
void plotBlastwaveSpectra() {
    std::map<int, std::map<std::string, CentralityData>> data =
        loadParametersAndYields("data_TableIV.root", "HEPData-ins1222333-v1-Table_31.root");

    std::map<std::string, Hadron> hadrons = {
        {"pi+", {"pi+", 0.13957, kRed}},
        {"K+", {"K+", 0.49367, kBlue}},
        {"p",  {"p", 0.93827, kGreen+2}}
    };

    double pT_min = 0.0, pT_max = 3.0;
    int n_bins = 150;
    int centrality = 2;  // example centrality class

    TCanvas* c = new TCanvas("c", "Blast-Wave pT Spectra", 800, 600);
    TLegend* legend = new TLegend(0.6, 0.6, 0.88, 0.88);

    for (const auto& [name, hadron] : hadrons) {
        const CentralityData& cd = data[centrality][name];
        TH1D* h_central = generatePtSpectrum(hadron, cd.params, pT_min, pT_max, n_bins, cd.yield);
        auto [h_low, h_high] = generatePtSpectrumEnvelope(hadron, cd.params, pT_min, pT_max, n_bins, cd.yield);

        h_central->Draw(h_central == hadrons.begin()->second ? "HIST" : "HIST SAME");
        h_low->Draw("HIST SAME");
        h_high->Draw("HIST SAME");

        legend->AddEntry(h_central, hadron.name.c_str(), "l");
    }

    legend->Draw();
    c->Update();
}
