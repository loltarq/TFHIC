#ifndef HADRON_LIST_H
#define HADRON_LIST_H

#include <vector>
#include <string>

// Struct to hold particle info
struct Hadron
{
    std::string name;
    double mass;     // in GeV
    int color;       // no QCD shenanigans, just ROOT plotting
};

// Struct to hold integration data for each hadron
struct HadronIntegrationInfo
{
    Hadron hadron;
    double yield; // to normalize integration in pT

    // integration parameters
    double beta_t;
    double Tkin;
    double n_profile;

    // uncertainties
    double beta_t_unc1, beta_t_unc2;
    double Tkin_t_unc1, Tkin_t_unc2;
    double n_profile_unc1, n_profile_unc2;
};

// List of hadrons: pion, kaon, proton
extern Hadron pion;
extern Hadron kaon;
extern Hadron proton;

#endif