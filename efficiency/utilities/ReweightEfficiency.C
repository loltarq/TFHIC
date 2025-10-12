#include "functions.h"
#include "TFile.h"
#include "TH1.h"
#include "TGraph.h"
#include "TF1.h"
#include "TMath.h"

void isCompatibleBinning(TH1F* h0,TH1F* h1);
void isSameBinning(TH1F* h0,TH1F* h1);
void rebin(TH1F** h);
void do_fit(TH1F* m,TF1* f,int test);
void ScaleHistoByBinWidth(TH1F * hin = 0x0, Bool_t debugMode = 0);
int ReweightEfficiency(TH1F* M,TF1* F,TH1F* G,TH1F* R,TFile* file,int test=0,int save=2,double tolerance=0.001);

int runReweightEfficiency()
{
  TFile * fMC = TFile::Open("eff_final_default_LowBdca.root");
  TFile * fCorrSpectra[5];  
  TH1F * hCorrSpectra_syst_stat[5];
  TH1F * hPhiGen[5];
  TH1F * hPhiRec[5];
  TH1F * hNewEff[5];
  TH1F * hNewGen[5];
  TH1F * hNewRec[5];
  TString newname = Form("reweight_%s", fMC->GetName()); //newname.ReplaceAll("A1", "A3");
  TFile * fout = new TFile(newname.Data(), "recreate");
  
  for (int ic = 0; ic<5; ic++){
    fCorrSpectra[ic] = TFile::Open(Form("/Users/fbellini/alice/resonances/RsnAnaRun2/phiXeXe/final/systematics/finalWsyst_smooth1_04dec20_%i.root", ic));
    if (!fCorrSpectra[ic]) return 1;
    hCorrSpectra_syst_stat[ic] = (TH1F *)fCorrSpectra[ic]->Get(Form("hCorrected_%i%i_syst_stat",ic,ic));
    
    //hack to be used when using merged centrality bins for efficiency: e.g. 30-90%
    int icmc = ic;
    if (ic>2) icmc = 3;

    hPhiGen[ic] = (TH1F *) fMC->Get(Form("hMotherCounts%i", icmc));
    hPhiGen[ic]->SetName(Form("hPhiGen%i", ic));
    ScaleHistoByBinWidth(hPhiGen[ic]);
    hPhiRec[ic] = (TH1F *) fMC->Get(Form("hTrueCounts%i", icmc));
    hPhiRec[ic]->SetName(Form("hPhiRec%i", ic));
    ScaleHistoByBinWidth(hPhiRec[ic]);

    //Define functions for fit
    Double_t temp = 0.17; 
    Double_t norm = 1.0;
    Double_t npar = 5.0;
    Double_t beta_max = 0.9;
    Double_t pdgMass = 1.01946;
    TF1 * myLevyTsallis = LevyTsallis("levy", pdgMass, npar, temp, norm);
    TF1 * myBGBlastWave = BGBlastWave("bgbw", pdgMass, beta_max, temp, npar, norm);
    
    float fitRmin = 0.5; float fitRmax = 10.0;
    if (ic==4) {fitRmin = 0.7; fitRmax = 7.0;}
    hCorrSpectra_syst_stat[ic]->Fit(myBGBlastWave, "IRSq","same", fitRmin, fitRmax);

    //REWEIGHT EFFICIENCY
    ReweightEfficiency(hCorrSpectra_syst_stat[ic], myBGBlastWave, hPhiGen[ic], hPhiRec[ic], fout, 0, 2, 0.0001);
    hNewGen[ic] = (TH1F *)fout->Get(Form("hPhiGen%i_rebin_i1", ic));
    hNewRec[ic] = (TH1F *)fout->Get(Form("hPhiRec%i_rebin_i1", ic));
    hNewEff[ic] = (TH1F *) (hNewRec[ic]->Clone(Form("hEffVsPt%i", ic)));
    hNewEff[ic]->Divide(hNewEff[ic], hNewGen[ic], 1.0, 1.0, "B");
    if (!fout->IsOpen()) fout = TFile::Open(newname.Data(), "update");
    else fout->cd();
    hNewEff[ic]->Write();

  }

  return 0;
}


int ReweightEfficiency(TH1F* M,TF1* F,TH1F* G,TH1F* R,TFile* file,int test,int save,double tolerance){
  /* This macro implements an iterative procedure to re-weight efficiencies.
     
     Input Parameters:
     M: histogram containing the (fully corrected) measured pT spectrum (d^2N/dpTdy).  Its uncertainties should be the sum in quadrature of the statistical uncertainties and the systematic uncertainties.  (Check if you should exclude systematic uncertainties that are not correlated between pT bins.)
     F: fit of histogram M (do the fit before passing the function to this macro, the fit to the input histogram M will not be redone inside this macro), see also important NOTE 6 below
     G: the generated pT spectrum from simulation, i.e., the denominator of the efficiency calculation; see NOTE 4 below
     R: the reconstructed pT spectrum from simulation, i.e., the numerator of the efficiency calculation; see NOTE 4 below
     file: file to which results may be saved (optional)
     test: run the macro in test mode (disables fit options M and E to speed the fitting procedure)
     save: 0: do not save anything, 1: save only the final results, 2: save input, intermediate steps, and final results
     tolerance: The macro stops when the fractional changes in the measured pT spectrum between consecutive iterations fall below this value.  2-3 iterations should be sufficient for this to happen if tolerance=0.001.

     Saved Objects:
     For each iteration (if save=1) or the last iteration (if save=2), the following objects are saved:
     - The measured histogram, corrected to account for the reweighted efficiency
     - The fit function (see NOTE 7)
     - The correction factor (see NOTE 3)
     - The generated and reconstructed histograms
     - The generated and reconstructed histograms, rebinned (note: these are not scaled by the bin width; if you want to plot them, you should scale them by dpT yourself.)

     NOTES:
     1.) The histograms M, G, and R and the function F are all modified in this macro to contain the final results (see also note 7).
     2.) If results are saved, each histogram or function name will contain the suffix "_iX" where X is an integer.  This gives the iteration that corresponds to the saved object.  "_i0" is iteration 0: the input objects.
     3.) The histograms stored in the array c and named "[name of M]_correction_iX" are correction factors: the ratio of M(iteration X) to M(iteration 0).  Multiplying M(iteration 0) by this correction factor gives the final version of M, adjusted for the re-weighted efficiency.  This can be useful if you store the separate uncertainties of M (statistical and systematic) in different TH1 or TGraph objects.  Alternatively, you can calculate the reweighted efficiency from the final iterations of G and R (see note 5).
     4.) The histograms G and R should have fine bins (100 MeV is probably OK).  They SHOULD NOT have the same binning as histogram M; if the binning is the same, then the calculation will not work.  Instead, use finer binning than M.  You should, however, make sure that there are no bins in histograms G and R that span a bin boundary in histogram M.  Histogram R must have the same binning as histogram G.
     5.) This macro does not store the efficiency.  Of course, to get the efficiency, you just need to take the ratio of histograms R and G.  The output measured histogram (which will be stored in pointer M) is already corrected by the re-weighted efficiency.  Do not double-correct it.
     6.) Some care must be taken with the fit function F.  You may not be able to use a function that has been saved to a file.  If the saved function does not contain the function's formula, it will not work properly with the ROOT fitter.  To check, do F->GetTitle().  If the result is the explicit mathematicl formula for the function, like "[0]*exp(x*[1])" (an exponential in pT), then the function should be OK.  If the result is some more abstract name, the function was probably defined using a pointer to a user-defined function (the standard implementation of the blast-wave function is such a case).  In this case, you cannot use a function that has been saved to a file.  Instead, you must redefine the function in the macro that calls ReweightEfficiency.  When in doubt, do M->Fit(F,"RI") and see if the fitter behaves as you would expect.  If you need to fix or constrain any parameters of function F, do so before passing the function to this macro, or modify the do_fit function at the bottom of this file.
     7.) The final saved version of F is not the fit to the final saved version of M.  It is a fit to the next-to-last version of M.  You must fit the final version of M yourself.

    EXAMPLE: Here is some example code that may be useful:

    TFile* f1=TFile::Open("measured_file.root");//open the file that contains your measured histogram
    TH1F* M=(TH1F*) f1->Get("measured_histogram");//get your measured histogram, the systematic uncertainties should be the sum in quadrature of the statistical and systematic uncertainties (but you may want to exclude sources of systematic uncertainty that are correlated between pT bins)

    TF1* F=new TF1(***);//define the fit function
    F->SetParameter(*,1.23456);//fix the mass parameter of the function (assuming your function has a mass parameter)
    F->FixParameter(*,1.23456);
    F->SetParameter(*,*);//set the other parameters
    //Note that you may not be able to use a TF1 stored in a file for this purpose.  Please read note 6 above.

    TFile* f2=TFile::Open("simulated_file.root");//open the file that contains your simulated histograms
    TH1F* G=(TH1F*) f2->Get("generated_histogram");//get your generated histogram
    TH1F* R=(TH1F*) f2->Get("reconstructed_histogram");//get your reconstructed histogram

    TFile* f3=new TFile("output_file.root","RECREATE","HistoFile");//open the new file that will contain your output

    gROOT->LoadMacro("*path/PWGLF/RESONANCES/macros/utils/ReweightEfficiency.C");

    ReweightEfficiency(M,F,G,R,f3,1);//run the macro

    //Take the ratio R/G to get the reweighted efficiency; store it in f3.
    //The other objects stored in f3 can be useful, especially for plotting, but are not required.

    f1->Close();
    f2->Close();
    f3->Close();
  */

  if(!M){cerr<<"Error in ReweightEfficiency(): missing input: measured pT spectrum"<<endl; return 1;}
  if(!F){cerr<<"Error in ReweightEfficiency(): missing input: fit of measured pT spectrum"<<endl; return 1;}
  if(!G){cerr<<"Error in ReweightEfficiency(): missing input: generated pT spectrum from simulation"<<endl; return 1;}
  if(!R){cerr<<"Error in ReweightEfficiency(): missing input: reconstructed pT spectrum from simulation"<<endl; return 1;}

  char mname[500]; sprintf(mname,M->GetName());
  char fname[500]; sprintf(fname,F->GetName());
  char gname[500]; sprintf(gname,G->GetName());
  char rname[500]; sprintf(rname,R->GetName());

  cout<<"using ReweightEfficiency():\n  M="<<mname<<"\n  F="<<fname<<"\n  G="<<gname<<"\n  R="<<rname<<endl;

  if(save && !file){cerr<<"Warning in ReweightEfficiency(): flag save="<<save<<" but no output file given"<<endl; save=0;}
  if(test) cerr<<"Warning in ReweightEfficiency(): running in test mode (not using fit options M or E)"<<endl;

  //check that the histogram binning is correct (see note 3).
  isCompatibleBinning(G,M);
  isSameBinning(G,R);

  int i,j,status=0;
  int imax=10;//maximum number of iterations
  double A,B,y,w,g0,g1,r0,r1,diff;

  TH1F *m[10],*g[10][2],*r[10][2],*c[10];
  F->SetName(Form("%s_i0",fname));

  for(i=0;i<imax;i++){//iterate
    g[i][0]=(TH1F*) G->Clone(Form("%s_i%i",gname,i));
    g[i][1]=(TH1F*) M->Clone(Form("%s_rebin_i%i",gname,i));
    r[i][0]=(TH1F*) R->Clone(Form("%s_i%i",rname,i));
    r[i][1]=(TH1F*) M->Clone(Form("%s_rebin_i%i",rname,i));
    c[i]=(TH1F*) M->Clone(Form("%s_correction_i%i",mname,i));

    //re-weight simulated histograms
    if(i){
      for(j=1;j<=g[i][0]->GetNbinsX();j++){
	A=g[i][0]->GetXaxis()->GetBinLowEdge(j);
	B=g[i][0]->GetXaxis()->GetBinLowEdge(j+1);
	w=F->Integral(A,B)/(B-A);
	y=g[i][0]->GetBinContent(j);
	if(fabs(y)>1.e-30){
	  g[i][0]->SetBinContent(j,w);
	  w/=y;
	  g[i][0]->SetBinError(j,w*g[i][0]->GetBinError(j));
	  r[i][0]->SetBinContent(j,w*r[i][0]->GetBinContent(j));
	  r[i][0]->SetBinError(j,w*r[i][0]->GetBinError(j));
	}else{
	  r[i][0]->SetBinContent(j,0.);
	  r[i][0]->SetBinError(j,0.);
	}
      }
    }

    //rebin simulated histograms
    rebin(g[i]);
    rebin(r[i]);

    m[i]=(TH1F*) M->Clone(Form("%s_i%i",mname,i));

    if(!i) continue;

    //adjust measured histogram
    for(j=1;j<=m[i]->GetNbinsX();j++){
      g0=g[0][1]->GetBinContent(j);
      r0=r[0][1]->GetBinContent(j);
      g1=g[i][1]->GetBinContent(j);
      r1=r[i][1]->GetBinContent(j);
      if(fabs(g0)>1.e-30 && fabs(r1)>1.e-30){
	w=r0/g0*g1/r1;//ratio of the old (input) efficiency to the new efficiency
	m[i]->SetBinContent(j,w*m[i]->GetBinContent(j));
	m[i]->SetBinError(j,w*m[i]->GetBinError(j));
	c[i]->SetBinContent(j,w);
	c[i]->SetBinError(j,0.);
      }else{
	m[i]->SetBinContent(j,0.);
	m[i]->SetBinError(j,0.);
	c[i]->SetBinContent(j,0.);
	c[i]->SetBinError(j,0.);
      }
    }

    //Check the adjusted measured histogram.  Are the differences between this and the previous iteration small enough (<tolerance) that the process can stop?
    diff=0.;
    for(j=1;j<=m[i]->GetNbinsX();j++){
      w=m[i-1]->GetBinContent(j);
      if(w<1.e-10) continue;
      w=fabs(m[i]->GetBinContent(j)/w-1.);
      if(w>diff) diff=w;
    }
    cerr<<"  iteration "<<i<<", diff="<<diff<<endl;
    if(diff<tolerance){cerr<<"re-weighting finished successfully"<<endl; break;}
    if(i==imax-1){cerr<<"Error in ReweightEfficiency(): maximum number of iterations ("<<imax<<") reached and results have not stabilized, diff="<<diff<<endl; status=2; break;}

    //new fit
    F->SetName(Form("%s_i%i",fname,i));
    do_fit(m[i],F,test);
    if(save==2){
      file->cd();
      F->Write();
    }
  }

  if(save){
    file->cd();
    for(j=0;j<=i;j++){
      if(save==1 && j<i) continue;
      if(save==1 && j==i) F->Write();
      m[j]->Write();
      g[j][0]->Write();
      g[j][1]->Write();
      r[j][0]->Write();
      r[j][1]->Write();
      if(j) c[j]->Write();
    }
  }

  //store the final values in the pointers M, G, and R (F has already been modified)
  for(j=1;j<=M->GetNbinsX();j++){
    M->SetBinContent(j,m[i]->GetBinContent(j));
    M->SetBinError(j,m[i]->GetBinError(j));
  }

 for(j=1;j<=G->GetNbinsX();j++){
    G->SetBinContent(j,g[i][0]->GetBinContent(j));
    G->SetBinError(j,g[i][0]->GetBinError(j));
  }

 for(j=1;j<=R->GetNbinsX();j++){
    R->SetBinContent(j,r[i][0]->GetBinContent(j));
    R->SetBinError(j,r[i][0]->GetBinError(j));
  }

 return status;//if status is 0, everything is OK
}


void isCompatibleBinning(TH1F* h0,TH1F* h1)
{
  //Is there any bin in h0 that spans a bin boundary in h1?
  int j,k,b1,b2;
  double A,B,x;

  for(j=1;j<=h0->GetNbinsX();j++){
    A=h0->GetXaxis()->GetBinLowEdge(j);
    b1=h1->GetXaxis()->FindBin(A);
    B=h0->GetXaxis()->GetBinLowEdge(j+1);
    b2=h1->GetXaxis()->FindBin(B);

    if(b1==b2) continue;
    else{
      if(b1==b2-1){
	x=h1->GetXaxis()->GetBinLowEdge(b2);
	if(fabs(x-A)<1.e-6 || fabs(x-B)<1.e-6) continue;
      }

      cerr<<"Error in ReweightEfficiency(): mismatched bins detected for histograms "<<h0->GetName()<<" and "<<h1->GetName()<<endl;
      break;
    }
  }

  return;
}

  
void isSameBinning(TH1F* h0,TH1F* h1){
  //do h0 and h1 have the same binning?
  int j;
  double A0,B0,A1,B1;

  for(j=1;j<=h0->GetNbinsX();j++){
    A0=h0->GetXaxis()->GetBinLowEdge(j);
    B0=h0->GetXaxis()->GetBinLowEdge(j+1);
    A1=h1->GetXaxis()->GetBinLowEdge(j);
    B1=h1->GetXaxis()->GetBinLowEdge(j+1);

    if(fabs(A0-A1)<1.e-6 && fabs(B0-B1)<1.e-6) continue;
    else{
      cerr<<"Error in ReweightEfficiency(): mismatched bins detected for histograms "<<h0->GetName()<<" and "<<h1->GetName()<<endl;
      break;
    }
  }

  return;
}


void rebin(TH1F** h){
  //rebins from h[0] (small bins) to h[1] (larger bins)
  int j,k;
  double A,B,x,v,u;

  for(k=1;k<=h[1]->GetNbinsX();k++){
    A=h[1]->GetXaxis()->GetBinLowEdge(k);
    B=h[1]->GetXaxis()->GetBinLowEdge(k+1);
    v=u=0.;
    for(j=1;j<=h[0]->GetNbinsX();j++){
      x=h[0]->GetXaxis()->GetBinCenter(j);
      if(x<A || x>B) continue;
      v+=h[0]->GetBinContent(j);
      u+=pow(h[0]->GetBinError(j),2);
    }
    h[1]->SetBinContent(k,v);
    h[1]->SetBinError(k,sqrt(u));
  }
  return;
}


void do_fit(TH1F* m,TF1* f,int test){
  //modify this funciton if you have a special fitting procedure
  if (f->GetNpar()==0) return;  
  if(!test) m->Fit(f,"RNIEM");
  else m->Fit(f,"RNI");
  return;
}

void ScaleHistoByBinWidth(TH1F * hin, Bool_t debugMode)
{
  if (!hin) return;
  for (int j=1; j<hin->GetNbinsX()+1;j++){
    Double_t dummy = hin->GetBinContent(j) / hin->GetXaxis()->GetBinWidth(j);
    Double_t dummyErr = hin->GetBinError(j) / hin->GetXaxis()->GetBinWidth(j);
    hin->SetBinContent(j, dummy);
    hin->SetBinError(j, dummyErr);
  }
  if (debugMode) hin->Draw();
  return;
}



