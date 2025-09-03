# TFHIC

Toolkit for thermal/femtoscopic heavy-ion calculations in modern C++ (CMake/make build).  
*Thesis project for master's degree in Nuclear & Subnuclear Physics.*

---

## Table of Contents
- [Overview](#overview)
- [Repository Structure](#repository-structure)
- [Requirements](#requirements)
- [Build](#build)
- [Config & Run](#config--run)
- [Validation](#validation)
- [License](#license)
- [References](#references)


## Overview
**Problem.** Determine particle-species detection efficiencies for heavy-ion collisions, resolving their dependence on pₜ and centrality.

**Approach.**
1) Generate expected pₜ spectra by combining statistical-hadronization (thermal) yields with a blast-wave flow parameterization.
2) Feed generated particles into an MC-based fast-simulation/reconstruction to measure detection and selection efficiencies.

**Scope & assumptions.**
- Hadron species: π±, K±, p/ p̄ (extendable).  
- Kinematic range: pₜ < 5 GeV/c; acceptance: |η| < 0.5.  
- Centrality classes: I to X.  
- Model parameters: T, μ_B, ⟨β_T⟩, T_kin (documented in `Configuration`).
- More details in `docs/physics.md`.

**Status.** Core spectrum/yield components implemented; the MC efficiency module is under development and scheduled next.


## Repository Structure
**blastwave/** - Blast-wave model components, spectra kernels  
**thermal_yields/** - Thermal/statistical hadronization routines  
**auxiliary/** - Helper macros, I/O utilities; not critical for build  

## Requirements

### Toolchain
- **Compiler:** C++17-capable (e.g., GCC ≥ 7, Clang ≥ 5, MSVC 19.14+).
- **CMake:** ≥ **2.8.11**  
  *Reason:* `thermal_yields/CMakeLists.txt` sets `cmake_minimum_required(VERSION 2.8)`, while the bundled **Thermal-FIST** uses `2.8.11`. Any newer 3.x is fine.
- **GNU Make:** needed for the `blastwave/` Makefile build.

### Libraries
- **CERN ROOT 6.x** — **required only for `blastwave/`**  
  Used to build and load `libTFHIC.so` (`root-config` is invoked by the Makefile; typical workflow is loading the `.so` in ROOT/Cling and running macros).
  - Installation instructions available @ https://root.cern/install
  - Verify version: `which root-config` and `root-config --version`

- **Bundled/Third-party (no system install needed):**
  - **Eigen3** — vendored under `thermal_yields/Thermal-FIST/thirdparty/Eigen3`
  - **Minuit2** — vendored under `thermal_yields/Thermal-FIST/thirdparty/Minuit2; uses module from ROOT installation if available`
  *(CMake includes these from the source tree; there’s no `find_package` for them.)*

### Notes
- `thermal_yields/src/CMakeLists.txt` sets `CMAKE_CXX_STANDARD 17`; the Thermal-FIST subproject enforces C++11 internally. Any modern compiler that supports C++17 will handle both.

## Build

### 0) Clone (include submodules)
```bash
git clone --recurse-submodules https://github.com/loltarq/TFHIC.git
# if you already cloned without submodules:
#   cd TFHIC && git submodule update --init --recursive
cd TFHIC
```

### A) `thermal_yields/` (CMake; ROOT **not required**)
```bash
cd thermal_yields
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```
This produces the executable in `thermal_yields/build/`:
- `TF_CSM-vs-dNpidy`

> Notes  
> • The project sets **C++17**.  
> • Eigen is bundled.  
> • If ROOT/Minuit2 is found, it’s used; otherwise a local Minuit2 is built.  
> • Run from the **build** dir: programs write results to `../out/`.

### B) `blastwave/` (Makefile; **requires ROOT 6**)
```bash
cd blastwave
make        # uses root-config to find headers/libs
```
This builds `blastwave/libTFHIC.so`.

Rebuild after changes to core src:
```bash
make clean
make
```

---

## Config & Run

### A) `thermal_yields/` examples
**Set thermal model analysis configuration** in `thermal_yields/conf/_AnalysisConfig.config`:
- line 1: output file relative path; only used if runtime arg custom output is set to 1.
- line 2: relative path to particle injection list for thermal model.
- line 3: list of particles to analyze; txt file stored in same folder as config file.
- line 4: resonance width scheme to use in thermal model.
- line 5: correlation volume factors for scan.

**Run from the build directory** so outputs land in `thermal_yields/out/`:

```bash
cd thermal_yields/build
```

- CSM vs. dN/dy (**requires args**; the program prints guidance on missing args). Example:
```bash
./TF_CSM-vs-dNpidy
# Not enough arguments provided
# Required arguments, in order: custom output file flag [0,1], toGCE flag [0,1], Ensemble [GCE,CE,SCE], GammaS model flag [0,1], Ensemble, GammaS model flag ...
# E.g. to compute yield ratios to GCE in Vanilla Strangeness-canonical and GammaS full canonical picture, with default output file, run the script as follows:
# ./TF_CSM-vs-dNpidy 0 1 SCE 0 CE 1
```

Thermal yield results are stored under the `thermal_yields/out/` folder in .dat files. These files serve as input for the blastwave flow parametrization, and need to be moved under the `blastwave/data` folder for this purpose.

### B) `blastwave/` from ROOT
```bash
cd blastwave
root -l
```
In the ROOT prompt:
```
root [0] .L libTFHIC.so
root [1] .L test.cpp++
root [2] histo()                    // generates spectra; writes ROOT files under out/
root [3] compareHepData()           // optional comparison macro
root [4] compareHepData_asTGraphs() // optional TGraph comparison
```

### Notes
- For reference on the thermal routines args and blastwave flow calculations see `docs/physics.md`.
  
---

## Validation

**Thermal sanity checks.** Canonical suppression increases with |S| and decreases with larger correlation volume V_c; proton/kaon/pion ordering behaves as expected.

**Spectra comparison.** Blast-wave spectra reproduce the qualitative pₜ-shapes of reference data. See `docs/plots` for some samples.

**Reproduce the spectra plots:**
1. Run thermal yields with default config:
   ```bash
   cd thermal_yields/build
   ./TF_CSM-vs-dNpidy 0 0 GCE 0
   ```
2. Move results to blastwave data folder:
   ```bash
   mv ../out/*.dat ../../blastwave/data/
   ```
3. Run blastwave flow propagation and compare with HEP data:
   ```bash
   root -l
   .L libTFHIC.so
   .L test.cpp++
   histo()
   compareHepData_asTGraphs()
   ```
4. Spectra comparison results stored as .root files under `blastwave/out`; can be explored with a TBrowser instance:
   ```bash
   # in ROOT
   TBrowser* t = new TBrowser()
   # use UI to open and explore .root files
   ```

## Limitations
- Current executable interface is minimal; configuration split between simple txt files and rigid runtime input.
- Systematics (model parameter uncertainties) not propagated to final spectra.

## Roadmap
- [ ] Implement MC efficiency module.
- [ ] Unify configuration via CLI flags or a single YAML file.
- [ ] Include systematics propragation.

## License
MIT © 2025 Lorenzo (loltarq). See [LICENSE](LICENSE).

> **Note on third-party code:** This repository vendors/depends on Thermal-FIST (GPL-3.0) and uses CERN ROOT.
> Third-party components retain their original licenses. When distributing binaries linked with Thermal-FIST,
> ensure compliance with **GPL-3.0**; your own original code in this repo is under **MIT**.


## References
[1] V. Vovchenko and H. Stoecker, *Thermal‑FIST: A package for heavy-ion collisions and hadronic equation of state*, *Comput. Phys. Commun.* **244**, 295–310 (2019). [arXiv:1901.05249](https://arxiv.org/abs/1901.05249), [doi:10.1016/j.cpc.2019.06.024](https://doi.org/10.1016/j.cpc.2019.06.024)  
