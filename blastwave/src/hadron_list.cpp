
#include "hadron_list.h"
#include <TColor.h>

Hadron make_hadron_from_catalog(int pdg, const HadronCatalog& cat) {
    const auto* def = cat.get(pdg);
    Hadron h{};
    h.pdg  = pdg;
    h.mass = def ? def->mass_GeV : 0.13957039; // fallback: pi mass
    // Prefer LaTeX-friendly label for ROOT titles; fall back to ascii.
    h.name = def ? (!def->latex.empty() ? def->latex : def->name)
                 : ("pdg" + std::to_string(pdg));
    // If Hadron has more fields (charge, etc.), may be set here later
    return h;
  }
