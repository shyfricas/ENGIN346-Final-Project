#ifndef __STM32L476G_DISCOVERY_UART_H
#define __STM32L476G_DISCOVERY_UART_H

#include "stm32l476xx.h"
#include "config.h"

void UART2_Init(void);
void UART2_GPIO_Init(void);
void USART_Init(USART_TypeDef * USARTx, uint32_t baud);
void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes);
uint8_t USART_Read(USART_TypeDef * USARTx);

extern volatile char line_buf[LINE_MAX];
extern volatile uint32_t line_ready;

#endif /* __STM32L476G_DISCOVERY_UART_H */
