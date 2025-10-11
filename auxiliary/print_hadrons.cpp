#include "hadron_catalog.h"
#include <iostream>
#include <vector>

int main(int argc, char** argv){
  std::string path = (argc>1) ? argv[1] : "data/hadrons.json";
  HadronCatalog cat;
  std::string err;
  if(!cat.load(path, &err)){
    std::cerr << "Failed to load catalog: " << err << "\n";
    return 2;
  }
  auto ids = cat.pdgs();
  std::cout << "Loaded " << ids.size() << " hadrons from " << path << "\n";
  for(int pdg : ids){
    const auto* d = cat.get(pdg);
    std::cout << pdg << "\t" << d->name << "\tm=" << d->mass_GeV
              << "\ttoken=" << d->token << "\n";
  }
  return 0;
}
