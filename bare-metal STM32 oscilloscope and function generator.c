/**
 ******************************************************************************
 * @file    main.c
 * @brief   EduScope-10 bare-metal oscilloscope and function generator
 * @author  Musaab Taha
 ******************************************************************************
 * Prototype platform: STM32F429I-DISC1
 *
 * Function generator:
 *   PA4  - DAC1 output
 *   PA1  - amplitude potentiometer, ADC1 channel 1
 *   PA2  - frequency potentiometer, ADC1 channel 2
 *   PB1  - output enable/disable button
 *   PB2  - waveform selection button
 *
 * Oscilloscope:
 *   PA3  - ADC2 channel 3 input
 *   TIM8 - ADC2 sampling clock
 *
 * The application uses direct register access. STM32Cube supplies only the
 * startup files and linker script through PlatformIO.
 ******************************************************************************
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/* ========================================================================== */
/* Configuration                                                              */
/* ========================================================================== */

#define SYSTEM_CLOCK_HZ                 16000000U
#define TIMER_AUTO_RELOAD                     49U
#define WAVE_LUT_SIZE                        128U
#define SCOPE_SAMPLE_RATE_HZ                5000U

#define DEFAULT_FREQUENCY_HZ              1000.0f
#define MIN_FREQUENCY_HZ                     0.1f
#define MAX_FREQUENCY_HZ                  2500.0f

#define DEFAULT_AMPLITUDE_V                   0.5f
#define MAX_AMPLITUDE_V                       1.65f

#define DAC_CENTER_CODE                      2048U
#define DAC_MAX_CODE                         4095U

/* Prototype calibration values retained from the validated firmware */
#define AMPLITUDE_POT_FULL_SCALE           3822.0f
#define FREQUENCY_POT_FULL_SCALE           3799.0f
#define SCOPE_CALIBRATED_VREF                  3.0f
#define SCOPE_CALIBRATED_COUNTS             3722.0f

#define TWO_PI                    6.28318530717958647692f

/* ========================================================================== */
/* Register maps                                                              */
/* ========================================================================== */

typedef struct
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_Registers;

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t SWTRIGR;
    volatile uint32_t DHR12R1;
    volatile uint32_t DHR12L1;
    volatile uint32_t DHR8R1;
    volatile uint32_t DHR12R2;
    volatile uint32_t DHR12L2;
    volatile uint32_t DHR8R2;
    volatile uint32_t DHR12RD;
    volatile uint32_t DHR12LD;
    volatile uint32_t DHR8RD;
    volatile uint32_t DOR1;
    volatile uint32_t DOR2;
    volatile uint32_t SR;
} DAC_Registers;

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    volatile uint32_t RESERVED0;
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    volatile uint32_t RESERVED2;
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t RESERVED3[2];
    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    volatile uint32_t AHB3LPENR;
    volatile uint32_t RESERVED4;
    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    volatile uint32_t RESERVED5[2];
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    volatile uint32_t RESERVED6[2];
    volatile uint32_t SSCGR;
    volatile uint32_t PLLI2SCFGR;
    volatile uint32_t PLLSAICFGR;
    volatile uint32_t DCKCFGR;
} RCC_Registers;

typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_Registers;

typedef struct
{
    volatile uint32_t ISER[8];
    uint32_t RESERVED0[24];
    volatile uint32_t ICER[8];
    uint32_t RESERVED1[24];
    volatile uint32_t ISPR[8];
    uint32_t RESERVED2[24];
    volatile uint32_t ICPR[8];
    uint32_t RESERVED3[24];
    volatile uint32_t IABR[8];
    uint32_t RESERVED4[56];
    volatile uint8_t IPR[240];
    uint32_t RESERVED5[644];
    volatile uint32_t STIR;
} NVIC_Registers;

typedef struct
{
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR1;
    volatile uint32_t JOFR2;
    volatile uint32_t JOFR3;
    volatile uint32_t JOFR4;
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR1;
    volatile uint32_t JDR2;
    volatile uint32_t JDR3;
    volatile uint32_t JDR4;
    volatile uint32_t DR;
} ADC_Registers;

#define RCC_BASE_ADDRESS      0x40023800U
#define GPIOA_BASE_ADDRESS    0x40020000U
#define GPIOB_BASE_ADDRESS    0x40020400U
#define DAC_BASE_ADDRESS      0x40007400U
#define TIM1_BASE_ADDRESS     0x40010000U
#define TIM8_BASE_ADDRESS     0x40010400U
#define ADC1_BASE_ADDRESS     0x40012000U
#define ADC2_BASE_ADDRESS     0x40012100U
#define NVIC_BASE_ADDRESS     0xE000E100U
#define CPACR_ADDRESS         0xE000ED88U

#define RCC     ((RCC_Registers *)RCC_BASE_ADDRESS)
#define GPIOA   ((GPIO_Registers *)GPIOA_BASE_ADDRESS)
#define GPIOB   ((GPIO_Registers *)GPIOB_BASE_ADDRESS)
#define DAC1    ((DAC_Registers *)DAC_BASE_ADDRESS)
#define TIM1    ((TIM_Registers *)TIM1_BASE_ADDRESS)
#define TIM8    ((TIM_Registers *)TIM8_BASE_ADDRESS)
#define ADC1    ((ADC_Registers *)ADC1_BASE_ADDRESS)
#define ADC2    ((ADC_Registers *)ADC2_BASE_ADDRESS)
#define NVIC    ((NVIC_Registers *)NVIC_BASE_ADDRESS)

/* ========================================================================== */
/* Register bits                                                              */
/* ========================================================================== */

#define TIM_CR1_CEN               (1U << 0)
#define TIM_CR1_URS               (1U << 2)
#define TIM_DIER_UIE              (1U << 0)
#define TIM_SR_UIF                (1U << 0)
#define TIM_EGR_UG                (1U << 0)

#define ADC_SR_EOC                (1U << 1)
#define ADC_CR1_EOCIE             (1U << 5)
#define ADC_CR1_SCAN              (1U << 8)
#define ADC_CR2_ADON              (1U << 0)
#define ADC_CR2_CONT              (1U << 1)
#define ADC_CR2_EOCS              (1U << 10)
#define ADC_CR2_SWSTART           (1U << 30)

#define ADC_IRQ_NUMBER                   18U
#define TIM1_UP_TIM10_IRQ_NUMBER         25U
#define TIM8_UP_TIM13_IRQ_NUMBER         44U

/* ========================================================================== */
/* Application state                                                          */
/* ========================================================================== */

typedef enum
{
    WAVEFORM_SINE = 0,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_COUNT
} Waveform;

static uint16_t s_wave_lut[WAVE_LUT_SIZE];
static volatile uint16_t s_lut_index = 0U;
static volatile bool s_output_enabled = true;
static bool s_awg_timer_initialized = false;

static Waveform s_waveform = WAVEFORM_SINE;
static float s_amplitude_v = DEFAULT_AMPLITUDE_V;
static float s_frequency_hz = DEFAULT_FREQUENCY_HZ;

static bool s_previous_output_button = false;
static bool s_previous_wave_button = false;

/* Watch these two variables in STM32 Viewer */
volatile uint16_t g_scope_adc_raw = 0U;
volatile float g_scope_voltage_v = 0.0f;

/* ========================================================================== */
/* Function declarations                                                      */
/* ========================================================================== */

static void FPU_Init(void);
static void GPIO_Init(void);
static void DAC_Init(void);
static void ADC_Init(void);
static void NVIC_Init(void);

static uint16_t Timer_CalculatePrescaler(float event_rate_hz);
static void Timer_Configure(TIM_Registers *timer, uint16_t prescaler);
static void AWG_TimerInit(float output_frequency_hz);
static void AWG_TimerSetFrequency(float output_frequency_hz);
static void Scope_TimerInit(uint32_t sample_rate_hz);

static uint16_t ADC1_ReadNextConversion(void);
static void ADC1_ReadControls(uint16_t *amplitude_raw, uint16_t *frequency_raw);

static uint16_t AWG_ClampDacCode(float value);
static float AWG_AmplitudeToCounts(float amplitude_v);
static void AWG_GenerateLut(void);
static void AWG_Init(void);
static void AWG_SetAmplitude(float amplitude_v);
static void AWG_SetFrequency(float frequency_hz);
static void AWG_SelectNextWaveform(void);
static void AWG_ToggleOutput(void);

static void Scope_Init(void);
static void Controls_Process(void);

/* ========================================================================== */
/* Peripheral initialization                                                  */
/* ========================================================================== */

static void FPU_Init(void)
{
    volatile uint32_t *const cpacr = (volatile uint32_t *)CPACR_ADDRESS;
    *cpacr |= (3U << 20) | (3U << 22);
}

static void GPIO_Init(void)
{
    RCC->AHB1ENR |= (1U << 0); /* GPIOA clock */
    RCC->AHB1ENR |= (1U << 1); /* GPIOB clock */

    /* PA1: amplitude pot, PA2: frequency pot, PA3: scope input, PA4: DAC */
    GPIOA->MODER &= ~((3U << 2) | (3U << 4) | (3U << 6) | (3U << 8));
    GPIOA->MODER |=  ((3U << 2) | (3U << 4) | (3U << 6) | (3U << 8));
    GPIOA->PUPDR &= ~((3U << 2) | (3U << 4) | (3U << 6) | (3U << 8));

    /* PB1 and PB2: buttons connected to 3.3 V, internal pull-down enabled */
    GPIOB->MODER &= ~((3U << 2) | (3U << 4));
    GPIOB->PUPDR &= ~((3U << 2) | (3U << 4));
    GPIOB->PUPDR |=  ((2U << 2) | (2U << 4));
}

static void DAC_Init(void)
{
    RCC->APB1ENR |= (1U << 29);

    DAC1->CR |=  (1U << 0); /* DAC channel 1 enable */
    DAC1->CR &= ~(1U << 1); /* Output buffer enable */
    DAC1->DHR12R1 = DAC_CENTER_CODE;
}

static void ADC_Init(void)
{
    RCC->APB2ENR |= (1U << 8); /* ADC1 clock */
    RCC->APB2ENR |= (1U << 9); /* ADC2 clock */

    /* ADC1 regular sequence: channel 1, then channel 2 */
    ADC1->SQR3 &= ~((0x1FU << 0) | (0x1FU << 5));
    ADC1->SQR3 |=  ((1U << 0) | (2U << 5));
    ADC1->SQR1 &= ~(0xFU << 20);
    ADC1->SQR1 |=  (1U << 20); /* Two conversions */

    ADC1->CR1 |= ADC_CR1_SCAN;
    ADC1->CR1 &= ~ADC_CR1_EOCIE;
    ADC1->CR2 &= ~ADC_CR2_CONT;
    ADC1->CR2 |= ADC_CR2_EOCS;
    ADC1->CR2 |= ADC_CR2_ADON;

    /* ADC2 regular sequence: channel 3 */
    ADC2->SQR1 &= ~(0xFU << 20);
    ADC2->SQR3 &= ~(0x1FU << 0);
    ADC2->SQR3 |=  (3U << 0);

    ADC2->CR1 |= ADC_CR1_EOCIE;
    ADC2->CR2 &= ~ADC_CR2_CONT;
    ADC2->CR2 |= ADC_CR2_EOCS;
    ADC2->CR2 |= ADC_CR2_ADON;
}

static void NVIC_Init(void)
{
    NVIC->ISER[0] |= (1U << ADC_IRQ_NUMBER);
    NVIC->ISER[0] |= (1U << TIM1_UP_TIM10_IRQ_NUMBER);
    NVIC->ISER[1] |= (1U << (TIM8_UP_TIM13_IRQ_NUMBER - 32U));

    NVIC->IPR[ADC_IRQ_NUMBER] = 0x00U;
    NVIC->IPR[TIM1_UP_TIM10_IRQ_NUMBER] = 0x10U;
    NVIC->IPR[TIM8_UP_TIM13_IRQ_NUMBER] = 0x20U;
}

/* ========================================================================== */
/* Timer control                                                              */
/* ========================================================================== */

static uint16_t Timer_CalculatePrescaler(float event_rate_hz)
{
    float divider;
    uint32_t prescaler;

    if (event_rate_hz <= 0.0f)
    {
        return 0xFFFFU;
    }

    divider = (float)SYSTEM_CLOCK_HZ /
              (event_rate_hz * (float)(TIMER_AUTO_RELOAD + 1U));

    if (divider <= 1.0f)
    {
        return 0U;
    }

    prescaler = (uint32_t)(divider - 1.0f + 0.5f);

    if (prescaler > 0xFFFFU)
    {
        prescaler = 0xFFFFU;
    }

    return (uint16_t)prescaler;
}

static void Timer_Configure(TIM_Registers *timer, uint16_t prescaler)
{
    timer->CR1 = 0U;
    timer->DIER = 0U;
    timer->PSC = prescaler;
    timer->ARR = TIMER_AUTO_RELOAD;
    timer->EGR = TIM_EGR_UG;
    timer->SR = 0U;
    timer->CR1 = TIM_CR1_URS;
    timer->DIER = TIM_DIER_UIE;
    timer->CR1 |= TIM_CR1_CEN;
}

static void AWG_TimerInit(float output_frequency_hz)
{
    RCC->APB2ENR |= (1U << 0); /* TIM1 clock */

    Timer_Configure(
        TIM1,
        Timer_CalculatePrescaler(output_frequency_hz * (float)WAVE_LUT_SIZE));

    s_awg_timer_initialized = true;
}

static void AWG_TimerSetFrequency(float output_frequency_hz)
{
    uint16_t new_prescaler = Timer_CalculatePrescaler(
        output_frequency_hz * (float)WAVE_LUT_SIZE);

    if (TIM1->PSC != new_prescaler)
    {
        TIM1->PSC = new_prescaler;
        TIM1->EGR = TIM_EGR_UG;
        TIM1->SR &= ~TIM_SR_UIF;
    }
}

static void Scope_TimerInit(uint32_t sample_rate_hz)
{
    RCC->APB2ENR |= (1U << 1); /* TIM8 clock */
    Timer_Configure(TIM8, Timer_CalculatePrescaler((float)sample_rate_hz));
}

/* ========================================================================== */
/* ADC control inputs                                                         */
/* ========================================================================== */

static uint16_t ADC1_ReadNextConversion(void)
{
    while ((ADC1->SR & ADC_SR_EOC) == 0U)
    {
    }

    return (uint16_t)ADC1->DR;
}

static void ADC1_ReadControls(uint16_t *amplitude_raw, uint16_t *frequency_raw)
{
    ADC1->CR2 |= ADC_CR2_SWSTART;
    *amplitude_raw = ADC1_ReadNextConversion();
    *frequency_raw = ADC1_ReadNextConversion();
}

/* ========================================================================== */
/* Function generator                                                         */
/* ========================================================================== */

static uint16_t AWG_ClampDacCode(float value)
{
    if (value < 0.0f)
    {
        return 0U;
    }

    if (value > (float)DAC_MAX_CODE)
    {
        return DAC_MAX_CODE;
    }

    return (uint16_t)value;
}

static float AWG_AmplitudeToCounts(float amplitude_v)
{
    return (2047.5f * amplitude_v) / MAX_AMPLITUDE_V;
}

static void AWG_GenerateLut(void)
{
    bool interrupt_was_enabled = false;
    float amplitude_counts = AWG_AmplitudeToCounts(s_amplitude_v);
    float maximum = (float)DAC_CENTER_CODE + amplitude_counts;
    float minimum = (float)DAC_CENTER_CODE - amplitude_counts;
    float triangle_value = (float)DAC_CENTER_CODE;
    float triangle_step = amplitude_counts / ((float)WAVE_LUT_SIZE / 4.0f);
    float sawtooth_value = minimum;
    float sawtooth_step = (maximum - minimum) / (float)WAVE_LUT_SIZE;

    if (s_awg_timer_initialized)
    {
        interrupt_was_enabled = ((TIM1->DIER & TIM_DIER_UIE) != 0U);
        TIM1->DIER &= ~TIM_DIER_UIE;
    }

    for (uint32_t i = 0U; i < WAVE_LUT_SIZE; ++i)
    {
        switch (s_waveform)
        {
            case WAVEFORM_SINE:
            {
                float angle = (TWO_PI * (float)i) / (float)(WAVE_LUT_SIZE - 1U);
                s_wave_lut[i] = AWG_ClampDacCode(
                    (sinf(angle) * amplitude_counts) + (float)DAC_CENTER_CODE);
                break;
            }

            case WAVEFORM_SQUARE:
                s_wave_lut[i] = AWG_ClampDacCode(
                    (i < (WAVE_LUT_SIZE / 2U)) ? maximum : minimum);
                break;

            case WAVEFORM_TRIANGLE:
                s_wave_lut[i] = AWG_ClampDacCode(triangle_value);

                if (i < (WAVE_LUT_SIZE / 4U))
                {
                    triangle_value += triangle_step;
                }
                else if (i < ((3U * WAVE_LUT_SIZE) / 4U))
                {
                    triangle_value -= triangle_step;
                }
                else
                {
                    triangle_value += triangle_step;
                }
                break;

            case WAVEFORM_SAWTOOTH:
                s_wave_lut[i] = AWG_ClampDacCode(sawtooth_value);
                sawtooth_value += sawtooth_step;
                break;

            default:
                s_wave_lut[i] = DAC_CENTER_CODE;
                break;
        }
    }

    s_lut_index = 0U;

    if (s_awg_timer_initialized && interrupt_was_enabled)
    {
        TIM1->DIER |= TIM_DIER_UIE;
    }
}

static void AWG_Init(void)
{
    s_waveform = WAVEFORM_SINE;
    s_amplitude_v = DEFAULT_AMPLITUDE_V;
    s_frequency_hz = DEFAULT_FREQUENCY_HZ;
    s_output_enabled = true;
    s_lut_index = 0U;

    AWG_GenerateLut();
    AWG_TimerInit(s_frequency_hz);
}

static void AWG_SetAmplitude(float amplitude_v)
{
    if (amplitude_v < 0.0f)
    {
        amplitude_v = 0.0f;
    }
    else if (amplitude_v > MAX_AMPLITUDE_V)
    {
        amplitude_v = MAX_AMPLITUDE_V;
    }

    if (fabsf(amplitude_v - s_amplitude_v) >= 0.002f)
    {
        s_amplitude_v = amplitude_v;
        AWG_GenerateLut();
    }
}

static void AWG_SetFrequency(float frequency_hz)
{
    if (frequency_hz < MIN_FREQUENCY_HZ)
    {
        frequency_hz = MIN_FREQUENCY_HZ;
    }
    else if (frequency_hz > MAX_FREQUENCY_HZ)
    {
        frequency_hz = MAX_FREQUENCY_HZ;
    }

    if (fabsf(frequency_hz - s_frequency_hz) >= 0.1f)
    {
        s_frequency_hz = frequency_hz;
        AWG_TimerSetFrequency(s_frequency_hz);
    }
}

static void AWG_SelectNextWaveform(void)
{
    s_waveform = (Waveform)(((uint32_t)s_waveform + 1U) % (uint32_t)WAVEFORM_COUNT);
    AWG_GenerateLut();
}

static void AWG_ToggleOutput(void)
{
    s_output_enabled = !s_output_enabled;

    if (!s_output_enabled)
    {
        DAC1->DHR12R1 = DAC_CENTER_CODE;
    }
}

/* ========================================================================== */
/* Oscilloscope                                                               */
/* ========================================================================== */

static void Scope_Init(void)
{
    Scope_TimerInit(SCOPE_SAMPLE_RATE_HZ);
}

/* ========================================================================== */
/* User controls                                                              */
/* ========================================================================== */

static void Controls_Process(void)
{
    uint16_t amplitude_raw;
    uint16_t frequency_raw;
    float amplitude_v;
    float frequency_hz;
    bool output_button;
    bool wave_button;

    ADC1_ReadControls(&amplitude_raw, &frequency_raw);

    amplitude_v = ((float)amplitude_raw * MAX_AMPLITUDE_V) /
                  AMPLITUDE_POT_FULL_SCALE;

    frequency_hz = MIN_FREQUENCY_HZ +
                   (((float)frequency_raw / FREQUENCY_POT_FULL_SCALE) *
                    (MAX_FREQUENCY_HZ - MIN_FREQUENCY_HZ));

    AWG_SetAmplitude(amplitude_v);
    AWG_SetFrequency(frequency_hz);

    output_button = ((GPIOB->IDR & (1U << 1)) != 0U);
    wave_button = ((GPIOB->IDR & (1U << 2)) != 0U);

    if (output_button && !s_previous_output_button)
    {
        AWG_ToggleOutput();
    }

    if (wave_button && !s_previous_wave_button)
    {
        AWG_SelectNextWaveform();
    }

    s_previous_output_button = output_button;
    s_previous_wave_button = wave_button;
}

/* ========================================================================== */
/* Interrupt handlers                                                         */
/* ========================================================================== */

void TIM1_UP_TIM10_IRQHandler(void)
{
    if ((TIM1->SR & TIM_SR_UIF) != 0U)
    {
        TIM1->SR &= ~TIM_SR_UIF;

        if (s_output_enabled)
        {
            DAC1->DHR12R1 = s_wave_lut[s_lut_index];
            ++s_lut_index;

            if (s_lut_index >= WAVE_LUT_SIZE)
            {
                s_lut_index = 0U;
            }
        }
    }
}

void TIM8_UP_TIM13_IRQHandler(void)
{
    if ((TIM8->SR & TIM_SR_UIF) != 0U)
    {
        TIM8->SR &= ~TIM_SR_UIF;
        ADC2->CR2 |= ADC_CR2_SWSTART;
    }
}

void ADC_IRQHandler(void)
{
    if ((ADC2->SR & ADC_SR_EOC) != 0U)
    {
        g_scope_adc_raw = (uint16_t)ADC2->DR;
        g_scope_voltage_v =
            (SCOPE_CALIBRATED_VREF * (float)g_scope_adc_raw) /
            SCOPE_CALIBRATED_COUNTS;
    }
}

/* ========================================================================== */
/* Main                                                                       */
/* ========================================================================== */

int main(void)
{
    FPU_Init();
    GPIO_Init();
    DAC_Init();
    ADC_Init();
    NVIC_Init();

    AWG_Init();
    Scope_Init();

    while (1)
    {
        Controls_Process();
    }
}
