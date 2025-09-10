
#ifndef HADRON_LIST_H
#define HADRON_LIST_H

#include <vector>
#include <string>

// Enable PDG support for apply_thermal_yields
#define HADRON_HAS_PDG 1

// Struct to hold particle info
struct Hadron
{
    std::string name;   // e.g. "pi^{+}"
    double mass;        // GeV
    int color;          // ROOT color code for plotting
#ifdef HADRON_HAS_PDG
    int pdg;            // PDG code (e.g. 211 for pi+, 321 for K+, 2212 for p)
#endif
};

// Struct to hold integration data for each hadron / centrality bin
struct HadronIntegrationInfo
{
    Hadron hadron;

    // Blast-wave parameters (kinetic freeze-out)
    double beta_t;      // <beta_T>
    double Tkin;        // GeV
    double n_profile;   // flow profile exponent n

    // Normalization target (dN/dy). This is what computePtSpectrum* will scale to.
    // Historically this came from experimental yields; we now allow replacing it with thermal-model yields.
    double yield;

    // Optional uncertainties for scanning/tuning (unused by core computation, kept for compatibility)
    double beta_t_unc1 = 0, beta_t_unc2 = 0;
    double Tkin_t_unc1 = 0, Tkin_t_unc2 = 0;
    double n_profile_unc1 = 0, n_profile_unc2 = 0;

    // Centrality class index (0..9 typically)
    int centrality_class = -1;
};

// Predefined hadrons
extern Hadron pion;
extern Hadron kaon;
extern Hadron proton;

#endif
