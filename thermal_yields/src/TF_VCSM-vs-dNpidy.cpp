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

void ComputeGCEratios(vector<double>& yields, vector<int> pid, vector<string> pname, ThermalParticleSystem& TPS)
{
    ThermalModelIdeal GCEmodel(&TPS);
    GCEmodel.SetTemperature(0.155);
    GCEmodel.SetBaryonChemicalPotential(0.);
    GCEmodel.SetElectricChemicalPotential(0.);
    GCEmodel.SetStrangenessChemicalPotential(0.);
    //GCEmodel.SetCharmChemicalPotential(0.);

    if(useWidth)
        GCEmodel.SetUseWidth(ThermalParticle::eBW);
    else
        GCEmodel.SetUseWidth(ThermalParticle::ZeroWidth);

    GCEmodel.SetStatistics(useQStats);
    if(useQStats)//set use of quantum statistics only for mesons!
    {
        for(int i=0; i<GCEmodel.TPS()->Particles().size(); i++)
        {
            ThermalParticle &part = GCEmodel.TPS()->Particle(i);
            if (part.BaryonCharge()!=0)//meson have 0 baryon charge!
                part.UseStatistics(false);
        }
    }

    GCEmodel.FillChemicalPotentials();
    GCEmodel.CalculateDensities();

    for(int i=0; i<pname.size(); i++)
    {
        yields.push_back(GCEmodel.GetDensity(pid[i], 1) / GCEmodel.GetDensity(211, 1));//second argument of GetDensity is feeddown flag, 211 is pion+
        //cout << yields[i] << endl;
    }
}

void ComputeVscanRangeK(vector<double>& Vscan, double k, double Vmin=1., double Vmax=15000., int slices=30)//k determines Vc = k*dV/dy
{
    double VminK = Vmin * k;
    double VmaxK = Vmax * k;
    double logVminK = log10(VminK), logVmaxK = log10(VmaxK);
    double dlogV = (logVmaxK - logVminK)/(slices-1);

    for (double V = logVminK; V <= logVmaxK; V+=dlogV)
    {
        Vscan.push_back(V);
    }
}

void ComputeCEratios(vector<int> pid, vector<string> pname, ThermalParticleSystem& TPS, vector<double>& GCEratios, vector<double> k)
{
    ThermalModelCanonical CEmodel(&TPS);
    CEmodel.SetTemperature(0.155);
    CEmodel.SetBaryonChemicalPotential(0.);
    CEmodel.SetElectricChemicalPotential(0.);
    CEmodel.SetStrangenessChemicalPotential(0.);
    //CEmodel.SetCharmChemicalPotential(0.);
    CEmodel.SetBaryonCharge(0);
    CEmodel.SetElectricCharge(0);
    CEmodel.SetStrangeness(0);

    CEmodel.ConserveBaryonCharge(true);
    CEmodel.ConserveElectricCharge(true);
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
    
//   vector<ThermalParticle> particles_vector = particles.Particles();

//   for (int i=0; i<particles_vector.size(); i++)
//   {
//       cout << particles_vector[i].Name() << " , " << particles_vector[i].PdgId() << endl;
//   }

    
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