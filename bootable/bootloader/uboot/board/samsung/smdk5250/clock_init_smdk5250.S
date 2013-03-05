/*
 * (C) Copyright 2011 Samsung Electronics Co. Ltd
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <config.h>
#include <asm/arch/cpu.h>

#include "smdk5250_val.h"

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */

	.globl system_clock_init
system_clock_init:
	push	{lr}

	ldr	r0, =ELFIN_CLOCK_BASE	@0x1001_0000

@ CMU_CPU MUX / DIV
	ldr	r1, =0x0
	ldr	r2, =CLK_SRC_CPU_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =0x0
	ldr	r2, =CLK_SRC_CORE1_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =0x01100000
	ldr	r2, =CLK_SRC_TOP2_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =0x0
	ldr	r2, =CLK_SRC_CDREX_OFFSET
	str	r1, [r0, r2]

/*
 *	Workaround Code for JPEG
 */
	ldr	r2, =CLK_DIV_GEN_OFFSET
	ldr	r1, [r0, r2]
	orr	r1, r1, #(0x7 << 4)
	str	r1, [r0, r2]

@ Set PLL locktime
	ldr	r1, =APLL_LOCK_VAL
	ldr	r2, =APLL_LOCK_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =MPLL_LOCK_VAL
	ldr	r2, =MPLL_LOCK_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =BPLL_LOCK_VAL
	ldr	r2, =BPLL_LOCK_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =CPLL_LOCK_VAL
	ldr	r2, =CPLL_LOCK_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =EPLL_LOCK_VAL
	ldr	r2, =EPLL_LOCK_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =VPLL_LOCK_VAL
	ldr	r2, =VPLL_LOCK_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
1:	subs	r1, r1, #1
	bne	1b

@ ARM_CLK
	ldr	r1, =CLK_DIV_CPU0_VAL		@0x01137710
	ldr	r2, =CLK_DIV_CPU0_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =CLK_DIV_CPU1_VAL		@0x30
	ldr	r2, =CLK_DIV_CPU1_OFFSET
	str	r1, [r0, r2]

@ Set APLL
	ldr	r1, =APLL_CON1_VAL
	ldr	r2, =APLL_CON1_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =APLL_CON0_VAL
	ldr	r2, =APLL_CON0_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
2:	subs	r1, r1, #1
	bne	2b

	ldr	r1, =CLK_SRC_CORE0_VAL		@0x00060000
	ldr	r2, =CLK_SRC_CORE0_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
3:	subs	r1, r1, #1
	bne	3b

@ CMU_CORE MUX / DIV
	ldr	r1, =CLK_DIV_CORE0_VAL		@0x00120000
	ldr	r2, =CLK_DIV_CORE0_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =CLK_DIV_CORE1_VAL		@0x07070700
	ldr	r2, =CLK_DIV_CORE1_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
4:	subs	r1, r1, #1
	bne	4b

@ CMU_CDREX MUX / DIV
	ldr	r1, =0x71771111
	ldr	r2, =CLK_DIV_CDREX_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =0x4
	ldr	r2, =CLK_DIV_CDREX2_OFFSET
	str	r1, [r0, r2]

@ Set MPLL
	ldr	r1, =MPLL_CON1_VAL
	ldr	r2, =MPLL_CON1_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =MPLL_CON0_VAL
	ldr	r2, =MPLL_CON0_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
5:	subs	r1, r1, #1
	bne	5b

@ Set BPLL
	ldr	r1, =BPLL_CON1_VAL	@0x00203800
	ldr	r2, =BPLL_CON1_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =BPLL_CON0_VAL	@0x82150C01
	ldr	r2, =BPLL_CON0_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
6:	subs	r1, r1, #1
	bne	6b

@ Set CPLL
	ldr	r1, =CPLL_CON1_VAL	@0x00203800
	ldr	r2, =CPLL_CON1_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =CPLL_CON0_VAL	@0x80960400
	ldr	r2, =CPLL_CON0_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
7:	subs	r1, r1, #1
	bne	7b

@ Set EPLL
	ldr     r1, =EPLL_CON2_VAL	@0x00000080
	ldr     r2, =EPLL_CON2_OFFSET
	str     r1, [r0, r2]
	ldr     r1, =EPLL_CON1_VAL	@0x0
	ldr     r2, =EPLL_CON1_OFFSET
	str     r1, [r0, r2]
	ldr     r1, =EPLL_CON0_VAL	@0x80600303
	ldr     r2, =EPLL_CON0_OFFSET
	str     r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
8:	subs	r1, r1, #1
	bne	8b

@ Set VPLL
	ldr     r1, =VPLL_CON2_VAL	@0x00000080
	ldr     r2, =VPLL_CON2_OFFSET
	str     r1, [r0, r2]
	ldr     r1, =VPLL_CON1_VAL	@0x0
	ldr     r2, =VPLL_CON1_OFFSET
	str     r1, [r0, r2]
	ldr     r1, =VPLL_CON0_VAL	@0x80960302
	ldr     r2, =VPLL_CON0_OFFSET
	str     r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
9:	subs	r1, r1, #1
	bne	9b

@ CMU_ACP DIV
	ldr	r1, =CLK_DIV_ACP_VAL	@0x12
	ldr	r2, =CLK_DIV_ACP_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
10:	subs	r1, r1, #1
	bne	10b

@ CMU_ISP DIV
	ldr	r1, =CLK_DIV_ISP0_VAL	@0x31
	ldr	r2, =CLK_DIV_ISP0_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =CLK_DIV_ISP1_VAL	@0x00
	ldr	r2, =CLK_DIV_ISP1_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =CLK_DIV_ISP2_VAL	@0x1
	ldr	r2, =CLK_DIV_ISP2_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
11:	subs	r1, r1, #1
	bne	11b

@ CMU_TOP SRC
	ldr	r1, =CLK_SRC_TOP0_VAL	@0x02008000
	ldr	r2, =CLK_SRC_TOP0_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =CLK_SRC_TOP1_VAL	@0x0
	ldr	r2, =CLK_SRC_TOP1_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =0x01100000
	ldr	r2, =CLK_SRC_TOP2_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =CLK_SRC_TOP3_VAL	@0x01111550
	ldr	r2, =CLK_SRC_TOP3_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
12:	subs	r1, r1, #1
	bne	12b

@ CMU_TOP MUX / DIV
	ldr	r1, =CLK_DIV_TOP0_VAL	@0x03223515
	ldr	r2, =CLK_DIV_TOP0_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =CLK_DIV_TOP1_VAL	@0x31110700
	ldr	r2, =CLK_DIV_TOP1_OFFSET
	str	r1, [r0, r2]


@ CMU_LEX SRC / DIV
	ldr	r1, =CLK_SRC_LEX_VAL	@0x0
	ldr	r2, =CLK_SRC_LEX_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =CLK_DIV_LEX_VAL	@0x10
	ldr	r2, =CLK_DIV_LEX_OFFSET
	str	r1, [r0, r2]

@ CMU_R0X DIV
	ldr	r1, =CLK_DIV_R0X_VAL	@0x10
	ldr	r2, =CLK_DIV_R0X_OFFSET
	str	r1, [r0, r2]

@ CMU_R1X DIV
	ldr	r1, =CLK_DIV_R1X_VAL	@0x10
	ldr	r2, =CLK_DIV_R1X_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
13:	subs	r1, r1, #1
	bne	13b

@CMU_CPU SRC
	ldr	r1, =CLK_SRC_CPU_VAL		@0x1
	ldr	r2, =CLK_SRC_CPU_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =0x01111100
	ldr	r2, =CLK_SRC_TOP2_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =CLK_SRC_CORE1_VAL		@0x100
	ldr	r2, =CLK_SRC_CORE1_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =0x66666
	ldr	r2, =CLK_SRC_FSYS_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =0x0BB00000
	ldr	r2, =CLK_DIV_FSYS0_OFFSET
	str	r1, [r0, r2]

	ldr	r1, =CLK_SRC_CDREX_VAL		@0x1
	ldr	r2, =CLK_SRC_CDREX_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
14:	subs	r1, r1, #1
	bne	14b

	pop	{pc}
