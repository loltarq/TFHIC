# TFHIC

Toolkit for thermal/femtoscopic heavy-ion calculations in modern C++ (CMake build).  
*Thesis project for master's degree in Nuclear & Subnuclear Physics.*

---

## Table of Contents
- [Overview](#overview)
- [Repository Structure](#repository-structure)
- [Requirements](#requirements)
- [Build](#build)
- [Run / Usage](#run--usage)
- [Configuration](#configuration)
- [Testing](#testing)
- [Results & Validation](#results--validation)
- [Architecture](#architecture)
- [Roadmap](#roadmap)
- [License](#license)
- [Citation](#citation)

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

**Status.** Core spectrum/yield components implemented; the MC efficiency module is under development and scheduled next.


## Repository Structure
**blastwave/** - Blast-wave model components / spectra kernels  
**thermal_yields/** - Thermal/statistical hadronization routines  

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

## Run

### A) `thermal_yields/` examples
**Run from the build directory** so outputs land in `thermal_yields/out/` (the code writes to `../out/`):

```bash
cd thermal_yields/build
```

- CSM vs. dN/dy (**requires args**; the program prints guidance on missing args). Example:
```bash
./TF_CSM-vs-dNpidy 0 1 SCE 0 CE 1
# meaning:
#   custom output flag (0/1), {GammaS flag (0/1), Ensemble [GCE|CE|SCE], GammaS flag, Ensemble..}
#   outputs -> ../out/
```

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
- for reference on the thermal routines args see docs/physics.md
  
---

