#include "hadron_catalog.h"
#include <fstream>
#include <sstream>

#if __has_include(<nlohmann/json.hpp>)
  #include <nlohmann/json.hpp>
  using json = nlohmann::json;
#else
  #include "third_party/json.hpp"
  using json = nlohmann::json;
#endif

bool HadronCatalog::load(const std::string& path, std::string* err) {
  byPdg_.clear();
  std::ifstream in(path);
  if(!in) {
    if (err) *err = "Cannot open hadron catalog JSON: " + path;
    return false;
  }
  json j;
  try { in >> j; }
  catch (const std::exception& e) {
    if (err) *err = std::string("JSON parse error: ")+e.what();
    return false;
  }
  if (!j.contains("hadrons") || !j["hadrons"].is_array()) {
    if (err) *err = "Missing 'hadrons' array in catalog";
    return false;
  }
  for (const auto& h : j["hadrons"]) {
    HadronDef d;
    d.pdg      = h.value("pdg", 0);
    d.name     = h.value("name", "");
    d.latex    = h.value("latex", d.name);
    d.mass_GeV = h.value("mass_GeV", 0.0);
    d.token    = h.value("token", "ALL");
    d.color    = h.value("color", 1);
    if (d.pdg == 0 || d.mass_GeV <= 0) continue;
    byPdg_[d.pdg] = std::move(d);
  }
  return !byPdg_.empty();
}

const HadronDef* HadronCatalog::get(int pdg) const {
  auto it = byPdg_.find(pdg);
  return (it==byPdg_.end()) ? nullptr : &it->second;
}

std::string HadronCatalog::tokenFor(int pdg) const {
  auto* d = get(pdg);
  return d ? d->token : "ALL";
}

std::vector<int> HadronCatalog::pdgs() const {
  std::vector<int> v; v.reserve(byPdg_.size());
  for (auto& kv : byPdg_) v.push_back(kv.first);
  return v;
}
