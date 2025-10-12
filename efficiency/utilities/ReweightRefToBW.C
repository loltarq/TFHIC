// Load order in ROOT:
//   .L ReweightEfficiency.C+              // your original macro MUST be loaded first
//   .x ReweightRefToBW.C+                 // compile this wrapper
//
// Single cent:
//   ReweightRefToBW("AxE_p_ITS-TPC_vsPt_Prm.root","p_den","p_num","p_AxE",
//                   "spectra_pp_CE-gs_ALL+D_1-1.6-3.root",
//                   "k_3/cent_0/h_p_pdg2212",
//                   "AxE_p_bwRew_c0.root");
//
// All cents into one file:
//   ReweightRefToBW_AllCents("AxE_p_ITS-TPC_vsPt_Prm.root","p_den","p_num","p_AxE",
//                            "spectra_pp_CE-gs_ALL+D_1-1.6-3.root",
//                            "k_3/cent_%d/h_p_pdg2212",
//                            0,9,"AxE_p_bwRew_allc.root");

#include "TFile.h"
#include "TH1.h"
#include "TGraph.h"
#include "TF1.h"
#include "TString.h"
#include "TSystem.h"   // for gSystem (optional if you use tmp files)

#include <vector>
#include <algorithm>
#include <cstdint>
#include <iostream>

// Convert any TH1 (D/F) to a fresh TH1F with identical binning/contents
static TH1F* CloneAsTH1F(const TH1* hin, const char* newname) {
  if (!hin) return nullptr;
  const int nb = hin->GetNbinsX();
  std::vector<double> edges(nb+1);
  for (int b=1; b<=nb; ++b) edges[b-1] = hin->GetXaxis()->GetBinLowEdge(b);
  edges[nb] = hin->GetXaxis()->GetBinUpEdge(nb);
  TH1F* h = new TH1F(newname, hin->GetTitle(), nb, edges.data());
  for (int b=1; b<=nb; ++b) {
    h->SetBinContent(b, hin->GetBinContent(b));
    h->SetBinError(b,   hin->GetBinError(b));
  }
  h->GetXaxis()->SetTitle(hin->GetXaxis()->GetTitle());
  h->GetYaxis()->SetTitle(hin->GetYaxis()->GetTitle());
  return h;
}

// ---- forward declaration from ReweightEfficiency.C ----
extern int ReweightEfficiency(TH1F* M, TF1* F, TH1F* G, TH1F* R,
                              TFile* file, int test, int save, double tolerance);

// ---------- TF1 prior from TH1 ----------
struct PriorH1Ctx { TH1* h; double norm; };
static PriorH1Ctx gPriorH1Ctx;

static Double_t prior_h1_eval(Double_t* x, Double_t* /*p*/) {
  TH1* h = gPriorH1Ctx.h;
  if (!h) return 0.0;
  int b = h->GetXaxis()->FindBin(x[0]);
  if (b < 1 || b > h->GetNbinsX()) return 0.0;
  return gPriorH1Ctx.norm * h->GetBinContent(b);
}

static TF1* TF1FromTH1(TH1* h, const char* name="prior_from_h") {
  if (!h) return nullptr;
  // normalise to area 1
  double area = 0.0;
  for (int b=1;b<=h->GetNbinsX();++b)
    area += h->GetBinContent(b) * h->GetXaxis()->GetBinWidth(b);
  gPriorH1Ctx.h = h;
  gPriorH1Ctx.norm = (area>0 ? 1.0/area : 1.0);
  TF1* f = new TF1(name, prior_h1_eval, h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax(), 0);
  f->SetNpx(1000);
  f->SetTitle("prior(step TH1)");
  return f;
}

// ---------- TF1 prior from TGraph ----------
struct PriorGrCtx { std::vector<double> xs, ys; double invA; };
static PriorGrCtx gPriorGrCtx;

static Double_t prior_gr_eval(Double_t* x, Double_t* /*p*/) {
  const auto& xs = gPriorGrCtx.xs;
  const auto& ys = gPriorGrCtx.ys;
  if (xs.empty()) return 0.0;
  double xx = x[0];
  if (xx <= xs.front() || xx >= xs.back()) return 0.0;
  auto it = std::upper_bound(xs.begin(), xs.end(), xx);
  int j = int(it - xs.begin());
  int i = j - 1;
  double t = (xx - xs[i]) / (xs[j] - xs[i] + 1e-12);
  double y = (1.0 - t) * ys[i] + t * ys[j];
  return gPriorGrCtx.invA * y;
}

static TF1* TF1FromTGraph(TGraph* g, const char* name="prior_from_gr") {
  if (!g) return nullptr;
  gPriorGrCtx.xs.resize(g->GetN());
  gPriorGrCtx.ys.resize(g->GetN());
  for (int i=0;i<g->GetN();++i) g->GetPoint(i, gPriorGrCtx.xs[i], gPriorGrCtx.ys[i]);
  double area = 0.0;
  for (int i=1;i<g->GetN();++i)
    area += 0.5 * (gPriorGrCtx.ys[i-1] + gPriorGrCtx.ys[i]) * (gPriorGrCtx.xs[i] - gPriorGrCtx.xs[i-1]);
  gPriorGrCtx.invA = (area>0 ? 1.0/area : 1.0);
  TF1* f = new TF1(name, prior_gr_eval, gPriorGrCtx.xs.front(), gPriorGrCtx.xs.back(), 0);
  f->SetNpx(2000);
  f->SetTitle("prior(linear TGraph)");
  return f;
}

// ---------- Build a coarse M compatible with fine MC binning ----------
static TH1F* MakeCoarseMCompatibleWith(const TH1F* Gfine, TF1* prior,
                                       int group=2, const char* name="M_prior") {
  std::vector<double> edges;
  auto* ax = Gfine->GetXaxis();
  const int nb = Gfine->GetNbinsX();
  for (int b=1; b<=nb; b+=group) edges.push_back(ax->GetBinLowEdge(b));
  edges.push_back(ax->GetBinUpEdge(nb));
  TH1F* M = new TH1F(name, "dummy measured (BW prior);p_{T};(a.u.)",
                     (int)edges.size()-1, edges.data());
  for (int b=1; b<=M->GetNbinsX(); ++b) {
    double A = M->GetXaxis()->GetBinLowEdge(b);
    double B = M->GetXaxis()->GetBinUpEdge(b);
    double val = prior->Integral(A,B) / (B-A); // average density in bin
    M->SetBinContent(b, val);
    M->SetBinError(b, 0.0);
  }
  return M;
}

// -------- single-centrality wrapper --------
int ReweightRefToBW(const char* refFile,
                    const char* denName, const char* numName, const char* axeName,
                    const char* bwRoot, const char* bwObj,
                    const char* outFile,
                    int group=2,   // how many fine MC bins per coarse M bin
                    int test=1,    // pass test=1 to reduce fit overhead
                    int save=2,    // save inputs + i0,i1 outputs
                    double tol=1e-3)
{
  TFile fref(refFile,"READ");
  if (fref.IsZombie()) { std::cerr<<"Cannot open "<<refFile<<"\n"; return 1; }
  auto* Gany = dynamic_cast<TH1*>(fref.Get(denName));
  auto* Rany = dynamic_cast<TH1*>(fref.Get(numName));
  auto* Haxeany = dynamic_cast<TH1*>(fref.Get(axeName));
  if (!Gany || !Rany) { std::cerr<<"Missing den/num in "<<refFile<<"\n"; return 2; }
  auto* Gfine = CloneAsTH1F(Gany, (std::string(denName)+"_F").c_str());
  auto* Rfine = CloneAsTH1F(Rany, (std::string(numName)+"_F").c_str());
  auto* Haxe  = CloneAsTH1F(Haxeany, (std::string(numName)+"_F").c_str()); // optional

  TFile fbw(bwRoot,"READ");
  if (fbw.IsZombie()) { std::cerr<<"Cannot open "<<bwRoot<<"\n"; return 3; }
  TObject* sobj = fbw.Get(bwObj);
  if (!sobj) { std::cerr<<"BW obj "<<bwObj<<" not found\n"; return 4; }

  TF1* prior = nullptr;
  if (auto* hS = dynamic_cast<TH1*>(sobj)) prior = TF1FromTH1(hS);
  else if (auto* gS = dynamic_cast<TGraph*>(sobj)) prior = TF1FromTGraph(gS);
  else { std::cerr<<"BW obj must be TH1 or TGraph\n"; return 5; }

  TH1F* M = MakeCoarseMCompatibleWith(Gfine, prior, group, "M_prior");

  TFile* fout = new TFile(outFile, "RECREATE");
  int status = ReweightEfficiency(M, prior, Gfine, Rfine, fout, test, save, tol);
  if (status!=0) std::cerr<<"ReweightEfficiency returned status "<<status<<"\n";

  auto* hEff = (TH1F*) Gfine->Clone("AxE_reweighted_BW");
  hEff->SetTitle("A#times#epsilon reweighted to BW prior; p_{T}; A#times#epsilon");
  for (int b=1; b<=hEff->GetNbinsX(); ++b) {
    double g = Gfine->GetBinContent(b);
    double r = Rfine->GetBinContent(b);
    hEff->SetBinContent(b, (g>0 ? r/g : 0.0));
    hEff->SetBinError(b, 0.0);
  }
  fout->cd();
  hEff->Write();
  if (Haxe) Haxe->Write("AxE_original_copy");
  fout->Close();

  std::cout<<"[OK] Wrote "<<outFile<<" with AxE_reweighted_BW\n";
  return status;
}

// -------- multi-centrality wrapper --------
// bwObjFmt must contain one %d, e.g. "k_3/cent_%d/h_p_pdg2212"
int ReweightRefToBW_AllCents(const char* refFile,
                             const char* denName, const char* numName, const char* /*axeName*/,
                             const char* bwRoot, const char* bwObjFmt,
                             int cmin, int cmax,
                             const char* outFileAll,
                             int group=2, int test=1, int save=1, double tol=1e-3)
{
  TFile fref(refFile,"READ");
  if (fref.IsZombie()) { std::cerr<<"Cannot open "<<refFile<<"\n"; return 1; }
  auto* GenD = dynamic_cast<TH1*>(fref.Get(denName));
  auto* RecD = dynamic_cast<TH1*>(fref.Get(numName));
  if (!GenD || !RecD) { std::cerr<<"Missing den/num in "<<refFile<<"\n"; return 2; }
  auto* GenF = CloneAsTH1F(GenD, (std::string(denName)+"_F").c_str());
  auto* RecF = CloneAsTH1F(RecD, (std::string(numName)+"_F").c_str());
  ScaleHistoByBinWidth(GenF);
  ScaleHistoByBinWidth(RecF);

  TFile fbw(bwRoot,"READ");
  if (fbw.IsZombie()) { std::cerr<<"Cannot open "<<bwRoot<<"\n"; return 3; }

  TFile fout(outFileAll,"RECREATE");
  for (int c=cmin; c<=cmax; ++c){
    auto* Gfine = (TH1F*) GenF->Clone(Form("Gfine_c%d",c));
    auto* Rfine = (TH1F*) RecF->Clone(Form("Rfine_c%d",c));

    TString obj = Form(bwObjFmt, c);
    TObject* sobj = fbw.Get(obj);
    if (!sobj) { std::cerr<<"[WARN] BW obj "<<obj<<" not found, skipping\n"; continue; }

    TF1* prior = nullptr;
    if (auto* hS = dynamic_cast<TH1*>(sobj)) prior = TF1FromTH1(hS);
    else if (auto* gS = dynamic_cast<TGraph*>(sobj)) prior = TF1FromTGraph(gS);
    else { std::cerr<<"[WARN] BW obj "<<obj<<" neither TH1 nor TGraph, skipping\n"; continue; }

    auto* M = MakeCoarseMCompatibleWith(Gfine, prior, group, Form("M_prior_c%d",c));

    fout.mkdir(Form("cent_%d", c));
    fout.cd(Form("cent_%d", c));
    int status = ReweightEfficiency(M, prior, Gfine, Rfine, &fout, test, save, tol);
    if (status!=0) std::cerr<<"[WARN] ReweightEfficiency status "<<status<<" at cent "<<c<<"\n";

    auto* hEff = (TH1F*) Gfine->Clone(Form("AxE_reweighted_BW_c%d",c));
    hEff->SetTitle(Form("A#times#epsilon reweighted to BW prior (cent %d); p_{T}; A#times#epsilon",c));
    for (int b=1; b<=hEff->GetNbinsX(); ++b) {
      double g = Gfine->GetBinContent(b);
      double r = Rfine->GetBinContent(b);
      hEff->SetBinContent(b, (g>0 ? r/g : 0.0));
      hEff->SetBinError(b, 0.0);
    }
    hEff->Write();
  }
  std::cout<<"[OK] Wrote "<<outFileAll<<" with AxE_reweighted_BW_c{c}\n";
  return 0;
}