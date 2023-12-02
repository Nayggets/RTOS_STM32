#include "uart.h"
#include <stdio.h>
#include "osKernel.h"

#define QUANTUM 2



void motor_run()
{
	printf("motor run ------->\n\r");
}

void valve_open()
{
	printf("valve open ------->\n\r");
}

void valve_close()
{
	printf("valve close ------->\n\r");
}

void motor_stop()
{
	printf("motor stop ------->\n\r");
}

typedef uint32_t TaskProfiler;
TaskProfiler Task0_profiler,Task1_profiler,Task2_profiler,pTask1_Profiler,pTask2_Profiler;

int32_t semaphore1,semaphore2;

void TIM2_IRQHandler(void)
{
	/*Clear Update Interrupt flag*/
	TIM2->SR &= ~SR_UIF;
	/*Do Somethings*/
	pTask2_Profiler++;
}
void task3()
{
	pTask1_Profiler++;
}
void task0()
{
	while(1)
	{
		Task0_profiler++;
		osThreadYield();
	}
}

void task1()
{
	while(1)
	{
		osSemaphoreWait(&semaphore1);
		motor_run();
		osSemaphoreSet(&semaphore1);
	}
}

void task2()
{
	while(1)
	{
		osSemaphoreWait(&semaphore2);
		valve_open();
		osSemaphoreSet(&semaphore2);

	}
}

int main()
{
	uart_tx_init();
	//initialise hardware timer
	tim2_1hz_interrupt_init();
	/*INitialise semaphores*/
	osSemaphoreInit(&semaphore1,1);
	osSemaphoreInit(&semaphore2,0);

	//initialise kernel
	osKernelInit();

	/*Add Thread*/
	osKernelAddThread(&task0,&task1,&task2);
	/*Set RoundRobin time quanta*/
	osKernelLaunch(QUANTUM);
	while(1)
	{

	}
}
