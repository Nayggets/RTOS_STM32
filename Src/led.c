#include "led.h"

#define GPIOAEN (1U<<0)
#define LED_PIN (1U<<5)
void led_init()
{
	/*Enable clock access to led port (Port A)*/

	//Initial state =      0b 0000 0000 0000 0000 1100 0000 0000 0000
	//set bit0 = (1u<<0) = 0b 0000 0000 0000 0000 0000 0000 0001
	//final state = Initial State Or set bit0 = 0b 0000 0000 0000 0000 1100 0000 0000 0001
	RCC->AHB1ENR |= GPIOAEN;
	/*Set led pin as output pin*/
	GPIOA->MODER |= (1U<<10);
	GPIOA->MODER &= ~(1U<<11);
}

void led_on()
{
	GPIOA->ODR |= LED_PIN;
}

void led_off()
{
	GPIOA->ODR &= ~LED_PIN;
	/*Set led pin LOW (PA5)*/
}
