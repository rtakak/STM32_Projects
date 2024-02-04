/*
 * problem3.c
 * 4D7S display / Stm32g0 Nucleo
 * D1 -> PA4
 * D2 -> PA5
 * D3 -> PA6
 * D4 -> PA7
 *
 * A -> PB3
 * B -> PB4
 * C -> PB5
 * D -> PB6
 * E -> PB7
 * F -> PB8
 * G -> PB9
 * Button -> PA8
 */

#include "stm32g0xx.h"

#define LEDDELAY    1000

// Define digits. we use common anode display so 0 is enabled value.
const char digit0 = 0b1000000;  // Display shows 0
const char digit1 = 0b1111001;  // Display shows 1
const char digit2 = 0b0100100;  // Display shows 2
const char digit3 = 0b0110000;  // Display shows 3
const char digit4 = 0b0011001;  // Display shows 4
const char digit5 = 0b0010010;  // Display shows 5
const char digit6 = 0b0000010;  // Display shows 6
const char digit7 = 0b1111000;  // Display shows 7
const char digit8 = 0b0000000;  // Display shows 8
const char digit9 = 0b0010000;  // Display shows 9

//Make an array
const uint32_t digits[] = {digit0, digit1,digit2, digit3, digit4, digit5, digit6, digit7, digit8, digit9};

uint32_t button_input;
volatile int millis = 0;
volatile int number_digit[] = {0, 0, 0, 0};
int counter = 0;
int prev_millis = 0;
int startup_flag = 0;
int n_digits_display = 4;
int currentDigit = 0;

void systickinit(void);
void delay_ms(int);
void display_digit(int);
void EXTI4_15_IRQHandler(void);
int main(void);

int main(void) {
	systickinit();  //Calls function

	while(1) {

	}
}

void display_digit(int digit){
	uint32_t output = digits[digit];  //taking digit
	output = (output << 3);
	GPIOB->ODR = 0;                   //reset GPIOB
	GPIOB->ODR |= output;             //equals to output
}

void systickinit(void)
       {


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
	GPIOA->PUPDR |= (1U << (1 + 8 * 2));

	/* Setup PA9 as input for button. */
	GPIOA->MODER &= ~(3U << 2*9);
	GPIOA->PUPDR |= (1U << (1 + 9 * 2));

	// Configure EXTI for rising edge trigger interrupt
	EXTI->EXTICR[2] &= (0U << 0);
	EXTI->RTSR1 |= (3U << 8);   // enabled the rising edge trigger interrupt on the PA 8
	EXTI->IMR1 |= (3U << 8);


	GPIOB->ODR = 0;
	__disable_irq();

	 // Configure CPU clock: Internal OSC8M divided by 8.

	 // Configure SysTick to trigger every second using the CPU Clock
	 SysTick->CTRL = 0;            // Disable the SysTick Module
	 SysTick->LOAD = 6500;    // Set the Reload Register for 1mS
	 SysTick->VAL = 0;             // Clear the Current Value register
	 SysTick->CTRL |= 0x00000007;   // Enable SysTick, Enable SysTick Exceptions, Use CPU Clock

	 NVIC_SetPriority(SysTick_IRQn, 3);     // Set the interrupt priority to least urgency
	 NVIC_EnableIRQ(SysTick_IRQn);
	 NVIC_EnableIRQ(EXTI4_15_IRQn);
	 NVIC_SetPriority(EXTI4_15_IRQn, 2);
	 __enable_irq();
	}

// Interrupt handler for EXTI4_15
void EXTI4_15_IRQHandler(void) {
	uint32_t inp = GPIOA->IDR;
	if ((inp & (1U << 8)) && (millis > prev_millis + 50)) {
		counter = 0;
		number_digit[0] = 0;
		number_digit[1] = 0;
		number_digit[2] = 0;
		number_digit[3] = 0;
		n_digits_display = 1;

		prev_millis = millis;
		EXTI->RPR1 |= (3U << 8);
		return;
	}

	if ((inp & (1U << 9)) && (millis > prev_millis + 150)) {
		int num = ++counter;
		int i = 0;
		 while(num != 0){
				number_digit[i++] = (num % 10);
				num = num / 10;
		}
		 n_digits_display = i;
		 prev_millis = millis;
		}


	EXTI->RPR1 |= (3U << 8);
}

void SysTick_Handler(void) // Function to enter when interrupt occurs
	{
	 millis++;      // Increase millis value by 1

	 GPIOA->ODR = 0;
	 display_digit(number_digit[currentDigit]);
	 switch (currentDigit) {
		case 0:
			GPIOA->ODR |= (1U << 7);
			break;
		case 1:
			GPIOA->ODR |= (1U << 6); // Setting the 5th bit
			break;
		case 2:
			GPIOA->ODR |= (1U << 5); // Setting the 6th bit
			break;
		case 3:
		{
			GPIOA->ODR |= (1U << 4); // Setting the 7th bit
			currentDigit = -1;
			break;
		}

	    }
	 currentDigit++;
	     if (1 + currentDigit > n_digits_display) {
	    	 currentDigit = 0; // Reset to the first case if limit is reached
	     }
	}

void delay_ms(int ms)  // delay function in milliseconds
	{
	  int delay = millis+ms;
	  while(millis<delay);
	}
