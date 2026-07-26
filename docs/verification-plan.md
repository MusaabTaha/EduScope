# EduScope-10 Verification Plan

## Verification strategy

The verification plan follows the same traceable chain as the development workflow:

```text
Requirements
-> technical specification
-> hardware and firmware design
-> implementation
-> unit testing
-> integration testing
-> system testing
-> acceptance evidence
```

## Test levels

### Unit testing

Test hardware-independent logic on the host where practical:

- waveform LUT generation
- waveform selection wraparound
- amplitude clamping and DAC-code conversion
- frequency-to-timer-prescaler calculation
- ADC raw-value scaling
- control-value mapping

### Integration testing

Validate connected firmware modules on STM32 hardware:

- TIM1 interrupt to LUT index to DAC output
- ADC1 control reading to function-generator update
- TIM8 interrupt to ADC2 conversion to ADC interrupt
- button input to output enable and waveform state
- concurrent function-generator and oscilloscope operation

### System testing

Validate complete instrument behavior:

- waveform type, frequency, amplitude, and load response
- acquisition sample rate and capture integrity
- bandwidth and trigger stability
- USB control and data transfer
- calibration storage and recovery

### Stress, performance, power, and safety testing

- eight-hour continuous dual-function run
- maximum supported sample/output rates
- current consumption in all operating modes
- thermal inspection
- abnormal-operation and single-fault tests
- input overvoltage and protection tests on Rev-A hardware

## Current prototype evidence

| Test | Result |
|---|---|
| DAC output changes from LUT samples | Passed |
| Sine waveform generation | Passed |
| Square waveform generation | Passed |
| Triangle waveform generation | Passed |
| Sawtooth waveform generation | Passed |
| Potentiometer amplitude variation | Passed |
| Potentiometer frequency variation | Passed |
| Button waveform selection | Passed |
| ADC2 timer-driven conversions | Passed |
| Raw ADC signal visible in STM32 Viewer | Passed |
| Scaled voltage variable visible in STM32 Viewer | Passed |

## Evidence still required

- calibrated frequency-counter measurements
- oscilloscope bandwidth sweep
- ADC ENOB test
- AWG THD test
- load regulation test
- input impedance and capacitance measurement
- DMA capture overrun test
- trigger acceptance tests
- power and thermal measurements
- safety and abnormal-operation reports
