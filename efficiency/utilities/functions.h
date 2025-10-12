#ifndef functions_H
#define functions_H

//#if !defined(__CINT__) || (defined(__MAKECINT__))
#include "TF1.h"
#include "TMath.h"
//#endif

Double_t
LevyTsallis_Func(const Double_t* x, const Double_t* p)
{
  /* dN/dpt */

  Double_t pt = x[0];
  Double_t mass = p[0];
  Double_t mt = TMath::Sqrt(pt * pt + mass * mass);
  Double_t n = p[1];
  Double_t C = p[2];
  Double_t norm = p[3];

  Double_t part1 = (n - 1.) * (n - 2.);
  Double_t part2 = n * C * (n * C + mass * (n - 2.));
  Double_t part3 = part1 / part2;
  Double_t part4 = 1. + (mt - mass) / n / C;
  Double_t part5 = TMath::Power(part4, -n);
  return pt * norm * part3 * part5;
}

TF1* LevyTsallis(const Char_t* name, Double_t mass, Double_t n = 5., Double_t C = 0.1, Double_t norm = 1.)
{

  TF1* fLevyTsallis = new TF1(name, LevyTsallis_Func, 0., 10., 4);
  fLevyTsallis->SetParameters(mass, n, C, norm);
  fLevyTsallis->SetParNames("mass", "n", "C", "norm");
  fLevyTsallis->FixParameter(0, mass);
  fLevyTsallis->SetParLimits(1, 1.e-3, 1.e3);
  fLevyTsallis->SetParLimits(2, 1.e-3, 1.e3);
  fLevyTsallis->SetParLimits(3, 1.e-6, 1.e6);
  return fLevyTsallis;
}

Double_t Hagedorn_Func(Double_t* x, Double_t* par)
{
  Float_t pt = x[0];
  Double_t mt = TMath::Sqrt(pt * pt + par[5] * par[5]);
  Double_t f = -1.0 * par[0] * mt - par[1] * mt * mt;
  f = TMath::Exp(f) + mt / par[2];
  f = TMath::Power(f, par[3]);
  f = pt * par[4] / f;
  return f;
}

TF1* Hagedorn(const Char_t* name, Double_t mass)
{
  TF1* fun = new TF1(name, Hagedorn_Func, 0, 50., 6);
  fun->SetParNames("a", "b", "m0", "n", "Scale", "mass");
  fun->FixParameter(5, mass);
  return fun;
}

Double_t PowerLawdNdptTimesPt(double * x, double* p)
{
  Double_t f = p[0] * x[0] * TMath::Power((1.0 + x[0] / p[1]), (-1.0 * p[2]));
  return f;
}

TF1 * PowerLawdNdptTimesPtFunc(const Char_t *name, Double_t norm = 1.0, Double_t T = 0.1, Double_t n = 1.)
{
  TF1 *fPowerLawdNdptTimesPt = new TF1(name, PowerLawdNdptTimesPt, 0., 10., 3);
  fPowerLawdNdptTimesPt->SetParameters(norm, T, n);
  fPowerLawdNdptTimesPt->SetParNames("norm", "T", "n");
  return fPowerLawdNdptTimesPt;
}


Double_t MTExpdNdptTimesPt(double * x, double* p)
{
  return p[0] * x[0] * TMath::Exp(-1.0 * TMath::Sqrt(x[0] * x[0] + p[2] * p[2]) / p[1]);
}


TF1 * MTExpdNdptTimesPtFunc(const Char_t *name, Double_t norm = 1.0, Double_t T = 0.1, Double_t mass = 1.)
{
  TF1 *fMTExpdNdptTimesPt = new TF1(name, MTExpdNdptTimesPt, 0., 10., 3);
  fMTExpdNdptTimesPt->SetParameters(norm, T, mass);
  fMTExpdNdptTimesPt->SetParNames("norm", "T", "mass");
  return fMTExpdNdptTimesPt;
}

/* Double_t UA1_Func(const double* x, const double* p) */
/* { */

/*   // "mass","p0star","pt0","n","T","norm" */
/*   Double_t mass = p[0]; */
/*   Double_t p0star = p[1]; */
/*   Double_t pt0 = p[2]; */
/*   Double_t n = p[3]; */
/*   Double_t temp = p[4]; */
/*   Double_t norm = p[5]; */

/*   Double_t xx = x[0]; */

/*   Double_t parplaw[3] = { 0.0, 0.0, 0.0 }; */
/*   parplaw[0] = norm; */
/*   parplaw[1] = pt0; */
/*   parplaw[2] = n; */
/*   Double_t parpexp[3] = { 0.0, 0.0, 0.0 }; */
/*   parpexp[0] = norm; */
/*   parpexp[1] = temp; */
/*   parpexp[2] = mass; */

/*   Double_t normMT = MTExpdNdptTimesPt(p0star, parpexp) > 0 ? PowerLawdNdptTimesPt(p0star, parplaw) / MTExpdNdptTimesPt(p0star, parpexp) * parpexp[0] : 1; */
/*   parpexp[0] = normMT; */

/*   if (TMath::Abs(MTExpdNdptTimesPt(p0star, parpexp) - PowerLawdNdptTimesPt(p0star, parplaw)) > 0.0001) { */
/*     Printf("UA1 - Wrong norm"); */
/*     Printf(" p0* %f  NMT: %f  N: %f  PL: %f  MT: %f", p0star, normMT, norm, PowerLawdNdptTimesPt(p0star, parplaw), MTExpdNdptTimesPt(p0star, parpexp)); */
/*   } */

/*   if (xx > p0star) */
/*     return PowerLawdNdptTimesPt(xx, parplaw); */
/*   return MTExpdNdptTimesPt(xx, parpexp); */
/* } */

/* TF1* UA1(const Char_t* name, Double_t mass) */
/* { */

/*   TF1* fun = new TF1(name, UA1_Func, 0, 50., 6); */
/*   fun->FixParameter(0, mass); */
/*   return fun; */
/* } */

Double_t Bylinkin_Func(const double* x, const double* p)
{
  Double_t mass = p[0];
  Double_t a1 = p[1];
  Double_t a2 = p[2];
  Double_t t1 = p[3];
  Double_t t2 = p[4];
  Double_t n = p[5];

  Double_t pt = x[0];
  Double_t mt = TMath::Sqrt(pt * pt + mass * mass);

  Double_t f1 = TMath::Exp(-1.0 * (mt - mass) / t1);
  Double_t f2 = 1.0 + (pt * pt) / (t2 * t2 * n);
  f2 = TMath::Power(f2, -1.0 * n);
  return pt * a1 * (f1 + a2 * f2);
}

TF1* Bylinkin(const Char_t* name, Double_t mass)
{

  TF1* fun = new TF1(name, Bylinkin_Func, 0, 50., 6);
  fun->SetParNames("mass", "Norm_{global}", "Norm_{n}", "T_{exp}", "T_{n}", "n");
  fun->FixParameter(0, mass);
  return fun;
}

Double_t FermiDirac_Func(const Double_t* x, const Double_t* p)
{
  /* dN/dpt */

  Double_t pt = x[0];
  Double_t mass = p[0];
  Double_t mt = TMath::Sqrt(pt * pt + mass * mass);
  Double_t T = p[1];
  Double_t norm = p[2];

  return pt * norm * 1.0 / (TMath::Exp(mt / T) + 1) * (TMath::Exp(-mass / T) + 1);
}

TF1 * FermiDirac(const Char_t *name, Double_t mass, Double_t T = 0.1, Double_t norm = 1.)
{
  
  TF1 *fFermiDirac = new TF1(name, FermiDirac_Func, 0., 10., 3);
  fFermiDirac->SetParameters(mass, T, norm);
  fFermiDirac->SetParNames("mass", "T", "norm");
  fFermiDirac->FixParameter(0, mass);
  return fFermiDirac;
}

Double_t BoseEinstein_Func(const Double_t* x, const Double_t* p)
{
  /* dN/dpt */

  Double_t pt = x[0];
  Double_t mass = p[0];
  Double_t mt = TMath::Sqrt(pt * pt + mass * mass);
  Double_t T = p[1];
  Double_t norm = p[2];

  return pt * norm * 1.0 / (TMath::Exp(mt / T) - 1) * (TMath::Exp(-mass / T) - 1);
}

TF1 * BoseEinstein(const Char_t *name, Double_t mass, Double_t T = 0.1, Double_t norm = 1.)
{
  
  TF1 *fBoseEinstein = new TF1(name, BoseEinstein_Func, 0., 10., 3);
  fBoseEinstein->SetParameters(mass, T, norm);
  fBoseEinstein->SetParNames("mass", "T", "norm");
  fBoseEinstein->FixParameter(0, mass);
  return fBoseEinstein;
}


Double_t Boltzmann_Func(const Double_t* x, const Double_t* p)
{
  /* dN/dpt */

  Double_t pt = x[0];
  Double_t mass = p[0];
  Double_t mt = TMath::Sqrt(pt * pt + mass * mass);
  Double_t T = p[1];
  Double_t norm = p[2];

  return pt * norm * mt * TMath::Exp(-mt / T);
}

TF1 * Boltzmann(const Char_t *name, Double_t mass, Double_t T = 0.1, Double_t norm = 1.)
{
  
  TF1 *fBoltzmann = new TF1(name, Boltzmann_Func, 0., 10., 3);
  fBoltzmann->SetParameters(mass, T, norm);
  fBoltzmann->SetParNames("mass", "T", "norm");
  fBoltzmann->FixParameter(0, mass);
  return fBoltzmann;
}

//
//BlastWave function
//

static TF1* fBGBlastWave_Integrand = NULL;
static TF1* fBGBlastWave_Integrand_num = NULL;
static TF1* fBGBlastWave_Integrand_den = NULL;
Double_t BGBlastWave_Integrand(const Double_t* x, const Double_t* p)
{

  /* 
  x[0] -> r (radius)
  p[0] -> mT (transverse mass)
  p[1] -> pT (transverse momentum)
  p[2] -> beta_max (surface velocity)
  p[3] -> T (freezout temperature)
  p[4] -> n (velocity profile)
  */

  Double_t r = x[0];
  Double_t mt = p[0];
  Double_t pt = p[1];
  Double_t beta_max = p[2];
  Double_t temp_1 = 1. / p[3];
  Double_t n = p[4];

  Double_t beta = beta_max * TMath::Power(r, n);
  if (beta > 0.9999999999999999)
    beta = 0.9999999999999999;
  Double_t rho = TMath::ATanH(beta);
  Double_t argI0 = pt * TMath::SinH(rho) * temp_1;
  if (argI0 > 700.)
    argI0 = 700.;
  Double_t argK1 = mt * TMath::CosH(rho) * temp_1;
  //  if (argI0 > 100 || argI0 < -100)
  //    printf("r=%f, pt=%f, beta_max=%f, temp=%f, n=%f, mt=%f, beta=%f, rho=%f, argI0=%f, argK1=%f\n", r, pt, beta_max, 1. / temp_1, n, mt, beta, rho, argI0, argK1);
  return r * mt * TMath::BesselI0(argI0) * TMath::BesselK1(argK1);
}

Double_t BGBlastWave_Func(const Double_t* x, const Double_t* p)
{
  /* dN/dpt */

  Double_t pt = x[0];
  Double_t mass = p[0];
  Double_t mt = TMath::Sqrt(pt * pt + mass * mass);
  Double_t beta_max = p[1];
  Double_t temp = p[2];
  Double_t n = p[3];
  Double_t norm = p[4];

  if (!fBGBlastWave_Integrand)
    fBGBlastWave_Integrand = new TF1("fBGBlastWave_Integrand", BGBlastWave_Integrand, 0., 1., 5);
  fBGBlastWave_Integrand->SetParameters(mt, pt, beta_max, temp, n);
  //  Double_t integral = fBGBlastWave_Integrand->Integral(0., 1., (Double_t*)0, 1.e-6);
  Double_t integral = fBGBlastWave_Integrand->Integral(0., 1., 1.e-6);
  return norm * pt * integral;
}

TF1* BGBlastWave(const Char_t* name, Double_t mass, Double_t beta_max = 0.6, Double_t temp = 0.1, Double_t n = 0.9, Double_t norm = 1.e6)
{

  TF1* fBGBlastWave = new TF1(name, BGBlastWave_Func, 0., 10., 5);
  fBGBlastWave->SetParameters(mass, beta_max, temp, n, norm);
  fBGBlastWave->SetParNames("mass", "beta_max", "T", "n", "norm");
  fBGBlastWave->FixParameter(0, mass);
  fBGBlastWave->SetParLimits(1, 0.01, 0.99);
  fBGBlastWave->SetParLimits(2, 0.01, 1.);
  fBGBlastWave->SetParLimits(3, 0.01, 50.);
  return fBGBlastWave;
}

#endif
