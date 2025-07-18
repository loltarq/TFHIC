#include <io_utils.h>
#include <TFile.h>
#include <TTree.h>
#include <TDirectory.h>
#include <string>
#include <vector>
#include <iostream>
#include <TH1F.h>
#include <fstream>

std::vector<std::vector<HadronIntegrationInfo>> get_integration_info(const std::string& filename1, const std::string& filename2, const std::string& path)
{

    std::string fullPath1 = (fs::path(path) / filename1).string();
    std::string fullPath2 = (fs::path(path) / filename2).string();
    std::vector<std::vector<HadronIntegrationInfo>> allHadrons;

    // Open the ROOT file
    TFile *file1 = TFile::Open(fullPath1.c_str(), "READ");
    if (!file1 || file1->IsZombie()) {
        std::cerr << "Error opening ROOT file!" << std::endl;
        return {};
    }

    // Access the TTree stored in the ROOT file
    TTree *tree = (TTree*)file1->Get("tree");
    if (!tree) {
        std::cerr << "Error accessing TTree in the ROOT file!" << std::endl;
        return {};
    }

    //file1->Close();

    double beta_t[3];
    double Tkin[3];
    double n_profile[3];
    double chisquare_dof;

    int centrality_class;
    std::string centrality;

    // tree->Print();
    
    tree->SetBranchAddress("Class", &centrality_class);
    //tree->SetBranchAddress("Centrality", &centrality);
    tree->SetBranchAddress("beta_t", &beta_t[0]);
    tree->SetBranchAddress("beta_t_btb", &beta_t[1]);
    tree->SetBranchAddress("beta_t_sys", &beta_t[2]);
    tree->SetBranchAddress("Tkin", &Tkin[0]);
    tree->SetBranchAddress("Tkin_btb", &Tkin[1]);
    tree->SetBranchAddress("Tkin_sys", &Tkin[2]);
    tree->SetBranchAddress("n_profile", &n_profile[0]);
    tree->SetBranchAddress("n_profile_btb", &n_profile[1]);
    tree->SetBranchAddress("n_profile_sys", &n_profile[2]);
    tree->SetBranchAddress("chi^2/dof", &chisquare_dof);

    TFile * file2 = new TFile(fullPath2.c_str(), "READ");
    if (!file2 || file2->IsZombie()) 
    {
        std::cerr << "Error opening ROOT file!" << std::endl;
        return {};
    }

    TDirectory *td = (TDirectory*)file2->Get("Table 31");
    if (!td)
    {
        std::cerr << "Failed to find the 'table' folder!" << std::endl;
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
            {pion, yields[0][i], beta_t[0], Tkin[0], n_profile[0], beta_t[1], beta_t[2], Tkin[1], Tkin[2], n_profile[1], n_profile[2], centrality_class},
            {kaon, yields[1][i], beta_t[0], Tkin[0], n_profile[0], beta_t[1], beta_t[2], Tkin[1], Tkin[2], n_profile[1], n_profile[2], centrality_class},
            {proton, yields[2][i], beta_t[0], Tkin[0], n_profile[0], beta_t[1], beta_t[2], Tkin[1], Tkin[2], n_profile[1], n_profile[2], centrality_class}
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