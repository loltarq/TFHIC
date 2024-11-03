#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <filesystem>

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

//following two vectors to be loaded with config file

static vector<string> ConfigParameters;
//string OutputFolder;
//string ParticleListFile;
//string ParticleConfigFile;
//string ResonanceDecayScheme;

static vector<double> k_scan_values;
//correlation volume values

bool LoadConfigFile(string ConfigFile)
{

    ifstream inf(ConfigFile.c_str());

    if (!inf.good())
    {
        return false;
    }   
    else
    {
        string line;
        while(getline(inf, line))
        {
            if (line.size() == 0)
            {
                break;
            }

            if (line.find(",") == string::npos)
            {
                ConfigParameters.push_back(line);
            }
            else
            {
                istringstream iss(line);
                string substring;
                while(getline(iss,substring,','))
                {
                    k_scan_values.push_back(stod(substring));
                }
            }
        }

        inf.close();

        if (ConfigParameters.size() == 0 || k_scan_values.size() == 0)
        {
            cout << "Config vectors empty." << endl;
            return false;
        }
    }
    return true;
}

string dtos(double d) {
    stringstream ss;
    ss << d;

    return ss.str();
}

//parameters dependence on multiplicity in gammaS variant from 1906.03145
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
void PrepareModel(ThermalModelBase * &model, ThermalParticleSystem *particles, const string& ensemble, const string& width_scheme)
{
    if (ensemble == "GCE")
        model = new ThermalModelIdeal(particles);
    else
    {
        model = new ThermalModelCanonical(particles);

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

//multiplicity scan for gammaS variant
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

//volume scan for vanilla variant
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

void ComputeYieldRatios(vector<int> pid1, vector<int> pid2, vector<string> pname1, vector<string> pname2, ThermalModelBase * &model, ThermalParticleSystem particles, vector<double> k, const string& ensemble, bool GsFlag, bool toGCEflag, string output="")
{
    ThermalModelBase * modelGCE;
    if (toGCEflag)//prepare GCE model to compute ratios to GCE
    {
        PrepareModel(modelGCE, &particles, "GCE", ConfigParameters[3]);
        if (!GsFlag)
            modelGCE->CalculateDensities();//if vanilla, no need to cycle over the volume scan when computing densities
    }

    for (int j=0; j<k.size(); j++)
    {
        if (!GsFlag)//Vanilla
        {
            vector<double> VScan;
            VScanFill(VScan, k[j]);

            ofstream fout;

            if (output == "")
                fout.open(ConfigParameters[0] + "ratios_" + ensemble + "_scan_k" + dtos(k[j]) + string(toGCEflag ? "_toGCE.dat" : ".dat"), ofstream::out | ofstream::trunc);
            else
                fout.open(ConfigParameters[0] + output + dtos(k[j]) + (output.find(".dat") != string::npos ? "" : ".dat"), ofstream::out | ofstream::trunc);

            fout << setw(15) << "dNpi/dy";
            fout << setw(15) << "Vc[fm^3]";

            for (int i=0; i<pname1.size(); i++)
            {
                fout << setw(15) << (pname1[i] + "/" + pname2[i] + string(toGCEflag ? "_toGCE" : ""));
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

                for (int i=0; i<pname1.size(); i++)
                { 
                    fout << setw(15) << model->GetDensity(pid1[i], 1) / model->GetDensity(pid2[i], 1) / (toGCEflag ? (modelGCE->GetDensity(pid1[i], 1) / modelGCE->GetDensity(pid2[i] ,1)) : 1);
                }

                fout << endl;
            }

            fout.close();
        }
        else//GammaS
        {
            vector<double> MultiplicityScan;
            MultiplicityScanFill(MultiplicityScan);

            ofstream fout;

            if (output == "")
                fout.open(ConfigParameters[0] + "ratios_gs_" + ensemble + "_scan_k" + dtos(k[j]) + string(toGCEflag ? "_toGCE.dat" : ".dat"), ofstream::out | ofstream::trunc);
            else
                fout.open(ConfigParameters[0] + output + dtos(k[j]) + (output.find(".dat") != string::npos ? "" : ".dat"), ofstream::out | ofstream::trunc);

            fout << setw(15) << "dNpi/dy";
            fout << setw(15) << "Tch[MeV]";
            fout << setw(15) << "dVdy[fm^3]";
            fout << setw(15) << "Vc[fm^3]";
            fout << setw(15) << "gammaS";

            for (int i=0; i<pname1.size(); i++)
            {
                fout << setw(15) << pname1[i] + "/" + pname2[i] + string(toGCEflag ? "_toGCE" : "");
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

                if(toGCEflag)
                {
                    modelGCE->SetTemperature(Tch);
                    modelGCE->SetGammaS(gammaS);
                    modelGCE->SetVolume(dVdy);
                    modelGCE->SetCanonicalVolume(Vc);
                    modelGCE->CalculateDensities();
                }

                fout << setw(15) << Nch;
                fout << setw(15) << Tch;
                fout << setw(15) << dVdy;
                fout << setw(15) << Vc;
                fout << setw(15) << gammaS;

                for (int i=0; i<pname1.size(); i++)
                { 
                    fout << setw(15) << model->GetDensity(pid1[i], 1) / model->GetDensity(pid2[i], 1) / (toGCEflag ? (modelGCE->GetDensity(pid1[i], 1) / modelGCE->GetDensity(pid2[i], 1)) : 1);
                }

                fout << endl;
            }

            fout.close();
        }       
    }   
}

void LoadParticleRef(vector<int>& pdg1, vector<int>& pdg2, vector<string>& name1, vector<string>& name2, string input="", string path = "../conf/")
{
    ifstream inf((path + input).c_str());
    string line;

    if(inf.good())
    {
        while(getline(inf, line))
        {
                if(line.size() == 0)
                    break;

                istringstream iss(line);
                string substring;
                vector<string> substrings;
                while(getline(iss,substring,','))
                {
                    substrings.push_back(substring);
                }

                name1.push_back(substrings[0]);
                pdg1.push_back(stoi(substrings[1]));              
                name2.push_back(substrings[2]);            
                pdg2.push_back(stoi(substrings[3]));
        }
        inf.close();
    }
    else
        cout << "Can't open or find particle analysis config file" << endl << endl;

}


int main(int argc, char *argv[])
{

    if (argc<=4)
    {
                cout << "Not enough arguments provided" << endl;
                cout << "Required arguments, in order: custom output file flag [0,1], toGCE flag [0,1], Ensemble [GCE,CE,SCE], GammaS model flag [0,1], Ensemble, GammaS model flag ..." << endl;
                cout << "E.g. to compute yield ratios to GCE in Vanilla Strangeness-canonical and GammaS full canonical picture, with default output file, run the script as follows:" << endl << endl;
                cout << "./TF_CSM-vs-dNpidy 0 1 SCE 0 CE 1" << endl << endl;

                return 0;
    }

    if (argc>4)
    {

        string ConfigFile = "../conf/_AnalysisConfig.config";

        if (!LoadConfigFile(ConfigFile))
        {
            cout << "Couldn't load analysis config file - execution aborted." << endl;
            return 0;
        }

        cout << "Config file successfully loaded." << endl << endl;
        cout << "Config parameters - output folder, list of particles injected in model, list of particles to be analyzed, resonance width scheme:" << endl;
        for (auto& item : ConfigParameters)
            cout << item << endl;
        cout << endl;
        cout << "Correlation volume scan values: ";
        for (auto& item : k_scan_values)
            cout << item << " ";
        cout << endl << endl << endl;

        
        try 
        { 
            if (filesystem::create_directory(ConfigParameters[0]))
            { 
                cout << "New output directory created successfully: " << ConfigParameters[0] << endl; 
            }
        }
        catch (const filesystem::filesystem_error& e)
        { 
            cerr << "Error creating directory: " << e.what() << endl;
            cout << "Output directory will be defaulted to: ../out/" << endl;
            ConfigParameters[0] = "../out/";
        }

        ThermalParticleSystem particles(string(ThermalFIST_INPUT_FOLDER)+ConfigParameters[1]);

        //Considering specific hadrons
        vector<int> pdgs1;
        vector<int> pdgs2;
        vector<string> names1;
        vector<string> names2;

        LoadParticleRef(pdgs1,pdgs2,names1,names2,ConfigParameters[2]);

        double toGCEflag = atoi(argv[2]);
        double outputFlag = atoi(argv[1]);

        for (int i = 3; i<argc; i+=2)
        {
            ThermalModelBase * model;
            double gsflag = atoi(argv[i+1]);

            cout << "Computing ratios " << (toGCEflag ? "to GCE " : "") << "in " << argv[i] << " formulation, " << (!gsflag ? "vanilla" : "gammaS") << endl;

            PrepareModel(model, &particles, argv[i], ConfigParameters[3]);
            //model->TPS()->ParticleByPDG(9010221).SetAbsoluteStrangeness(2.0);
            if(outputFlag)
            {
                string outputString;
                cout << "Insert output file name: ";
                cin >> outputString;
                ComputeYieldRatios(pdgs1, pdgs2, names1, names2, model, particles, k_scan_values, string(argv[i]), gsflag, toGCEflag, outputString);
            }
            else
                ComputeYieldRatios(pdgs1, pdgs2, names1, names2, model, particles, k_scan_values, string(argv[i]), gsflag, toGCEflag);

            cout << endl;
        }

    }
      
}