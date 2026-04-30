
 /******************************************************************************
 * @file           : main.c
 * @author         : Musaab Taha
 * @Email          : mosab_taha@hotmail.com
 * @LinkedIn       : https://www.linkedin.com/in/musaab-taha-956bb7105/
 ******************************************************************************/


#include <stdint.h>
#include <math.h>


/********************** Registers Types Definitions ******************/

typedef struct
{
    volatile uint32_t MODER;    // 0x00
    volatile uint32_t OTYPER;   // 0x04
    volatile uint32_t OSPEEDR;  // 0x08
    volatile uint32_t PUPDR;    // 0x0C
    volatile uint32_t IDR;      // 0x10
    volatile uint32_t ODR;      // 0x14
    volatile uint32_t BSRR;     // 0x18
    volatile uint32_t LCKR;     // 0x1C
    volatile uint32_t AFR[2];   // 0x20, 0x24

} GPIO_t;


typedef struct{

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

}DAC_t;


typedef struct
{
    volatile uint32_t CR;          // 0x00
    volatile uint32_t PLLCFGR;     // 0x04
    volatile uint32_t CFGR;        // 0x08
    volatile uint32_t CIR;         // 0x0C

    volatile uint32_t AHB1RSTR;    // 0x10
    volatile uint32_t AHB2RSTR;    // 0x14
    volatile uint32_t AHB3RSTR;    // 0x18
    volatile uint32_t RESERVED0;   // 0x1C

    volatile uint32_t APB1RSTR;    // 0x20
    volatile uint32_t APB2RSTR;    // 0x24
    volatile uint32_t RESERVED1[2];// 0x28, 0x2C

    volatile uint32_t AHB1ENR;     // 0x30
    volatile uint32_t AHB2ENR;     // 0x34
    volatile uint32_t AHB3ENR;     // 0x38
    volatile uint32_t RESERVED2;   // 0x3C

    volatile uint32_t APB1ENR;     // 0x40
    volatile uint32_t APB2ENR;     // 0x44
    volatile uint32_t RESERVED3[2];// 0x48, 0x4C

    volatile uint32_t AHB1LPENR;   // 0x50
    volatile uint32_t AHB2LPENR;   // 0x54
    volatile uint32_t AHB3LPENR;   // 0x58
    volatile uint32_t RESERVED4;   // 0x5C

    volatile uint32_t APB1LPENR;   // 0x60
    volatile uint32_t APB2LPENR;   // 0x64
    volatile uint32_t RESERVED5[2];// 0x68, 0x6C

    volatile uint32_t BDCR;        // 0x70
    volatile uint32_t CSR;         // 0x74
    volatile uint32_t RESERVED6[2];// 0x78, 0x7C

    volatile uint32_t SSCGR;       // 0x80
    volatile uint32_t PLLI2SCFGR;  // 0x84
    volatile uint32_t PLLSAICFGR;  // 0x88
    volatile uint32_t DCKCFGR;     // 0x8C

} RCC_t;

/********************** Registers Types Definitions ******************/
/********************** Macros Definitions ******************/

#define RCC_BASE    0x40023800U
#define GPIOA_BASE  0x40020000U
#define DAC_BASE 	0x40007400U
#define FPU         0xE000ED88U

#define RCC         ((RCC_t *) RCC_BASE)
#define GPIOA       ((GPIO_t *) GPIOA_BASE)
#define DAC 		((DAC_t *) DAC_BASE)

#define NS 128
#define TWO_PI 6.28318530717958647692f

uint16_t Wave_LUT[NS];

/********************** Macros Definitions ******************/
/********************** Functions Declaration ******************/


void GPIO_Init(void);
void DAC_Init(void);
void WaveLUT_Generate_Sinewave(void);


/********************** Functions Declaration ******************/



int main(void)
{

	*(volatile uint32_t *)FPU |= (3 << 20);
	*(volatile uint32_t *)FPU |= (3 << 22);

    GPIO_Init();
    DAC_Init();
    WaveLUT_Generate_Sinewave();

    while(1){

    	for(int i = 0;i<128;i++){

    		DAC->DHR12R1 = Wave_LUT[i];

    	}

    }

}

/*************************** Functions Definitions ***********/

void GPIO_Init(void){

	RCC->AHB1ENR |= (1 << 0);   // Enable GPIOA Clock

	GPIOA->MODER |= (3 << 8); // Setting pin to analog mode

}


void DAC_Init(void){

	RCC->APB1ENR |= (1 << 29); //Enable the DAC clock

	DAC->CR |= (1 << 0); //Enable DAC channel 1
	DAC->CR &= ~(1 << 1); //Enable DAC output buffer
	DAC->CR &= ~(1 << 2); //Enable Software trigger

}


void WaveLUT_Generate_Sinewave(void) {

	for (int i = 0; i < NS; i++) {

		float t = (TWO_PI * i) /(NS - 1);
		Wave_LUT[i] = (uint16_t)((sinf(t) + 1.0) * 2047.5 + 0.5);

	}

}
