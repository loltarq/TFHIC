#pragma once
#include <map>
#include <string>
#include <limits>

// Which yield to use from the JSON
enum class ThermalYieldKind { Total, Primordial };

// CLI-driven selection for a bin within the JSON
struct ThermalSelection {
  std::string json_path;      // required
  int bin_index = -1;         // >=0 to pick by index; else auto-select by params below
  std::string mode;           // "gs" or "vanilla" (optional; if set, must match)
  double k = std::numeric_limits<double>::quiet_NaN();             // optional; closest-match if finite
  double Nch = std::numeric_limits<double>::quiet_NaN();           // only for mode="gs" (closest)
  double dVdy = std::numeric_limits<double>::quiet_NaN();          // only for mode="vanilla" (closest)
  ThermalYieldKind kind = ThermalYieldKind::Total; // default = Total
};

// Load PDG->dN/dy for the selected bin. Returns true on success.
// - out_desc gets a short description of the chosen bin (for logging).
bool load_yields_from_json(const ThermalSelection& sel,
                           std::map<int,double>& outYields,
                           int* used_bin /*optional*/ = nullptr,
                           std::string* out_desc /*optional*/ = nullptr);

// Parse CLI flags related to thermal JSON selection, removing them from argv.
// Returns a filled ThermalSelection. If required flags are missing, prints usage and exits.
ThermalSelection parse_thermal_cli(int& argc, char**& argv);

// Print only the thermal-related CLI help lines.
void print_thermal_cli_help();
