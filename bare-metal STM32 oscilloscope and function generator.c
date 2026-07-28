/**
 ******************************************************************************
 * @file    main.c
 * @author  Musaab Taha - mosab_taha@hotmail.com
 * @brief   EduScope-10 bare-metal function generator and oscilloscope prototype
 ******************************************************************************
 *
 * Hardware prototype: STM32F429I-DISC1
 *
 * Function generator
 *   PA4  -> DAC channel 1 output
 *   PA1  -> ADC1 channel 1, amplitude potentiometer
 *   PA2  -> ADC1 channel 2, frequency potentiometer
 *   PB1  -> Function-generator output button
 *   PB2  -> Waveform-selection button
 *
 * Oscilloscope
 *   PA3  -> ADC2 channel 3 input
 *   TIM8 -> ADC2 sampling timer
 *
 * The firmware uses direct register access without HAL or CubeMX-generated
 * peripheral code. ADC2_Val_temp and ADC2_Val can be observed with STM32 Viewer.
 ******************************************************************************
 */

#include <math.h>
#include <stdint.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
#warning "FPU is not initialized, but the project is compiling for an FPU."
#endif

/* ========================================================================== */
/* Register type definitions                                                  */
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
} GPIO_t;

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
} DAC_t;

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
} RCC_t;

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
} TIM_t;

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
} NVIC_t;

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
} ADC_t;

typedef struct
{
    volatile uint32_t IDCODE;
    volatile uint32_t CR;
    volatile uint32_t APB1;
    volatile uint32_t APB2;
} DBG_t;

enum wave_type
{
    sine,
    square,
    triangle,
    sawtooth
};

/* ========================================================================== */
/* Peripheral addresses                                                       */
/* ========================================================================== */

#define RCC_BASE     0x40023800U
#define GPIOA_BASE   0x40020000U
#define GPIOB_BASE   0x40020400U
#define DAC_BASE     0x40007400U
#define CPACR        0xE000ED88U
#define TIM1_BASE    0x40010000U
#define TIM8_BASE    0x40010400U
#define NVIC_BASE    0xE000E100U
#define ADC1_BASE    0x40012000U
#define ADC2_BASE    0x40012100U
#define DBG_BASE     0xE0042000U
#define DEMCR_BASE   0xE000EDFCU

#define RCC          ((RCC_t *)RCC_BASE)
#define GPIOA        ((GPIO_t *)GPIOA_BASE)
#define GPIOB        ((GPIO_t *)GPIOB_BASE)
#define DAC           ((DAC_t *)DAC_BASE)
#define TIM1          ((TIM_t *)TIM1_BASE)
#define TIM8          ((TIM_t *)TIM8_BASE)
#define NVIC          ((NVIC_t *)NVIC_BASE)
#define ADC1          ((ADC_t *)ADC1_BASE)
#define ADC2          ((ADC_t *)ADC2_BASE)
#define DBG           ((DBG_t *)DBG_BASE)

/* ========================================================================== */
/* Application constants and state                                            */
/* ========================================================================== */

#define NS          128U
#define TWO_PI      6.28318530717958647692f
#define ARR_Val     49U
#define MCU_CLK     16000000U
#define center      2048U

uint16_t Wave_LUT[NS];

volatile uint8_t flg = 0U;
volatile uint16_t pot1 = 0U;
volatile uint16_t pot2 = 0U;
volatile uint16_t ADC2_Val_temp = 0U;
volatile float ADC2_Val = 0.0f;

uint8_t lut_index = 0U;
uint32_t psc = 0U;
float desired_Amplitude = 0.5f;
float frequency = 1.0f;
enum wave_type desired_wave = sine;

/* ========================================================================== */
/* Function declarations                                                      */
/* ========================================================================== */

void GPIO_Init(void);
void DAC_Init(void);
void TIMx_Init(int freq);
void NVIC_Init(void);
void ADC_Init(void);
void DBG_Init(void);
float Amplitude_calc(float desired_amplitude);
void WaveLUT_Generate(enum wave_type wave);

/* ========================================================================== */
/* Main                                                                       */
/* ========================================================================== */

int main(void)
{
    /* Enable CP10 and CP11 for the Cortex-M4 floating-point unit */
    *(volatile uint32_t *)CPACR |= (3U << 20);
    *(volatile uint32_t *)CPACR |= (3U << 22);

    GPIO_Init();
    DAC_Init();
    NVIC_Init();
    ADC_Init();
    DBG_Init();

    /* TIM8 uses this value as the ADC2 oscilloscope sampling frequency */
    TIMx_Init(5000);

    while (1)
    {
        /* Start one ADC1 scan sequence for the two potentiometers */
        for (volatile int i = 0; i < 1000; i++)
        {
            /* Short delay retained from the working prototype */
        }
        ADC1->CR2 |= (1U << 30);

        /* PB1: preserve the original DAC trigger-enable toggle behavior */
        if ((GPIOB->IDR >> 1) & 1U)
        {
            DAC->CR ^= (1U << 2);
            for (volatile int i = 0; i < 100000; i++)
            {
                /* Simple button debounce delay */
            }
        }

        /* PB2: select the next waveform */
        if ((GPIOB->IDR >> 2) & 1U)
        {
            desired_wave++;

            for (volatile int i = 0; i < 100000; i++)
            {
                /* Simple button debounce delay */
            }
        }

        /* Convert the two potentiometer readings into amplitude and frequency */
        desired_Amplitude = (pot1 * 1.65f) / 3822.0f;
        frequency = (pot2 * 2500.0f) / 3799.0f;

        /* TIM1 update frequency = waveform frequency x LUT sample count */
        psc = (uint32_t)((MCU_CLK / ((frequency * NS) * (ARR_Val + 1U))) - 1.0f);

        TIM1->ARR = ARR_Val;
        TIM1->PSC = psc;

        /* Rebuild the LUT when amplitude or waveform selection changes */
        WaveLUT_Generate(desired_wave);
    }
}

/* ========================================================================== */
/* Peripheral initialization                                                  */
/* ========================================================================== */

void GPIO_Init(void)
{
    RCC->AHB1ENR |= (1U << 0); /* GPIOA clock */

    GPIOA->MODER |= (3U << 8); /* PA4: DAC output, analog mode */
    GPIOA->MODER |= (3U << 2); /* PA1: ADC1 channel 1, amplitude pot */
    GPIOA->MODER |= (3U << 4); /* PA2: ADC1 channel 2, frequency pot */
    GPIOA->MODER |= (3U << 6); /* PA3: ADC2 channel 3, scope input */

    RCC->AHB1ENR |= (1U << 1); /* GPIOB clock */

    GPIOB->MODER &= ~(3U << 2); /* PB1 input */
    GPIOB->MODER &= ~(3U << 4); /* PB2 input */

    GPIOB->PUPDR &= ~(3U << 2);
    GPIOB->PUPDR |=  (2U << 2); /* PB1 pull-down */

    GPIOB->PUPDR &= ~(3U << 4);
    GPIOB->PUPDR |=  (2U << 4); /* PB2 pull-down */
}

void DAC_Init(void)
{
    RCC->APB1ENR |= (1U << 29); /* DAC clock */

    DAC->CR |=  (1U << 0);      /* Enable DAC channel 1 */
    DAC->CR &= ~(1U << 1);      /* Enable the DAC output buffer */
}

void TIMx_Init(int freq)
{
    /* TIM1 drives the DAC waveform sample updates */
    RCC->APB2ENR |= (1U << 0);

    TIM1->ARR = ARR_Val;
    TIM1->PSC = psc;
    TIM1->EGR |= (1U << 0);
    TIM1->SR &= ~(1U << 0);
    TIM1->CR1 &= ~(1U << 1);
    TIM1->CR1 |= (1U << 2);
    TIM1->CR1 &= ~(1U << 4);
    TIM1->DIER |= (1U << 0);
    TIM1->CR1 |= (1U << 0);

    /* TIM8 triggers one ADC2 conversion at the requested sample frequency */
    psc = (MCU_CLK / ((uint32_t)freq * (ARR_Val + 1U))) - 1U;

    RCC->APB2ENR |= (1U << 1);

    TIM8->ARR = ARR_Val;
    TIM8->PSC = psc;
    TIM8->EGR |= (1U << 0);
    TIM8->SR &= ~(1U << 0);
    TIM8->CR1 &= ~(1U << 1);
    TIM8->CR1 |= (1U << 2);
    TIM8->CR1 &= ~(1U << 4);
    TIM8->DIER |= (1U << 0);
    TIM8->CR1 |= (1U << 0);
}

void NVIC_Init(void)
{
    NVIC->ISER[0] |= (1U << 25); /* TIM1_UP_TIM10 IRQ */
    NVIC->ISER[0] |= (1U << 18); /* Shared ADC IRQ */
    NVIC->ISER[1] |= (1U << 12); /* TIM8_UP_TIM13 IRQ */

    NVIC->IPR[25] = 0x10;
    NVIC->IPR[18] = 0x00;
}

void ADC_Init(void)
{
    RCC->APB2ENR |= (1U << 8); /* ADC1 clock */
    RCC->APB2ENR |= (1U << 9); /* ADC2 clock */

    /* ADC1 scan sequence: channel 1 followed by channel 2 */
    ADC1->SQR3 |= (1U << 0);
    ADC1->SQR3 |= (2U << 5);
    ADC1->SQR1 |= (1U << 20);  /* Two regular conversions */

    ADC1->CR2 |= (1U << 0);    /* Enable ADC1 */
    ADC1->CR1 |= (1U << 5);    /* EOC interrupt */
    ADC1->CR1 |= (1U << 8);    /* Scan mode */
    ADC1->CR2 &= ~(1U << 1);   /* Single scan sequence per software start */
    ADC1->CR2 |= (1U << 10);   /* EOC after every conversion */

    /* ADC2 regular sequence: channel 3 only */
    ADC2->SQR3 |= (3U << 0);

    ADC2->CR2 |= (1U << 0);    /* Enable ADC2 */
    ADC2->CR1 |= (1U << 5);    /* EOC interrupt */
    ADC2->CR2 &= ~(1U << 1);   /* Single conversion mode */
    ADC2->CR2 |= (1U << 10);   /* EOC after conversion */
}

void DBG_Init(void)
{
    *(volatile uint32_t *)DEMCR_BASE |= (1U << 24); /* Trace enable */
    DBG->CR |= (1U << 5);                         /* Trace I/O enable */
}

/* ========================================================================== */
/* Function-generator waveform generation                                     */
/* ========================================================================== */

float Amplitude_calc(float desired_amplitude)
{
    return (2047.5f * desired_amplitude) / 1.65f;
}

void WaveLUT_Generate(enum wave_type wave)
{
    float cnt = center;
    float amp = Amplitude_calc(desired_Amplitude);
    float max = center + amp;
    float min = center - amp;
    float step = (max - center) / (NS / 4U);

    float step_sawtooth = (max - min) / NS;
    float cnt_sawtooth = min;

    switch (wave)
    {
        case sine:
            for (int i = 0; i < NS; i++)
            {
                float t = (TWO_PI * i) / (NS - 1U);
                Wave_LUT[i] = (uint16_t)((sinf(t) * Amplitude_calc(desired_Amplitude)) + center);
            }
            break;

        case square:
            for (int i = 0; i < NS; i++)
            {
                if (i < (NS / 2U))
                {
                    Wave_LUT[i] = (uint16_t)(center + Amplitude_calc(desired_Amplitude));
                }
                else
                {
                    Wave_LUT[i] = (uint16_t)(center - Amplitude_calc(desired_Amplitude));
                }
            }
            break;

        case triangle:
            for (int i = 0; i < NS; i++)
            {
                if (i < (NS / 4U))
                {
                    Wave_LUT[i] = (uint16_t)cnt;
                    cnt += step;
                }
                else if ((i >= (NS / 4U)) && (i < ((3U * NS) / 4U)))
                {
                    Wave_LUT[i] = (uint16_t)cnt;
                    cnt -= step;
                }
                else
                {
                    Wave_LUT[i] = (uint16_t)cnt;
                    cnt += step;
                }
            }
            break;

        case sawtooth:
            for (int i = 0; i < NS; i++)
            {
                Wave_LUT[i] = (uint16_t)cnt_sawtooth;
                cnt_sawtooth += step_sawtooth;
            }
            break;
    }
}

/* ========================================================================== */
/* Interrupt handlers                                                         */
/* ========================================================================== */

void TIM1_UP_TIM10_IRQHandler(void)
{
    if (TIM1->SR & 1U)
    {
        DAC->DHR12R1 = Wave_LUT[lut_index++];
        TIM1->SR &= ~(1U << 0);

        if (lut_index >= NS)
        {
            lut_index = 0U;
        }
    }
}

void TIM8_UP_TIM13_IRQHandler(void)
{
    if (TIM8->SR & 1U)
    {
        ADC2->CR2 |= (1U << 30); /* Start one ADC2 conversion */
        TIM8->SR &= ~(1U << 0);
    }
}

void ADC_IRQHandler(void)
{
    /* ADC1 and ADC2 share the same interrupt vector */
    if (ADC1->SR & (1U << 1))
    {
        uint16_t ADC1_Val = (uint16_t)ADC1->DR;

        if (flg == 0U)
        {
            pot1 = ADC1_Val;
            flg = 1U;
        }
        else
        {
            pot2 = ADC1_Val;
            flg = 0U;
        }
    }

    if (ADC2->SR & (1U << 1))
    {
        ADC2_Val_temp = (uint16_t)ADC2->DR;
        ADC2_Val = (3.0f * ADC2_Val_temp) / 3722.0f;
    }
}
