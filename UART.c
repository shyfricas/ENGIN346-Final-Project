#include "UART.h"

// UART Ports:
// ===================================================
// PA.2 = USART2_TX (AF7)
// PA.3 = USART2_RX (AF7)

#define TX_PIN 2
#define RX_PIN 3

volatile char line_buf[LINE_MAX];
volatile uint32_t line_ready=0;
volatile uint32_t line_len=0;

void UART2_Init(void) {
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

	// 00: PCLK, 01: SYSCLK, 10: HSI16, 11: LSE
	RCC->CCIPR &= ~RCC_CCIPR_USART2SEL;
	RCC->CCIPR |=  RCC_CCIPR_USART2SEL_0;

	UART2_GPIO_Init();
	USART_Init(USART2, UART2_BAUD);

	USART2->CR1 |= USART_CR1_RXNEIE;
	NVIC_SetPriority(USART2_IRQn, PRIO_USART2);
	NVIC_EnableIRQ(USART2_IRQn);
}

void UART2_GPIO_Init(void) {

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// Input(00), Output(01), AlterFunc(10), Analog(11)
	GPIOA->MODER   &= ~(3<<(2*TX_PIN) | 3<<(2*RX_PIN));
	GPIOA->MODER   |=   2<<(2*TX_PIN) | 2<<(2*RX_PIN);
	GPIOA->AFR[0]  &= ~(0xF<<(4*TX_PIN) | 0xF<<(4*RX_PIN));
	GPIOA->AFR[0]  |=   7<<(4*TX_PIN) | 7<<(4*RX_PIN);
	GPIOA->OSPEEDR |=   3<<(2*TX_PIN) | 3<<(2*RX_PIN);
	GPIOA->PUPDR   &= ~(3<<(2*TX_PIN) | 3<<(2*RX_PIN));
	GPIOA->OTYPER  &= ~(1<<TX_PIN | 1<<RX_PIN);
}

// 8 data bits, no parity, 1 stop bit, oversampling by 16
void USART_Init(USART_TypeDef * USARTx, uint32_t baud) {
	USARTx->CR1 &= ~USART_CR1_UE;

	USARTx->CR1 &= ~USART_CR1_M;
	USARTx->CR1 &= ~USART_CR1_OVER8;
	USARTx->CR2 &= ~USART_CR2_STOP;

	// BRR = 80MHz / baud, rounded. 460800 -> 174 (0.2% error)
	USARTx->BRR = (SYSCLK_HZ + baud/2) / baud;

	USARTx->CR1 |= (USART_CR1_RE | USART_CR1_TE);
	USARTx->CR1 |= USART_CR1_UE;

	while ((USARTx->ISR & USART_ISR_TEACK) == 0);
	while ((USARTx->ISR & USART_ISR_REACK) == 0);
}

uint8_t USART_Read(USART_TypeDef * USARTx) {
	while (!(USARTx->ISR & USART_ISR_RXNE));
	return ((uint8_t)(USARTx->RDR & 0xFF));
}

void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes) {
	uint32_t i;
	for (i = 0; i < nBytes; i++) {
		while (!(USARTx->ISR & USART_ISR_TXE));
		USARTx->TDR = buffer[i] & 0xFF;
	}
	while (!(USARTx->ISR & USART_ISR_TC));
}

// builds one command line, main loop handles it when line_ready = 1
void USART2_IRQHandler(void) {
	char c;

	if (USART2->ISR & USART_ISR_ORE)
		USART2->ICR = USART_ICR_ORECF;

	if (USART2->ISR & USART_ISR_RXNE) {
		c = USART2->RDR;

		if (line_ready)  // last line not handled yet, drop this byte
			return;

		// accept CR, LF or CRLF so Tera Term works with default settings
		if (c == '\r' || c == '\n') {
			if (line_len == 0)
				return;
			line_buf[line_len] = 0;
			line_len = 0;
			line_ready = 1;
		} else if (line_len < LINE_MAX-1) {
			line_buf[line_len] = c;
			line_len++;
		}
	}
}
