/*
 * Copyright (C) 2010 Samsung Electronics Co. Ltd
 *
 * Many parts of this program were copied from the work of Windriver.
 * Major modifications are as follows:
 * - Adding default partition table.
 * - Supporting OneNAND device.
 * - Supporting SDMMC device.
 * - Adding new command, sdfuse.
 * - Removing Lock scheme.
 * - Removing direct flash operations because they are implemented at others.
 * - Fixing several bugs
 * This program is under the same License with the their work.
 *
 * This is their Copyright:
 *
 * (C) Copyright 2008 - 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Part of the rx_handler were copied from the Android project. 
 * Specifically rx command parsing in the  usb_rx_data_complete 
 * function of the file bootable/bootloader/legacy/usbloader/usbloader.c
 *
 * The logical naming of flash comes from the Android project
 * Thse structures and functions that look like fastboot_flash_* 
 * They come from bootable/bootloader/legacy/libboot/flash.c
 *
 * This is their Copyright:
 * 
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <asm/byteorder.h>
#include <common.h>
#include <command.h>
//#include <asm/io.h>
#include <asm/arch/cpu.h>

#define ENABLE_HANDLE_RSP     0
#define FIFO_ENABLE 0
#define MODEM_DEBUG_INFO 1

/*platform related gpio and uart base adresses*/
#define EXYNOS4412_GPIO_BASE	0x11000000
#define EXYNOS4412_UART_BASE	0x13800000

#define GPIO_BASE			EXYNOS4412_GPIO_BASE
#define UART_BASE			EXYNOS4412_UART_BASE
#define UART0_OFFSET		0x00000
#define UART1_OFFSET		0x10000
#define UART2_OFFSET		0x20000
#define UART3_OFFSET		0x30000

/*uart registers*/
#define PC_UART_BASE		UART_BASE + UART3_OFFSET
#define MODEM_UART_BASE	UART_BASE + UART1_OFFSET

volatile u32 *pc_uart_xmit = (volatile u32 *)(PC_UART_BASE+ UTXH_OFFSET);
volatile u32 *pc_uart_rx = (volatile u32 *)(PC_UART_BASE + URXH_OFFSET);
volatile u32 *pc_uart_stat = (volatile u32 *)(PC_UART_BASE + UTRSTAT_OFFSET);
	
volatile u32 *modem_uart_xmit = (volatile u32 *)(MODEM_UART_BASE + UTXH_OFFSET);
volatile u32 *modem_uart_rx = (volatile u32 *)(MODEM_UART_BASE + URXH_OFFSET);
volatile u32 *modem_uart_stat = (volatile u32 *)(MODEM_UART_BASE + UTRSTAT_OFFSET);

#define MASK_UART_RX_FIFO_COUNT     0xff
#define MASK_UART_XMIT_FIFO_FULL    0x10000
volatile u32 *modem_uart_fconf= (volatile u32 *)(MODEM_UART_BASE + UFCON_OFFSET);
volatile u32 *modem_uart_fstat= (volatile u32 *)(MODEM_UART_BASE + UFSTAT_OFFSET);

/*AT command*/
#define ESERIAL -1
#define RSP_GET_RETRY                   10
#define AT_BUFFER_SIZE                  256

#define AT_RSP_MACH                      1
#define AT_RSP_UNMACH                 2
#define AT_RSP_FAIL                        3

#define AT_CMD_AMT_ENABLE                             "AT\r"
#define AT_CMD_AMT_ENABLE_LEN                    (sizeof(AT_CMD_AMT_ENABLE)-1)
#define AT_CMD_AMT_ENABLE_RSP                    "\r\nOK\r\n"
#define AT_CMD_AMT_ENABLE_RSP_LEN           (sizeof(AT_CMD_AMT_ENABLE_RSP)-1)
#define AT_CMD_CR                              '\r'
#define AT_CMD_LF                              '\n'
#define AT_RESPONSE_END             0x0D

inline void sleep (int i)
{
	while (i--) {
		udelay (1000000);
	}
}

#ifndef mdelay
#define mdelay(x)	udelay(1000*x)
#endif

typedef enum GPIO_Bank	// Address Offset                                                         
{                                                                            
       /*EXYNOS4412 Part 1*/	                                     
	eGPIO_A0 = 0x400000,          
	eGPIO_A1 = 0x400020,    	                                     
	eGPIO_B  = 0x400040,    	                                     
	eGPIO_C0 = 0x400060,    	                                     
	eGPIO_C1 = 0x400080,    	                                     
	eGPIO_D0 = 0x4000A0,    	                                     
	eGPIO_D1 = 0x4000C0,    	                                     
	eGPIO_F0 = 0x400180,    	                                     
	eGPIO_F1 = 0x4001A0,    	                                     
	eGPIO_F2 = 0x4001C0,    	                                     
	eGPIO_F3 = 0x4001E0,
	/*more to be added*/
} GPIO_eBank;                                                                    

typedef enum GPIO_Bit
{
	eGPIO_0	        = 0,
	eGPIO_1	        = 1,
	eGPIO_2	        = 2,
	eGPIO_3	        = 3,
	eGPIO_4	        = 4,
	eGPIO_5	        = 5,
	eGPIO_6	        = 6,
	eGPIO_7	        = 7,
} GPIO_eBit;

typedef enum GPIO_Func
{
	GPIO_INPUT		=0,
	GPIO_OUTPUT	=1,
	GPIO_FUNC0		=2,
	GPIO_IRQ		= 15,
} GPIO_eFunc;


/* Function Name : GPIO_SetBitFunction
 * Function Desctiption : This function set each GPIO function
 * Parameters : 	Id : GPIO bank
 *			eBitPos : GPIO bit
 *			uFunction : Select the function
 */
static void GPIO_SetBitFunction(GPIO_eBank Id, 
			GPIO_eBit eBitPos, GPIO_eFunc uFunction)
{
	volatile int *pGPIOx_Reg0;
	volatile int *pGPIO_Base_Addr;
	int uMuxBit,  uOffset;
	int uConValue;

	uMuxBit = 4; // 4bit
	uOffset = Id&0xFFFFFF;  

	pGPIO_Base_Addr = (volatile int *)GPIO_BASE;
	pGPIOx_Reg0 = pGPIO_Base_Addr + uOffset/4;
	
	uConValue = *pGPIOx_Reg0;
	uConValue = (uConValue & ~(0xF<<(uMuxBit*eBitPos))) | (uFunction<<(uMuxBit*eBitPos));
	*pGPIOx_Reg0 = uConValue;

//	printf("address is %x, function is %x\n", pGPIOx_Reg0, uConValue);	
}

/* Function Name : GPIO_SetBitData
 * Function Desctiption : This function set each GPIO data bit
 * Parameters : 	Id : GPIO bank
 *			eBitPos : GPIO bit
 *			uValue : value
 */
static void GPIO_SetBitData(GPIO_eBank Id, 
			GPIO_eBit eBitPos, GPIO_eFunc uValue)
{
	volatile int *pGPIOx_DataReg;
	volatile int *pGPIO_Base_Addr;
	int  uOffset, uConRegNum;
	int uDataValue;

	uOffset = Id&0xFFFFFF; 
	uConRegNum = 1;
	
	pGPIO_Base_Addr = (volatile int *)GPIO_BASE;
	
	pGPIOx_DataReg = pGPIO_Base_Addr + (uOffset/4) + uConRegNum;
	uDataValue = *pGPIOx_DataReg;
	uDataValue = (uDataValue & ~(0x1<<eBitPos)) | (uValue<<eBitPos);
	*pGPIOx_DataReg = uDataValue;

//	printf("address is %8x, data is %8x\n", pGPIOx_DataReg, uDataValue);	

}

static int smm6260_power_on(void)
{
	int tmp = 0;

	GPIO_SetBitFunction(eGPIO_F3, eGPIO_1, GPIO_OUTPUT); //PHONE_ON
	GPIO_SetBitFunction(eGPIO_F2, eGPIO_3, GPIO_OUTPUT); //CP_PMU_RST
	GPIO_SetBitFunction(eGPIO_F2, eGPIO_2, GPIO_OUTPUT); //CP_RST

	/*assert reset single*/
	GPIO_SetBitData(eGPIO_F3, eGPIO_1, 0); //PHONE_ON  0
	GPIO_SetBitData(eGPIO_F2, eGPIO_3, 0); //CP_PMU_RST  0
	GPIO_SetBitData(eGPIO_F2, eGPIO_2, 0); //CP_RST  0

	mdelay(100);

	/*release reset single*/
	GPIO_SetBitData(eGPIO_F2, eGPIO_2, 1); //CP_RST 1
	mdelay(1);
	GPIO_SetBitData(eGPIO_F2, eGPIO_3, 1); //CP_PMU_RST 1
	mdelay(2);
	/*triger reset single*/
	GPIO_SetBitData(eGPIO_F3, eGPIO_1, 1); //PHONE_ON 1
	mdelay(100);

	return 0;
}

static int smm6260_power_off(void)
{
	return 0;
}

static int smm6260_usb_to_pc(void)
{
	GPIO_SetBitFunction(eGPIO_F0, eGPIO_6, GPIO_OUTPUT); //usb switch control
	GPIO_SetBitData(eGPIO_F0, eGPIO_6, 1); //pc-->modem
	return 0;
}

static int modem_power_off(void)
{
#ifdef CONFIG_STUTTGART
	smm6260_power_off();
#endif
	return 0;
}

static int modem_pcdownload_power_on(void)
{
	printf("modem_pcdownload_power_on\n");

#ifdef CONFIG_STUTTGART
       smm6260_usb_to_pc();
	smm6260_power_on();
#endif
 	return 0;
}

static int modem_normal_power_on(void)
{
	printf("modem_normal_power_on\n");
	
#ifdef CONFIG_STUTTGART
	smm6260_power_on();
#endif
    //sleep(7);
	printf("modem_normal_power_on done\n");
	return 0;
}

void dump_uart_regs(void)
{
	u32 val;

	printf("modem uart regs: ");
	val = readl(MODEM_UART_BASE + ULCON_OFFSET);
	printf("ULCON-%x\n", val);
	val = readl(MODEM_UART_BASE + UCON_OFFSET);
	printf("UCON-%x\n", val);
	val = readl(MODEM_UART_BASE + UFCON_OFFSET);
	printf("UFCON-%x\n", val);
	val = readl(MODEM_UART_BASE + UMCON_OFFSET);
	printf("UMCON-%x\n", val);
	val = readl(MODEM_UART_BASE + UTRSTAT_OFFSET);
	printf("UMSTAT-%x\n", val);    
	val = readl(MODEM_UART_BASE + UBRDIV_OFFSET);
	printf("UBRDIV-%x\n", val);
	val = readl(MODEM_UART_BASE + UDIVSLOT_OFFSET);
	printf("UDIVSLOT-%x\n", val);
#if FIFO_ENABLE
	val = readl(modem_uart_fconf);
	printf("modem_uart_fconf-%x\n", val);    
	val = readl(modem_uart_fstat);
	printf("modem_uart_fstat-%x\n", val); 
#endif

	val = readl(PC_UART_BASE + UTRSTAT_OFFSET);
	printf("UMSTAT-%x\n", val);      

}

static int modem_uart_init(void)
{
    	u32 tmp;

    	/* set up GPIOs to enable modem UART */
	GPIO_SetBitFunction(eGPIO_A0, eGPIO_4, GPIO_FUNC0);
	GPIO_SetBitFunction(eGPIO_A0, eGPIO_5, GPIO_FUNC0);

    	/*set up modem UART SFRs*/
	writel(0x3, MODEM_UART_BASE + ULCON_OFFSET);	
       writel(0x3c5, MODEM_UART_BASE + UCON_OFFSET);	
       writel(0x0, MODEM_UART_BASE + UFCON_OFFSET);	
    	writel(0x10, MODEM_UART_BASE + UMCON_OFFSET);

    	/*baudrate is 115200*/
    	writel(0x35, MODEM_UART_BASE + UBRDIV_OFFSET);	
    	writel(0x04, MODEM_UART_BASE + UDIVSLOT_OFFSET); 

#if FIFO_ENABLE 
    	// fifo enable, other func use 0 as default.
    	writel(0x1, modem_uart_fconf);
#endif

#if MODEM_DEBUG_INFO
    	dump_uart_regs( );
#endif

    	return 0;
}

static int pc_uart_init(void)
{
    return 0;
}

// uart connect to modem, check data ready.
// return 1 means there is data for reading
static int modem_uart_read_poll(void)
{
#if FIFO_ENABLE
    // fifo enable, other func use 0 as default.
    return (*modem_uart_fstat & MASK_UART_RX_FIFO_COUNT);
#else    
    return (*modem_uart_stat & 0x1);
#endif //FIFO_ENABLE
}

// uart connect to PC, check data ready.
// return 1 means there is data for reading
static int pc_uart_read_poll(void)
{
    return (*pc_uart_stat & 0x1);
}

// uart connect to modem, check fifo empty.
// return 1 means there is space for writing
static int modem_uart_write_poll(void)
{
#if FIFO_ENABLE
    return ((*modem_uart_fstat & MASK_UART_XMIT_FIFO_FULL)?0:1);
#else
    return (*modem_uart_stat & 0x2);
#endif //FIFO_ENABLE
}

// uart connect to PC, check fifo empty.
// return 1 means there is space for writing
static int pc_uart_write_poll(void)
{
    return (*pc_uart_stat & 0x2);
}

// uart connect to modem, check data ready.
static char modem_uart_read_byte(void)
{
    // return uart data.
    return *modem_uart_rx;
}

// uart connect to PC, check data ready.
static char pc_uart_read_byte(void)
{
    // return uart data.
    return *pc_uart_rx;
}

// uart connect to modem, check data ready.
static int modem_uart_write_byte(char data)
{
    // send data.
    *modem_uart_xmit = data;
    return 0;
}

// uart connect to PC, check data ready.
static int pc_uart_write_byte(char data)
{
    // send data.
    *pc_uart_xmit = data;
    return 0;
}

static void modem_uart_purge(void)
{
    char data = 0;
    //printf("modem_uart_purge++\n");
    while(1)
    {
        if (modem_uart_read_poll())
        {
            data = modem_uart_read_byte();
            //printf("purge:%x\n", data);
        }
        else
        {
            // modem usrt must be read before using the fifo status, or the status give the error value.
            data = modem_uart_read_byte();

            // write back to pc advoid lose data.
            pc_uart_write_byte( data );          
            break;
        }
    }

     //printf("modem_uart_purge--\n");
    
}

// send at command to modem
static int send_at_command(char *cmd, int len)
{
    printf("send:%s\n", cmd);

    while (len > 0)
    {
        // wait modem uart write space.
        //while( !modem_uart_write_poll()) 
        //    ;

        //printf("%x\n", *cmd);
        modem_uart_write_byte(*cmd++);
        len--;
    }
    return 0;
}

/*
inline int rspcmp(char *source, char *expect, int len)
{
    do
    {
        if (*source++ != *expect++)
        {
            return 1;
        }
    }while(len-->0);

    return 0;
}
*/

// get at response from modem,the rsp read from uart must start with \r\n
static int get_at_response(char *rsp, int len)
{
    int ret = AT_RSP_FAIL;
    int rsp_len = len;
    char rsp_tmp[AT_BUFFER_SIZE];
    int i = 0;

    memset(rsp_tmp, 0, sizeof(rsp_tmp));
    
    //printf("get_at_response:\n");
    for (i=0; i<rsp_len; i++)
    {
        if( modem_uart_read_poll()) 
            ;
        rsp_tmp[i] = modem_uart_read_byte();
        
        printf("%x ", rsp_tmp[i]);
    }
    /*
    for (i=0; i<rsp_len; i++)
    {
        printf("%x ", rsp_tmp[i]);
    }
     printf("\n");
     */
    if (!strcmp(rsp_tmp, rsp))
    {
        printf("get:%s\n", rsp_tmp);

        ret = AT_RSP_MACH;
    }
    else
    {
        printf("get:%s\n", rsp_tmp);
#if  MODEM_DEBUG_INFO
        printf("rsp unmach\n");
        printf("expect:%s", rsp);
#endif        
        ret = AT_RSP_UNMACH;
    }

    return ret;

}

static int send_at_and_handlersp(char *cmd, int cmd_len, const char *rsp, int rsp_len)
{
    int ret = 0;
    int i;

    //printf("send_at_and_handlersp:cmd:%s,len:%d,rsp:%s,len:%d\n", cmd, cmd_len, rsp, rsp_len);    
    
#if ENABLE_HANDLE_RSP
    for (i=0; i<RSP_GET_RETRY; i++)
    {
        send_at_command(cmd, cmd_len);
        
        if (AT_RSP_MACH == get_at_response(rsp, rsp_len))
        {
            ret = 1;
            break;
        }
        
        // clear the uart data and resend
        modem_uart_purge();
        printf("retry...\n");
        sleep(1);
    }

#else
    send_at_command(cmd, cmd_len);
#endif //ENABLE_HANDLE_RSP
    return ret;
}

// setup uart bridge for pc control cp by uart.
static int setup_uartbridge(void)
{
    int ret = 0;
    char data = 0;
    //uart init
    modem_uart_init();
    pc_uart_init();
    
    // enable AMT mode
    // uart purge
    modem_uart_purge();
    
    // modem_power_on
    modem_normal_power_on();


#if ENABLE_HANDLE_RSP
    ret = send_at_and_handlersp(AT_CMD_AMT_ENABLE, AT_CMD_AMT_ENABLE_LEN, 
                                                    AT_CMD_AMT_ENABLE_RSP, AT_CMD_AMT_ENABLE_RSP_LEN);

    if (0 == ret)
    {
        printf("cmd modem:enable AMT mode failed\n");
    }
    else
    {
        printf("cmd modem:enable AMT mode successful\n");
    }
#else
    //sleep(3);
#endif //ENABLE_HANDLE_RSP

    // modem usrt must be read before using the fifo status, or the status give the error value.
    // write back to pc advoid lose data.
    //modem_uart_purge();

    //printf("setup uart bridge\n");

    
    // start uart bridge connect, this may be continue forever.
    while(1)
    {
        // modem uart rx -> pc uart tx
        if ( modem_uart_read_poll()) 
        {
            data = modem_uart_read_byte();
            // wait pc uart write space
            while(!pc_uart_write_poll())
            	;

            pc_uart_write_byte( data );
        }        

        // pc uart rx -> modem uart tx
        if ( pc_uart_read_poll()) 
        {
            data = pc_uart_read_byte();
            // wait modem uart write space.
            while( !modem_uart_write_poll()) 
            	;
            
            //pc_uart_write_byte( data );
            modem_uart_write_byte( data );
        }

    }
    
    return 0;
}

//for cp image download
int do_modem (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    printf("Usage:%s\n", cmdtp->usage);

    if (( 2 == argc ) && !strcmp(argv[1], "info"))
    {
        // print some info
        printf("modem operation\n");
    }
    else if ((( 2 == argc ) && !strcmp(argv[1], "-d")) || (1 == argc) )
    {   
        // boot modem in download mode
        printf("do modem on off operation\n");

        // off first for safe power on
        modem_power_off();

        // cp power on
        modem_pcdownload_power_on();
    }
    else if (( 2 == argc ) && !strcmp(argv[1], "-x"))
    {
        // config and setup uart bridge.
        printf("setup uart bridge\n");
        setup_uartbridge();
    }
    else
    {
        printf("command error\n");
    }

	return 0;
}

U_BOOT_CMD(
	modem,		4,	1,	do_modem,
	"\nmodem -d contrl modem for cp download,-x uart bridge.\n", 
	"modem -d cp download\n"
	"modem -x- create uart bridge\n"
	"info   - print primitive infomation.\n"
);
