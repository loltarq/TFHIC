#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// Minimal definition used across modules (separate from plotting-specific Hadron struct)
struct HadronDef {
  int pdg = 0;
  std::string name;    // short (ASCII)
  std::string latex;   // for ROOT titles, etc.
  double mass_GeV = 0.0;
  std::string token;   // "PI", "K", "P", "KS", ...
  int color = 1;       // ROOT-like int is fine
  double ctau_m = 0.0;      // meters; 0 means “treat as stable” for our purposes
  bool is_unstable() const { return ctau_m > 0.0; }
};

class HadronCatalog {
public:
  // Load from JSON file. Returns false and sets *err on error.
  bool load(const std::string& path, std::string* err=nullptr);

  // Lookup helpers
  const HadronDef* get(int pdg) const;
  bool has(int pdg) const { return byPdg_.count(pdg) != 0; }
  std::string tokenFor(int pdg) const;  // returns "ALL" if unknown
  std::vector<int> pdgs() const;

private:
  std::unordered_map<int, HadronDef> byPdg_;
};
