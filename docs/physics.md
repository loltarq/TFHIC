# TFHIC Physics Notes

This document summarizes the physics foundations of TFHIC and how the modules connect. For practical build/run details, see the project **README.md**.

## Overview of the framework

TFHIC comprises three physics modules and a thin orchestrator:

1. **Thermal yields** — hadron resonance gas (HRG) at chemical freeze-out computes primary midrapidity yields $N_i \equiv \mathrm{d}N_i/\mathrm{d}y$, with canonical options and resonance feed-down.
2. **Blast-wave spectra** — the Boltzmann–Gibbs blast-wave (BGBW) source converts $N_i$ to normalized $p_T$-differential spectra.
3. **Efficiency extrapolation** — uses predicted spectra of π, K, p and measured/simulated reference efficiencies to infer $\varepsilon_i(p_T)$ for other hadrons.

A pipeline schematic is: **thermal yields → blast-wave spectra → efficiency curves**.

## Thermal yields (HRG)

**Model.** Primary densities follow the HRG partition function at chemical freeze-out $T_{ch}$ with chemical potentials $\boldsymbol{\mu}=(\mu_B,\mu_S,\mu_Q)$:

```math
n_i^{\mathrm{prim}}(T_{ch},\boldsymbol{\mu})=\frac{g_i}{2\pi^2}\int_0^\infty \frac{p^2 \mathrm{d}p}{\exp(\frac{E_i(p)-\mu_i}{T_{ch}})\pm 1},\quad
E_i=\sqrt{p^2+m_i^2},\quad \mu_i=B_i\mu_B+S_i\mu_S+Q_i\mu_Q .
```

Final yields include resonance decays:

```math
N_i = V n_i^{\mathrm{prim}} + \sum_R \mathrm{Br}(R\to i+X) N_R^{\mathrm{prim}} .
```

**Ensembles & constraints.** TFHIC supports GCE/SCE/CE via **Thermal‑FIST** ([arXiv:1901.05249](https://arxiv.org/abs/1901.05249)). Canonical suppression is parameterized by a correlation volume $V_c=k \mathrm{d}V/\mathrm{d}y$. An alternative $\gamma_S$ mode applies a strangeness saturation factor to strange hadrons: $n_i\to \gamma_S^{|S_i|}n_i$.

**System-size trends.** Parameterizations vs multiplicity (e.g. $T_{ch}$, $\gamma_S$ as functions of $N_{ch}$) allow automatic scans over centrality/multiplicity.

**Outputs.** The thermal app exports JSON mapping **PDG → $\mathrm{d}N/\mathrm{d}y$** with configuration metadata (ensemble, widths, feed-down, k or $\gamma_S$, etc.).

## Blast–wave spectra (BGBW)

**Model.** For a hadron of mass $m_i$, the invariant spectrum at midrapidity

```math
\frac{\mathrm{d}N_i}{p_T \mathrm{d}p_T \mathrm{d}y}\propto\int_0^R r \mathrm{d}r  m_T 
I_0(\frac{p_T\sinh\rho(r)}{T_kin}) 
K_1(\frac{m_T\cosh\rho(r)}{T_kin}),
```

with $m_T=(p_T^2+m_i^2)^\frac{1}{2}$, $\rho(r)=\tanh^{-1}\beta(r)$, and $\beta(r)=\beta_s(r/R)^n$.
We use parameters $T_{kin}$, $\langle\beta_T\rangle$, and profile `n`, with $\beta_s=\tfrac{n+2}{2}\langle\beta_T\rangle$.

**Normalization.** Each spectrum is normalized to the thermal yield $N_i$ by rescaling so that $\int \mathrm{d}p_T \mathrm{d}N_i/\mathrm{d}p_T = N_i$. Bin-wise integrals are used when asymmetric binning is provided.

**Inputs.** Hadron masses/labels from `common/data/hadrons.json`. Blast-wave parameters from CSV (centrality-dependent; species overrides allowed).

**Numerics & checks.** Radial integral via ROOT; guard `clampR` ensures $\beta(r)<1$. Sanity tests verify normalization to $N_i$ and positivity.

## Efficiency extrapolation (summary)

Efficiencies are represented either as pointwise curves (logit interpolation) or binned step functions. Reference efficiencies for $(\pi, K, p)$ are ingested from ROOT objects. For a target species `i` with token `T` (meson, strange–meson, baryon), TFHIC combines reference logits with weights $(w_\pi,w_K,w_p)$:

```math
\mathrm{logit} \varepsilon_i(p_T) \approx w_\pi^{(T)} g_\pi(p_T)+w_K^{(T)} g_K(p_T)+w_p^{(T)} g_p(p_T), \qquad \varepsilon_i=(1+e^{-g})^{-1}.
```

## Analysis products

- Baseline $p_T$ spectra for $\pi, K, p$ across centrality classes.
- Ratios $K/\pi$, $p/\pi$ vs $p_T$.
- Integrated $\mathrm{d}N/\mathrm{d}y$, $\langle p_T\rangle$, and species ratios vs centrality.
- Efficiency curves for $\Lambda,\Xi,\Omega,\phi$ with uncertainty bands.
- Sensitivity scans in $T_{ch}, \gamma_S$ or k (thermal) and $T_{kin}, \langle\beta_T\rangle, n$ (blast-wave).

## Uncertainties

Sources: HRG inputs (particle list/decays/widths), canonical scheme, system-size parametrizations, blast-wave parameters, efficiency mapping weights, and numerical integration tolerances. Propagation via pseudo-experiments yields covariances for yields and spectra.

## Reproducibility

Results are tied to a physics configuration card (freeze-out parameters, ensemble, particle list version) and a software tag (commit hash, build flags). Plots should embed sidecar metadata (binning, covariances).

## References

- **Thermal‑FIST:** V. Vovchenko *et al.*, *Thermal‑FIST: A package for hadron resonance gas model calculations*, arXiv:1901.05249 — https://arxiv.org/abs/1901.05249
