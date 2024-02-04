# STM 32 Projects on NUCLEO-G031K8

## Overview
This repository consists of a series of projects developed as part of a Microprocessors course using the STM32 NUCLEO-G031K8. These projects illustrate our ability to implement and integrate various hardware and software concepts, such as ADC (Analog to Digital Conversion), PWM (Pulse Width Modulation), UART communication and timer functionalities. Each project was designed not only as an academic exercise but also as a stepping stone towards real-world embedded systems applications.


## Lab 5 ADC
>**Knock Counter**
- A knock counter application. There is a microphone and seven segment display in the circuit. Incoming knock sounds are detected by the microphone and the number of knocks is displayed on the screen.

>**Light Dimmer**
- A light dimmer with a potentiometer. An application where the light brightness changes according to the changing resistor value.The analog value from the potentiometer is read with the ADC pin. PWM is used to drive the LED to change the brightness. A 0 duty cycle will turn the LEDs off and a 100% duty cycle will turn them on completely.

## Lab 4 PWM, UART and Keypad
>**PWM_Keypad**
- Application of driving an LED at different brightness with PWM signal using keypad. Keypad is used as to get input for the brightness percantage. The value entered from the keypad is also sent via UART. UART receiver displays the transmitted value.

## Lab 3 Timers and Sevent Segment Display
>**SSDwithWatchdog**
- A count up timer with a seven segment display. Once the external button is pressed a interrupt is triggered. It counts up to the max value (9999) starting from 0. There is a watchdog timer to prevent from application getting stuck. 

## Lab 2 Led Pattern
>**KnightRider**
- This is a LED knight rider implementation. The pattern light 3 LEDs at the same time. These 3 LEDs shift right until the LED8 lights up and, then these 3 LEDs shift left until the LED1 lights up. When LED1 lit, these 3LEDs shift right until the LED8 lit and, after these 3 LEDs shift left until the LED1 lights up. This process continue indefinitely.

>**ShiftPattern**
- This is a LED shift pattern implementation. 8 external LEDs and 1 button are connected. The pattern light 3 LEDs at the same time.  These 3 LEDs shift right or left indefinitely.  The button toggle the shift direction when pressed.

#### These projects and repository were made by @Furkannkeskin , @EsraKrmn and @rtakak .