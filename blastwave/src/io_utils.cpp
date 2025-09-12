#include <io_utils.h>
#include <TFile.h>
#include <TTree.h>
#include <TDirectory.h>
#include <string>
#include <vector>
#include <iostream>
#include <TH1F.h>
#include <fstream>
#include <filesystem>
#include <unordered_map>

#include "io_utils.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace fs = std::filesystem;

namespace {

// ---- trim helper
static inline std::string trim(std::string s){
  auto issp = [](unsigned char c){ return std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char c){ return !issp(c); }));
  s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char c){ return !issp(c); }).base(), s.end());
  return s;
}

// ---- split on a single char
static std::vector<std::string> split(const std::string& s, char delim){
  std::vector<std::string> out; std::stringstream ss(s); std::string item;
  while (std::getline(ss, item, delim)) out.push_back(item);
  return out;
}

// ---- case-insensitive compare to "ALL"
static bool is_all_token(std::string t){
  for (auto& c: t) c = std::toupper((unsigned char)c);
  return t == "ALL";
}

// ---- normalize particle token (case-insensitive, strip whitespace)
static std::string norm_token(std::string t){
  t = trim(t);
  for (auto& c: t) c = std::toupper((unsigned char)c);
  return t;
}

// ---- map PDG to our group token used in CSV
// Extend as needed; current CSV uses Pi/K/P groups.
static std::string token_for_pdg(int pdg){
  switch (pdg) {
    case 211: case -211: return "PI";
    case 321: case -321: return "K";
    case 2212: case -2212: return "P";
    // Add common ones if you plan to include them in the CSV later:
    case 310:             return "KS";      // K0S
    case 333:             return "PHI";
    case 3122: case -3122:return "LAMBDA";
    case 3312: case -3312:return "XI";
    case 3334: case -3334:return "OMEGA";
    default:              return "ALL";     // fallback to catch-all rule
  }
}

// ---- minimal hadron constructor
static Hadron make_hadron_from_pdg(int pdg){
  Hadron h{}; h.pdg = pdg;
  // Set reasonable masses/names for common species; extend as needed.
  switch (pdg){
    case 211:  h.name="pi^{+}";  h.mass=0.13957039; break;
    case -211: h.name="pi^{-}";  h.mass=0.13957039; break;
    case 321:  h.name="K^{+}";   h.mass=0.49367; break;
    case -321: h.name="K^{-}";   h.mass=0.49367; break;
    case 2212: h.name="p";       h.mass=0.93827208816; break;
    case -2212:h.name="\\bar{p}";h.mass=0.93827208816; break;
    case 310:  h.name="K^{0}_{S}"; h.mass=0.49761; break;
    case 333:  h.name="\\phi";   h.mass=1.01946; break;
    case 3122: h.name="\\Lambda";h.mass=1.11568; break;
    case -3122:h.name="\\bar{\\Lambda}"; h.mass=1.11568; break;
    case 3312: h.name="\\Xi^{-}"; h.mass=1.32171; break;
    case -3312:h.name="\\Xi^{+}"; h.mass=1.32171; break;
    case 3334: h.name="\\Omega^{-}"; h.mass=1.67245; break;
    case -3334:h.name="\\Omega^{+}"; h.mass=1.67245; break;
    default:   h.name="pdg"+std::to_string(pdg); h.mass=0.0; break;
  }
  return h;
}

struct BWParams {
  double beta_t=0, beta_t_unc1=0, beta_t_unc2=0;
  double Tkin=0,   Tkin_unc1=0,   Tkin_unc2=0;
  double n=0,      n_unc1=0,      n_unc2=0;
};

// For each centrality, keep:
// - params for ALL
// - params per token (PI, K, P, ...)
// Later, per PDG, prefer the token-specific one, else ALL, else error.
struct PerCentrality {
  int centrality_class = -1; // as in CSV
  bool has_all = false;
  BWParams all_params;
  std::map<std::string, BWParams> per_token; // token -> params
};

// naive CSV reader; expects comma-separated, with the header
static bool read_bw_csv(const std::string& path, std::vector<std::map<std::string,std::string>>& rows){
  std::ifstream in(path);
  if(!in) return false;
  std::string header;
  if(!std::getline(in, header)) return false;
  auto cols = split(header, ',');
  // normalize header names (strip)
  for (auto& c : cols) c = trim(c);

  std::string line;
  while (std::getline(in, line)) {
    if (trim(line).empty()) continue;
    auto cells = split(line, ',');
    // allow missing trailing cells
    while (cells.size() < cols.size()) cells.push_back("");
    std::map<std::string,std::string> m;
    for (size_t i=0;i<cols.size();++i){
      m[cols[i]] = trim(cells[i]);
    }
    rows.push_back(std::move(m));
  }
  return true;
}

} // namespace

std::vector<std::vector<HadronIntegrationInfo>>
get_integration_info_from_csv(const std::string& csv_path,
                              const std::vector<int>& species_pdgs,
                              bool verbose)
{
  std::vector<std::vector<HadronIntegrationInfo>> out;

  std::vector<std::map<std::string,std::string>> rows;
  if (!read_bw_csv(csv_path, rows)) {
    std::cerr << "[io_utils] ERROR: cannot read " << csv_path << "\n";
    return out;
  }

  // Group rows by centrality_class (int), and for each centrality build token-specific and ALL params.
  std::map<int, PerCentrality> byC;
  for (const auto& r : rows) {
    if (!r.count("centrality_class") || !r.count("particle")) continue;
    int c = std::stoi(r.at("centrality_class"));
    auto& pc = byC[c];
    pc.centrality_class = c;

    BWParams p;
    auto getd = [&](const char* k)->double{
      auto it=r.find(k); if (it==r.end() || it->second.empty()) return 0.0;
      return std::stod(it->second);
    };
    p.beta_t      = getd("beta_t");
    p.beta_t_unc1 = getd("beta_t_unc1");
    p.beta_t_unc2 = getd("beta_t_unc2");
    p.Tkin        = getd("Tkin");
    p.Tkin_unc1   = getd("Tkin_unc1");
    p.Tkin_unc2   = getd("Tkin_unc2");
    p.n           = getd("n_profile");
    p.n_unc1      = getd("n_profile_unc1");
    p.n_unc2      = getd("n_profile_unc2");

    std::string particle = norm_token(r.at("particle"));
    if (is_all_token(particle)) {
      pc.has_all   = true;
      pc.all_params= p;
    } else {
      // allow "K;Pi;P" lists
      for (auto tk : split(particle, ';')) {
        tk = norm_token(tk);
        if (!tk.empty()) pc.per_token[tk] = p;
      }
    }
  }

  // Sort centrality classes ascending and emit rows
  std::vector<int> cents;
  for (auto& kv : byC) cents.push_back(kv.first);
  std::sort(cents.begin(), cents.end());

  out.reserve(cents.size());
  for (int c : cents) {
    const PerCentrality& pc = byC[c];
    std::vector<HadronIntegrationInfo> row;
    row.reserve(species_pdgs.size());

    for (int pdg : species_pdgs) {
      // choose params: token-specific > ALL > (warn & skip)
      const std::string t = token_for_pdg(pdg);
      const BWParams* use = nullptr;
      auto it = pc.per_token.find(t);
      if (it != pc.per_token.end()) use = &it->second;
      else if (pc.has_all)          use = &pc.all_params;

      if (!use) {
        std::cerr << "[io_utils] WARNING: no BW params for PDG " << pdg
                  << " at centrality " << pc.centrality_class
                  << " (no token '"<<t<<"' and no ALL). Skipping species.\n";
        continue;
      }

      // Build the integration info with yield=0 (thermal JSON will fill it later)
      HadronIntegrationInfo info{};
      info.hadron = make_hadron_from_pdg(pdg);
      if (info.hadron.mass <= 0.0) {
        std::cerr << "[io_utils] WARNING: unknown mass for PDG "
                  << pdg << " (name="<< info.hadron.name <<")\n";
      }

 

      info.beta_t          = use->beta_t;
      info.Tkin            = use->Tkin;
      info.n_profile       = use->n;
      info.beta_t_unc1     = use->beta_t_unc1;
      info.beta_t_unc2     = use->beta_t_unc2;
      info.Tkin_t_unc1     = use->Tkin_unc1;
      info.Tkin_t_unc2     = use->Tkin_unc2;
      info.n_profile_unc1  = use->n_unc1;
      info.n_profile_unc2  = use->n_unc2;
      info.centrality_class= pc.centrality_class;
      info.yield           = 0.0;

      row.push_back(std::move(info));
    }

    if (verbose) {
      std::cerr << "[io_utils] cent="<< pc.centrality_class
                << " species="<< row.size() << "\n";
    }

    out.push_back(std::move(row));
  }

  return out;
}


// only used in test.cpp
std::vector<std::vector<HadronIntegrationInfo>> get_integration_info(const std::string& filename1, const std::string& filename2, const std::string& path)
{

    std::string fullPath1 = (fs::path(path) / filename1).string();
    std::string fullPath2 = (fs::path(path) / filename2).string();
    std::vector<std::vector<HadronIntegrationInfo>> allHadrons;

    // Open the ROOT file
    TFile *file1 = TFile::Open(fullPath1.c_str(), "READ");
    if (!file1 || file1->IsZombie()) {
        std::cerr << "[io_utils][get_integration_info] Error opening ROOT file: " << fullPath1 << "\n";
        return {};
    }

    // Access the TTree stored in the ROOT file
    TTree *tree = (TTree*)file1->Get("tree");
    if (!tree) {
        std::cerr << "[io_utils][get_integration_info] Error accessing TTree in ROOT file: " << fullPath1 << "\n";
        return {};
    }

    double beta_t[3]     = {0,0,0}; // value, bt-b, sys
    double Tkin_col[3]   = {0,0,0}; // value, bt-b, sys  (see note below)
    double n_col[3]      = {0,0,0}; // value, bt-b, sys
    double chisq_over_dof = 0.0;
    int    centrality_class = -1;

    //int centrality_class;
    //std::string centrality;

    // tree->Print();
    
    tree->SetBranchAddress("Class",         &centrality_class);
    tree->SetBranchAddress("beta_t",        &beta_t[0]);
    tree->SetBranchAddress("beta_t_btb",    &beta_t[1]);
    tree->SetBranchAddress("beta_t_sys",    &beta_t[2]);
    tree->SetBranchAddress("Tkin",          &Tkin_col[0]);   // NOTE: file appears to have Tkin and n swapped by label
    tree->SetBranchAddress("Tkin_btb",      &Tkin_col[1]);
    tree->SetBranchAddress("Tkin_sys",      &Tkin_col[2]);
    tree->SetBranchAddress("n_profile",     &n_col[0]);
    tree->SetBranchAddress("n_profile_btb", &n_col[1]);
    tree->SetBranchAddress("n_profile_sys", &n_col[2]);
    tree->SetBranchAddress("chi^2/dof",     &chisq_over_dof);


    TFile * file2 = new TFile(fullPath2.c_str(), "READ");
    if (!file2 || file2->IsZombie()) 
    {
        std::cerr << "[io_utils][get_integration_info] Error opening ROOT file!" << std::endl;
        return {};
    }

    TDirectory *td = (TDirectory*)file2->Get("Table 31");
    if (!td)
    {
        std::cerr << "[io_utils][get_integration_info] Failed to find the 'table' folder in" << std::endl;
        file1->Close();
        return {};
    }

    std::vector<std::vector<double>> yields;
    yields.resize(3);

    for (int i = 2; i <= 7; i+=2)
    { 
        TString hist_name = TString::Format("Hist1D_y%d", i);

        // Get the histogram
        TH1F* hist = (TH1F*)td->Get(hist_name);
        if (hist)
        {
            for (int bin = 1; bin <= hist->GetNbinsX(); ++bin)
            {
                //double x_value = hist->GetBinCenter(bin);  // Bin center
                double y_value = hist->GetBinContent(bin);  // Bin content (value)
                yields[i/2-1].push_back(y_value);
            }
        }   
        else 
        {
            std::cerr << "Histogram " << hist_name << " not found!" << std::endl;
        }
    }

        for (int i = 0; i < 10; i++)
    {

        tree->GetEntry(i);

        std::vector<HadronIntegrationInfo> hif = 
        {
            {pion, beta_t[0], Tkin_col[0], n_col[0], yields[0][i], beta_t[1], beta_t[2], Tkin_col[1], Tkin_col[2], n_col[1], n_col[2], centrality_class},
            {kaon, beta_t[0], Tkin_col[0], n_col[0], yields[1][i], beta_t[1], beta_t[2], Tkin_col[1], Tkin_col[2], n_col[1], n_col[2], centrality_class},
            {proton, beta_t[0], Tkin_col[0], n_col[0], yields[2][i], beta_t[1], beta_t[2], Tkin_col[1], Tkin_col[2], n_col[1], n_col[2], centrality_class}
        };

        allHadrons.push_back(hif);
    }

    file1->Close();
    file2->Close();

    return allHadrons;
}

void get_correlation_volume_info(const std::string& code,
                 double k_value,
                 std::vector<double>& col1,
                 std::vector<double>& col2,
                 const std::string& base_path) 
{
    // Format k_value nicely, e.g., 1.6 instead of 1.600000
    std::ostringstream k_str;

    if (std::floor(k_value) == k_value)
    {
    // It’s an integer → no decimals
    k_str << static_cast<int>(k_value);
    }
    else
    {
    // Not an integer → keep 1 decimal digit
    k_str << std::fixed << std::setprecision(1) << k_value;
    }

    std::string k_label = k_str.str();

    // Build full filename
    std::ostringstream filename_stream;
    filename_stream << base_path
                    << "ratios_" << code << "_scan_k" << k_str.str() << ".dat";

    std::string filename = filename_stream.str();

    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;

    // Skip header
    std::getline(infile, line);

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        double val1, val2;
        if (!(iss >> val1 >> val2)) {
            continue;
        }
        col1.push_back(val1); //pion yield
        col2.push_back(val2); //Vc
    }

    infile.close();
}

double extract_correlation_volume(const std::vector<double>& Vc,
                const std::vector<double>& thermal_yields,
                const double& target_yield)
{
    int inx = -1;
    double diff = 9999999.;

    //find closest match between target yield and thermal yields
    for (const auto& yields : thermal_yields)
    {
        double temp = TMath::Abs(yields - target_yield);

        if (temp < diff)
        {
            inx++;
            diff = temp;
            continue;
        }
        else
            break;
    }

    if (inx == -1)
    {
        std::cout << "\nERROR: correlation volume not found\n";
        return -1;
    }

    return Vc[inx];
}