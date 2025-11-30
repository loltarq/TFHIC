#pragma once
#include <map>
#include <string>

struct HadronDef {
  int pdg = 0;
  std::string name;
  std::string token;      // short token like "PI","K","P","D","HE3","HE4"
  double mass_GeV = 0.0;
  double ctau_m   = 0.0;  // 0 for stable
  // Optional (default 0 if missing in JSON)
  int Z = 0;              // atomic number
  int A = 0;              // mass number
};

class HadronCatalog {
public:
  // Load a JSON file. Top-level may be:
  //  - an array of hadron objects
  //  - an object with a single array at keys: "hadrons","particles","entries","data","list"
  //  - an object map { "211": {pdg:211,...}, "-211": {...}, ... }
  bool load(const std::string& jsonPath, std::string* err=nullptr);
  const HadronDef* get(int pdg) const;
private:
  std::map<int, HadronDef> byPDG_;
};

// Signal downstream headers that A/Z exist
#define TFHIC_HADRONDEF_HAS_AZ 1
