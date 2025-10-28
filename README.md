# EduScope-10 — Low-Cost 2-in-1 STM32 Oscilloscope + Function Generator

> **Mission:** a professionally engineered, ultra-affordable instrument so every engineering student can learn by *measuring*, not guessing.

---

## 📜 Story & Purpose

This project started when I was an Electronics Engineering student in **Sudan**, where many labs lacked basic measurement instruments. The idea behind **EduScope-10** is simple: build a **low-cost (≈ 10 USD core board)**, **2-in-1 STM32-based** **oscilloscope + function generator** that students can actually own. The same core can scale for **DIY** and **professional** work with add-on modules (enclosure, front-end booster, better UI), but the baseline stays *radically affordable* to improve education quality everywhere.

> **Cost note:** the 10 USD target refers to the *core PCB BOM* at volume (excludes probes, BNCs, USB cable, and enclosure).

---

## 🛡️ Standards (what we use and why)

### Anchor Standard (Rev-A must-have)
- **IEC 61010-1** — *Safety requirements for electrical equipment for measurement, control, and laboratory use.*

**Why this first:** It governs user protection and the most critical hardware decisions (input protection, creepage/clearance, insulation, fusing, protective earth/SELV, single-fault safety). Building to 61010-1 from day one keeps the design professionally grounded, even at low cost.

**What it drives in EduScope-10**
- Declare **Measurement Category** (CAT I), **Vmax**, and SELV boundaries.
- Size **creepage/clearance** and **insulation** around the input/AFE and connectors.
- Design **input protection** (series R, RC damping, fast clamps, TVS/MOV as needed) and **fusing/current-limit** so the device remains safe under single-fault.
- Verify **dielectric strength, leakage current, PE continuity** (or document SELV strategy), **abnormal-operation** tests, and proper **labels/warnings**.

### Phase-II (nice-to-have as the project matures)
- **EMC:** IEC **61326-1** (emissions & immunity for test gear) — plan enclosure, filtering, ESD points.
- **Environment:** **RoHS** / **WEEE** — maintain supplier declarations and recyclability docs.

> We anchor on **one standard (IEC 61010-1)** to keep Rev-A focused; EMC/RoHS can be added in later design spins without re-architecting.

---

## 🔧 Development Workflow — Lean **V-Modell**

We apply a pragmatic V-Modell:
- **Left side:** Requirements → Architecture → Design → Implementation  
- **Right side:** Unit → Integration → System → Acceptance (mapped back to the left)

In this README we **start with Requirements** (the base of the V). Architecture/Design/Test Plans will follow the same IDs so everything is traceable.

---

## ✅ System Requirements (Rev-A, traceable)

> All requirements are **measurable** and carry an **acceptance criterion** to support V-Modell verification. IDs are stable and used in docs, code, and tests.

### A. Product & Cost
- **SYS-001 — Form factor & scope**  
  The product is a **2-in-1 core board**: one **oscilloscope input** and one **function-generator output**.  
  *Acceptance:* BOM and schematic show one input channel and one output channel.

- **SYS-002 — Cost target**  
  **Core PCB BOM ≤ 10 USD** in ≥ 1k volume (excludes probes, cables, enclosure, shipping).  
  *Acceptance:* BOM spreadsheet shows extended price ≤ 10 USD at quoted MOQ.

### B. Safety (IEC 61010-1 Anchor)
- **SFT-001 — Measurement category & limits**  
  **CAT I**, **SELV only**; **Vmax (native input) ≤ 3.6 V**; with a **10:1 passive probe**, safe operation up to **±33 V**.  
  *Acceptance:* Risk analysis & labels reflect CAT I/SELV; test with 33 V source using 10:1 probe shows no damage or hazard.

- **SFT-002 — Creepage/clearance & insulation**  
  PCB and connector geometry meet **IEC 61010-1** tables for declared **Vmax** and **PD2** environment.  
  *Acceptance:* PCB review checklist with measured distances ≥ required values.

- **SFT-003 — Single-fault safety**  
  Under single-fault (e.g., clamp short, fuse open), the device remains safe: no fire, no accessible hazardous voltage/current, temperature within limits.  
  *Acceptance:* Documented abnormal-operation tests pass (no hazardous outcomes).

- **SFT-004 — Documentation & labeling**  
  Safety warnings, CAT rating, Vmax, and SELV notes present in silkscreen/manual.  
  *Acceptance:* User manual & artwork include required markings.

### C. Oscilloscope (Acquisition)
- **OSC-001 — Sample rate**  
  **≥ 1 MS/s single-channel** sustained capture to internal RAM for **≥ 10 k samples**.  
  *Acceptance:* Bench test records 10 k samples at ≥ 1 MS/s without overrun.

- **OSC-002 — Analog bandwidth**  
  **≥ 100 kHz (-3 dB)** with standard 10:1 probe and native front-end.  
  *Acceptance:* Sine sweep shows ≥ 100 kHz at −3 dB point or better.

- **OSC-003 — Resolution / ENOB**  
  12-bit ADC; **ENOB ≥ 8.5 bits** at **1 kHz**, 1 Vpp input.  
  *Acceptance:* FFT/linearity test per IEEE-style method shows ENOB ≥ 8.5.

- **OSC-004 — Input impedance**  
  **1 MΩ ±5% // ≤ 25 pF** at the input connector.  
  *Acceptance:* Meter + LCR measurement confirms spec within tolerance.

- **OSC-005 — Trigger**  
  Edge trigger (rising/falling), **adjustable level**; pre/post trigger capture.  
  *Acceptance:* Functional test captures a stable square wave at set levels.

### D. Function Generator (AWG)
- **AWG-001 — Waveforms**  
  **Sine, Square, Triangle, DC**, plus Arbitrary (binary upload).  
  *Acceptance:* Commands/selectors generate each waveform type.

- **AWG-002 — Frequency range**  
  Sine: **0.1 Hz–20 kHz**; Square/Triangle: **0.1 Hz–10 kHz** (duty 50% min).  
  *Acceptance:* Frequency counter verifies ranges; jitter ≤ 1% of period.

- **AWG-003 — Amplitude & load**  
  **0–3.0 Vpp** into **≥ 1 kΩ** load (single-ended, 0–3.3 V domain).  
  *Acceptance:* DMM/scope confirm Vpp within ±5% at 1 kHz, 1 kΩ.

- **AWG-004 — Spectral purity (base)**  
  **THD ≤ 3%** at **1 kHz**, **1 Vpp** sine into 1 kΩ.  
  *Acceptance:* FFT measurement shows THD within limit.

### E. Interfaces & Control
- **IF-001 — USB device (control & data)**  
  **USB CDC** control CLI + binary data for capture/waveform upload.  
  *Acceptance:* PC script can start capture and fetch waveform over USB.

- **IF-002 — Optional host UI**  
  **Raspberry Pi** option: simple web UI + SCPI-like TCP control (port 5025).  
  *Acceptance:* Demo image runs; `*IDN?` returns device string; basic controls operate.

### F. Power & Mechanics
- **PWR-001 — Power source**  
  **5 V USB**, **≤ 500 mA** during continuous operation at rated loads.  
  *Acceptance:* Inline USB meter shows ≤ 500 mA average.

- **MEC-001 — Board & assembly**  
  **2-layer PCB**, **0603 passives**, single-sided SMD assembly preferred.  
  *Acceptance:* Gerbers and pick-and-place reflect constraints.

### G. Calibration & Data Integrity
- **CAL-001 — DC calibration**  
  One-point **offset** and **gain** calibration stored in NVRAM with **CRC**.  
  *Acceptance:* After calibration, a 1.000 V DC input reads **1.000 V ±1%**.

- **CAL-002 — AWG amplitude calibration**  
  1 kHz sine *gain trim* stored with CRC; applied on boot.  
  *Acceptance:* 1.000 Vpp target outputs **1.000 Vpp ±3%** after trim.

### H. Reliability & Environment
- **REL-001 — Uptime stress**  
  8-hour continuous run (capture + AWG) without crash or over-temperature.  
  *Acceptance:* Log shows no resets; surface temp < **60 °C** at any touch point.

- **ENV-001 — Operating conditions**  
  **0–50 °C**, **≤ 85% RH** non-condensing.  
  *Acceptance:* Basic thermal chamber test passes functional checks.

---

## 🧭 What’s Next in the V-Modell (coming with matching IDs)

- **System Architecture (ARC-###):** STM32 partition (ADC/DMA/timing), AFE, AWG path (PWM+filter or DAC/R-2R), USB stack, optional Pi service, clock tree & jitter budget, memory map, safety blocks (fuse/clamps).  
- **HW Design (HW-###):** Schematics/PCB with rule checks for 61010-1 distances, protection BOM, test points, shielding/filters.  
- **SW Design (SW-###):** Driver/state machine diagrams, timing budgets, command set, storage layout for calibration, boot/update.  
- **Verification Plans (TST-###):** Unit (firmware modules), Integration (data path), System (performance), Safety (61010-1 test list), Acceptance (map to SYS-/SFT-/AWG-/OSC- IDs).

---

## 🔌 Quick Reality Notes

- Keeping the **core at ≈ 10 USD** means the base board targets **CAT I / SELV** and modest performance (100 kHz class). Higher voltage ranges, BNCs, metal enclosure, and full EMC compliance are **Phase-II** add-ons.  
- The design **starts safe (61010-1)** so upgrading later doesn’t force a redesign of fundamentals.

---

## 📄 Repo Skeleton (suggested)
