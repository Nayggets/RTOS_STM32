#include "timebase.h"
#include "stm32f4xx.h"


#define ONE_SEC_LOAD 	16000000

#define CTRL_ENABLE 	(1U<<0)
#define CTRL_TICKINT	(1U<<1)
#define CTRL_CLCKSRC	(1U<<2)
#define CTRL_COUNTFLAG 	(1U<<16)

#define MAX_DELAY		(0xFFFFFFFF)
volatile uint32_t g_curr_tick;
volatile uint32_t g_curr_tick_p;
volatile uint32_t tick_freq = 1;


void delay(uint32_t delay)
{
	uint32_t tickstart = get_tick();
	uint32_t wait = delay;

	if(wait < MAX_DELAY)
	{
		wait += (uint32_t)(tick_freq);
	}

	while((get_tick() - tickstart) < wait){}
}
void tick_increment()
{
	g_curr_tick += tick_freq;
}

uint32_t get_tick()
{
	__disable_irq();
	g_curr_tick_p = g_curr_tick;
	__enable_irq();

	return g_curr_tick_p;
}
void timebase_init()
{
	/*Reload the timer with number of cycles for a second*/
	SysTick->LOAD = ONE_SEC_LOAD -1;
	/*Clear Systick current value register*/
	SysTick->VAL = 0;
	/*Select internal clock source*/
	SysTick->CTRL = CTRL_CLCKSRC;
	/*Enable interrupt*/
	SysTick->CTRL |= CTRL_TICKINT;
	/*Enable Systick*/
	SysTick->CTRL |= CTRL_ENABLE;

	/*Enable global interrupts*/
	__enable_irq();

	/**/
}
/*
void SysTick_Handler(void)
{
	tick_increment();
}
*/
