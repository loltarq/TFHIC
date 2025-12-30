# Physics background

TFHIC predicts identified-hadron $p_T$ spectra by combining a thermal (statistical hadronization) description of yields with a blast-wave description of spectral shapes. The goal is to extrapolate spectra to collision systems or centrality bins where data are sparse, using published blast-wave fits and consistent thermal assumptions.

This document summarizes the physics motivation, modeling assumptions, and how the workflow maps to the code.

---

## 1. Motivation and scope

Heavy-ion collisions produce a strongly interacting medium that expands and cools. Two stages are relevant for hadron observables:

- **Chemical freeze-out**: inelastic reactions cease and hadron yields are fixed.
- **Kinetic freeze-out**: elastic scatterings cease and spectral shapes are fixed.

At earlier times the system is expected to form a deconfined quark-gluon plasma (QGP). Thermal models treat the hadronization stage as an equilibrated hadron resonance gas, capturing the bulk chemistry of the QGP-to-hadron transition through a small set of freeze-out parameters. Blast-wave parametrizations then encode the collective expansion of the medium at kinetic decoupling, translating the macroscopic flow field into species-dependent $p_T$ spectra.

A key motivation for TFHIC is to extrapolate from well-constrained Pb--Pb measurements to lfuture collision systems (Ne--Ne, O--O, p--O) that will be explored in upcoming ALICE runs. The workflow provides predictions in regimes where direct data are limited, while keeping consistency with the physics assumptions used in the reference Pb--Pb analyses.

TFHIC uses:
- **Thermal yields** at chemical freeze-out.
- **Blast-wave spectra** at kinetic freeze-out.
- **Data-driven parameterizations** of blast-wave parameters vs centrality or $dN_{\mathrm{ch}}/d\eta$.

The code is not a full event generator or detector simulation. It produces model spectra for comparison and feasibility studies.

---

## 2. Workflow summary (what the program does)

1) **Thermal yields**: compute per-species $dN/dy$ at chemical freeze-out using a hadron resonance gas (HRG) model. Options include Canonical/Strangeness-canonical/Grand-canonical ensembles (CE/SCE/GCE) and a $\gamma_S$ variant.
2) **Blast-wave fits**: read a CSV of published blast-wave parameters vs centrality (or $dN_{\mathrm{ch}}/d\eta$), then fit $\beta_T$, $T_{\mathrm{kin}}$, and $n$ as functions of multiplicity.
3) **Extrapolation**: evaluate those fits at target multiplicities (for pO, OO, NeNe, etc.).
4) **Spectra**: generate $p_T$ spectra using the blast-wave kernel and normalize each species to the thermal yields.

Outputs are written to ROOT files (graphs and spectra) plus optional PDF quick looks.

---

## 3. Thermal model (statistical hadronization)

### 3.1 Ensembles

Conserved charges (B, S, Q) can be treated in different ensembles:
- **Grand-canonical ensemble (GCE)**: charges conserved on average via chemical potentials.
- **Strangeness-canonical ensemble (SCE)**: strangeness canonical (exact conservation), B and Q grand-canonical.
- **Canonical ensemble**: all charges canonical.

### 3.2 Canonical suppression and correlation volume

Exact conservation in a finite volume reduces yields of particles carrying conserved quantum numbers. The correlation volume is modeled as

$$
V_c = k \, (dV/dy), \qquad k \ge 1,
$$

where $dV/dy$ is the chemical freeze-out volume per unit rapidity. Larger $k$ weakens canonical suppression (GCE recovered as $k \to \infty$).

### 3.3 Strangeness saturation gammaS

$\gamma_S$ accounts for strangeness undersaturation:

$$
N_i \propto \gamma_S^{s_i}
$$

with $s_i$ the number of valence strange + anti-strange quarks. In the $\gamma_S$ workflow, $T_{\mathrm{ch}}$ and $dV/dy$ are parameterized as functions of $dN_{\mathrm{ch}}/d\eta$.

### 3.4 Primary vs total yields

Thermal-FIST provides:
- **Primary yields** (before resonance decays).
- **Total yields** (after decays, per chosen feeddown).

TFHIC can output both in the JSON tables.

---

## 4. Freeze-out scenarios used in TFHIC

### Two-step freeze-out (gammaS workflow)

This is the default when using `--mode gammaS`:
- $T_{\mathrm{ch}}$ and $dV/dy$ are functions of $dN_{\mathrm{ch}}/d\eta$.
- Yields are tabulated vs $dN_{\mathrm{ch}}/d\eta$ for each $k$.
- Blast-wave parameters are fit vs $dN_{\mathrm{ch}}/d\eta$ and evaluated at target multiplicities.

### Single freeze-out (vanilla workflow)

For single freeze-out, $T_{\mathrm{ch}} = T_{\mathrm{kin}}$ is fixed (e.g., 150 MeV) and $\gamma_S = 1$. In `--mode vanilla`:
- Yields are tabulated vs $dV/dy$ (not vs $dN_{\mathrm{ch}}/d\eta$).
- Blast-wave parameters are still fit vs $dN_{\mathrm{ch}}/d\eta$.
- A data-driven mapping $N_{\mathrm{ch}} \to dV/dy$ is used to normalize the spectra consistently.

This mapping is built by pairing:
- $dV/dy$ values from the blast-wave table (e.g., Table III), and
- $N_{\mathrm{ch}}$ values from an external centrality table for the same bins.

Piecewise linear interpolation provides $dV/dy(N_{\mathrm{ch}})$ for target systems.

---

## 5. Blast-wave spectra

At kinetic freeze-out, the blast-wave model combines local thermal motion with a collective transverse flow profile. A standard kernel is:

$$
\frac{dN}{p_T dp_T dy} \propto \int_0^R r \, dr \, m_T \,
I_0\left(\frac{p_T \sinh \rho}{T_{\mathrm{kin}}}\right)
K_1\left(\frac{m_T \cosh \rho}{T_{\mathrm{kin}}}\right)
$$

with $m_T = \sqrt{p_T^2 + m^2}$ and $\rho(r) = \tanh^{-1}\beta_T(r)$. The flow profile is:

$$
\beta_T(r) = \beta_s \left(\frac{r}{R}\right)^n, \qquad
\langle \beta_T \rangle = \frac{2}{2+n} \, \beta_s.
$$

Parameters:
- $T_{\mathrm{kin}}$ (kinetic freeze-out temperature)
- $\langle \beta_T \rangle$ (mean transverse flow)
- $n$ (profile exponent)

TFHIC fits these parameters vs multiplicity and applies them to target systems.

---

## 6. Centrality, multiplicity, and mapping

TFHIC assumes a monotonic relation between:
- Centrality class <-> $dN_{\mathrm{ch}}/d\eta$ <-> $dV/dy$.

Key points:
- For $\gamma_S$ mode, yields are tabulated vs $dN_{\mathrm{ch}}/d\eta$ directly.
- For vanilla mode, yields are tabulated vs $dV/dy$ and mapped from $N_{\mathrm{ch}}$ using a centrality-matched table.

This keeps the thermal normalization consistent with the freeze-out assumptions used by the blast-wave fits.

---

## 7. Outputs and interpretation

The prediction step produces:
- A ROOT file with fit graphs, fit functions, and spectra.
- A quick-look PDF (optional) with fitted curves and spectra plots.

These outputs are intended for comparison studies and feasibility assessments. They are not a replacement for full event simulations.

---

## 8. Command sequences (physics rationale)

This section mirrors `docs/template_commands.txt` and explains the physics intent behind each command block. In each case, the output filename of `predict_light_spectra` encodes the source paper or table used for the blast-wave inputs, and acts as the provenance tag for the assumptions.

### 8.1 CE + gammaS, Pb-Pb 5.02 TeV blast-wave (1910.07678)

Physics rationale: compute CE yields in $\gamma_S$ mode on a $dN_{\mathrm{ch}}/d\eta$ grid for the target systems, so that $T_{\mathrm{ch}}$ and $dV/dy$ follow the multiplicity-dependent parameterization (Vovchenko 1906.03145). Then extrapolate Pb-Pb blast-wave parameters from 1910.07678 vs $dN_{\mathrm{ch}}/d\eta$ and normalize the spectra with those yields. The output filename records the source dataset.

```bash
./export_dndy_json --out yields-pO-NeNe-OO-gs.json --nch-file Nch_pO_NeNe_OO.txt --mode gs --species 211,321,2212,3122,3312,3334,1000010020 --k 1.0,1.6,3.0,6.0

./predict_light_spectra --thermal-json ../../thermal_yields/out/yields-pO-NeNe-OO-gs.json --systems "pO:33.3899,25.5256,20.5644,16.2348,13.0413,10.3316,8.0894,4.7780;OO:129.6660,106.8340,87.2877,67.1562,51.1201,37.8919,26.9060,11.6993;NeNe:158.4020,131.3850,107.4230,82.2728,62.1508,45.8975,32.4722,13.5333" --mode gammaS --species 211,321,2212,3122,3312,3334,1000010020 --timesPt --pt 0,10,400 --out prediction_PbPb5.02_1910.07678_CEgs.root --pdf prediction_PbPb5.02_1910.07678_CEgs.pdf
```

### 8.2 CE + gammaS with fixed Tch, Pb-Pb 2.76 TeV Table III (1907.11059)

Physics rationale: enforce a single-freeze-out temperature by fixing $T_{\mathrm{ch}} = 0.150$ GeV and $\gamma_S = 1$, but still use the $\gamma_S$ workflow to keep the yields tabulated vs $dN_{\mathrm{ch}}/d\eta$ for the target systems. Blast-wave parameters come from Table III (single freeze-out) and are fit vs $dN_{\mathrm{ch}}/d\eta$, with $T_{\mathrm{kin}}$ fixed to 0.150 GeV to match the table assumptions.

```bash
./export_dndy_json --out yields-pO-NeNe-OO-gs-fixedTchem.json --nch-file Nch_pO_NeNe_OO.txt --mode gs --species 211,321,2212,3122,3312,3334,1000010020 --k 1.0,1.6,3.0,6.0 --tch-a 0.150 --tch-b 0 --gs-a 1 --gs-b 0

./predict_light_spectra --thermal-json ../../thermal_yields/out/yields-pO-NeNe-OO-gs-fixedTchem.json --systems "pO:33.3899,25.5256,20.5644,16.2348,13.0413,10.3316,8.0894,4.7780;OO:129.6660,106.8340,87.2877,67.1562,51.1201,37.8919,26.9060,11.6993;NeNe:158.4020,131.3850,107.4230,82.2728,62.1508,45.8975,32.4722,13.5333" --mode gammaS --species 211,321,2212,3122,3312,3334,1000010020 --timesPt --pt 0,10,400 --out prediction_PbPb2.76_1910.11059_CEgs_TchemFIXED.root --pdf prediction_PbPb2.76_1910.11059_CEgs_TchemFIXED.pdf --bw-csv bw_data_1907.11059.csv --fit-T "[0]" --fit-T-pars 0.150 --fit-n-pars 0.23,3.55,70
```

### 8.3 CE + vanilla, fixed Tch, dV/dy scan from Table III

Physics rationale: implement single freeze-out consistently by fixing $T_{\mathrm{ch}} = 0.150$ GeV and $\gamma_S = 1$, but now in vanilla mode where yields are tabulated vs $dV/dy$. The $dV/dy$ grid is taken directly from Table III, and the $N_{\mathrm{ch}} \to dV/dy$ mapping used in prediction ensures the normalization matches the same table assumptions.

```bash
./export_dndy_json --out yields-pO-NeNe-OO-vanilla_dVscan.json --mode vanilla --species 211,321,2212,3122,3312,3334,1000010020 --k 1.0,1.6,3.0,6.0 --Tch 0.150 --v-file dVdy_TableIII_1907.11059.txt

./predict_light_spectra --thermal-json ../../thermal_yields/out/yields-pO-NeNe-OO-vanilla_dVscan.json --systems "pO:33.3899,25.5256,20.5644,16.2348,13.0413,10.3316,8.0894,4.7780;OO:129.6660,106.8340,87.2877,67.1562,51.1201,37.8919,26.9060,11.6993;NeNe:158.4020,131.3850,107.4230,82.2728,62.1508,45.8975,32.4722,13.5333" --mode vanilla --species 211,321,2212,3122,3312,3334,1000010020 --timesPt --pt 0,10,400 --out prediction_PbPb2.76_1910.11059_CEvanilla_TchemFIXED.root --pdf prediction_PbPb2.76_1910.11059_CEvanilla_TchemFIXED.pdf --bw-csv bw_data_1907.11059.csv --fit-T "[0]" --fit-T-pars 0.150 --fit-n-pars 0.23,3.55,70
```

### 8.4 Docker equivalents (same physics, containerized)

Physics rationale: the same three workflows above, but executed inside the container. The output filenames still encode the dataset provenance; all outputs are written to a single host-mounted directory for reproducibility.

```bash
sudo docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest   /opt/tfhic/install/bin/export_dndy_json --out yields-pO-NeNe-OO-gs.json --nch-file Nch_pO_NeNe_OO.txt --mode gs --species 211,321,2212,3122,3312,3334,1000010020 --k 1.0,1.6,3.0,6.0

sudo docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest   /opt/tfhic/install/bin/predict_light_spectra --thermal-json /data/out/yields-pO-NeNe-OO-gs.json --systems "pO:33.3899,25.5256,20.5644,16.2348,13.0413,10.3316,8.0894,4.7780;OO:129.6660,106.8340,87.2877,67.1562,51.1201,37.8919,26.9060,11.6993;NeNe:158.4020,131.3850,107.4230,82.2728,62.1508,45.8975,32.4722,13.5333" --mode gammaS --species 211,321,2212,3122,3312,3334,1000010020 --timesPt --pt 0,10,400 --out /data/out/prediction_PbPb5.02_1910.07678_CEgs.root --pdf /data/out/prediction_PbPb5.02_1910.07678_CEgs.pdf

sudo docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest   /opt/tfhic/install/bin/export_dndy_json --out yields-pO-NeNe-OO-gs-fixedTchem.json --nch-file Nch_pO_NeNe_OO.txt --mode gs --species 211,321,2212,3122,3312,3334,1000010020 --k 1.0,1.6,3.0,6.0 --tch-a 0.150 --tch-b 0 --gs-a 1 --gs-b 0

sudo docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest   /opt/tfhic/install/bin/predict_light_spectra --thermal-json /data/out/yields-pO-NeNe-OO-gs-fixedTchem.json --systems "pO:33.3899,25.5256,20.5644,16.2348,13.0413,10.3316,8.0894,4.7780;OO:129.6660,106.8340,87.2877,67.1562,51.1201,37.8919,26.9060,11.6993;NeNe:158.4020,131.3850,107.4230,82.2728,62.1508,45.8975,32.4722,13.5333" --mode gammaS --species 211,321,2212,3122,3312,3334,1000010020 --timesPt --pt 0,10,400 --out /data/out/prediction_PbPb2.76_1910.11059_CEgs_TchemFIXED.root --pdf /data/out/prediction_PbPb2.76_1910.11059_CEgs_TchemFIXED.pdf --bw-csv bw_data_1907.11059.csv --fit-T "[0]" --fit-T-pars 0.150 --fit-n-pars 0.23,3.55,70

sudo docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest   /opt/tfhic/install/bin/export_dndy_json --out yields-pO-NeNe-OO-vanilla_dVscan.json --mode vanilla --species 211,321,2212,3122,3312,3334,1000010020 --k 1.0,1.6,3.0,6.0 --Tch 0.150 --v-file dVdy_TableIII_1907.11059.txt

sudo docker run --rm -it -v "$PWD/out:/data/out" tfhic:latest   /opt/tfhic/install/bin/predict_light_spectra --thermal-json /data/out/yields-pO-NeNe-OO-vanilla_dVscan.json --systems "pO:33.3899,25.5256,20.5644,16.2348,13.0413,10.3316,8.0894,4.7780;OO:129.6660,106.8340,87.2877,67.1562,51.1201,37.8919,26.9060,11.6993;NeNe:158.4020,131.3850,107.4230,82.2728,62.1508,45.8975,32.4722,13.5333" --mode vanilla --species 211,321,2212,3122,3312,3334,1000010020 --timesPt --pt 0,10,400 --out /data/out/prediction_PbPb2.76_1910.11059_CEvanilla_TchemFIXED.root --pdf /data/out/prediction_PbPb2.76_1910.11059_CEvanilla_TchemFIXED.pdf --bw-csv bw_data_1907.11059.csv --fit-T "[0]" --fit-T-pars 0.150 --fit-n-pars 0.23,3.55,70
```

---

## 9. References (context)

[1] V. Vovchenko and H. Stoecker, Thermal-FIST, Comput. Phys. Commun. 244, 295 (2019).  
[2] V. Vovchenko, B. Dönigus, and H. Stoecker, Canonical statistical model analysis of p-p, p-Pb, and Pb-Pb collisions at the LHC, Phys. Rev. C 100, 054906 (2019).  
[3] ALICE Collaboration, Pb-Pb spectra at sqrt(sNN)=2.76 TeV, Phys. Rev. C 88, 044910 (2013).  
[4] ALICE Collaboration, Identified hadron spectra in Pb-Pb at sqrt(sNN)=5.02 TeV, Phys. Rev. C 101, 044907 (2020).  
[5] Blast-wave fits and tables referenced in the repository CSV inputs (see `blastwave/data/`), including arXiv:1907.11059 and arXiv:1910.07678.
