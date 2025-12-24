// thermal_yields/src/export_dndy_json.cpp
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <filesystem>

#include "runtime_paths.h"

#include "HRGBase.h"
#include "HRGEV.h"
#include "ThermalFISTConfig.h"

#ifdef ThermalFIST_USENAMESPACE
using namespace thermalfist;
#endif

static std::string ensure_json_ext(const std::string& path) {
    namespace fs = std::filesystem;
    fs::path p(path);
    if (p.has_extension()) return path;   // user specified an extension; keep it
    return (p.string() + ".json");        // append default
}

static inline bool is_bare_path(const std::string& s) {
    std::filesystem::path p(s);
    return !s.empty() && !p.is_absolute() && !p.has_parent_path();
}

static std::string default_particles_list() {
#ifdef ThermalFIST_INPUT_FOLDER
    return std::string(ThermalFIST_INPUT_FOLDER) + "/list/PDG2014/list-withnuclei.dat";
#else
    // Fallback if ThermalFIST_INPUT_FOLDER isn't defined for some reason
    return "../../Thermal-FIST/input/list/PDG2014/list-withnuclei.dat";
#endif
}
static std::string default_decays_for(const std::string& listPath) {
    std::filesystem::path p(listPath);
    return (p.parent_path() / "decays.dat").string();
}
static bool file_exists(const std::string& p) {
    std::error_code ec; 
    return std::filesystem::exists(p, ec);
}

struct GSParams {
    double aT=0.176, bT=0.0026;           // Tch = aT - bT ln Nch
    double aG=1.0,  bG=0.25,  cG=59.0;    // gammaS = aG - bG exp(-Nch/cG)
    double aV=2.4;                        // dV/dy = aV * Nch
};

struct Defaults {
    std::string ensemble = "CE";
    std::string width    = "eBW";
    std::string species_csv = "211,-211,321,-321,2212,-2212";
    std::string k_csv       = "3";
    bool toGCE = false;
    int  QStats = 1;
    std::string feeddown = "stabilityflag";
    std::string mode     = "vanilla";
    std::string list_default = default_particles_list();
    // vanilla
    double Tch_fixed = 0.155;
    double vMin=10., vMax=15000.; int vN=30;
    // gammaS
    double nchMin=3., nchMax=2000.; int nchN=100; std::string nchFile="";
    GSParams gspar; // aT,bT,aG,bG,cG,aV
};

static void print_help_with_defaults(const char* prog, const Defaults& D){
    std::cerr <<
"\nUsage:\n"
"  " << prog << " --out PATH --list PATH/particles.dat [--decays PATH/decays.dat]\n"
"         --ensemble GCE|SCE|CE --width eBW|ZeroWidth|BWTwoGamma\n"
"         --species \"211,-211,321,-321,2212,-2212\" --k \"1.0,1.6,3.0\"\n"
"         [--QStats 1|0] [--feeddown primordial|weak|strong|em|stabilityflag]\n"
"         [--toGCE 0|1] --mode vanilla|gs  [flags per mode below]\n"
"\nRequired:\n"
"  --out PATH_OR_NAME             (no default; if only a name is given, outputs to out-dir)\n"
"\nModel & I/O (defaults shown):\n"
"  --list PATH/particles.dat      (default: " << D.list_default << ")\n"
"  --decays PATH/decays.dat       (default: <dir_of_list>/decays.dat)\n"
"  --data-dir PATH                (override data dir for bare filenames)\n"
"  --conf-dir PATH                (override conf dir for --nch-file)\n"
"  --out-dir PATH                 (override output dir for bare --out)\n"
"  --ensemble                     (default: " << D.ensemble << ")\n"
"  --width                        (default: " << D.width    << ")\n"
"  --species                      (default: " << D.species_csv << ")\n"
"  --k                            (default: " << D.k_csv       << ")\n"
"  --toGCE 0|1                    (default: " << (D.toGCE?1:0) << ")\n"
"  --QStats 0|1                   (default: " << D.QStats      << ")\n"
"  --feeddown                     (default: " << D.feeddown    << ")\n"
"  --mode vanilla|gs              (default: " << D.mode        << ")\n"
"\nVanilla mode (no gammaS, defaults shown):\n"
"  --Tch                          (default: " << D.Tch_fixed << " GeV)\n"
"  --v-min                        (default: " << D.vMin << ")\n"
"  --v-max                        (default: " << D.vMax << ")\n"
"  --v-n                          (default: " << D.vN   << ")\n"
"\n gammaS mode (defaults shown):\n"
"  --nch-min                      (default: " << D.nchMin << ")\n"
"  --nch-max                      (default: " << D.nchMax << ")\n"
"  --nch-n                        (default: " << D.nchN   << ")\n"
"  --nch-file PATH_OR_NAME        (default: <unset>; if only a name is given, reads from ../conf/)\n"
"  --tch-a, --tch-b               (defaults: " << D.gspar.aT << ", " << D.gspar.bT << ")\n"
"  --gs-a, --gs-b, --gs-c         (defaults: " << D.gspar.aG << ", " << D.gspar.bG << ", " << D.gspar.cG << ")\n"
"  --vol-a                        (default: " << D.gspar.aV << ")\n"
"\nNotes:\n"
"  * dV/dy is the thermodynamic volume per unit rapidity; canonical volume Vc = k * dV/dy.\n"
"  * B, Q, S (and charm) chemical potentials are set to 0; QS on for mesons, off for baryons.\n"
"  * JSON contains dNdy_primary and dNdy_total (per chosen feeddown).\n"
"  * Env overrides: TFHIC_DATA, TFHIC_CONF, TFHIC_OUT.\n\n";
}

// ---------- Small helpers ----------
static bool useQStats = true; // quantum statistics ON, then disable for baryons

static std::vector<double> parse_csv_doubles(const std::string& s) {
    std::vector<double> out; std::stringstream ss(s); std::string tok;
    while (std::getline(ss, tok, ',')) if (!tok.empty()) out.push_back(std::atof(tok.c_str()));
    return out;
}
static std::vector<long long> parse_csv_pdgs(const std::string& s) {
    std::vector<long long> out; std::stringstream ss(s); std::string tok;
    while (std::getline(ss, tok, ',')) if (!tok.empty()) out.push_back(std::atoll(tok.c_str()));
    return out;
}
static void logspace(std::vector<double>& v, double xmin, double xmax, int n) {
    v.clear(); if (n<=0) return;
    const double lmin = std::log10(xmin), lmax = std::log10(xmax);
    for (int i=0;i<n;i++) v.push_back(std::pow(10.0, lmin + (lmax-lmin)*(double(i)/double(n-1))));
}
static ThermalParticle* find_particle_by_pdg(ThermalParticleSystem& ps, long long pdg) {
    for (int i=0;i<(int)ps.Particles().size(); ++i)
        if (ps.Particle(i).PdgId()==pdg) return &ps.Particle(i);
    return nullptr;
}
static const char* feeddown_name(Feeddown::Type t) {
    switch (t) {
        case Feeddown::Primordial:        return "primordial";
        case Feeddown::StabilityFlag:     return "stabilityflag";
        case Feeddown::Weak:              return "weak";
        case Feeddown::Electromagnetic:   return "em";
        case Feeddown::Strong:            return "strong";
        default:                          return "unknown";
    }
}

static inline double Tch_from_Nch(double Nch, const GSParams& p){ return p.aT - p.bT*std::log(Nch); }
static inline double Gs_from_Nch (double Nch, const GSParams& p){ return p.aG - p.bG*std::exp(-Nch/p.cG); }
static inline double dVdy_from_Nch(double Nch, const GSParams& p){ return p.aV*Nch; }

// Prepare model
static void PrepareModel(ThermalModelBase*& model, ThermalParticleSystem* ps,
                         const std::string& ensemble, const std::string& widthScheme)
{
    if (ensemble=="GCE") {
        model = new ThermalModelIdeal(ps);
    } else {
        auto* m = new ThermalModelCanonical(ps);
        if (ensemble=="SCE") {
            m->ConserveBaryonCharge(false);
            m->ConserveElectricCharge(false);
        }
        model = m;
    }

    model->SetTemperature(0.155);                       // overwritten later
    model->SetBaryonChemicalPotential(0.);
    model->SetElectricChemicalPotential(0.);
    model->SetStrangenessChemicalPotential(0.);
    model->SetCharmChemicalPotential(0.);
    model->SetBaryonCharge(0);
    model->SetElectricCharge(0);
    model->SetStrangeness(0);
    model->SetCharm(0);

    model->SetStatistics(useQStats);
    if (useQStats) {
        for (int i=0;i<(int)model->TPS()->Particles().size(); ++i) {
            ThermalParticle& p = model->TPS()->Particle(i);
            if (p.BaryonCharge()!=0) p.UseStatistics(false); // disable QS for baryons
        }
    }

    if (ensemble!="GCE")
        static_cast<ThermalModelCanonical*>(model)->CalculateQuantumNumbersRange();

    if (widthScheme=="ZeroWidth")      model->SetUseWidth(ThermalParticle::ZeroWidth);
    else if (widthScheme=="BWTwoGamma")model->SetUseWidth(ThermalParticle::BWTwoGamma);
    else                               model->SetUseWidth(ThermalParticle::eBW);

    model->FillChemicalPotentials();
}

int main(int argc, char** argv){
    // Defaults
    Defaults D;

    // Early help: if user passed -h/--help, print defaults and exit
    for (int i=1;i<argc;i++){
        std::string a = argv[i];
        if (a=="--help" || a=="-h") { print_help_with_defaults(argv[0], D); return 0; }
    }

    // Required args
    std::string outPath;
    std::string listPath = D.list_default;
    std::string decaysPath;

    // Initialize from defaults
    std::string ensemble = D.ensemble;
    std::string width    = D.width;
    std::vector<long long> pdgs = parse_csv_pdgs(D.species_csv);
    std::vector<double>    klist = parse_csv_doubles(D.k_csv);
    bool toGCE = D.toGCE;
    int  QStats = D.QStats;
    std::string fdStr = D.feeddown;
    std::string mode  = D.mode;

    // Vanilla
    double Tch_fixed = D.Tch_fixed;
    double vMin=D.vMin, vMax=D.vMax; int vN=D.vN;

    // gammaS
    GSParams gspar = D.gspar;
    double nchMin=D.nchMin, nchMax=D.nchMax; int nchN=D.nchN; std::string nchFile=D.nchFile;

    // Parse CLI
    std::string dataDirFlag, confDirFlag, outDirFlag;
    auto need = [&](bool ok){ if(!ok){ print_help_with_defaults(argv[0], D); std::exit(2);} };
    for(int i=1;i<argc;i++){
        std::string a = argv[i];
        auto nexts=[&](){ need(i+1<argc); return std::string(argv[++i]); };
        auto nextd=[&](){ need(i+1<argc); return std::atof(argv[++i]); };
        auto nexti=[&](){ need(i+1<argc); return std::atoi(argv[++i]); };

        if(a=="--help"||a=="-h"){ print_help_with_defaults(argv[0], D); return 0; } // in-loop safety

        if(a=="--out") outPath = nexts();
        else if(a=="--list") listPath = nexts();
        else if(a=="--decays") decaysPath = nexts();
        else if(a=="--data-dir") dataDirFlag = nexts();
        else if(a=="--conf-dir") confDirFlag = nexts();
        else if(a=="--out-dir")  outDirFlag  = nexts();
        else if(a=="--ensemble") ensemble = nexts();
        else if(a=="--width") width = nexts();
        else if(a=="--species") pdgs = parse_csv_pdgs(nexts());
        else if(a=="--k") klist = parse_csv_doubles(nexts());
        else if(a=="--toGCE") toGCE = nexti()!=0;
        else if(a=="--QStats") QStats = nexti();
        else if(a=="--feeddown") fdStr = nexts();
        else if(a=="--mode") mode = nexts();

        else if(a=="--Tch") Tch_fixed = nextd();
        else if(a=="--v-min") vMin = nextd();
        else if(a=="--v-max") vMax = nextd();
        else if(a=="--v-n")   vN   = nexti();

        else if(a=="--nch-min") nchMin = nextd();
        else if(a=="--nch-max") nchMax = nextd();
        else if(a=="--nch-n")   nchN   = nexti();
        else if(a=="--nch-file")nchFile= nexts();
        else if(a=="--tch-a")   gspar.aT = nextd();
        else if(a=="--tch-b")   gspar.bT = nextd();
        else if(a=="--gs-a")    gspar.aG = nextd();
        else if(a=="--gs-b")    gspar.bG = nextd();
        else if(a=="--gs-c")    gspar.cG = nextd();
        else if(a=="--vol-a")   gspar.aV = nextd();
        else { std::cerr << "Unknown arg: " << a << "\n"; print_help_with_defaults(argv[0], D); return 2; }
    }

    auto paths = resolve_runtime_paths(argv[0], dataDirFlag, confDirFlag, outDirFlag,
                                       "thermal_yields/out");

    // Make --out land in out-dir if it's just a filename
    if (outPath.empty()) {
        print_help_with_defaults(argv[0], D);
        return 2;
    }
    outPath = resolve_out_path(paths, outPath).string();
    // Ensure output file has json extension if not specified in the CLI flag
    outPath = ensure_json_ext(outPath);

    // If --nch-file is a bare name, look for it in conf dir
    if (!nchFile.empty()) {
        nchFile = resolve_conf_path(paths, nchFile).string();
    }

    // Allow --list/--decays bare names to resolve via data dir if not found locally.
    if (is_bare_path(listPath) && !file_exists(listPath)) {
        std::string candidate = resolve_data_path(paths, listPath).string();
        if (file_exists(candidate)) listPath = candidate;
    }
    if (!decaysPath.empty() && is_bare_path(decaysPath) && !file_exists(decaysPath)) {
        std::string candidate = resolve_data_path(paths, decaysPath).string();
        if (file_exists(candidate)) decaysPath = candidate;
    }

    // Ensure output directory exists
    std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());

    if (!file_exists(listPath)) {
        std::cerr << "ERROR: particles list not found at '" << listPath
                << "'. Pass a valid path with --list.\n";
        return 2;
    }
    if (decaysPath.empty())
        decaysPath = default_decays_for(listPath);
    if (!file_exists(decaysPath)) {
        std::cerr << "ERROR: decays file not found at '" << decaysPath
                << "'. Pass a valid path with --decays or fix your installation.\n";
        return 2;
    }

    useQStats = (QStats!=0);

    // Feeddown choice
    Feeddown::Type fd = Feeddown::StabilityFlag;
    if      (fdStr=="primordial")    fd = Feeddown::Primordial;
    else if (fdStr=="weak")          fd = Feeddown::Weak;
    else if (fdStr=="em")            fd = Feeddown::Electromagnetic;
    else if (fdStr=="strong")        fd = Feeddown::Strong;
    else if (fdStr=="stabilityflag") fd = Feeddown::StabilityFlag;

    // ---- Load particles & model(s) ----
    ThermalParticleSystem particles(listPath, decaysPath);
    ThermalModelBase* model=nullptr;
    PrepareModel(model, &particles, ensemble, width);

    ThermalModelBase* modelGCE=nullptr;
    if (toGCE) PrepareModel(modelGCE, &particles, "GCE", width);

    // ---- Output JSON header ----
    std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());
    std::ofstream out(outPath);
    if(!out){ std::cerr<<"Cannot open "<<outPath<<"\n"; return 1; }

    out << "{\n  \"meta\": {\n";
    out << "    \"ensemble\": \""<<ensemble<<"\",\n";
    out << "    \"width_scheme\": \""<<width<<"\",\n";
    out << "    \"gammaS_variant\": "<<(mode=="gs"?"true":"false")<<",\n";
    out << "    \"feeddown\": \""<<feeddown_name(fd)<<"\",\n";
    out << "    \"QStats\": "<<(useQStats?"true":"false")<<",\n";
    out << "    \"klist\": [";
    for(size_t i=0;i<klist.size();++i){ out<<std::fixed<<std::setprecision(3)<<klist[i]<<(i+1<klist.size()?",":""); }
    out << "],\n";
    out << "    \"species\": [";
    for(size_t i=0;i<pdgs.size();++i){ out<<pdgs[i]<<(i+1<pdgs.size()?",":""); }
    out << "]\n  },\n  \"bins\": [\n";

    bool firstBin=true;
    auto binsep=[&](){ if(!firstBin) out << ",\n"; firstBin=false; };

    out << std::setprecision(10);

    if (mode=="vanilla"){
        std::vector<double> vscan; logspace(vscan, vMin, vMax, vN);
        for (double k : klist){
            for (double dVdy : vscan){
                const double Vc = k * dVdy;
                // Set thermodynamics
                model->SetTemperature(Tch_fixed);
                model->SetGammaS(1.0);
                model->SetVolume(dVdy);          // dV/dy
                model->SetCanonicalVolume(Vc);    // correlation volume
                model->CalculateDensities();

                if (toGCE){
                    modelGCE->SetTemperature(Tch_fixed);
                    modelGCE->SetGammaS(1.0);
                    modelGCE->SetVolume(dVdy);
                    modelGCE->SetCanonicalVolume(Vc);
                    modelGCE->CalculateDensities();
                }

                binsep();
                out << "    {\"mode\":\"vanilla\",\"k\":"<<k
                    <<",\"dVdy\":"<<dVdy
                    <<",\"Vc\":"<<Vc
                    <<",\"Tch_GeV\":"<<Tch_fixed
                    <<",\"gammaS\":1.0";

                // Names (optional convenience)
                out << ",\"names\":{";
                for (size_t i=0;i<pdgs.size();++i){
                    auto* P = find_particle_by_pdg(*model->TPS(), pdgs[i]);
                    std::string nm = P ? P->Name() : std::to_string(pdgs[i]);
                    out << "\"" << pdgs[i] << "\":\"" << nm << "\"" << (i+1<pdgs.size()?",":"");
                }
                out << "}";

                // Yields
                out << ",\"dNdy_primary\":{";
                for (size_t i=0;i<pdgs.size();++i){
                    double y = model->GetYield(pdgs[i], Feeddown::Primordial);
                    out << "\"" << pdgs[i] << "\":" << y << (i+1<pdgs.size()?",":"");
                }
                out << "},\"dNdy_total\":{";
                for (size_t i=0;i<pdgs.size();++i){
                    double y = model->GetYield(pdgs[i], fd);
                    out << "\"" << pdgs[i] << "\":" << y << (i+1<pdgs.size()?",":"");
                }
                out << "}";

                if (toGCE){
                    out << ",\"dNdy_total_GCE\":{";
                    for (size_t i=0;i<pdgs.size();++i){
                        double y = modelGCE->GetYield(pdgs[i], fd);
                        out << "\"" << pdgs[i] << "\":" << y << (i+1<pdgs.size()?",":"");
                    }
                    out << "}";
                }
                out << "}";
            }
        }
    } else if (mode=="gs"){
        std::vector<double> Nch;
        if (!nchFile.empty()) {
            std::ifstream fin(nchFile);
            if (!fin) {
                std::cerr << "Cannot open --nch-file " << nchFile << ", falling back to range.\n";
            } else {
                std::string line;
                while (std::getline(fin, line)) {
                    auto pos = line.find_first_not_of(" \t");
                    if (pos == std::string::npos || line[pos] == '#') continue; // skip blanks/comments
                    try {
                        double x = std::stod(line.substr(pos));
                        if (x > 0) Nch.push_back(x);
                    } catch (...) { /* ignore malformed lines */ }
                }
            }
        }
        if (Nch.empty()) logspace(Nch, nchMin, nchMax, nchN);

        for (double k : klist){
            for (double n : Nch){
                const double Tch   = Tch_from_Nch(n, gspar);
                const double gamma = Gs_from_Nch(n, gspar);
                const double dVdy  = dVdy_from_Nch(n, gspar);
                const double Vc    = k * dVdy;

                model->SetTemperature(Tch);
                model->SetGammaS(gamma);
                model->SetVolume(dVdy);
                model->SetCanonicalVolume(Vc);
                model->CalculateDensities();

                if (toGCE){
                    modelGCE->SetTemperature(Tch);
                    modelGCE->SetGammaS(gamma);
                    modelGCE->SetVolume(dVdy);
                    modelGCE->SetCanonicalVolume(Vc);
                    modelGCE->CalculateDensities();
                }

                binsep();
                out << "    {\"mode\":\"gammaS\",\"k\":"<<k
                    <<",\"Nch\":"<<n
                    <<",\"dVdy\":"<<dVdy
                    <<",\"Vc\":"<<Vc
                    <<",\"Tch_GeV\":"<<Tch
                    <<",\"gammaS\":"<<gamma;

                out << ",\"names\":{";
                for (size_t i=0;i<pdgs.size();++i){
                    auto* P = find_particle_by_pdg(*model->TPS(), pdgs[i]);
                    std::string nm = P ? P->Name() : std::to_string(pdgs[i]);
                    out << "\"" << pdgs[i] << "\":\"" << nm << "\"" << (i+1<pdgs.size()?",":"");
                }
                out << "}";

                out << ",\"dNdy_primary\":{";
                for (size_t i=0;i<pdgs.size();++i){
                    double y = model->GetYield(pdgs[i], Feeddown::Primordial);
                    out << "\"" << pdgs[i] << "\":" << y << (i+1<pdgs.size()?",":"");
                }
                out << "},\"dNdy_total\":{";
                for (size_t i=0;i<pdgs.size();++i){
                    double y = model->GetYield(pdgs[i], fd);
                    out << "\"" << pdgs[i] << "\":" << y << (i+1<pdgs.size()?",":"");
                }
                out << "}";
                if (toGCE){
                    out << ",\"dNdy_total_GCE\":{";
                    for (size_t i=0;i<pdgs.size();++i){
                        double y = modelGCE->GetYield(pdgs[i], fd);
                        out << "\"" << pdgs[i] << "\":" << y << (i+1<pdgs.size()?",":"");
                    }
                    out << "}";
                }
                out << "}";
            }
        }
    } else {
        std::cerr<<"Unknown --mode "<<mode<<"\n";
        return 2;
    }

    out << "\n  ]\n}\n";
    out.close();

    delete model;
    if (modelGCE) delete modelGCE;

    std::cerr << "Wrote yields JSON to " << outPath << "\n";
    return 0;
}
