# Physics background

This repository uses a **statistical hadronization (thermal) model** and a **blast-wave** description for spectra. 
This approach is based on the notion that, in heavy-ion collisions, primary hadron yields can be determined assuming production from a thermalized source.
At **chemical freeze-out**, primary particle yields are computed from a **hadron resonance gas (HRG)** in thermal and chemical equilibrium, characterized by a **temperature T** and **chemical potentials μ**. Conservation of charges can be enforced either on average (GCE) or exactly (canonical ensembles), which introduces *canonical suppression* factors. The programs in `thermal_yields/` explore how yields/ratios change with **system size** and **ensemble choice**, providing insight into freeze-out conditions across different collision systems.
At **kinetic freeze-out**, hadrons decouple from the system's collective expansion and stop interacting. The blast-wave model describes the **transverse momentum (p<sub>T</sub>)** spectra by combining:

- A thermal component characterized by a **kinetic temperature T<sub>kin</sub>**, and
- A collective radial flow component parameterized by a **transverse velocity profile β<sub>T</sub>(r)**.

This separation allows one to generate full p<sub>T</sub> spectra for each hadron species, using only the total yields (from the thermal model) and a few macroscopic parameters. The output can then be used to compare with experimental spectra or as input to simulations for computing detection efficiencies.  
  
This document summarizes the physics model behind the TFHIC toolkit.

---

## 1. Freeze‑out hierarchy

- **Hadronization / pseudo‑critical temperature $T_c$.**
  The QCD crossover from QGP to a hadron gas. At LHC midrapidity, $\mu_B \approx 0$ and lattice QCD gives $T_c \sim 155\text{–}160\,\mathrm{MeV}$.

- **Chemical freeze‑out $T_{\mathrm{ch}}$.**
  Inelastic reactions effectively cease; **hadron yields** are fixed. At the LHC, data‑driven $T_{\mathrm{ch}}$ is **very close to** $T_c$.

- **Kinetic freeze‑out $T_{\mathrm{kin}}$.**
  Elastic scatterings cease; **spectral shapes** are fixed. Typically $T_{\mathrm{kin}} \sim 90\text{–}120\,\mathrm{MeV}$ and centrality dependent.

**Ordering:**

$$
T_c \gtrsim T_{\mathrm{ch}} > T_{\mathrm{kin}} .
$$

We separate:
1) **Thermal production** at $T_{\mathrm{ch}}$ $\Rightarrow$ per‑species $dN/dy$.
2) **Blast‑wave spectra** at $T_{\mathrm{kin}}$ $\Rightarrow$ $p_T$ shapes normalized to those $dN/dy$.

---

## 2. Thermal production (statistical hadronization)

We describe chemical freeze‑out with a hadron resonance gas (HRG). Conserved charges $(B,S,Q)$ are treated **grand‑canonically** (via chemical potentials) or **canonically** (exact conservation in a finite domain).

### 2.1 Ensembles

- **GCE:** charges conserved on average via $\mu_B,\mu_S,\mu_Q$.
- **SCE:** strangeness canonical, $B,Q$ grand‑canonical.
- **CE:** full canonical conservation of $B,S,Q$.

### 2.2 Canonical suppression and correlation volume

Exact conservation in a **finite domain** reduces the yield of particles carrying the conserved quantum number(s). For strangeness we write

$$
V_c = k\frac{dV}{dy}, \qquad k \ge 1 ,
$$

with $\frac{dV}{dy}$ being the fireball volume per unit rapidity at chemical freeze‑out and $k$ a dimensionless scale. Larger $k$ $\Rightarrow$ weaker canonical suppression (GCE is recovered as $k\to\infty$).

### 2.3 Strangeness saturation $\gamma_S$

A phenomenological modifier for (under)saturation of strangeness:

$$
N_i \propto \gamma_S^{s_i} ,
$$

where $s_i$ is the number of valence strange + anti‑strange quarks in species $i$. $\gamma_S$ and canonical suppression are **different mechanisms**; assuming $\gamma_S \le 1$ introduces a dependency on the multiplicity $dN_{\mathrm{ch}}/d\eta$ of the chemical freeze-out temperature $T_{\mathrm{ch}}$ and volume.

### 2.4 Primary vs total yields

- **Primary:** direct thermal production (before resonance decays).
- **Total:** after applying strong/weak decays (branching ratios).

### 2.5 From multiplicity/centrality to volume

Centrality or event activity (often via $dN_{\mathrm{ch}}/d\eta$) is mapped to a chemical freeze‑out volume $V$ or a rapidity density of volume $dV/dy$. The precise mapping is analysis‑specific; in TFHIC we treat $V$ (or $dV/dy$) as a monotonic function of the chosen multiplicity estimator.

### 2.6 Modes

- **Canonical‑suppression (CE) mode — volume scan in $k$.** For a fixed centrality (fixed $V$ and $T_{\mathrm{ch}}$), vary $V_{c} = k\frac{dV}{dy}$ to study canonical effects based on the size of the correlation volume

- **$\gamma_S$ variant — scan in multiplicity $dN_{\mathrm{ch}}/d\eta$.** The control variable is multiplicity. For a provided grid in $\{dN_{\mathrm{ch}}/d\eta\}$ the thermal model parameters are evaluated as functions of multiplicity, so that

$$
  T_{\mathrm{ch}} = T_{\mathrm{ch}}(dN_{\mathrm{ch}}/d\eta), \qquad
  \frac{dV}{dy} = \frac{dV}{dy}(dN_{\mathrm{ch}}/d\eta), \qquad
  V_c = k\frac{dV}{dy}(dN_{\mathrm{ch}}/d\eta) .
$$

---

## 3. Blast‑wave spectra

At kinetic freeze‑out we model the $p_T$ spectra with a standard blast‑wave ansatz (Boltzmann–Gibbs form plus radial flow). A common expression for the **shape kernel** is

$$
\frac{dN}{p_Tdp_Tdy} \propto
\int_0^R rdr m_T
I_0\left(\frac{p_T\sinh \rho(r)}{T_{\mathrm{kin}}}\right)
K_1\left(\frac{m_T\cosh \rho(r)}{T_{\mathrm{kin}}}\right),
$$

with $m_T=\sqrt{p_T^2+m^2}$, $\rho(r)=\tanh^{-1}\beta_T(r)$, and a transverse‑flow profile

$$
\beta_T(r) = \beta_s\big( r/R \big)^{n} \qquad
\langle \beta_T \rangle = \frac{2}{2+n}\beta_s .
$$

Parameters: $T_{\mathrm{kin}}$, $\langle \beta_T \rangle$ (or $\beta_s$), and the profile exponent $n$. Mass ordering of spectra emerges naturally from flow.  
Spectra are normalized to yields determined via thermal model.

### 3.2 Centrality and species dependence

Blast‑wave parameters may depend on **centrality** and, optionally, on the **particle species**. In TFHIC they are provided via a CSV. A token `ALL` applies to all species in that centrality unless overridden by species groups.

---

## 4. CLI flag mapping

### 4.1 Thermal yields exporter

| Symbol | Meaning | CLI flag vanilla | CLI flag $\gamma_S$ |
|---|---|---|---|
| $T_{\mathrm{ch}}$ | chemical freeze-out temperature | --Tch | --tch-a, --tch-b |
| Ensemble | statistical ensemble for charge conservation | --ensemble | --ensemble |
| Resonance decay width | handle for resonance decay width (eBW, ZeroWidth, BWTwoGamma) | --width | --width |
| $dV/dy$ | rapidity density of volume | multiplicity dependent in $\gamma_S$ mode |
| $V_c = kdV/dy$ | correlation volume | $k$ still applies |
| $\gamma_S$ | strangeness saturation factor | $0.6\text{–}1.0$ phenomenologically |
| $dN/dy$ | rapidity density at midrapidity | input to normalization |
| $\langle\beta_T\rangle, n$ | mean flow and profile exponent | centrality dependent |

**Rapidity vs pseudorapidity.** The thermal model outputs $dN/dy$ (midrapidity). Experimental centrality is often specified by $dN_{\mathrm{ch}}/d\eta$; TFHIC assumes a monotonic mapping from $dN_{\mathrm{ch}}/d\eta$ to $V$ or $dV/dy$.


---


## 5. References and further readings
[1] V. Vovchenko and H. Stoecker, *Thermal‑FIST: A package for heavy-ion collisions and hadronic equation of state*, *Comput. Phys. Commun.* **244**, 295–310 (2019). [arXiv:1901.05249](https://arxiv.org/abs/1901.05249), [doi:10.1016/j.cpc.2019.06.024](https://doi.org/10.1016/j.cpc.2019.06.024)  
[2] V. Vovchenko, B. Dönigus, and H. Stoecker, *Canonical statistical model analysis of p‑p, p‑Pb, and Pb‑Pb collisions at the LHC*, *Phys. Rev. C* **100**, 054906 (2019). [arXiv:1906.03145](https://arxiv.org/abs/1906.03145), [doi:10.1103/PhysRevC.100.054906](https://doi.org/10.1103/PhysRevC.100.054906)  
[3] ALICE Collaboration, *Centrality dependence of π, K, p production in Pb–Pb collisions at √sₙₙ = 2.76 TeV*, *Phys. Rev. C* **88**, 044910 (2013). [arXiv:1303.0737](https://arxiv.org/abs/1303.0737), [doi:10.1103/PhysRevC.88.044910](https://doi.org/10.1103/PhysRevC.88.044910)


