#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>

#include "HRGBase.h"
#include "HRGEV.h"
#include "HRGFit.h"

#include "ThermalFISTConfig.h"

using namespace std;

#ifdef ThermalFIST_USENAMESPACE
using namespace thermalfist;
#endif

bool useWidth  = true;
bool useQStats = true;

//parameters dependence on multiplicity from 1906.03145
//temperature of chemical freeze out
double TchVsNch(double dNchdy)
{
    return 0.176 - 0.0026 * log(dNchdy);
}
//Strangeness saturation parameter
double GsVsNch(double dNchdy)
{
    return 1. - 0.25 * exp(-dNchdy / 59.);
}
//Canonical volume per unit of rapidity
double dVdyVsNch(double dNchdy)
{
    return 2.4 * dNchdy;
}

//unified template for statistical model set-up
//here we set all parameters not affected by gammaS fits, if any
void PrepareModel(ThermalModelBase * &model, ThermalParticleSystem *parts, const string& ensemble, const string& width_scheme)
{
    if (ensemble == "GCE")
        model = new ThermalModelIdeal(parts);
    else
    {
        model = new ThermalModelCanonical(parts);

        if (ensemble == "SCE")//Strangeness-canonical. Need cast to canonical daughter, base has no method to set exact conservation
        {

            static_cast<ThermalModelCanonical*>(model)->ConserveBaryonCharge(false);
            static_cast<ThermalModelCanonical*>(model)->ConserveElectricCharge(false);
        }

    }

    //starting temp value from fits
    model->SetTemperature(0.155);
    //chemical potentials to zero
    model->SetBaryonChemicalPotential(0.);
    model->SetElectricChemicalPotential(0.);
    model->SetStrangenessChemicalPotential(0.);
    model->SetCharmChemicalPotential(0.);
    //quantum numbers to 0
    model->SetBaryonCharge(0);
    model->SetElectricCharge(0);
    model->SetStrangeness(0);
    model->SetCharm(0);

    model->SetStatistics(useQStats);
    if(useQStats)//set use of quantum statistics only for mesons!
    {
        for(int i=0; i<model->TPS()->Particles().size(); i++)
        {
            ThermalParticle &part = model->TPS()->Particle(i);
            if (part.BaryonCharge()!=0)//meson have 0 baryon charge!
                part.UseStatistics(false);
        }
    }

    //if canonical
    //Calculates the range of quantum numbers values for which it is necessary to compute the canonical partition functions. 
    if (ensemble != "GCE")
        static_cast<ThermalModelCanonical*>(model)->CalculateQuantumNumbersRange();

    //resonance decay schemes
    if (width_scheme == "ZeroWidth")
        model->SetUseWidth(ThermalParticle::ZeroWidth);
    else if (width_scheme == "BWTwoGamma")
        model->SetUseWidth(ThermalParticle::BWTwoGamma);
    else
        model->SetUseWidth(ThermalParticle::eBW);

    //compute chemical potential of each species, from QNs and related chemical potentials
    model->FillChemicalPotentials();
}

//note: relation between Canonical Volume and multiplicity is linear in gammaS according to 1906.03145
void MultiplicityScanFill(vector<double>& NchScan, double NchMin=3., double NchMax=2000., int iter=100)
{
    double logNchMin = log10(NchMin), logNchMax = log10(NchMax);
    double dlogNch = (logNchMax - logNchMin)/(iter-1);

    for (double logNch = logNchMin; logNch <= logNchMax + 1.e-3; logNch+=dlogNch)
    {
        NchScan.push_back(logNch);
    }
}

void VScanFill(vector<double>& VScan, double k, double VMin=1., double VMax=15000., int iter=30)
{
    double VminK = VMin * k;
    double VmaxK = VMax * k;
    double logVminK = log10(VminK), logVmaxK = log10(VmaxK);
    double dlogV = (logVmaxK - logVminK)/(iter-1);

    for (double V = logVminK; V <= logVmaxK + 1.e-3; V+=dlogV)
    {
        VScan.push_back(V);
    }
}

void ComputeYieldRatios(vector<int> pid, vector<string> pname, ThermalModelBase * model, vector<double> k, const string& ensemble, bool GsFlag)
{
    for (int j=0; j<k.size(); j++)
    {
        if (!GsFlag)//no gammaS
        {
            vector<double> VScan;
            VScanFill(VScan, k[j]);

            ofstream fout("../out/piRatios_" + ensemble + "_scan_k" + to_string(static_cast<int>(k[j])) + ".dat", ofstream::out | ofstream::trunc);
            fout << setw(15) << "dNpi/dy";
            fout << setw(15) << "Vc[fm^3]";

            for (int i=0; i<pname.size(); i++)
            {
                fout << setw(15) << (pname[i] + "/pi");
            }

            fout << endl;
            
            for (int i=0; i<VScan.size(); i++) //here multiplicityscan is volume scan
            {
                double V = pow(10., VScan[i]);

                model->SetVolume(V);
                model->SetCanonicalVolume(V);
                model->CalculateDensities();

                fout << setw(15) << 2. * model->GetDensity(211, 1) * V / k[j]; //charged pions dN/dy, thus multiply by 2 to include pi- and rescale k.
                fout << setw(15) << V;

                for (int i=0; i<pname.size(); i++)
                { 
                    fout << setw(15) << model->GetDensity(pid[i], 1) / model->GetDensity(211, 1);
                }

                fout << endl;
            }

            fout.close();
        }
        else//gammaS
        {
            vector<double> MultiplicityScan;
            MultiplicityScanFill(MultiplicityScan);

            ofstream fout("../out/piRatios_gs_" + ensemble + "_scan_k" + to_string(static_cast<int>(k[j])) + ".dat", ofstream::out | ofstream::trunc);
            fout << setw(15) << "dNpi/dy";
            fout << setw(15) << "Tch[MeV]";
            fout << setw(15) << "dVdy[fm^3]";
            fout << setw(15) << "Vc[fm^3]";
            fout << setw(15) << "gammaS";

            for (int i=0; i<pname.size(); i++)
            {
                fout << setw(15) << pname[i] + "/pi";
            }

            fout << endl;

            for (int i=0; i<MultiplicityScan.size(); i++) //here multiplicityscan is on Nch, volume is determined by fitted parameters
            {
                double Nch = pow(10., MultiplicityScan[i]);

                double Tch = TchVsNch(Nch);
                double gammaS = GsVsNch(Nch);
                double dVdy = dVdyVsNch(Nch);
                double Vc = k[j] * dVdy;

                model->SetTemperature(Tch);
                model->SetGammaS(gammaS);
                model->SetVolume(dVdy);
                model->SetCanonicalVolume(Vc);
                model->CalculateDensities();

                fout << setw(15) << Nch;
                fout << setw(15) << Tch;
                fout << setw(15) << dVdy;
                fout << setw(15) << Vc;
                fout << setw(15) << gammaS;

                for (int i=0; i<pname.size(); i++)
                { 
                    fout << setw(15) << model->GetDensity(pid[i], 1) / model->GetDensity(211, 1);
                }

                fout << endl;
            }

            fout.close();
        }       
    }   
}

int main(int argc, char *argv[])
{
    if (argc<=2)
    {
                cout << "Not enough arguments provided" << endl;
                cout << "Required arguments, in order: Ensemble [GCE,CE,SCE],  GammaS model flag [0,1]" << endl;
    }

    if (argc>2)
    {

        ThermalParticleSystem particles(string(ThermalFIST_INPUT_FOLDER)+"/list/PDG2014/list.dat");

        //Considering specific hadrons
        vector<int> pdgsCE;
        vector<string> namesCE;

        //    pdgsCE.push_back(211);
        //    namesCE.push_back("pi+");

        pdgsCE.push_back(321);
        namesCE.push_back("K+");

        pdgsCE.push_back(3312);
        namesCE.push_back("Xi");

        pdgsCE.push_back(333);
        namesCE.push_back("phi");

        pdgsCE.push_back(2212);
        namesCE.push_back("p");

        pdgsCE.push_back(3334);
        namesCE.push_back("Omega");

        pdgsCE.push_back(3122);
        namesCE.push_back("Lambda");


        //initialize k vector for Volume scan
        vector<double> k = {3.};

        for (int i = 1; i<argc; i+=2)
        {
            ThermalModelBase * model;
            double gsflag = atoi(argv[i+1]);

            cout << "Computing ratios in " << argv[i] << " formulation, " << (gsflag == 0 ? "vanilla" : "gammaS") << endl;

            PrepareModel(model, &particles, argv[i], "eBW");
            ComputeYieldRatios(pdgsCE, namesCE, model, k, string(argv[i]), gsflag);

            cout << endl;
        }

    }
      
}