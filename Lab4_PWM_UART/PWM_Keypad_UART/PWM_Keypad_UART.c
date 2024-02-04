#include "stm32g0xx.h"

uint32_t counter_value = 0;
uint32_t duty_cycle = 50;
volatile int millis = 0;
volatile int previous_millis = 0;
volatile int input_keys[2] = {0, 0};

void systickinit(void);
void delay_ms(int);
void BSP_TIM1_1_init(volatile uint32_t duty_cycle, volatile uint32_t freq, volatile uint32_t pscV);
void EXTI4_15_IRQHandler(void);
void UartInit(uint32_t baud);
void PrintChar(uint8_t c);
uint8_t uart_rx(void);
void uart_tx (uint8_t data);

int main(void)
{
	// Initialize UART, SysTick, and TIM1
	UartInit(9600);
	systickinit();
	uint32_t total_steps = 50;
	duty_cycle = 50;
	uint32_t freq = 1100;
	uint32_t pscV = 9;
	counter_value = (SystemCoreClock / ((pscV + 1) * freq)) - 1;

	// Configure GPIO for keypad:
    // Column 1-4 on PB 0, 1, 4, 5 output
    // Row 1-4 on PA 4, 5, 6, 7 pulled down for input
	RCC->IOPENR |= (3U << 0); // /O port A and B clock enabled
	GPIOA->MODER &= ~(0xFF00 << 0);
	GPIOA->PUPDR |= (0xAA00 << 0);

	GPIOB->MODER &= ~(0x0F0F << 0); //0000  1111 0000 1111 = 0F0F
	GPIOB->MODER |= (0x0505 << 0); //0000 0101 0000 0101 = 0505
	GPIOB->ODR |= (0x33 << 0);

	// Configure EXTI for rising edge trigger interrupt
	EXTI->RTSR1 |= (0xF0 << 0);   // enabled the rising edge trigger interrupt on the PA 4, 5, 6, 7
	EXTI->EXTICR[2] &= (0x0 << 0);
	EXTI->IMR1 |= (0xF0 << 0);
	BSP_TIM1_1_init(duty_cycle, freq, pscV);
	NVIC_EnableIRQ(EXTI4_15_IRQn);

	int timer_count = millis;
	while(1) {
		if (millis > timer_count + 2000) {
			int digit1 = duty_cycle % 10;
			int digit2 = (duty_cycle - digit1) / 10;
			PrintChar(48 + digit2);
			PrintChar(48 + digit1);
			PrintChar('\n');
			timer_count = millis;
		}
	}

	 return 0;
}

// Interrupt handler for EXTI4_15
void EXTI4_15_IRQHandler(void) {
	volatile uint16_t temp_input;
	int row = 0;
	int column = 0;
	int keypad[4][4] = {
	        {1, 2, 3, 10},   // Assuming A=10
	        {4, 5, 6, 11},   // Assuming B=11
	        {7, 8, 9, 12},   // Assuming C=12
	        {14, 0, 15, 13}  // Assuming *=14, #=15, D=13
	    };

	volatile uint16_t input = GPIOA->IDR;
    input &= (0xF0 << 0);
    input = (input >> 4);
    for (int i = 0; i < 4; i++) {
			if (input & (1 << i)) {
				row = i + 1;  // Set the position of the set bit
			}
		}
    NVIC_DisableIRQ(EXTI4_15_IRQn); // Disable EXTI4_15 Interrupt
    NVIC_ClearPendingIRQ(EXTI4_15_IRQn);

    if (millis - previous_millis > 20) {

    	int k = 0;
    	for(int i = 0; i < 4; i++) {
    		GPIOB->ODR &= ~(0x33 << 0);
    		if (i > 1) {
    			k = 2;
    		}
    		GPIOB->ODR |= (1U << i + k);

    		temp_input = GPIOA->IDR;
    		temp_input &= (0xF0 << 0);
    		temp_input = (temp_input >> 4);
    		if (input == temp_input) {
    			column = i + 1;
    			break;
    		}

    	}

 	    int input_key = keypad[row-1][column-1];
 	    if (input_key < 10) {
 	    	input_keys[0] = input_keys[1];
 	    	input_keys[1] = input_key;
 	    }

 	    if (input_key == 15){
 	    	duty_cycle = (input_keys[0] * 10 + input_keys[1]);
 	    	TIM1->CCR1 = (duty_cycle * counter_value / 100);
 	    	PrintChar(input_keys[0] + 48);
 	    	PrintChar(input_keys[1] + 48);
 	    	PrintChar('\n');
 	    }

    	GPIOB->ODR |= (0x33 << 0);
    	previous_millis = millis;
    }
    EXTI->RPR1 |= (1U << row + 3);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void BSP_TIM1_1_init(volatile uint32_t duty_cycle, volatile uint32_t freq, volatile uint32_t pscV){
	// Enable GPIOA clock
	RCC->IOPENR |= (1U << 0);
	RCC->APBENR2 |= (1U << 11);

	// Setup PA8 as TIM1_CH1 output
	GPIOA->MODER &= ~(1U << 2*8);
	GPIOA->AFR[1] |= (1U << 1);

	// TIM1 configuration
	TIM1->CR1 = 0;
	TIM1->CCMR1 |= (3U<<5);
	TIM1->CCMR1 |= (1U<<3);
	TIM1->CR1 |= (1U << 7); // Auto-reload preload enabled
	TIM1->CNT = 0;
	TIM1->CR1 |= (1U << 4);
	TIM1->CCER |= (1U << 0);
	TIM1->BDTR |= (1U << 15);

	// Set counter value, prescaler, and auto-reload value
	uint32_t counter_value = (SystemCoreClock / ((pscV + 1) * freq)) - 1;
	TIM1->CCR1 = duty_cycle * counter_value / 100;
	TIM1->PSC = pscV; // Prescaler value set
	TIM1->ARR = counter_value; // Auto-reload value set

	//TIM1->DIER |= (1U); //  Update interrupt enabled
	TIM1->CR1 |= (1U); //  Counter enabled
//	NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
}

void systickinit(void)
       {
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
	}

void SysTick_Handler(void) // Function to enter when interrupt occurs
	{
	 millis++;      // Increase millis value by 1
	}

void delay_ms(int ms)  // delay function in milliseconds
	{
	  int delay = millis+ms;
	  while(millis<delay);
	}

uint8_t prev_data = 0;
void PrintChar(uint8_t c){
	USART2-> TDR = (uint16_t)c;  // Set the Transmit Data Register (TDR) with the character to be transmitted
	prev_data = c;      // Update the previous data with the current data
	while (!(USART2 -> ISR & (1U << 6)));  // Wait for the transmission to complete before proceeding
}


uint8_t uart_rx(void){
	uint8_t data = (uint8_t)USART2 -> RDR;
	if (data == 'a') {
		PrintChar('L');
			PrintChar('e');
			PrintChar('t');
			PrintChar('s');
			PrintChar(' ');
			PrintChar('G');
			PrintChar('o');
			PrintChar('!');
			PrintChar('\n');
	}
	return data;
}
void uart_tx (uint8_t data){
	if(prev_data != data)
	PrintChar(data);
}

void UartInit(uint32_t baud){
	RCC->IOPENR |= (1U << 0);		// Enable GPIOA clock
	RCC->APBENR1 |= (1U << 17);	// Enable USART2 clock

	// Configure PA2 as USART2 TX
	GPIOA-> MODER &= ~(3U << 2*2);	// Clear mode bits for PA2
	GPIOA-> MODER |=  (2U << 2*2);	// Set PA2 as Alternate Function mode

	GPIOA-> AFR[0] &= ~(0xFU << 4*2);  // Clear alternate function bits for PA2
	GPIOA-> AFR[0] |=  (1U << 4*2);	  // Set AF1 (USART2) for PA2

	// Configure PA3 as USART2 RX
	GPIOA-> MODER &= ~(3U << 2*3);	// Clear mode bits for PA3
	GPIOA-> MODER |=  (2U << 2*3);	// Set PA3 as Alternate Function mode

	GPIOA-> AFR[0] &= ~(0xFU << 4*3);  // Clear alternate function bits for PA3
	GPIOA-> AFR[0] |=  (1U << 4*3);	  // Set AF1 for PA3

	// USART2 Configuration
	USART2 -> CR1 = 0;			// Clear USART2 control register 1
	USART2 -> CR1 |= (1U << 3);	// Enable transmission (TE)
	USART2 -> CR1 |= (1U << 2);	// Enable reception (RE)
	USART2 -> CR1 |= (1U << 5);	// Enable USART2 for bit 5

	// Set Baud Rate
	USART2 -> BRR = (SystemCoreClock / baud);
	// Enable USART2 for bit 0
	USART2 -> CR1 |= (1U << 0);
}

