# Physics background & parameters

## Physics background & parameters (what the arguments mean)

This repository uses a **statistical hadronization (thermal) model** (via *Thermal-FIST*) and, optionally, a **blast-wave** description for spectra. At chemical freeze-out, primary particle yields are computed from a hadron resonance gas at temperature **T** and chemical potentials **μ**; conservation of charges can be enforced either on average (GCE) or exactly (canonical ensembles), which introduces *canonical suppression* factors. The programs in `thermal_yields/` explore how yields/ratios change with **system size** and **ensemble choice**.

### Ensembles (conceptual switch used by the executables)
- **GCE** (Grand Canonical): B, Q, S conserved *on average*. No canonical suppression; correlation volume **V_c** is irrelevant.
- **SCE** (Strangeness Canonical): exact S conservation; B and Q treated grand-canonically. Strange hadrons receive a suppression factor that is stronger for larger |S| and smaller **V_c**.
- **CE** (Canonical for B, Q, S): exact conservation of all three charges; strongest overall suppression.

### Key model knobs (appear as flags/choices in the executables)

| Name / flag concept | Meaning | Units | Typical range | Affects |
|---|---|---:|---|---|
| `T` | Chemical freeze-out temperature | MeV | 140–165 | all yields/ratios |
| `μ_B`, `μ_Q`, `μ_S` | Baryon/charge/strangeness chemical potentials (set by constraints like ⟨S⟩=0, ⟨Q⟩/⟨B⟩≈const) | MeV | 0–50 (at LHC), up to few hundred (lower √s) | yields, p/ p̄ asymmetry |
| `V` | Fireball volume at freeze-out (overall multiplicity scale) | fm³ | 10–10³+ | absolute yields |
| `V_c` | **Correlation volume** for canonical conservation (size of the “charge-conserving cell”) | fm³ | 1–100 | canonical suppression strength |
| `γ_s` (GammaS) | Strangeness saturation (γ_s=1 is fully equilibrated; γ_s<1 under-saturated) | — | 0.6–1.0 | strange-hadron yields |
| `ensemble` | One of `GCE`, `SCE`, `CE` | — | — | suppression pattern |
| `dN/dy` (π or charged) | Rapidity density used as a **proxy for system size**; the code maps this to `V` (and sometimes `V_c`) at fixed `T` | — | centrality-dependent | volume estimate |

> **Units & conventions.** Energies in **MeV**, volumes in **fm³**, momenta in **GeV/c**; yields are typically per unit rapidity.

### What each thermal executable scans

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
  1) output naming style,  
  2) whether to allow **γ_s≠1**,  
  3) the **ensemble** for the calculation (e.g., `GCE`, `SCE`, `CE`),  
  4) (optionally) the **reference ensemble/γ_s** when outputting ratios.

> **Tip.** If you’re unsure about the exact flag order on your machine, run the executable **without arguments** to get the built-in usage line and defaults; the physics meanings above are exactly what each toggle controls.

### (Short) blast-wave notes (if you use the spectra utilities)
- **Tₖᵢₙ**: kinetic freeze-out temperature (controls spectral slope at low pₜ).  
- **⟨β_T⟩ or ρ₀, n**: mean transverse flow (and profile shape).  
- **dN/dy** or normalization**: overall scale (often fixed by the thermal yields).  
The blast-wave folds thermal emission with radial flow to produce **pₜ spectra**; it is separate from chemical yields but uses consistent particle lists and masses.

### Suggested exam paragraph
> Yields are computed with a hadron resonance gas at chemical freeze-out (T, μ), implemented via Thermal-FIST. Exact conservation of charges in small systems is handled with canonical ensembles (SCE/CE), which introduce a correlation-volume parameter `V_c` that suppresses strange (and, in CE, also baryon/charge-carrying) hadrons. We explore system-size effects by scanning either `V_c` directly or by mapping measured multiplicity `dN/dy` to a thermal volume `V`. A strangeness-saturation factor `γ_s≤1` optionally accounts for incomplete equilibration. Spectra are obtained with a blast-wave model using `Tₖᵢₙ` and ⟨β_T⟩.

