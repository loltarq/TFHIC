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
void SetModel(ThermalModelBase * &model, ThermalParticleSystem *parts, const string& ensemble, const string& width_scheme)
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
void ComputeNchScanRange(vector<double>& NchScan, double NchMin=3., double NchMax=2000., int iter=100)
{
    double logNchMinK = log10(NchMinK), logNchMaxK = log10(NchMaxK);
    double dlogNch = (logNchMaxK - logNchMinK)/(iter-1);

    for (double logNch = logNchMinK; logNch <= logNchMaxK + 1.e-3; Nch+=dlogNch)
    {
        NchScan.push_back(logNch);
    }
}

void ComputeRatios(vector<int> pid, vector<string> pname, ThermalParticleSystem& TPS, vector<double>& GCEratios, vector<double> k)
{
    ThermalModelCanonical CEmodel(&TPS);
    CEmodel.SetTemperature(0.155);
    CEmodel.SetBaryonChemicalPotential(0.);
    CEmodel.SetElectricChemicalPotential(0.);
    CEmodel.SetStrangenessChemicalPotential(0.);
    CEmodel.SetCharmChemicalPotential(0.);
    CEmodel.SetBaryonCharge(0);
    CEmodel.SetElectricCharge(0);
    CEmodel.SetStrangeness(0);

    //for Strangeness Canonical Ensemble treat B and Q grand-canonically
    CEmodel.ConserveBaryonCharge(false);
    CEmodel.ConserveElectricCharge(false);
    CEmodel.ConserveStrangeness(true);

    if(useWidth)
        CEmodel.SetUseWidth(ThermalParticle::eBW);
    else
        CEmodel.SetUseWidth(ThermalParticle::ZeroWidth);

    CEmodel.SetStatistics(useQStats);
    if(useQStats)//set use of quantum statistics only for mesons!
    {
        for(int i=0; i<CEmodel.TPS()->Particles().size(); i++)
        {
            ThermalParticle &part = CEmodel.TPS()->Particle(i);
            if (part.BaryonCharge()!=0)
                part.UseStatistics(false);
        }
    }

    CEmodel.FillChemicalPotentials();

    for (int j=0; j<k.size(); j++)
    {
        vector<double> Vscan;
        ComputeVscanRangeK(Vscan, k[j]);

        ofstream fout("../out/piRatios_CEscan" + to_string(static_cast<int>(k[j])) + ".dat", ofstream::out | ofstream::trunc);

        fout << setw(15) << "dNpi/dy";
        fout << setw(15) << "V[fm^3]";

        for (int i=0; i<pname.size(); i++)
        {
            fout << setw(15) << pname[i];
        }

        fout << endl;

        for (int i=0; i<Vscan.size(); i++)
        {
            double V = pow(10., Vscan[i]);
            //cout << V << endl;
            CEmodel.SetVolume(V);
            CEmodel.SetCanonicalVolume(V);
            CEmodel.CalculateDensities();

            fout << setw(15) << 2. * CEmodel.GetDensity(211, 1) * V / k[j]; //charged pions dN/dy, thus multiply by 2 to include pi- and rescale k. WEIRD
            fout << setw(15) << V;

            for (int i=0; i<pname.size(); i++)
            {
                fout << setw(15) << CEmodel.GetDensity(pid[i], 1) / GCEratios[i] / CEmodel.GetDensity(211, 1);
            }

            fout << endl;
        }

        fout.close();   

    }
    
}

int main(int argc, char *argv[])
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

    //Compute GCE ratios first
    vector<double> GCEratios;
    ComputeGCEratios(GCEratios, pdgsCE, namesCE, particles);

    //initialize k vector for V scan
    vector<double> k = {1., 3., 6.};
    //Compute CE ratios wrt to pi and GCE using k values
    ComputeCEratios(pdgsCE, namesCE, particles, GCEratios, k);
      
}