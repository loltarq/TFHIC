# TFHIC

Toolkit for thermal/femtoscopic heavy-ion calculations in modern C++ (CMake build).  
*Thesis project for master's degree in Nuclear & Subnuclear Physics.*

---

## Table of Contents
- [Overview](#overview)
- [Repository Structure](#repository-structure)
- [Requirements](#requirements)
- [Build](#build)
- [Container](#container)
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
- **CMake:** ≥ **3.16**  
  *Reason:* top-level build drives all modules; subprojects inherit this requirement.

### Libraries
- **CERN ROOT 6.x** — **required only for `blastwave/` targets**  
  Used to build `libTFHIC.so`, `blastwave_thermal`, and `predict_light_spectra`.  
  Build guard: `-DTFHIC_WITH_ROOT=ON` (default).
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

### 1) Unified CMake build (all modules)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTFHIC_WITH_ROOT=ON
cmake --build build -j
```
Outputs:
- `build/bin/export_dndy_json`
- `build/bin/blastwave_thermal`
- `build/bin/predict_light_spectra`
- `build/lib/libTFHIC.so`

### 2) Thermal-only build (no ROOT)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTFHIC_WITH_ROOT=OFF
cmake --build build -j
```
Qt GUI (QtThermalFIST) is disabled by default. To enable it:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTFHIC_WITH_QT=ON
```

### 3) Install (optional)
```bash
cmake --install build --prefix /opt/tfhic
```

### 4) Optional env setup (recommended for bare filenames)
```bash
source build/tfhic-env.sh
```
After install:
```bash
source /opt/tfhic/share/tfhic/tfhic-env.sh
```
Note: Docker images already set `TFHIC_DATA/TFHIC_CONF/TFHIC_OUT`, so you don’t need this when running inside the container.

## Container

Build the image:
```bash
docker build -t tfhic:latest .
```

Run with a writable output mount:
```bash
docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest \
  /opt/tfhic/install/bin/blastwave_thermal --help
```
Docker runs already have `TFHIC_DATA=/opt/tfhic/install/share/tfhic/data`,
`TFHIC_CONF=/opt/tfhic/install/share/tfhic/conf`, and `TFHIC_OUT=/data/out`.

---

## Config & Run

### A) `build/bin/` (or `install/bin/`)

#### 1) export_dndy_json
Main one-liner executable: allows to compute absolute thermal yields of 1 or more hadrons specifying all configuration parameters via CLI flags.  
Yields and metadata output is stored in .json file for later use (e.g. blastwave pT spectrum normalization).    

Paths are resolved via CLI flags or env vars (`TFHIC_DATA`, `TFHIC_CONF`, `TFHIC_OUT`).  
You can `source build/tfhic-env.sh` (or the installed env script) to set `TFHIC_DATA/CONF` automatically.
Bare filenames are placed under `--out-dir` (or `TFHIC_OUT`, or the repo out dir when running from source).

**Requires args in the form of CLI flags**; programs prints guidance on missing args. Example:
```bash
./export_dndy_json
Usage:
  ./export_dndy_json --out PATH --list PATH/particles.dat [--decays PATH/decays.dat]
         --ensemble GCE|SCE|CE --width eBW|ZeroWidth|BWTwoGamma
         --species "211,-211,321,-321,2212,-2212" --k "1.0,1.6,3.0"
         [--QStats 1|0] [--feeddown primordial|weak|strong|em|stabilityflag]
         [--toGCE 0|1] --mode vanilla|gs  [flags per mode below]

Required:
  --out PATH_OR_NAME                        (no default; if only a name is given, outputs to out-dir)

Model & I/O (defaults shown):
  --list PATH/particles.dat                 (default: <TFHIC_folder>/thermal_yields/Thermal-FIST/input/list/PDG2014/list-withnuclei.dat)
  --decays PATH/decays.dat                  (default: <dir_of_list>/decays.dat)
  --ensemble                                (default: CE)
  --width                                   (default: eBW)
  --species                                 (default: 211,-211,321,-321,2212,-2212)
  --k                                       (default: 3)
  --toGCE 0|1                               (default: 0)
  --QStats 0|1                              (default: 1)
  --feeddown                                (default: stabilityflag)
  --mode vanilla|gs                         (default: vanilla)

Vanilla mode (no gammaS, defaults shown):
  --Tch                                     (default: 0.155 GeV)
  --v-min                                   (default: 10)
  --v-max                                   (default: 15000)
  --v-n                                     (default: 30)

 gammaS mode (defaults shown):
  --nch-min                                 (default: 3)
  --nch-max                                 (default: 2000)
  --nch-n                                   (default: 100)
  --nch-file PATH_OR_NAME                   (default: <unset>; if only a name is given, reads from ../conf/)
  --tch-a, --tch-b                          (defaults: 0.176, 0.0026)
  --gs-a, --gs-b, --gs-c                    (defaults: 1, 0.25, 59)
  --vol-a                                   (default: 2.4)
```

### B) `blastwave/` from ROOT

#### 1) blastwave_thermal
Main one-liner executable: allows for the computation of the pT spectrum configuring all relevant parameters via CLI flags.  
Currently reads blastwave parameter values from suitable .csv files, yields from either thermal .json(s) or custom .csv files.  

Paths are resolved via CLI flags or env vars (`TFHIC_DATA`, `TFHIC_OUT`).  

**Requires args in the form of CLI flags**; programs prints guidance on missing args. Example:
```bash
cd build/bin
./blastwave_thermal
Usage:  ./blastwave_thermal
  --thermal-json FILE               path or bare filename; if no path, looks in data-dir
  OR
  --yields-csv FILE                 use experimental yields from CSV (instead of thermal JSON)
Options:
  --primordial                      (thermal) use JSON primordial yields (default: total)
  --mode gammaS|vanilla             (thermal) select JSON bins by mode (default: gammaS)
  --k k1[,k2,...]                   (thermal) restrict to these k values (default: all in JSON)
  --cent N                          take first N centralities per k (default: auto)
  --species PDG[,PDG,...]           restrict to these PDGs (default: all common)
  --pt min,max,nbins                pT grid (default: 0,10,400). Use --timesPt for dN/dpT.
  --out FILE.root                   path or bare filename; if no path, outputs in out-dir
  --data-dir PATH                   base data dir for bare filenames
  --out-dir PATH                    base output dir for bare filenames
  --bw-csv FILE.csv                 BW params csv file (default: bw_data_1303.0737.csv)
  --bw-path  DIR                    base path for the BW csv file (default: data-dir)
  --timesPt                         returns spectra as dN/dPt instead of (1/Pt)dN/dPt
  --clampR                          num stability: clamp fireball radius instead of forcing subluminal beta in blastwave calculation routine
  --tgraph                          store spectra as TGraph(s) instead of THist(s)
  --help                            show this help
  --verbose                         run with verbose output
```

#### 2) predict_light_spectra
Fits the Pb–Pb blast-wave parameters vs. multiplicity, evaluates them at user-provided dN/dη targets (e.g. O–O / Ne–Ne), interpolates thermal yields from a gammaS JSON scan, and produces normalized pT spectra (one PDF per species plus a ROOT file with graphs).  
Example (run from `build/bin`):
```
./predict_light_spectra \
  --thermal-json yields_CE_k6_gs_NchScan.json \
  --systems "OO:60,120;NeNe:150,220" \
  --k 6 --pt 0,10,400 --mode gammaS --pdf predict_OO_NeNe.pdf
```
Use `--help` for the full list of knobs (fit formulas, species list, primordial/total yields, etc.).

#### 3) libTFHIC.so (legacy)
Legacy shared library that allows to use the blastwave calculation routines to compute the pT spectrum of a hadron given the blastwave parameters and (optionally) a target yield for normalization.  
The blastwave routines store data in TGraph or TH1D objects; these can be analyzed with ROOT helper macros.

```bash
cd build/lib
root -l
```
In the ROOT prompt:
```
root [0] .L libTFHIC.so
```

### Notes
- For reference on the thermal routines args and blastwave flow calculations see `docs/physics.md`.
  
---

## Validation

**Thermal sanity checks.** Canonical suppression increases with |S| and decreases with larger correlation volume Vc; proton/kaon/pion ordering behaves as expected.

**Spectra comparison.** Blast-wave spectra reproduce the qualitative pₜ-shapes of reference data. See `docs/plots` for some samples.

### **Docker quick test (pO/OO/NeNe):**
Run the following from the repo root (outputs land in `./out` on the host):
```bash
sudo docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest \
  /opt/tfhic/install/bin/export_dndy_json \
  --out yields-pO-NeNe-OO-gs.json \
  --nch-file Nch_pO_NeNe_OO.txt \
  --mode gs \
  --species 211,321,2212,3122,3312,3334,1000010020

sudo docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest \
  /opt/tfhic/install/bin/predict_light_spectra \
  --thermal-json /data/out/yields-pO-NeNe-OO-gs.json \
  --systems "pO:33.3899,25.5256,20.5644,16.2348,13.0413,10.3316,8.0894,4.7780;OO:129.6660,106.8340,87.2877,67.1562,51.1201,37.8919,26.9060,11.6993;NeNe:158.4020,131.3850,107.4230,82.2728,62.1508,45.8975,32.4722,13.5333" \
  --mode gammaS \
  --species 211,321,2212,3122,3312,3334,1000010020 \
  --timesPt --pt 0,10,400 \
  --out /data/out/prediction.root \
  --pdf /data/out/prediction.pdf
```
Note: Using `sudo` will create root-owned files in `./out`. If you want user-owned outputs, add `--user "$(id -u):$(id -g)"` to each `docker run`.

### **Reproduce the spectra plots (w/ thermal yields):**
1. Generate thermal yields directly into the blastwave data dir (or use `--out` with a full path):
   ```bash
   cd build/bin
   ./export_dndy_json \
     --out yields_CE_k1.6_k3_k6_gs_1303.0737.json \
     --out-dir ../blastwave/data \
     --k "1.6,3.0,6.0" --mode "gs" \
     --nch-file Nch_PbPb_1303.0737_ALICE_template.txt \
     --conf-dir ../thermal_yields/conf
   ```
2. Run blastwave calculation on generated data as follows:
   ```bash
   cd build/bin
   ./blastwave_thermal \
     --thermal-json yields_CE_k1.6_k3_k6_gs_1303.0737.json \
     --data-dir ../blastwave/data \
     --tgraph \
     --out spectra_yields_CE_k1.6_k3_k6_gs_1303.0737_tgraph.root \
     --out-dir ../blastwave/out
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
Deprecated workflow removed; use the JSON-based `export_dndy_json` + `blastwave_thermal` path above.

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
