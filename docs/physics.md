# Physics background

TFHIC predicts identified-hadron pT spectra by combining a thermal (statistical hadronization) description of yields with a blast-wave description of spectral shapes. The goal is to extrapolate spectra to collision systems or centrality bins where data are sparse, using published blast-wave fits and consistent thermal assumptions.

This document summarizes the physics motivation, modeling assumptions, and how the workflow maps to the code.

---

## 1. Motivation and scope

Heavy-ion collisions produce a strongly interacting medium that expands and cools. Two stages are relevant for hadron observables:

- **Chemical freeze-out**: inelastic reactions cease and hadron yields are fixed.
- **Kinetic freeze-out**: elastic scatterings cease and spectral shapes are fixed.

TFHIC uses:
- **Thermal yields** at chemical freeze-out.
- **Blast-wave spectra** at kinetic freeze-out.
- **Data-driven parameterizations** of blast-wave parameters vs centrality or dNch/deta.

The code is not a full event generator or detector simulation. It produces model spectra and quick-look outputs (ROOT/PDF) for comparison and feasibility studies.

---

## 2. Workflow summary (what the program does)

1) **Thermal yields**: compute per-species dN/dy at chemical freeze-out using a hadron resonance gas (HRG) model. Options include CE/SCE/GCE and a gammaS variant.
2) **Blast-wave fits**: read a CSV of published blast-wave parameters vs centrality (or dNch/deta), then fit betaT, Tkin, and n as functions of multiplicity.
3) **Extrapolation**: evaluate those fits at target multiplicities (for pO, OO, NeNe, etc.).
4) **Spectra**: generate pT spectra using the blast-wave kernel and normalize each species to the thermal yields.

Outputs are written to ROOT files (graphs and spectra) plus optional PDF quick looks.

---

## 3. Thermal model (statistical hadronization)

### 3.1 Ensembles

Conserved charges (B, S, Q) can be treated in different ensembles:
- **GCE**: charges conserved on average via chemical potentials.
- **SCE**: strangeness canonical, B and Q grand-canonical.
- **CE**: all charges canonical.

### 3.2 Canonical suppression and correlation volume

Exact conservation in a finite volume reduces yields of particles carrying conserved quantum numbers. The correlation volume is modeled as

  Vc = k * (dV/dy),  with  k >= 1,

where dV/dy is the chemical freeze-out volume per unit rapidity. Larger k weakens canonical suppression (GCE recovered as k -> infinity).

### 3.3 Strangeness saturation gammaS

gammaS accounts for strangeness undersaturation:

  Ni ∝ gammaS^(si)

with si the number of valence strange + anti-strange quarks. In the gammaS workflow, Tch and dV/dy are parameterized as functions of dNch/deta.

### 3.4 Primary vs total yields

Thermal-FIST provides:
- **Primary yields** (before resonance decays).
- **Total yields** (after decays, per chosen feeddown).

TFHIC can output both in the JSON tables.

---

## 4. Freeze-out scenarios used in TFHIC

### Two-step freeze-out (gammaS workflow)

This is the default when using `--mode gammaS`:
- Tch and dV/dy are functions of dNch/deta.
- Yields are tabulated vs dNch/deta for each k.
- Blast-wave parameters are fit vs dNch/deta and evaluated at target multiplicities.

### Single freeze-out (vanilla workflow)

For single freeze-out, Tch = Tkin is fixed (e.g., 150 MeV) and gammaS = 1. In `--mode vanilla`:
- Yields are tabulated vs dV/dy (not vs dNch/deta).
- Blast-wave parameters are still fit vs dNch/deta.
- A data-driven mapping Nch -> dV/dy is used to normalize the spectra consistently.

This mapping is built by pairing:
- dV/dy values from the blast-wave table (e.g., Table III), and
- Nch values from an external centrality table for the same bins.

Piecewise linear interpolation provides dV/dy(Nch) for target systems.

---

## 5. Blast-wave spectra

At kinetic freeze-out, the blast-wave model combines local thermal motion with a collective transverse flow profile. A standard kernel is:

  dN/(pT dpT dy) ∝ ∫0^R r dr mT I0(pT sinh rho / Tkin) K1(mT cosh rho / Tkin)

with mT = sqrt(pT^2 + m^2) and rho(r) = atanh betaT(r). The flow profile is:

  betaT(r) = beta_s * (r/R)^n,
  <betaT> = 2/(2+n) * beta_s.

Parameters:
- Tkin (kinetic freeze-out temperature)
- <betaT> (mean transverse flow)
- n (profile exponent)

TFHIC fits these parameters vs multiplicity and applies them to target systems.

---

## 6. Centrality, multiplicity, and mapping

TFHIC assumes a monotonic relation between:
- Centrality class <-> dNch/deta <-> dV/dy.

Key points:
- For gammaS mode, yields are tabulated vs dNch/deta directly.
- For vanilla mode, yields are tabulated vs dV/dy and mapped from Nch using a centrality-matched table.

This keeps the thermal normalization consistent with the freeze-out assumptions used by the blast-wave fits.

---

## 7. Outputs and interpretation

The prediction step produces:
- A ROOT file with fit graphs, fit functions, and spectra.
- A quick-look PDF (optional) with fitted curves and spectra plots.

These outputs are intended for comparison studies and feasibility assessments. They are not a replacement for full event simulations.

---

## 8. References (context)

[1] V. Vovchenko and H. Stoecker, Thermal-FIST, Comput. Phys. Commun. 244, 295 (2019).  
[2] ALICE Collaboration, Pb-Pb spectra at sqrt(sNN)=2.76 TeV, Phys. Rev. C 88, 044910 (2013).  
[3] Blast-wave fits and tables referenced in the repository CSV inputs (see `blastwave/data/`).
