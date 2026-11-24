#ifndef __P2_TRIGGER_H
#define __P2_TRIGGER_H

#include "stm32l476xx.h"

void Trigger_Init(void);
void Trigger_Set(uint32_t hz);
void Trigger_Off(void);

extern volatile uint32_t trig_hz;
extern volatile uint32_t trig_psc;
extern volatile uint32_t trig_arr;

#endif /* __P2_TRIGGER_H */
