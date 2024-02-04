
#include "stm32g0xx.h"
#define   ResetIWDG   (IWDG->KR=0xAAAA)
#define LEDDELAY    1000

const char digit0 = 0b1000000;
const char digit1 = 0b1111001;
const char digit2 = 0b0100100;
const char digit3 = 0b0110000;
const char digit4 = 0b0011001;
const char digit5 = 0b0010010;
const char digit6 = 0b0000010;
const char digit7 = 0b1111000;
const char digit8 = 0b0000000;
const char digit9 = 0b0010000;
const uint32_t digits[] = {digit0, digit1,digit2, digit3, digit4, digit5, digit6, digit7, digit8, digit9};

uint32_t button_input;
volatile int millis = 0;
volatile int number_digit[] = {0, 0, 0, 0};
int counter;
int startup_flag = 0;

void systickinit(void);
void delay_ms(int);
void display_digit(int);
void display_counter(void);
int main(void);

void InitIWDG(void)
{
	IWDG->KR = 0x0000CCCC;    // Enable the IWDG
	IWDG->KR = 0x00005555;    // Enable register access

	IWDG->PR = 0x00000000 ;   // reset IWDG_PR
	IWDG->PR = (5U) ;         // 110 for divide 256

	IWDG->RLR = 0x00000FFF;   // Counter goes from 0xFFF to zero.

	IWDG->KR  = 0x00000000;   // Disable KR
	IWDG->KR  = 0x0000CCCC;   // starts the watchdog
}

int main(void) {
	systickinit();
    InitIWDG();
}

void display_digit(int digit){
	uint32_t output = digits[digit];
	output = (output << 3);
	GPIOB->ODR = 0;
	GPIOB->ODR |= output;
}

void systickinit(void)
       {
    /* Enable GPIOC clock */
	RCC->IOPENR |= (1U << 2);

	/* Setup PC6 as output */
	GPIOC->MODER &= ~(3U << 2*6);
	GPIOC->MODER |= (1U << 2*6);

	/* Enable GPIOA and B clock */
	RCC->IOPENR |= (1U << 0);
	RCC->IOPENR |= (1U << 1);

	/* Setup PB 3,4,5,6,7,8,9 as output */
	GPIOB->MODER &= ~(0xFFFC0 << 0); // 1111 1111 1111 1100 0000 = FFFC0
	GPIOB->MODER |= (0x55540 << 0);  // 0101 0101 0101 0100 0000 = 55540

	/* Setup PA 4,5,6,7 as output */
	GPIOA->MODER &= ~(0xFF00 << 0); // 1111 1111 0000 0000 = FF00
	GPIOA->MODER |= (0x5500 << 0); // 0101 0101 0000 0000 = 5500

	/* Setup PA8 as input for button. */
	GPIOA->MODER &= ~(3U << 2*8);

	GPIOB->ODR = 0;
	__disable_irq();

	 // Configure CPU clock: Internal OSC8M divided by 8.

	 // Configure SysTick to trigger every second using the CPU Clock
	 SysTick->CTRL = 0;            // Disable the SysTick Module
	 SysTick->LOAD = 12499;    // Set the Reload Register for 1mS
	 SysTick->VAL = 0;             // Clear the Current Value register
	 SysTick->CTRL |= 0x00000007;   // Enable SysTick, Enable SysTick Exceptions, Use CPU Clock

	 NVIC_SetPriority(SysTick_IRQn, 3);     // Set the interrupt priority to least urgency
	 NVIC_EnableIRQ(SysTick_IRQn);
	 __enable_irq();
	 display_counter();
	}

void SysTick_Handler(void) // Function to enter when interrupt occurs
	{
	ResetIWDG;
	 millis++;      // Increase millis value by 1
	 if (counter >= 9990)
	 	 {
	 		 startup_flag = 0;
	 		 number_digit[0] = 0;
	 		 number_digit[1] = 0;
	 		 number_digit[2] = 0;
	 		 number_digit[3] = 0;
	 		/* Turn on LED */
	 		GPIOC->ODR |= (1U << 6);
	 	 }
	 volatile uint32_t old_button_state = button_input;
	 button_input = GPIOA->IDR;
	 	 button_input &= (1U << 8);
	 	 if(!button_input && button_input != old_button_state){
	 		 if (!startup_flag){
	 			 counter = 0;
	 			 startup_flag = 1;
	 			GPIOC->ODR = 0;
	 		 }
	 		 else {
	 			 startup_flag = 0;
	 			 number_digit[0] = 0;
	 			 number_digit[1] = 0;
	 			 number_digit[2] = 0;
	 			 number_digit[3] = 0;

	 		 }
	 	 }

	 if (startup_flag){


	 int num = ++counter;
	 int i = 0;
	 while(num != 0){
			number_digit[i++] = (num % 10);
			num = num / 10;
		}
	 }
	}

void delay_ms(int ms)  // delay function in milliseconds
	{
	  int delay = millis+ms;
	  while(millis<delay);
	}

void display_counter() {   // Counter for display's 4 digits.
	while (1){
		for (int n = 0; n<4; n++) {
			GPIOA->ODR = 0;
			GPIOA->ODR |= (1U << (4 + n));
			display_digit(number_digit[n]);
			delay_ms(1);
		}
	}
}
