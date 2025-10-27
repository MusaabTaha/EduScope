# Open Instrument: STM32 + Raspberry Pi (Oscilloscope • AWG • Logic Analyzer)

[![Build CI](https://img.shields.io/github/actions/workflow/status/your-org/your-repo/ci.yml?label=build)](./.github/workflows/ci.yml)
[![Code Style](https://img.shields.io/badge/style-MISRA--C%20%26%20clang--tidy-blue)](#coding-style--quality)
[![License](https://img.shields.io/badge/license-Apache--2.0-green)](LICENSE)

A professional, open hardware + software measurement instrument:
- **Oscilloscope** (digitizer)
- **Arbitrary Waveform Generator (AWG)**
- **Logic Analyzer**
- **SCPI** remote control over TCP/USB

> **Single anchor standard:** **IEC 61010-1** (Safety for measurement equipment)  
> **Single development workflow:** **Lean V-Modell** (requirements ↔ tests, traceable)

---

## ✨ Features

- Up to N channels (configurable), target BW/SR/ENOB defined in `docs/requirements.md`
- AWG with basic waveforms (Sine, Square, Triangle, DC, Arbitrary via upload)
- Logic analyzer with threshold presets (1.2 V, 1.8 V, 3.3 V, 5 V)
- SCPI 1999-style command server on Raspberry Pi (TCP 5025) and optional USBTMC
- Web/desktop UI on Raspberry Pi (headless mode supported)
- On-device calibration storage with CRC and versioning
- Update/rollback for the Pi application (A/B slots)

---

## 🧱 Architecture

**Partitioning**

- **STM32 (Firmware, C)**  
  Real-time acquisition/control (ADC/DMA/Timers), AWG DDS, logic capture, safety interlocks.
- **Raspberry Pi (App/Services)**  
  UI, storage, SCPI server, networking, update agent, calibration tools.
- **Analog Front-End (AFE)**  
  Attenuation, protection, filtering, level shifting, clocking.

