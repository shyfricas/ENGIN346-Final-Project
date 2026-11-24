#ifndef __P2_CONFIG_H
#define __P2_CONFIG_H

// every tunable constant lives here, so another bench only changes this file

#define SYSCLK_HZ        80000000U
#define TIMER_CLK_HZ     80000000U  // TIM1 (APB2) and TIM2 (APB1), both prescalers = 1

#define UART2_BAUD       460800U    // host link through ST-Link

#define TRIG_MIN_HZ      10U
#define TRIG_MAX_HZ      10000U
#define TRIG_DEFAULT_HZ  1000U

#define CAPTURE_MAX      4096U      // events per block
#define LINE_MAX         64U        // longest command line

// NVIC priorities, lower number = higher priority
#define PRIO_TIM6        0U         // load storm, added later
#define PRIO_TIM2        1U
#define PRIO_USART2      1U         // same level as TIM2 so neither preempts the other

#endif /* __P2_CONFIG_H */
