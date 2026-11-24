#ifndef __P2_CAPTURE_H
#define __P2_CAPTURE_H

#include "stm32l476xx.h"
#include "config.h"

#define CAP_IDLE       0
#define CAP_CAPTURING  1
#define CAP_DONE       2

void Capture_Init(void);
void Capture_Start(uint32_t n);
void Capture_Dump(void);

extern volatile uint32_t cap_state;
extern volatile uint32_t n_overcapture;

#endif /* __P2_CAPTURE_H */
