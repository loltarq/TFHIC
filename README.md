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
- Model parameters: T, μ_B, ⟨β_T⟩, T_kin.
> More details in `docs/physics.md`.

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
  Used to build and load `blastwave/libTFHIC.so` and `blastwave/bin/blastwave_thermal` (`root-config` is invoked by the Makefile; typical workflow is loading the `.so` in ROOT/Cling and running macros).
  - Installation instructions available @ https://root.cern/install
  - Verify version: `which root-config` and `root-config --version`

- **Bundled/Third-party (no system install needed):**
  - **Eigen3** — vendored under `thermal_yields/Thermal-FIST/thirdparty/Eigen3`
  - **Minuit2** — vendored under `thermal_yields/Thermal-FIST/thirdparty/Minuit2`; uses module from ROOT installation if available
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
This produces the following executables:
- `thermal_yields/build/bin/export_dndy_json`
- `thermal_yields/build/bin/TF_CSM-vs-dNpidy`

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
This builds the following:
- `blastwave/libTFHIC.so`
- `blastwave/bin/blastwave_thermal`

Rebuild after changes to core src:
```bash
make clean
make
```

---

## Config & Run

### A) `thermal_yields/build/bin/`

#### 1) export_dndny_json
Main one-liner executable: allows to compute absolute thermal yields of 1 or more hadrons specifying all configuration parameters via CLI flags.  
Yields and metadata output is stored in .json file for later use (e.g. blastwave pT spectrum normalization).    

**Run from the build directory** so outputs land in `thermal_yields/out/` by default;    

**Requires args in the form of CLI flags**; programs prints guidance on missing args. Example:
```bash
./export_dndny_json
Usage:
  ./export_dndy_json --out PATH --list PATH/particles.dat [--decays PATH/decays.dat]
         --ensemble GCE|SCE|CE --width eBW|ZeroWidth|BWTwoGamma
         --species "211,-211,321,-321,2212,-2212" --k "1.0,1.6,3.0"
         [--QStats 1|0] [--feeddown primordial|weak|strong|em|stabilityflag]
         [--toGCE 0|1] --mode vanilla|gs  [flags per mode below]

Required:
  --out PATH_OR_NAME                (no default; if only a name is given, outputs to ../out/)

Model & I/O (defaults shown):
  --list PATH/particles.dat      		 (default: <TFHIC_folder>/thermal_yields/Thermal-FIST/input/list/PDG2014/list-withnuclei.dat)
  --decays PATH/decays.dat          (default: <dir_of_list>/decays.dat)
  --ensemble                     		 (default: CE)
  --width                        		 (default: eBW)
  --species                      		 (default: 211,-211,321,-321,2212,-2212)
  --k                            		 (default: 3)
  --toGCE 0|1                    		 (default: 0)
  --QStats 0|1                   		 (default: 1)
  --feeddown                     		 (default: stabilityflag)
  --mode vanilla|gs              		 (default: vanilla)

Vanilla mode (no gammaS, defaults shown):
  --Tch                          			(default: 0.155 GeV)
  --v-min                        			(default: 10)
  --v-max                        			(default: 15000)
  --v-n                          			(default: 30)

 gammaS mode (defaults shown):
  --nch-min                      			(default: 3)
  --nch-max                      			(default: 2000)
  --nch-n                        			(default: 100)
  --nch-file PATH_OR_NAME           (default: <unset>; if only a name is given, reads from ../conf/)
  --tch-a, --tch-b               			(defaults: 0.176, 0.0026)
  --gs-a, --gs-b, --gs-c         		 (defaults: 1, 0.25, 59)
  --vol-a                        			(default: 2.4)
```


#### 2) TF_CSM-vs-dNpidy (legacy)
Legacy program: allows to compute yields ratios w.r.t. specific hadrons given a statistical ensemble and model variant.  
Output stored in column-based .dat file.  

**Set thermal model analysis configuration** in `thermal_yields/conf/_AnalysisConfig.config`:
- line 1: output file relative path; only used if runtime arg custom output is set to 1.
- line 2: relative path to particle injection list for thermal model.
- line 3: list of particles to analyze; txt file stored in same folder as config file.
- line 4: resonance width scheme to use in thermal model.
- line 5: correlation volume factors for scan.

**Run from the build directory** so outputs land in `thermal_yields/out/` by default;  

**Requires args**; the program prints guidance on missing args. Example:
```bash
cd thermal_yields/build
./TF_CSM-vs-dNpidy
# Not enough arguments provided
# Required arguments, in order: custom output file flag [0,1], toGCE flag [0,1], Ensemble [GCE,CE,SCE], GammaS model flag [0,1], Ensemble, GammaS model flag ...
# E.g. to compute yield ratios to GCE in Vanilla Strangeness-canonical and GammaS full canonical picture, with default output file, run the script as follows:
# ./TF_CSM-vs-dNpidy 0 1 SCE 0 CE 1
```

Thermal yield results are stored under the `thermal_yields/out/` folder in .dat files.

### B) `blastwave/` from ROOT

#### 1) bin/blastwave_thermal
Main one-liner executable: allows for the computation of the pT spectrum configuring all relevant parameters via CLI flags.  
Currently reads blastwave parameter values from suitable .csv files, yields from either thermal .json(s) or custom .csv files.  

**Run from the build directory** so outputs land in `thermal_yields/out/` by default;    

**Requires args in the form of CLI flags**; programs prints guidance on missing args. Example:
```bash
cd blastwave/bin
./blastwave_thermal
Usage:  ./blastwave_thermal
  --thermal-json FILE          	 path or bare filename; if no path, looks in ../data/
  OR
  --yields-csv FILE            		use experimental yields from CSV (instead of thermal JSON)
Options:
  --primordial                 		(thermal) use JSON primordial yields (default: total)
  --mode gammaS|vanilla          (thermal) select JSON bins by mode (default: gammaS)
  --k k1[,k2,...]              		(thermal) restrict to these k values (default: all in JSON)
  --cent N                     		take first N centralities per k (default: auto)
  --species PDG[,PDG,...]      	 restrict to these PDGs (default: all common)
  --pt min,max,nbins           	 pT grid (default: 0,10,400). Use --timesPt for dN/dpT.
  --out FILE.root              		path or bare filename; if no path, outputs in ../out/
  --bw-csv FILE.csv            	 BW params csv file (default: bw_data_1303.0737.csv)
  --bw-path  DIR               		base path for the BW csv file (default: ../data)
  --timesPt                    		returns spectra as dN/dPt instead of (1/Pt)dN/dPt
  --clampR                     		num stability: clamp fireball radius instead of forcing subluminal beta in blastwave calculation routine
  --tgraph                     		store spectra as TGraph(s) instead of THist(s)
  --help                       		show this help
  --verbose                    		run with verbose output
```

#### 2) libTFHIC.so (legacy)
Legacy shared library that allows to use the blastwave calculation routines to compute the pT spectrum of a hadron given the blastwave parameters and (optionally) a target yield for normalization.  
The blastwave routines store data in TGraph or TH1D objects; these can be analyzed with ROOT helper macros.

```bash
cd blastwave
root -l
```
In the ROOT prompt:
```
root [0] .L libTFHIC.so
root [1] .L libTFHIC_test.cpp++     # ROOT wrapper macro
root [2] histo()                    # from test wrapper: generates spectra; writes ROOT files under out/
root [3] compareHepData()           # from test wrapper: optional comparison macro
root [4] compareHepData_asTGraphs() # from test wrapper: optional TGraph comparison
```

### Notes
- For reference on the thermal routines args and blastwave flow calculations see `docs/physics.md`.
  
---

## Validation

**Thermal sanity checks.** Canonical suppression increases with |S| and decreases with larger correlation volume Vc; proton/kaon/pion ordering behaves as expected.

**Spectra comparison.** Blast-wave spectra reproduce the qualitative pₜ-shapes of reference data. See `docs/plots` for some samples.

### **Reproduce the spectra plots (w/ thermal yields):**
1. Generate thermal yields and move them under blastwave/data (or specify suitable path after --out):
   ```bash
   cd /thermal_yields/build/bin
   ./export_dndy_json --out yields_CE_k1.6_k3_k6_gs_1303.0737.json --k "1.6,3.0,6.0" --mode "gs" --nch-file Nch_PbPb_1303.0737_ALICE_template.txt
   cd ../../out
   mv yields_CE_k1.6_k3_k6_gs_1303.0737.json ../../blastave/data/.
   ```
2. Run blastwave calculation on generated data as follows:
   ```bash
   cd /../bin
   ./blastwave_thermal --thermal-json ../data/yields_CE_k1.6_k3_k6_gs_1303.0737.json --tgraph --out spectra_yields_CE_k1.6_k3_k6_gs_1303.0737_tgraph.root
   ```
3. Run (custom) ROOT helper macro to compare against exp. data:
   ```
   root [0] .L thermalyields_test.cpp
   root [1] compareHepData_asTGraphs()
   ```
4. Spectra comparison results stored as .root files under `blastwave/out`; can be explored with a TBrowser instance:
   ```bash
   root [0] TBrowser* t = new TBrowser()
   # use UI to open and explore .root files
   ```

### **Legacy (w/ experimental yields):**
1. Run thermal yields with default config:
   ```bash
   cd thermal_yields/build/bin
   ./TF_CSM-vs-dNpidy 0 0 GCE 0
   ```
2. Move results to blastwave data folder:
   ```bash
   mv ../../out/*.dat ../../../blastwave/data/
   ```
3. Run blastwave flow propagation and compare with HEP data:
   ```bash
   root -l
   root [0] .L libTFHIC.so
   root [1] .L libTFHIC_test.cpp++
   root [2] histo()
   root [3] compareHepData_asTGraphs()
   ```
4. Spectra comparison results stored as .root files under `blastwave/out`; can be explored with a TBrowser instance:
   ```bash
   root [4] TBrowser* t = new TBrowser()
   # use UI to open and explore .root files
   ```

## Limitations
- Current executable interface is minimal; configuration split between simple txt files and rigid runtime input.
> Solved in v0.1.1: both thermal and blastwave module now feature CLI flag-based one-liner executables for configuration; input data format is now json or csv-based.

- Systematics not propagated to final spectra (for now).

## Roadmap
- [ ] Implement MC efficiency module.
- [x] Unify configuration via CLI flags or a single YAML file.
- [ ] Include systematics propragation.

## License
MIT © 2025 Lorenzo (loltarq). See [LICENSE](LICENSE).

> **Note on third-party code:** This repository vendors/depends on Thermal-FIST (GPL-3.0) and uses CERN ROOT.
> Third-party components retain their original licenses. When distributing binaries linked with Thermal-FIST,
> ensure compliance with **GPL-3.0**; your own original code in this repo is under **MIT**.


## References
[1] V. Vovchenko and H. Stoecker, *Thermal‑FIST: A package for heavy-ion collisions and hadronic equation of state*, *Comput. Phys. Commun.* **244**, 295–310 (2019). [arXiv:1901.05249](https://arxiv.org/abs/1901.05249), [doi:10.1016/j.cpc.2019.06.024](https://doi.org/10.1016/j.cpc.2019.06.024)  
