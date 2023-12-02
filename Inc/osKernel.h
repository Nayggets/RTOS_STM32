#ifndef __OSKERNEL_H
#define __OSKERNEL_H
#include <stdint.h>
#include "stm32f4xx.h"
#define PERIOD 100
#define SR_UIF	(1U<<0)

void tim2_1hz_interrupt_init(void);

uint8_t osKernelAddThread(void(*task0)(void),void(*task1)(void),void(*task2)(void));
void osKernelInit();
void osKernelLaunch(uint32_t quantum);
void osThreadYield();
void task3();
void osSemaphoreInit(int32_t *semaphore, int32_t value);
void osSemaphoreSet(int32_t *semaphore);
void osSemaphoreWait(int32_t *semaphore);

#endif
