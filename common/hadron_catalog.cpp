#include "hadron_catalog.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

static std::string upper(std::string s){ for(char&c:s) c=std::toupper(c); return s; }

static void fill_from_json(HadronDef& h, const json& it, int fallback_pdg=0){
  h.pdg = it.value("pdg", fallback_pdg);
  h.name = it.value("name", std::string());
  h.token = upper(it.value("token", std::string()));
  if(h.token.empty() && !h.name.empty()){
    // Try to derive token from name (first alpha block)
    for(char c: h.name){ if(std::isalpha((unsigned char)c)) h.token.push_back(std::toupper(c)); else if(!h.token.empty()) break; }
    if(h.token.empty()) h.token = "UNK";
  }
  // Accept several mass key variants
  if(it.contains("mass_GeV")) h.mass_GeV = it["mass_GeV"].get<double>();
  else if(it.contains("mass")) h.mass_GeV = it["mass"].get<double>();
  else if(it.contains("m"))    h.mass_GeV = it["m"].get<double>();
  else if(it.contains("massGeV")) h.mass_GeV = it["massGeV"].get<double>();
  else h.mass_GeV = 0.0;

  // c*tau in meters, if available (non-critical)
  if(it.contains("ctau_m")) h.ctau_m = it["ctau_m"].get<double>();
  else if(it.contains("ctau")) h.ctau_m = it["ctau"].get<double>();
  else h.ctau_m = 0.0;

  h.Z = it.value("Z", 0);
  h.A = it.value("A", 0);
}

bool HadronCatalog::load(const std::string& path, std::string* err){
  byPDG_.clear();
  std::ifstream in(path);
  if(!in){ if(err) *err="cannot open "+path; return false; }
  json j; in >> j;

  auto ingest_array = [&](const json& arr)->bool{
    if(!arr.is_array()) return false;
    for(const auto& it : arr){
      if(!it.is_object()) continue;
      HadronDef h; fill_from_json(h, it, it.value("pdg", 0));
      if(h.pdg!=0) byPDG_[h.pdg] = h;
    }
    return !byPDG_.empty();
  };

  // Case 1: top-level array
  if(ingest_array(j)) return true;

  // Case 2: object with a known array field
  if(j.is_object()){
    const char* keys[] = {"hadrons","particles","entries","data","list"};
    for(const char* k: keys){
      if(j.contains(k) && ingest_array(j.at(k))) return true;
    }
    // Case 3: object mapping PDG string -> hadron object
    bool any=false;
    for(auto it = j.begin(); it!=j.end(); ++it){
      if(!it.value().is_object()) continue;
      int pdg = 0;
      try{ pdg = std::stoi(it.key()); } catch(...) { pdg = it.value().value("pdg", 0); }
      HadronDef h; fill_from_json(h, it.value(), pdg);
      if(h.pdg!=0){ byPDG_[h.pdg] = h; any=true; }
    }
    if(any) return true;
  }

  if(err) *err = "unrecognized JSON schema (expected array or object with array/map)";
  return false;
}

const HadronDef* HadronCatalog::get(int pdg) const {
  auto it = byPDG_.find(pdg); if(it==byPDG_.end()) return nullptr;
  return &it->second;
}
