#include "osKernel.h"
#define NUM_OF_THREAD 3
#define STACKSIZE 400

#define BUS_FREQ 16000000
uint32_t MILLIS_PRESCALER;

#define CTRL_ENABLE 	(1U<<0)
#define CTRL_TICKINT	(1U<<1)
#define CTRL_CLCKSRC	(1U<<2)
#define CTRL_COUNTFLAG 	(1U<<16)

#define SYSTICK_RST 0

#define TIM2EN 		(1U<<0)
#define CR1_CEN		(1U<<0)
#define DIER_UIE 	(1U<<0)

#define INTCTRL		(*((volatile uint32_t*)0xE000ED04))
#define PENDSTSET	(1U<<26);

uint32_t period_tick;

typedef struct tcb
{
	int32_t* stackPt;
	struct tcb* nextPt;
}tcb_t;

tcb_t tcbs[NUM_OF_THREAD];

tcb_t* currentPt;

int32_t TCB_STACK[NUM_OF_THREAD][STACKSIZE];

void osSchedulerLaunch();
void osSchedulerRoundRobin();


void osKernelStackInit(int i)
{
	tcbs[i].stackPt = &TCB_STACK[i][STACKSIZE - 16];
	/*set Bit21 to one (T-BIT)*/
	TCB_STACK[i][STACKSIZE-1] = (1U<<24);

	/*Block below is optionnal, for debugging purpose only*/

	TCB_STACK[i][STACKSIZE-3] = 0xAAAAAAAA; // R14
	TCB_STACK[i][STACKSIZE-4] = 0xAAAAAAAA; // R12
	TCB_STACK[i][STACKSIZE-5] = 0xAAAAAAAA; // R3
	TCB_STACK[i][STACKSIZE-6] = 0xAAAAAAAA; // R2
	TCB_STACK[i][STACKSIZE-7] = 0xAAAAAAAA; // R1
	TCB_STACK[i][STACKSIZE-8] = 0xAAAAAAAA; // R0
	TCB_STACK[i][STACKSIZE-9] = 0xAAAAAAAA; // R11
	TCB_STACK[i][STACKSIZE-10] = 0xAAAAAAAA; // R10
	TCB_STACK[i][STACKSIZE-11] = 0xAAAAAAAA; // R9
	TCB_STACK[i][STACKSIZE-12] = 0xAAAAAAAA; // R8
	TCB_STACK[i][STACKSIZE-13] = 0xAAAAAAAA; // R7
	TCB_STACK[i][STACKSIZE-14] = 0xAAAAAAAA; // R6
	TCB_STACK[i][STACKSIZE-15] = 0xAAAAAAAA; // R5
	TCB_STACK[i][STACKSIZE-16] = 0xAAAAAAAA; // R4


}


uint8_t osKernelAddThread(void(*task0)(void),void(*task1)(void),void(*task2)(void))
{
	__disable_irq();
	tcbs[0].nextPt = &tcbs[1];
	tcbs[1].nextPt = &tcbs[2];
	tcbs[2].nextPt = &tcbs[0];

	//initial thread stack
	//thread 0
	osKernelStackInit(0);
	TCB_STACK[0][STACKSIZE-2] = (uint32_t)(task0);

	//thread 1
	osKernelStackInit(1);
	TCB_STACK[1][STACKSIZE-2] = (uint32_t)(task1);

	//thread 2
	osKernelStackInit(2);
	TCB_STACK[2][STACKSIZE-2] = (uint32_t)(task2);

	currentPt = &tcbs[0];

	__enable_irq();

	return 1;

}

void osKernelInit()
{
	MILLIS_PRESCALER = (BUS_FREQ/1000);
}

void osKernelLaunch(uint32_t quantum)
{
	/*reset systick*/
	SysTick->CTRL = SYSTICK_RST;
	/*clear systick current value register*/
	SysTick->VAL = 0;
	/*Load Quantum*/
	SysTick->LOAD = (quantum * MILLIS_PRESCALER) - 1;
	/*set systick to a low priority*/
	NVIC_SetPriority(SysTick_IRQn,15);
	/*Enable systick select internal clock*/
	SysTick->CTRL = CTRL_CLCKSRC | CTRL_ENABLE;
	/*Enable systick interupt*/
	SysTick->CTRL |= CTRL_TICKINT;
	/*Launch scheduler*/
	osSchedulerLaunch();
}

/*when exception occurs these registers are automatically
 *saved onto the stack,r1,r2,r3,r12,lr,pc,psr*/
__attribute__((naked)) void SysTick_Handler(void)
{
	/*Susend Current Thread*/
	/*Disable global interrupt*/
	__asm("CPSID	I");
	/*Save r4,r5,r6,r7,r8,r9,r10,r11*/
	__asm("PUSH {R4-R11}");
	/*load address of currentPt into r0*/
	__asm("LDR R0,=currentPt");
	/*Load r1 from address equals r0*/
	__asm("LDR R1,[R0]");

	/*Store Cortex-m SP at address equals r1, .e Save SP into tcb*/
	__asm("STR SP,[R1]");

	__asm("PUSH {R0,LR}");
	__asm("BL osSchedulerRoundRobin");
	__asm("POP {R0,LR}");


	/*R1 = CurrentPT i.e New Thread */
	__asm("LDR R1,[R0]");
	/*sp = CurrentPt->StackPt*/
	__asm("LDR SP,[R1]");
	/*Restore R4-R11*/
	__asm("POP {R4-R11}");

	/*Enable global interrupts*/
	__asm("CPSIE	I");

	/*Return from exception and restore r0,r1,r2,r3,r12,lr,pc,psr*/
	__asm("BX	LR");


}

void osSchedulerLaunch()
{
	/*Load address of currentPt into R0*/
	__asm("LDR R0,=currentPt");
	/*Load r2 from address equals r0, r2 = currentPt*/
	__asm("LDR R2,[r0]");
	/*Load Cortex-M SP from address equals R2, i.e SP = currentPt->stackPt*/
	__asm("LDR	SP,[R2]");

	/*Restore r4,r5,r6,r7,r8,r9,r10,r11*/
	__asm("POP {R4-R11}");


	/*REstore r12*/
	__asm("POP {R12}");


	/*Restore r0,r1,r2,r3*/
	__asm("POP {R0-R3}");



	/*skip LR*/
	__asm("ADD SP,SP,#4");

	/*Create a new start location by popping LR*/
	__asm("POP {LR}");

	/*skip PSR*/
	__asm("ADD SP,SP,#4");

	/*Enable global interrupts*/
	__asm("CPSIE	I");

	/*return from the exception*/
	__asm("BX	LR");

}

void osThreadYield()
{
	/*Clear Systick current value register*/
	SysTick->VAL = 0;

	/*Trigger Systick*/
	INTCTRL = PENDSTSET;

}

void osSchedulerRoundRobin()
{
	if((++period_tick) == PERIOD)
	{
		(*task3)();

		period_tick = 0;
	}

	currentPt = currentPt->nextPt;
}

void tim2_1hz_interrupt_init(void)
{
	/*Enable clock access to tim2*/
	RCC->APB1ENR |= TIM2EN;
	/*Set timer prescaler*/
	TIM2->PSC = 1600 - 1; // 16 000 000 / 1600 = 10 000
	/*Set auto-reload value*/
	TIM2->ARR = 10000-1; // 10000 / 10000 = 1
	/*Clear Counter*/
	TIM2->CNT = 0;
	/*Enable Timer*/
	TIM2->CR1 = CR1_CEN;
	/*Enable Timer Interrupt*/
	TIM2->DIER |= DIER_UIE;
	/*Enable timer interrupt in NVIC*/
	NVIC_EnableIRQ(TIM2_IRQn);
}


void osSemaphoreInit(int32_t *semaphore, int32_t value)
{
	*semaphore = value;

}

void osSemaphoreSet(int32_t *semaphore)
{
	__disable_irq();
	*semaphore += 1;
	__enable_irq();
}

void osSemaphoreWait(int32_t *semaphore)
{
	__disable_irq();
	while(*semaphore <= 0)
	{
		__disable_irq();
		__enable_irq();
	}
	*semaphore -=1;
	__enable_irq();
}
