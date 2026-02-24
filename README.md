# ThermoWave (previously TFHIC)

Thermal + blast-wave pipeline for multiplicity-resolved identified-hadron pT spectra in ion collisions. 
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
**Problem.** Predict identified-hadron pT spectra in heavy-ion and small collision systems by combining thermal yields with blast-wave flow fits, and extrapolating these to target centralities or multiplicities.

**Approach.**
1) Compute per-species yields at chemical freeze-out with a statistical hadronization model (Thermal-FIST).
2) Fit blast-wave parameters vs centrality/multiplicity from a reference dataset (e.g., Pb-Pb).
3) Extrapolate/interpolate those parameters to target systems, generate spectra, and normalize with the thermal yields.

**Scope & assumptions.**
- Focus on light/strange hadrons (configurable PDG list) and midrapidity yields.
- Thermal model: GCE/SCE/CE with optional gammaS variant or fixed Tch (vanilla).
- Blast-wave parameters are taken from literature tables and parameterized vs dNch/deta.
- Outputs are ROOT/PDF quick looks for spectra comparisons, not a full detector simulation.
> More details in `docs/physics.md`.

**Status.** Core thermal+yields and blast-wave prediction pipeline implemented; container build and analysis helpers are included.


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
git clone --recurse-submodules https://github.com/loltarq/ThermoWave.git
# if you already cloned without submodules:
#   cd ThermoWave && git submodule update --init --recursive
cd ThermoWave
```

There are 2 main options to build & run ThermoWave: dev build (A), and docker (B).
A third one - installation - is also available, but has not yet been extensively tested and may be not working correctly.

### A) Dev-build: unified CMake
At the repo root:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTFHIC_WITH_ROOT=ON
cmake --build build -j
```
Outputs:
- `build/bin/export_dndy_json`
- `build/bin/blastwave_thermal`
- `build/bin/predict_light_spectra`
- `build/lib/libTFHIC.so`

Notes: 
- ThermalFIST's Qt GUI is disabled by default, as it's not integral to this program. Regardless, a cmake option is available to enable it:
```bash
cmake -S . ... -DTFHIC_WITH_QT=ON
```
- It is possible to perform a ROOT-less build, albeit only the `thermal_yields` module would be built in this case:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTFHIC_WITH_ROOT=OFF
cmake --build build -j
```
- After build, it is recommended to source the environment variables setup script `tfhic-env.sh`, which guarantees consistent path resolution when providing path-less strings as input/output file names:
```bash
source build/tfhic-env.sh
```
- Default path resolutions when providing bare file-names in the Dev-build layout:
> `TFHIC/build/bin/export_dndy_json`, out: `TFHIC/thermal_yields/out/`, in: `TFHIC/thermal_yields/conf/`
> `TFHIC/build/bin/blastwave_thermal`, out: `TFHIC/blastwave/out/`, in: `TFHIC/blastwave/data/`
> `TFHIC/build/bin/predict_light_spectra`, out: `TFHIC/blastwave/out/`, in: `TFHIC/blastwave/data/`

### (B) Docker Container

Build the image:
```bash
docker build -t tfhic:latest .
```

It is recommended to run it with a writable input/output mount:
```bash
docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest \
  /opt/tfhic/install/bin/blastwave_thermal --help
```
Docker runs don't need the env script sourced, as they already have `TFHIC_DATA=/opt/tfhic/install/share/tfhic/data`,
`TFHIC_CONF=/opt/tfhic/install/share/tfhic/conf`, and `TFHIC_OUT=/data/out` set.

### C) Installation (with a grain of salt)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTFHIC_WITH_ROOT=ON -DCMAKE_INSTALL_PREFIX=/opt/tfhic
cmake --build build -j
cmake --install build --prefix /opt/tfhic
source /opt/tfhic/share/tfhic/tfhic-env.sh
```
Notes:
- The installed `tfhic-env.sh` is generated at **configure time** using `CMAKE_INSTALL_PREFIX`. Replace the sample `/opt/tfhic` with your preferred installation path.
- Default path resolutions when providing bare file-names in the Installation layout:
> input data: `<CMAKE_INSTALL_PREFIX>/share/tfhic/data` and `<CMAKE_INSTALL_PREFIX>/share/tfhic/conf`, with fallbacks to the repo Dev-build default paths.
> output data: `<CMAKE_INSTALL_PREFIX>/bin/out`
- The installation layout has not been extensively tested, and may not work as intended.

---

## Config & Run

### A) `build/bin/` (or `<CMAKE_INSTALL_PREFIX>/bin/`)

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

### B) `blastwave/`

#### 1) blastwave_thermal
Main one-liner executable: allows for the computation of pT spectra configuring all relevant parameters via CLI flags.  
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
Fits an input blast-wave parameter set vs. multiplicity, evaluates it at user-provided dN/dη targets (e.g. O–O / Ne–Ne), interpolates thermal yields from a thermal JSON scan, and produces normalized pT spectra.  
Use the following for the full list of configuration flags:
```
./predict_light_spectra --help
```
See `docs/physics` section for reference uses.


#### 3) libTFHIC.so (shared lib)
Shared library used by the blastwave apps; enables blastwave calculation routines to compute the pT spectrum of a hadron given the blastwave parameters and (optionally) a target yield for normalization. Library methods can be loaded and accessed in a ROOT session, however they are not intended for direct use.

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

### **Reproduce the spectra plots:**
`libTFHIC.so` blastwave calculation routines test against experimental data. Reproduce Pt spectrum trends of [2], fig. 4. Can be run only with Dev-built layout.
1. From repo root:
    ```bash
    cd auxiliary/thermal_yields-test
    root
    ```
2. Load ROOT macro to generate comparison of spectra from blastwave model against exp. data:
   ```
   root [0] .L thermalyields_test.cpp+
   root [1] compareHepData_asTGraphs()
   ```
3. Spectra comparison results stored as .root files under `/out`; can be explored with a TBrowser instance:
   ```bash
   root [0] TBrowser* t = new TBrowser()
   # use UI to open and explore .root files
   ```
   
See `docs/plots/blastwave_exp` plots for some sample outputs.

### **Spectra extrapolation:**
See `docs/physics.md` section 8 "Sample analysis" and `docs/plots/predictions` for the corresponding outputs.

---

## Current Limitations
- Current executable interface is minimal; configuration split between simple txt files and rigid runtime input.
> Solved in v0.1.1: both thermal and blastwave module now feature CLI flag-based one-liner executables for configuration; input data format is now json or csv-based.

- Systematics not propagated to final spectra.

- Lacking ease-of-use and QOL improvements.

- Physics documentation yet to be expanded with thesis work.


## License
MIT © 2025 Lorenzo (loltarq). See [LICENSE](LICENSE).

> **Note on third-party code:** This repository vendors/depends on Thermal-FIST (GPL-3.0) and uses CERN ROOT.
> Third-party components retain their original licenses. When distributing binaries linked with Thermal-FIST,
> ensure compliance with **GPL-3.0**; your own original code in this repo is under **MIT**.


## References
[1] V. Vovchenko and H. Stoecker, *Thermal‑FIST: A package for heavy-ion collisions and hadronic equation of state*, *Comput. Phys. Commun.* **244**, 295–310 (2019). [arXiv:1901.05249](https://arxiv.org/abs/1901.05249), [doi:10.1016/j.cpc.2019.06.024](https://doi.org/10.1016/j.cpc.2019.06.024)  
[2] ALICE Collaboration, Centrality dependence of π, K, p production in Pb-Pb collisions at sqrt(sNN) = 2.76 TeV, [arXiv: 1303.0737] (https://arxiv.org/abs/1303.0737)
