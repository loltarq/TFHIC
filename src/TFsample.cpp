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

void ComputeGCEyields(vector<double>& yields, vector<int> pid, vector<string> pname, ThermalParticleSystem& TPS)
{
    ThermalModelIdeal GCEmodel(&TPS);
    GCEmodel.SetTemperature(0.155);
    GCEmodel.SetBaryonChemicalPotential(0.);
    GCEmodel.SetElectricChemicalPotential(0.);
    GCEmodel.SetStrangenessChemicalPotential(0.);
    GCEmodel.SetCharmChemicalPotential(0.);

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
            if (part.BaryonCharge()!=0)
                part.UseStatistics(false);
        }
    }

    GCEmodel.FillChemicalPotentials();
    GCEmodel.CalculateDensities();

    for(int i=0; i<pname.size(); i++)
    {
        yields.push_back(GCEmodel.GetDensity(pid[i], 1));//second argument of GetDensity is feeddown flag
    }
}

void ComputeVscanRange(vector<double>& Vscan, double Vmin=4., double Vmax=30000., int slices=30)
{
    double logVmin = log10(Vmin), logVmax = log10(Vmax);
    double dlogV = (logVmax - logVmin)/(slices-1);

    for (double V = logVmin; V <= logVmax; V+=dlogV)
    {
        Vscan.push_back(V);
    }
}

void ComputeCEyields(vector<int> pid, vector<string> pname, ThermalParticleSystem& TPS, int flag, vector<double>& GCEyields)
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

    vector<double> Vscan;
    ComputeVscanRange(Vscan);

    ofstream fout("out/CEscan" + to_string(flag) + ".dat", ofstream::out | ofstream::trunc);

    fout << setw(15) << "V[fm^3]";
    
    for (int i=0; i<pname.size(); i++)
    {
        fout << setw(15) << pname[i];
    }

    fout << endl;

    for (int i=0; i<Vscan.size(); i++)
    {
        double V = pow(10, Vscan[i]);
        CEmodel.SetVolume(V);
        CEmodel.SetCanonicalVolume(V);

        fout << setw(15) << V;

        switch(flag)
        {
            case 0:
                CEmodel.ConserveBaryonCharge(true);
                CEmodel.ConserveElectricCharge(true);
                CEmodel.ConserveStrangeness(true);
                break;
            case 1:
                CEmodel.ConserveBaryonCharge(true);
                CEmodel.ConserveElectricCharge(true);
                CEmodel.ConserveStrangeness(false);  
                break; 
            case 2:
                CEmodel.ConserveBaryonCharge(true);
                CEmodel.ConserveElectricCharge(false);
                CEmodel.ConserveStrangeness(false); 
                break;
            case 3:
                CEmodel.ConserveBaryonCharge(false);
                CEmodel.ConserveElectricCharge(false);
                CEmodel.ConserveStrangeness(true);     
                break;
            default:
                break;
        }
        CEmodel.CalculateDensities();

        for (int i=0; i<pname.size(); i++)
        {
            fout << setw(15) << CEmodel.GetDensity(pid[i], 1) / GCEyields[i];
        }

        fout << endl;
    }

    fout.close();
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

    pdgsCE.push_back(111);
    namesCE.push_back("pi0");

    pdgsCE.push_back(211);
    namesCE.push_back("pi+");

    pdgsCE.push_back(321);
    namesCE.push_back("K+");

    pdgsCE.push_back(310);
    namesCE.push_back("K0S");

    pdgsCE.push_back(221);
    namesCE.push_back("eta");

    pdgsCE.push_back(323);
    namesCE.push_back("KStar+");

    pdgsCE.push_back(333);
    namesCE.push_back("phi");

    pdgsCE.push_back(2212);
    namesCE.push_back("p");

    pdgsCE.push_back(2112);
    namesCE.push_back("n");

    pdgsCE.push_back(3122);
    namesCE.push_back("Lambda");

    pdgsCE.push_back(3222);
    namesCE.push_back("Sigma+");

    pdgsCE.push_back(3112);
    namesCE.push_back("Sigma+");

    pdgsCE.push_back(3212);
    namesCE.push_back("Sigma0");

    pdgsCE.push_back(3322);
    namesCE.push_back("Ksi0");

    pdgsCE.push_back(3312);
    namesCE.push_back("Ksi-");

    pdgsCE.push_back(3334);
    namesCE.push_back("Omega");

    //Compute GCE Yields first
    vector<double> GCEyields;

    ComputeGCEyields(GCEyields, pdgsCE, namesCE, particles);

    cout << setw(15) << "Particle Name" << setw(15) << "Particle ID" << setw(15) << "GCE Yield" << endl << endl; 
    for(int i=0; i<GCEyields.size(); i++)
    {
        cout << setw(15) << namesCE[i] << setw(15) << pdgsCE[i] << setw(15) << GCEyields[i] << endl;        
    }
    cout << endl << endl;

    for (int flag=0; flag < 4; flag++)
    {
        ComputeCEyields(pdgsCE, namesCE, particles, flag, GCEyields);
    }



      
}