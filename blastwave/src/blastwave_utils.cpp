#include <blastwave_utils.h>
#include <TMath.h>
#include <TF1.h>

// Blast-wave integrand (thermal+flow), GeV units
Double_t blastwave_integrand(double* x, double* par) 
{
    double r = x[0];
    double mass = par[0];
    double pT = par[1];
    double beta_T = par[2];
    double Tkin = par[3];
    double n_profile = par[4];
    bool clampR = par[5];

    double beta_s = (n_profile + 1) * beta_T;           // compute beta_s from beta_t

    double mT = TMath::Sqrt(pT * pT + mass * mass);     // transverse mass
    long double beta = beta_s * TMath::Power(r, n_profile);

    if (!clampR)
        if (beta > 0.9999999999999999) beta = 0.9999999999999999;

    double rho = TMath::ATanH(beta);
    double arg1 = (pT * TMath::SinH(rho)) / Tkin;

    if (arg1 > 700.) arg1 = 700.; // avoid divergence of modified Bessel function I0 at high pT

    double arg2 = (mT * TMath::CosH(rho)) / Tkin;

    double result = r * mT * TMath::BesselI0(arg1) * TMath::BesselK1(arg2);

    return result;
}

Double_t dNdpT_pT(const double* x, const double* p)
{
  double pT = x[0];
  double mass    = p[0];
  double beta    = p[1];
  double temp    = p[2];
  double n       = p[3];
  double norm    = p[4];
  bool clampR = p[5];
  double rmax = p[6];

  // r/rmax from 0 to 1
  double r_rmax = 1;

  static TF1 * fIntBG = 0;

  if(!fIntBG)
    fIntBG = new TF1 ("integrand", blastwave_integrand, 0., 1., 6);

  fIntBG->SetParameters(mass, pT, beta, temp, n, clampR);

  if (clampR)
  {
    double beta_s = beta * (n + 1.);
    if (beta_s > 0.9999999999999999)
    {
        r_rmax = TMath::Power(1./beta_s, 1./n);
        r_rmax *= 0.9999999999999999; // numerical stability
    }
  }

  // transformed cooper-frye: r_rmax stands for r/rmax, so it's integrated from 0 to 1. Need to multiply by rmax^2 to transform back.
  // No need to transform back if re-normalizing to known yield, as the factor would be absorbed by the normalization
  double result = fIntBG->Integral(0,r_rmax);

  // if rmax is valued to something else than 0, then transform back. By default rmax = 0.
  if (rmax > 0)
    result *= TMath::Power(rmax,2);

  return result*norm;

}

Double_t dNdpT(const double* x, const double* p)
{
    double pT = x[0];
    return pT*dNdpT_pT(x, p);
}

TH1D* computePtSpectrum(HadronIntegrationInfo info, double pTmin, double pTmax, bool timesPt, bool clampR, double rmax, int nBins, int intPoints, double R_epsF)
{
    std::string htitle = info.hadron.name + " Blast-wave p_{T} spectrum, Centrality: " + std::to_string(info.centrality_class - 1);

    // Make reusable TF1
    TF1* f = new TF1("bw_integral", (timesPt ? dNdpT : dNdpT_pT), 0, 1, 7);
    
    //f->SetNpx(intPoints); 

    // Set params
    f->SetParameter(0, info.hadron.mass); //GeV
    f->SetParameter(1, info.beta_t);
    f->SetParameter(2, info.Tkin); //GeV
    f->SetParameter(3, info.n_profile);
    f->SetParameter(4, 1.); // set norm factor to 1. Will be updated later to account for yield
    f->SetParameter(5, clampR); // boolean to choose if clamping r or beta for numerical stability

    // fireball physical radius. if 0, then it's not used. defaulted to 0.
    // must be set to a value different than 0 only when not renormalizing to known yields
    f->SetParameter(6, rmax); 

    std::cout << "\nDetermining spectrum for hadron " << info.hadron.name << " in centrality class " << info.centrality_class << std::endl;
    std::cout << "beta_t: " << f->GetParameter(1) << "\n";
    std::cout << "Tkin: " << f->GetParameter(2) << "\n";
    std::cout << "n_profile: " << f->GetParameter(3) << "\n";

    // ignore: just warning print to terminal if clamp on r is used
    if (clampR)
    {
        double rmax = 1;
        double beta_s = info.beta_t * (info.n_profile + 1.);
        if (beta_s > 0.9999999999999999)
        {
        rmax = TMath::Power(1./beta_s, 1./info.n_profile);
        rmax *= 0.9999999999999999;
        }

        if (rmax < 1.)
            std::cout << "WARNING: unphysical beta_s; integration region in r constrained from 1 to " << rmax << "\n";
    }

    double rawIntegral = f->Integral(pTmin, pTmax);
    double scaleFactor = info.yield / rawIntegral;

    f->SetParameter(4, scaleFactor);
    TH1D* hPtNorm = new TH1D("hPt", htitle.c_str(), nBins, pTmin, pTmax);

    for (int i = 1; i <= hPtNorm->GetNbinsX(); ++i)
    {
        double pT = hPtNorm->GetBinCenter(i);
        double val = f->Eval(pT);
        
        hPtNorm->SetBinContent(i, val);
    }


    std::cout << "Blast-wave raw integral: " << rawIntegral << ", target yield: " << info.yield << ", scale factor: " << scaleFactor << std::endl;
    double check = hPtNorm->Integral("width");
    std::cout << "Normalized spectrum integral: " << check << std::endl;

    return hPtNorm;

}