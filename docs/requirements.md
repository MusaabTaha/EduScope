# EduScope-10 Rev-A System Requirements

## Requirement status convention

- **Prototype:** demonstrated on STM32F429I-DISC1
- **Planned:** required for Rev-A but not yet implemented
- **Verification pending:** implemented partly or conceptually, but the acceptance test has not been completed

## A. Product and cost

| ID | Requirement | Acceptance criterion | Status |
|---|---|---|---|
| SYS-001 | One oscilloscope input and one function-generator output on one core board | BOM and schematic show one input and one output channel | Prototype concept demonstrated, custom PCB planned |
| SYS-002 | Core PCB BOM at or below USD 10 at 1k volume, excluding probes, cables, enclosure, shipping, and optional modules | Quoted BOM extended price at or below USD 10 | Planned |

## B. Safety

| ID | Requirement | Acceptance criterion | Status |
|---|---|---|---|
| SFT-001 | CAT I, SELV only. Native input at or below 3.6 V. Planned use with a suitable 10:1 passive probe up to the declared protected range | Risk analysis and labels match the declared limits; overvoltage test causes no damage or hazard | Planned |
| SFT-002 | PCB and connector creepage, clearance, and insulation meet the applicable IEC 61010-1 requirements for the declared environment | Design review records measured distances at or above the required values | Planned |
| SFT-003 | A single fault does not create fire, accessible hazardous voltage/current, or excessive temperature | Documented abnormal-operation tests pass | Planned |
| SFT-004 | Safety warnings, CAT rating, Vmax, and SELV limitations appear in product artwork and user documentation | Manual and silkscreen review passes | Planned |

## C. Oscilloscope

| ID | Requirement | Acceptance criterion | Status |
|---|---|---|---|
| OSC-001 | At least 1 MS/s sustained single-channel capture for at least 10k samples in internal RAM | Capture test records 10k samples without overrun | Planned; timer-driven ADC proof of concept complete |
| OSC-002 | At least 100 kHz analog bandwidth at the declared probe/input configuration | Calibrated sine sweep reaches at least the 100 kHz -3 dB point | Planned |
| OSC-003 | 12-bit ADC and ENOB at least 8.5 bits at 1 kHz, 1 Vpp | FFT and linearity measurement demonstrates the requirement | Planned |
| OSC-004 | 1 Mohm +/-5% input resistance and no more than 25 pF input capacitance | DMM and LCR measurements pass | Planned |
| OSC-005 | Rising/falling edge trigger, adjustable level, pre-trigger and post-trigger capture | Stable square-wave capture at selected trigger levels | Planned |

## D. Function generator

| ID | Requirement | Acceptance criterion | Status |
|---|---|---|---|
| AWG-001 | Sine, square, triangle, DC, and arbitrary binary-upload waveform modes | Each waveform command or selector produces the requested output | Sine, square, triangle, and sawtooth prototyped; DC/arbitrary planned |
| AWG-002 | Sine 0.1 Hz to 20 kHz; square/triangle 0.1 Hz to 10 kHz | Frequency-counter test passes with period jitter at or below 1% | Prototype variable range demonstrated to approximately 2.5 kHz; Rev-A expansion planned |
| AWG-003 | 0 to 3.0 Vpp into at least 1 kohm, single-ended | DMM/scope measurement within +/-5% at 1 kHz into 1 kohm | Prototype amplitude control demonstrated; calibrated load test pending |
| AWG-004 | Sine-wave THD at or below 3% at 1 kHz, 1 Vpp into 1 kohm | FFT measurement passes | Planned |

## E. Interfaces and control

| ID | Requirement | Acceptance criterion | Status |
|---|---|---|---|
| IF-001 | USB CDC command interface and binary data transfer for captures and waveform upload | PC script starts capture and retrieves data over USB | Planned |
| IF-002 | Optional Raspberry Pi web UI and SCPI-like TCP control on port 5025 | Demo image operates and `*IDN?` returns a device string | Phase II |

## F. Power and mechanics

| ID | Requirement | Acceptance criterion | Status |
|---|---|---|---|
| PWR-001 | 5 V USB power and no more than 500 mA in continuous rated operation | Inline USB meter verifies average current | Development board uses USB; Rev-A measurement pending |
| MEC-001 | Two-layer PCB, 0603 passives, and single-sided SMD assembly where practical | Gerbers, BOM, and pick-and-place files reflect the constraint | Planned |

## G. Calibration and data integrity

| ID | Requirement | Acceptance criterion | Status |
|---|---|---|---|
| CAL-001 | DC offset and gain calibration stored in NVRAM with CRC | Calibrated 1.000 V input reads 1.000 V +/-1% | Planned |
| CAL-002 | AWG 1 kHz amplitude trim stored with CRC and applied at boot | 1.000 Vpp target produces 1.000 Vpp +/-3% | Planned |

## H. Reliability and environment

| ID | Requirement | Acceptance criterion | Status |
|---|---|---|---|
| REL-001 | Eight-hour simultaneous acquisition and AWG operation without crash or over-temperature | Log shows no resets and touch temperatures remain below 60 C | Planned |
| ENV-001 | Functional operation from 0 to 50 C and up to 85% RH non-condensing | Environmental test passes functional checks | Planned |
