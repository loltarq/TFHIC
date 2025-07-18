#ifndef BLASTWAVE_UTILS_H
#define BLASTWAVE_UTILS_H

#include "hadron_list.h"
#include <TH1.h>

// -------------------------
// Blast-wave integrand
// -------------------------
double blastwave_integrand(double* x, double* par);

// -------------------------
// 1/pT * dNdpT
// -------------------------
Double_t dNdpT_pT(const double* x, const double* par);

// -------------------------
// dNdpT
// -------------------------
Double_t dNdpT(const double* x, const double* par);

// -------------------------
// Compute pT spectrum method - returns a TH1D
// timesPt: true means PtSpectrum is dN/dpT, instead of 1/pT * dN/dpT
// clampR: method of numerical stability. if true, the integration region in r is constrained so that beta cannot reach 1; else, beta is forced to < 1 if higher.
// rmax: if set to different than 0, this represents the PHYSICAL radius of the fireball. Useful only if determining spectrum from first principles (i.e. no yield to normalize to)
// -------------------------
TH1D* computePtSpectrum(HadronIntegrationInfo info, double pTmin = 0., double pTmax = 5.0, bool timesPt = false, bool clampR = false, double rmax = 0, int nBins = 200, int intPoints = 1000, double R_epsF = 1e-2);

#endif // BLASTWAVE_UTILS_H

