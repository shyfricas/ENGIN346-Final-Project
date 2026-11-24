#include "trigger.h"
#include "config.h"

volatile uint32_t trig_hz=0;   // 0 = off
volatile uint32_t trig_psc=0;
volatile uint32_t trig_arr=0;

// PA.8 (D7) = TIM1_CH1, AF1, hardware trigger out
void Trigger_Init(void) {

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	GPIOA->MODER &= ~(0x3<<(2*8));
	GPIOA->MODER |= (0x2<<(2*8)); // Alternate Function Mode
	GPIOA->AFR[1] &= ~(0xF<<(4*0));
	GPIOA->AFR[1] |= (0x1<<(4*0)); // AF1 = TIM1_CH1
	GPIOA->OSPEEDR |= (0x3<<(2*8));
	GPIOA->PUPDR &= ~(0x3<<(2*8));

	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	TIM1->CR1 &= ~TIM_CR1_DIR;
	TIM1->CR1 |= TIM_CR1_ARPE; // new ARR only takes effect at the next update

	TIM1->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M);
	TIM1->CCMR1 |= TIM_CCMR1_OC1M_2; // 0100 = force inactive until TRIG
	TIM1->CCMR1 |= TIM_CCMR1_OC1PE;

	TIM1->CCER &= ~TIM_CCER_CC1P; // active high
	TIM1->CCER |= TIM_CCER_CC1E;

	TIM1->BDTR |= TIM_BDTR_MOE; // without MOE the pin stays idle
}

// f = 80MHz / ((PSC+1)(ARR+1))
// look for a PSC that divides exactly first, otherwise take the first one that fits in 16 bits
void Trigger_Set(uint32_t hz) {
	uint32_t psc, counts;
	uint32_t best_psc=0xFFFFFFFF, best_counts=0;

	for (psc=0; psc<=0xFFFF; psc++) {
		counts = (TIMER_CLK_HZ + hz*(psc+1)/2) / (hz*(psc+1));
		if (counts > 0x10000 || counts < 2)
			continue;
		if (best_psc == 0xFFFFFFFF) {
			best_psc = psc;
			best_counts = counts;
		}
		if (counts*(psc+1)*hz == TIMER_CLK_HZ) {
			best_psc = psc;
			best_counts = counts;
			break;
		}
	}

	trig_psc = best_psc;
	trig_arr = best_counts - 1;
	trig_hz = hz;

	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM1->PSC = trig_psc;
	TIM1->ARR = trig_arr;
	TIM1->CCR1 = best_counts/2; // 50% duty
	TIM1->CNT = 0;
	TIM1->EGR = TIM_EGR_UG; // load PSC and ARR now

	TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
	TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // 0110 = PWM mode 1

	TIM1->CR1 |= TIM_CR1_CEN;
}

void Trigger_Off(void) {
	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
	TIM1->CCMR1 |= TIM_CCMR1_OC1M_2; // force low
	trig_hz = 0;
}
