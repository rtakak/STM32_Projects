
.syntax unified
.cpu cortex-m0plus
.fpu softvfp
.thumb

/* make linker see this */
.global Reset_Handler

/* get these from linker script */
.word _sdata
.word _edata
.word _sbss
.word _ebss


/* define peripheral addresses from RM0444 */
.equ RCC_BASE,         (0x40021000)          // RCC base address
.equ RCC_IOPENR,       (RCC_BASE   + (0x34)) // RCC IOPENR register offset

.equ GPIOA_BASE,       (0x50000000)          // GPIOC base address
.equ GPIOA_MODER,      (GPIOA_BASE + (0x00)) // GPIOC MODER register offset
.equ GPIOA_ODR,        (GPIOA_BASE + (0x14)) // GPIOC ODR offset


.equ GPIOB_BASE,       (0x50000400)          // GPIOB base address
.equ GPIOB_MODER,      (GPIOB_BASE + (0x00)) // GPIOB MODER register offset
.equ GPIOB_ODR,        (GPIOB_BASE + (0x14)) // GPIOB ODR offset
.equ GPIOB_IDR,        (GPIOB_BASE + (0x10)) // GPIOB IDR offset

/* vector table, +1 thumb mode */
.section .vectors
vector_table:
	.word _estack             /*     Stack pointer */
	.word Reset_Handler +1    /*     Reset handler */
	.word Default_Handler +1  /*       NMI handler */
	.word Default_Handler +1  /* HardFault handler */
	/* add rest of them here if needed */


/* reset handler */
.section .text
Reset_Handler:
	/* set stack pointer */
	ldr r0, =_estack
	mov sp, r0

	/* initialize data and bss 
	 * not necessary for rom only code 
	 * */
	bl init_data
	/* call main */
	bl main
	/* trap if returned */
	b .


/* initialize data and bss sections */
.section .text
init_data:

	/* copy rom to ram */
	ldr r0, =_sdata
	ldr r1, =_edata
	ldr r2, =_sidata
	movs r3, #0
	b LoopCopyDataInit

	CopyDataInit:
		ldr r4, [r2, r3]
		str r4, [r0, r3]
		adds r3, r3, #4

	LoopCopyDataInit:
		adds r4, r0, r3
		cmp r4, r1
		bcc CopyDataInit

	/* zero bss */
	ldr r2, =_sbss
	ldr r4, =_ebss
	movs r3, #0
	b LoopFillZerobss

	FillZerobss:
		str  r3, [r2]
		adds r2, r2, #4

	LoopFillZerobss:
		cmp r2, r4
		bcc FillZerobss

	bx lr


/* default handler */
.section .text
Default_Handler:
	b Default_Handler


/* main function */
.section .text
main:
	/* enable GPIOA and B clock, bit 1 and 2 */
	ldr r6, =RCC_IOPENR
	ldr r5, [r6]
	/* movs expects imm8, so this should be fine */
	movs r4, 0x3 // 0011 so bit 1 and bit 0 is enable now
	orrs r5, r5, r4  // We use or for set, other pin doesn't change and bit 1 and 0 is 1
	str r5, [r6]

	/* setup PB2,3,4,5 (in MODER) */
	ldr r6, =GPIOB_MODER
	ldr r5, [r6]
	/* Set Moder for PB2,3,4,5 as general purpose output*/
	ldr r4, =0xFF0   // 1111 1111 0000
	bics r5, r5, r4
	ldr r4, =0x550  // 0101 0101 0000
	orrs r5, r5, r4
	str r5, [r6]

	/* setup PA6,7,8,9 ( MODER) */
	ldr r6, =GPIOA_MODER
	ldr r5, [r6]
	/* Set Moder for PA6,7,8,9 as general purpose output*/
	ldr r4, =0xFF000   // 1111 1111 0000 0000 0000
	bics r5, r5, r4
	ldr r4, =0x55000  // 0101 0101 0000 0000 0000
	orrs r5, r5, r4
	str r5, [r6]

	/* setup PB8 (bits 16-17 in MODER) */
	ldr r6, =GPIOB_MODER
	ldr r5, [r6]
	/* Set Moder for PB8 as general purpose input*/
	movs r4, 0x3   // 0011
	lsls r4, r4, #16
	bics r5, r5, r4
	ldr r4, =0x00000  // (00=input mode)
	orrs r5, r5, r4
	str r5, [r6]


	//init all the used gpio
	ldr r4, =0xFFF00000
	movs r1, #0

	loop:
		movs r7, #0
		movs r5, r4

		movs r0, #8
	loop_start:
		movs r6, #1
		ands r6, r5, r6
		orrs r7, r6, r7
		movs r6, #1
		rors r7, r7, r6
		lsrs r5, r5, #4
		subs r0, r0, #1
	    bne loop_start

	lsrs r7, r7, #24

	movs r6, #15
    ands r6, r7, r6
    lsrs r7, r7, #4


	/* LED 1-4 PB 5-2 and LED 5-8 PA 9-6 */
	ldr r0, =GPIOA_ODR
	ldr r5, [r0]
	lsls r6, r6, #6
	str r6, [r0]

	/* turn on led connected to PB2,3,4,5 in ODR */
	ldr r0, =GPIOB_ODR
	ldr r5, [r0]
	lsls r7, r7, #2
	str r7, [r0]


	ldr r7, =#0x516155 // Load the delay count into register r5
	lsrs r7, r7, #3
    delay_loop:
        subs r7, r7, #1 // Decrement r5
        bne delay_loop  // Branch if not equal to zero (R5 != 0)



	//write to ODR
	ldr r0, =GPIOB_IDR   // Load the address of the button status
	ldr r0, [r0]                  // Load the button status into r1
	lsrs r0, r0, #8
	eors r1, r0, r1  //flip the value
	cmp r1, #0                    // Compare the button status with 1 (pressed)
	beq right            // If button is pressed, branch to button_pressed

	left:
		movs r6, #28
	    rors r4, r4, r6
	    b end_if

	right:
		movs r6, #4
	    rors r4, r4, r6
	    b end_if                      // Skip the other part


	end_if:
	    b loop



	/* for(;;); */
	b .

	/* this should never get executed */
	nop

