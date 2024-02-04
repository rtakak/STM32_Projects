
#include "stm32g0xx.h"

volatile uint32_t millis = 0;
uint32_t adc_input = 0;
uint32_t perc;

uint32_t duty_cycle = 90;
uint32_t freq = 1100;
uint32_t pscV = 9;
uint32_t counter_value;

void systickinit(void);
void delay_ms(uint32_t);
void BSP_TIM1_1_init(volatile uint32_t duty_cycle, volatile uint32_t freq, volatile uint32_t pscV);
void ADC_COMP_IRQHandler(void);
int Set_Duty_Cycle(uint32_t duty_cycle);
int main(void)
{
	counter_value = (SystemCoreClock / ((pscV + 1) * freq)) - 1;
	systickinit();    // Initialize the SysTick timer for millisecond counting

	/*
	 	1. Enable ADC Clock
		2. Enable ADC Voltage Regulator, wait for at least 20us
		3. Enable Calibration, wait until completion
		4. Enable end of calibration or sequence interrupts
		5. Configure number of bits to sample (6, 8, 10, 12)
		6. Configure single/continuous mode
		7. Select sampling time from SMPR register
		8. Enable channels (for the ANx pins)
		9. Enable ADC and wait until it is ready
		10. Start conversion Silebiliriz burayı
	 */
	GPIOA->MODER &= ~(3U << 0);
	RCC->APBENR2 |= (1U << 20); // Enable ADC Clock
	ADC1->CR |= (1U << 28); // Enable ADC Voltage Regulator
	for(volatile int i = 0; i < 320; i++); // wait for at least 20us

	ADC1->CR |= (1U << 31); // Enable Calibration, wait until completion
	while((ADC1->CR & (1U << 31)) != 0);

	//uint8_t calibration_factor = ADC1->DR; // First 6 bit

	ADC1->IER |= (1U << 2); // Enable end of conversion interrupt

	ADC1->CFGR1 |= (1U << 13); // Set continuous conversion mode

	ADC1->SMPR |= (3U << 1); // Set sampling rate to 79.5 ADC clock cycles
	ADC1->CHSELR |= (1U << 0); //  Input Channel 0 is selected for conversion

	ADC1->CR |= (1U << 0); // Enable ADC
	while((ADC1->ISR & (1U << 0)) == 0); // Wait until it is ready
	NVIC_EnableIRQ(ADC1_IRQn);
	ADC1->CR |= (1U << 2);  // Start ADC conversion


	BSP_TIM1_1_init(duty_cycle, freq, pscV);  // Initialize TIM1 for PWM generation

	while(1){
		// User Code
		// Program is non blockage as it runs on interrupts
	}
	return 0;
}

void ADC_COMP_IRQHandler() { // Called by End of Conversion Interrupt of ADC

	uint32_t inp = (ADC1->DR & (0xFFFF)); // Max to Min is 4095 - 95
	if((inp > adc_input + 20) | (inp + 20 < adc_input)) { // Checks if input is slightly different compared to previous one
		adc_input = inp; // If so applies it as output

		if (adc_input < 50) { // Dead zone for 0 value
			perc = 0;
		}
		else if (adc_input > 3995) {
			perc = 100;
		}
		else {
			perc = (adc_input * 0.025 + 0.5); // Calculation to map the ADC values to 100-0 range
		}
		Set_Duty_Cycle(perc); // Sets the current duty cycle
	}
}


// Function to initialize TIM1 for PWM generation
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
	 SysTick->LOAD = 18499;        // Set the Reload Register for 1mS
	 SysTick->VAL = 0;             // Clear the Current Value register
	 SysTick->CTRL |= 0x00000007;  // Enable SysTick, Enable SysTick Exceptions, Use CPU Clock

	 NVIC_SetPriority(SysTick_IRQn, 3);     // Set the interrupt priority to least urgency
	 NVIC_EnableIRQ(SysTick_IRQn);
	 __enable_irq();
	}

void SysTick_Handler(void) // Function to enter when interrupt occurs
	{
	 millis++;      // Increase millis value by 1
	}

void delay_ms(uint32_t ms)  // delay function in milliseconds
	{
	uint32_t delay = millis+ms;
	  while(millis<delay);
	}

int Set_Duty_Cycle(uint32_t duty_cycle) { // Sets Duty Cycle
	if (100 >= duty_cycle) {
		TIM1->CCR1 = duty_cycle * counter_value / 100;
		return 1;
	}
	return 0;
}
