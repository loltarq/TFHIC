#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <vector>
#include <string>
#include <filesystem>
#include "hadron_list.h"
#include "../../common/hadron_catalog.h"

// NEW: species-aware, CSV-driven blastwave parameters.
// - csv_path: path to CSV with columns:
//     particle,centrality_class,beta_t,beta_t_unc1,beta_t_unc2,
//     Tkin,Tkin_unc1,Tkin_unc2,n_profile,n_profile_unc1,n_profile_unc2
//   where 'particle' is "ALL" or a semicolon-separated list like "K;Pi;P".
// - species_pdgs: the PDGs you want in the output.
// - verbose: print mapping/diagnostics.
// Returns a [centrality][species] table with BW params set and yield=0.
std::vector<std::vector<HadronIntegrationInfo>>
get_integration_info_from_csv(const std::string& csv_path,
                              const std::vector<int>& species_pdgs,
                              bool verbose = false);

std::vector<std::vector<HadronIntegrationInfo>>
get_integration_info_from_csv(const std::string& csv_path,
                              const HadronCatalog& cat,
                              const std::vector<int>& species_pdgs,
                              bool verbose = false);
                              
std::vector<std::vector<HadronIntegrationInfo>> get_integration_info(
    const std::string& filename1 = "data_TableIV.root",
    const std::string& filename2 = "HEPData-ins1222333-v1-Table_31.root",
    const std::string& path = "./data/");


void get_correlation_volume_info(const std::string& code,
                 double k_value,
                 std::vector<double>& col1,
                 std::vector<double>& col2,
                 const std::string& base_path = "./data/");

double extract_correlation_volume(const std::vector<double>& Vc,
                const std::vector<double>& thermal_yields,
                const double& target_yield);

#endif