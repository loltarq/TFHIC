# Changelog
All notable changes to this project will be documented here.

## [Unreleased]
- Unified top-level CMake build for all modules; removed blastwave Makefile.
- Added runtime path resolver with CLI/env overrides; supports install layout and repo fallbacks.
- Added Dockerfile (ROOT base image) and `.dockerignore` for reproducible container builds.
- Vendored full nlohmann/json headers for container portability.
- Added install-time env setup script (`build/tfhic-env.sh`, install to `share/tfhic`).
- Added CMake options `TFHIC_WITH_ROOT` and `TFHIC_WITH_QT` (Qt GUI disabled by default).
- Added RPATH for installed blastwave executables to find `libTFHIC.so`.
- Updated README build/run/validation instructions and Docker quick test.


## [0.1.0] - Initial public docs
### Adding
- Root README with build/run instructions
- Physics background (`docs/physics.md`) and docs landing page (`docs/README.md`)
- MIT `LICENSE`
- Code style via `.clang-format`
- `CITATION.cff` for academic citation

## [0.1.1] - Bridge between core modules
### Adding
- Json helpers (exporters/readers) for both thermal and blastwave modules
- New thermal routine `thermal_yields/src/export_dndy_json.cpp`, CLI flag-based for configuration, exports thermal yields in .json
- New blastwave stand-alone app `blastwave/apps/blastwave_thermal.cpp`, CLI flag-based for configuration, reads thermal yields from .json, experimental yields from .csv
### Updating
- Root README
- Physics documentation (`docs/physics.md`)
- fix to blastwave computePtSpectrum method in `blastwave/src/blastwave_utils.cpp` to perform normalization strictly on dN/dpT
