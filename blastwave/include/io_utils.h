#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <vector>
#include <string>
#include <filesystem>
#include "hadron_list.h"

namespace fs = std::filesystem;

std::vector<std::vector<HadronIntegrationInfo>> get_integration_info(
    const std::string& filename1 = "data_TableIV.root",
    const std::string& filename2 = "HEPData-ins1222333-v1-Table_31.root",
    const std::string& path = "./data/");

#endif