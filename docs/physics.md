# (Some) Physics background

This repository uses a **statistical hadronization (thermal) model** and a **blast-wave** description for spectra. 
This approach is based on the notion that, in heavy-ion collisions, primary hadron yields can be determined assuming production from a thermalized source.
At **chemical freeze-out**, primary particle yields are computed from a **hadron resonance gas (HRG)** in thermal and chemical equilibrium, characterized by a **temperature T** and **chemical potentials μ**. Conservation of charges can be enforced either on average (GCE) or exactly (canonical ensembles), which introduces *canonical suppression* factors. The programs in `thermal_yields/` explore how yields/ratios change with **system size** and **ensemble choice**, providing insight into freeze-out conditions across different collision systems.
At **kinetic freeze-out**, hadrons decouple from the system's collective expansion and stop interacting. The blast-wave model describes the **transverse momentum (p<sub>T</sub>)** spectra by combining:

- A thermal component characterized by a **kinetic temperature T<sub>kin</sub>**, and
- A collective radial flow component parameterized by a **transverse velocity profile β<sub>T</sub>(r)**.

This separation allows one to generate full p<sub>T</sub> spectra for each hadron species, using only the total yields (from the thermal model) and a few macroscopic parameters. The output can then be used to compare with experimental spectra or as input to simulations for computing detection efficiencies.

## Parameters

Yields are computed with a hadron resonance gas at chemical freeze-out (T, μ), implemented via **Thermal-FIST** [1]. Exact conservation of charges in small systems is handled with canonical ensembles (SCE/CE), which introduce a correlation-volume parameter `V_c` that suppresses strange (and, in CE, also baryon/charge-carrying) hadrons. We explore system-size effects by scanning either `V_c` directly or by mapping measured multiplicity `dN/dy` to a thermal volume `V`. A strangeness-saturation factor `γ_s≤1` optionally accounts for incomplete equilibration. Spectra are obtained with a blast-wave model using the kinetic freeze-out temperature `Tₖᵢₙ` and the average transverse flow veloctity ⟨β_T⟩.  




### Thermal Model Ensembles
- **GCE** (Grand Canonical): B, Q, S conserved *on average*. No canonical suppression; correlation volume **V<sub>C</sub>** is irrelevant.
- **SCE** (Strangeness Canonical): exact S conservation; B and Q treated grand-canonically. Strange hadrons receive a suppression factor that is stronger for larger |S| and smaller V<sub>C</sub>.
- **CE** (Canonical for B, Q, S): exact conservation of all three charges; strongest overall suppression.

### Key model knobs

| Name / flag concept | Meaning | Units | Typical range | Affects |
|---|---|---:|---|---|
| `T` | Chemical freeze-out temperature | MeV | 140–165 | all yields/ratios |
| `μ_B`, `μ_Q`, `μ_S` | Baryon/charge/strangeness chemical potentials (set by constraints like ⟨S⟩=0, ⟨Q⟩/⟨B⟩≈const) | MeV | 0–50 (at LHC), up to few hundred (lower √s) | yields, p/ p̄ asymmetry |
| `V` | Fireball volume at freeze-out (overall multiplicity scale) | fm³ | 10–10³+ | absolute yields |
| `V_c` | **Correlation volume** for canonical conservation (size of the “charge-conserving cell”) | fm³ | 1–100 | canonical suppression strength |
| `γ_s` (GammaS) | Strangeness saturation (γ_s=1 is fully equilibrated; γ_s<1 under-saturated) | — | 0.6–1.0 | strange-hadron yields |
| `ensemble` | One of `GCE`, `SCE`, `CE` | — | — | suppression pattern |
| `dN/dy` (π or charged) | Rapidity density used as a **proxy for system size**; the code maps this to `V` (and sometimes `V_c`) at fixed `T` | — | centrality-dependent | volume estimate |

> **Units & conventions.** Energies in **MeV**, volumes in **fm³**, momenta in **GeV/c**; yields are per unit rapidity.

### Thermal scan

- **`TF_CSM-vs-Vc`**  
  *Canonical model vs correlation volume.*  
  Keeps the thermodynamic state fixed (T, μ, γ_s, ensemble choice) and **scans `V_c`**, writing yields/ratios as functions of `V_c` to `thermal_yields/out/`. Use this to illustrate canonical suppression vs system size.

- **`TF_VCSM-vs-dNpidy`**  
  *Correlation-volume model vs multiplicity proxy.*  
  Treats **`dN_π/dy`** (or another multiplicity proxy) as input and maps it to an effective `V_c = V_c(dN/dy)` using the thermal model at fixed T and constraints. Outputs trends vs centrality/multiplicity.

- **`TF_CSM-vs-dNpidy`**  
  *Canonical model vs multiplicity proxy.*  
  Similar to the above but scanning the **fireball volume `V = V(dN/dy)`** (and, if canonical, applying suppression set by the chosen `ensemble` and `V_c`).  
  The program accepts small integer/word flags for convenience; when run with missing/invalid args it prints a short guide and an example. Conceptually, the flags toggle:  
  1) custom output naming,
  2) whether to compute yields as ratios against the GCE values,
  3) the **ensemble** for the calculation (e.g., `GCE`, `SCE`, `CE`),
  4) whether to allow **γ_s≠1**.

### Brief blast-wave notes
- **Tₖᵢₙ**: kinetic freeze-out temperature (controls spectral slope at low pₜ).  
- **⟨β_T⟩ or ρ₀, n**: mean transverse flow (and profile shape).  
- **dN/dy**: overall scale/normalization (fixed by the thermal yields).  
The blast-wave folds thermal emission with radial flow to produce **pₜ spectra**; it is separate from chemical yields but uses consistent particle lists and masses.

### References and further readings
[1] V. Vovchenko and H. Stoecker, *Thermal‑FIST: A package for heavy-ion collisions and hadronic equation of state*, *Comput. Phys. Commun.* **244**, 295–310 (2019). [arXiv:1901.05249](https://arxiv.org/abs/1901.05249), [doi:10.1016/j.cpc.2019.06.024](https://doi.org/10.1016/j.cpc.2019.06.024)  
[2] V. Vovchenko, B. Dönigus, and H. Stoecker, *Canonical statistical model analysis of p‑p, p‑Pb, and Pb‑Pb collisions at the LHC*, *Phys. Rev. C* **100**, 054906 (2019). [arXiv:1906.03145](https://arxiv.org/abs/1906.03145), [doi:10.1103/PhysRevC.100.054906](https://doi.org/10.1103/PhysRevC.100.054906)  
[3] ALICE Collaboration, *Centrality dependence of π, K, p production in Pb–Pb collisions at √sₙₙ = 2.76 TeV*, *Phys. Rev. C* **88**, 044910 (2013). [arXiv:1303.0737](https://arxiv.org/abs/1303.0737), [doi:10.1103/PhysRevC.88.044910](https://doi.org/10.1103/PhysRevC.88.044910)


