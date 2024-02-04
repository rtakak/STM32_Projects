#include "stm32g0xx.h"
#define   ResetIWDG   (IWDG->KR=0xAAAA)
#define  LEDDELAY    16000   // 10 ms

void delay(volatile uint32_t s);
void InitIWDG(void)
{
	IWDG->KR = 0x0000CCCC;    // Enable the IWDG
	IWDG->KR = 0x00005555;    // Enable register access

	IWDG->PR = 0x00000000 ;   // reset IWDG_PR
	IWDG->PR = (2U) ;         // 110 for divide 16

	IWDG->RLR = 0x00000FFF;   // Counter goes from 0xFFF to zero.

	IWDG->KR  = 0x00000000;   // Disable KR
	IWDG->KR  = 0x0000CCCC;   // starts the watchdog
}

int main(void)
{
	  InitIWDG();

	    /* Enable GPIOC clock */
	    RCC->IOPENR |= (1U << 2);

	    /* Setup PC6 as output */
	    GPIOC->MODER &= ~(3U << 2*6);
	    GPIOC->MODER |= (1U << 2*6);

	    /* Turn on LED */
	    GPIOC->ODR |= (1U << 6);

	    while(1) {
	        delay(LEDDELAY);
	        /* Toggle LED */
	        GPIOC->ODR ^= (1U << 6);
	    }
	    return 0;
}

void delay(volatile uint32_t s) {
    for(; s>0; s--);
}


