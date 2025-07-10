#include <blastwave_utils.h>
#include <TMath.h>
#include <TF1.h>

// Blast-wave integrand (thermal+flow), GeV units
double blastwave_integrand(double* x, double* par) 
{
    double r = x[0];
    double pT = par[0];
    double mass = par[1];
    double beta_t = par[2];
    double Tkin = par[3];
    double n_profile = par[4];
    double R_max = par[5];

    double beta_s = (n_profile + 1) * beta_t;           // compute beta_s from beta_t
    double mT = TMath::Sqrt(pT * pT + mass * mass);     // transverse mass
    double beta = beta_s * TMath::Power(r / R_max, n_profile);

    double rho = TMath::ATanH(beta);
    double arg1 = (pT * TMath::SinH(rho)) / Tkin;
    double arg2 = (mT * TMath::CosH(rho)) / Tkin;

    double result = r * mT * TMath::BesselI0(arg1) * TMath::BesselK1(arg2);

    return result;
}

TH1D* computePtSpectrum(HadronIntegrationInfo info, double R, double pTmin, double pTmax, int nBins, int intPoints, double R_epsF)
{
    std::string htitle = info.hadron.name + " Blast-wave p_{T} spectrum, Centrality: " + std::to_string(info.centrality_class - 1);

    // Create histogram
    TH1D* hPt = new TH1D("hPt", htitle.c_str(),
                         nBins, pTmin, pTmax);

    // Make reusable TF1
    TF1* f = new TF1("bw_integrand", blastwave_integrand, 0.0,
        1.0, //R_max, to be updated
        6);
    
    f->SetNpx(intPoints); 

    // Set params
    f->SetParameter(1, info.hadron.mass); //GeV
    f->SetParameter(2, info.beta_t);
    f->SetParameter(3, info.Tkin); //GeV
    f->SetParameter(4, info.n_profile);
    f->SetParameter(5, R);

    std::cout << "\nDetermining spectrum for hadron " << info.hadron.name << " in centrality class " << info.centrality_class << std::endl;
    std::cout << "beta_t: " << f->GetParameter(2) << "\n";
    std::cout << "Tkin: " << f->GetParameter(3) << "\n";
    std::cout << "n_profile: " << f->GetParameter(4) << "\n";

    // check on Beta_s -> constrain integration region if Beta_s >= 1

    double R_max = R;
    double beta_s = info.beta_t * (info.n_profile + 1);
    if (beta_s >= 1.)
    {
        R_max = R * TMath::Power(1/beta_s, 1/info.n_profile);
        double eps = R_max * R_epsF;
        R_max -= eps;
        std::cout << "WARNING: Unphysical beta_s = " << beta_s << " ≥ 1. Constraining integration region for r.\n";
        std::cout << "WARNING: R_max constrained from " << R << " to " << R_max;
    }

    for (int i = 1; i <= nBins; ++i) {
        double pT = hPt->GetBinCenter(i);

        f->SetParameter(0, pT); // update pT
        // Integrate over r = [0, R_max]
        double integral = f->Integral(0.0, R_max, 1e-8);
        //std::cout << "\npT = " << pT << ", Integral = " << integral << std::endl;

        hPt->SetBinContent(i, integral);
    }

    delete f; // cleanup

    double rawIntegral = hPt->Integral("width");
    double scaleFactor = info.yield / rawIntegral;
    hPt->Scale(scaleFactor);

    std::cout << "Blast-wave raw integral: " << rawIntegral
              << ", target yield: " << info.yield
              << ", scale factor: " << scaleFactor << std::endl;

    double check = hPt->Integral("width");
    std::cout << "Normalized spectrum integral: " << check << std::endl;

    return hPt;

}