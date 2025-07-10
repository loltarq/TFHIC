#ifndef BLASTWAVE_UTILS_H
#define BLASTWAVE_UTILS_H

#include "hadron_list.h"
#include <TH1.h>

// -------------------------
// Blast-wave integrand prototype
// -------------------------
double blastwave_integrand(double* x, double* par);

// -------------------------
// Compute pT spectrum prototype
// -------------------------
TH1D* computePtSpectrum(HadronIntegrationInfo info, double R, double pTmin = 0., double pTmax = 5.0, int nBins = 200, int intPoints = 1000, double R_epsF = 1e-2);

#endif // BLASTWAVE_UTILS_H

