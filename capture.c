#include "capture.h"
#include <stdio.h>

volatile uint32_t t_trig[CAPTURE_MAX];
volatile uint32_t t_strobe[CAPTURE_MAX];

volatile uint32_t n_trig=0;
volatile uint32_t n_strobe=0;
volatile uint32_t n_target=0;
volatile uint32_t n_overcapture=0;
volatile uint32_t cap_state=CAP_IDLE;

// TIM2 is the one time base: 32-bit, 80 MHz, 12.5 ns per tick, wraps after 53.7 s
// PA.0 (A0)  = TIM2_CH1, AF1, trigger in (jumper from PA.8)
// PB.10 (D6) = TIM2_CH3, AF1, strobe in from the function generator
void Capture_Init(void) {

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;

	GPIOA->MODER &= ~(0x3<<(2*0));
	GPIOA->MODER |= (0x2<<(2*0));
	GPIOA->AFR[0] &= ~(0xF<<(4*0));
	GPIOA->AFR[0] |= (0x1<<(4*0));
	GPIOA->PUPDR &= ~(0x3<<(2*0));
	GPIOA->PUPDR |= (0x2<<(2*0)); // pull-down so an unplugged jumper reads low

	GPIOB->MODER &= ~(0x3<<(2*10));
	GPIOB->MODER |= (0x2<<(2*10));
	GPIOB->AFR[1] &= ~(0xF<<(4*2));
	GPIOB->AFR[1] |= (0x1<<(4*2));
	GPIOB->PUPDR &= ~(0x3<<(2*10));
	GPIOB->PUPDR |= (0x2<<(2*10));

	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	TIM2->CR1 &= ~TIM_CR1_DIR;
	TIM2->PSC = 0;
	TIM2->ARR = 0xFFFFFFFF;

	// CC1S = 01, IC1 mapped on TI1 (PA.0)
	TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_IC1F | TIM_CCMR1_IC1PSC);
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;

	// CC3S = 01, IC3 mapped on TI3 (PB.10)
	TIM2->CCMR2 &= ~(TIM_CCMR2_CC3S | TIM_CCMR2_IC3F | TIM_CCMR2_IC3PSC);
	TIM2->CCMR2 |= TIM_CCMR2_CC3S_0;

	// rising edge on both
	TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP | TIM_CCER_CC3P | TIM_CCER_CC3NP);
	TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC3E;

	TIM2->EGR = TIM_EGR_UG;
	TIM2->SR = 0;
	TIM2->CR1 |= TIM_CR1_CEN;

	NVIC_SetPriority(TIM2_IRQn, PRIO_TIM2);
	NVIC_EnableIRQ(TIM2_IRQn);
}

void Capture_Start(uint32_t n) {
	n_trig = 0;
	n_strobe = 0;
	n_overcapture = 0;
	n_target = n;

	TIM2->SR = 0; // drop anything captured before the start
	cap_state = CAP_CAPTURING;
	TIM2->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC3IE;
}

// week 1 version: timestamps go into RAM from the ISR, the DMA path comes in week 3
void TIM2_IRQHandler(void) {
	uint32_t sr = TIM2->SR;

	if ((sr & TIM_SR_CC1IF) != 0) {
		t_trig[n_trig] = TIM2->CCR1; // reading CCR1 clears CC1IF
		n_trig++;
	}

	if ((sr & TIM_SR_CC3IF) != 0) {
		if (n_strobe < CAPTURE_MAX) {
			t_strobe[n_strobe] = TIM2->CCR3;
			n_strobe++;
		} else {
			(void)TIM2->CCR3;
		}
	}

	// a second edge came before we read the first one
	if ((sr & (TIM_SR_CC1OF | TIM_SR_CC3OF)) != 0) {
		n_overcapture++;
		TIM2->SR &= ~(TIM_SR_CC1OF | TIM_SR_CC3OF);
	}

	if (n_trig >= n_target) {
		TIM2->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_CC3IE);
		cap_state = CAP_DONE;
	}
}

// E,<k>,<t_trig>,<enc>,<lat>   enc and lat are "-" until the encoder latch is added
// S,<k>,<t_strobe>
// END,<n_trig>,<n_strobe>,<n_overcapture>
void Capture_Dump(void) {
	uint32_t i;

	for (i=0; i<n_trig; i++)
		printf("E,%u,%u,-,-\n", i, t_trig[i]);

	for (i=0; i<n_strobe; i++)
		printf("S,%u,%u\n", i, t_strobe[i]);

	printf("END,%u,%u,%u\n", n_trig, n_strobe, n_overcapture);

	cap_state = CAP_IDLE;
}
