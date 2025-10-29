#include <stdint.h>
#include "uart.h"

#define GPIOAEN (1U<<0)
#define UART1EN (1U<<4)

#define DBG_UART_BAUDRATE 115200
#define SYS_FREQ          180000000
#define APB1_CLK          45000000
#define CR1_TE            (1U<<3)
#define CR1_UE            (1U<<13)
#define SR_TXE            (1U<<7)

//static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate);
static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate);
static void uart_write(int ch);

int __io_putchar(int ch){

	uart_write(ch);
	return ch;

}

void debug_uart_init(void){

	/*Enable clock to gpioa*/
	RCC->AHB1ENR |= GPIOAEN;

	/*set the alternate function mode*/
	GPIOA->MODER &= ~(1<<18);
	GPIOA->MODER |= (1<<19);

	/*set alternate function type to AF7*/
	GPIOA->AFR[1] |= (1U<<4);
	GPIOA->AFR[1] |= (1U<<5);
	GPIOA->AFR[1] |= (1U<<6);
	GPIOA->AFR[1] &= ~(1U<7);

	/*Enable clock access to uart2*/
	RCC->APB2ENR |= UART1EN;

	/*configure uart baudrate*/
	uart_set_baudrate(APB1_CLK, DBG_UART_BAUDRATE);

	/*configure transfer direction*/
	USART1->CR1 |= CR1_TE;

	/*enable uart module*/
	USART1->CR1 |= CR1_UE;

}

static void uart_write(int ch)
{

	/*Make sure transmit data register*/
	while(!(USART1->SR & SR_TXE)){}


	/*Write to transmit data register*/
	USART1->DR = (ch & 0xFF);

}

static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate){

	return ((periph_clk + (baudrate/2U))/baudrate);

}


static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate){

	USART1->BRR = compute_uart_bd(periph_clk, baudrate);

}
