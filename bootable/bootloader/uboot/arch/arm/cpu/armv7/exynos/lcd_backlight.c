#include <asm/arch/pmic.h>
#include <asm/io.h>
#define BACKLIGHT_ADDR 0x70



void lcd_backlight_init()
{
	printf("lcd_backlight_init\n");
	unsigned char val = 0;
	writel((readl(0x114000a8) & (~(0x3 <<2))), 0x114000a8);//GPF0-0
	writel((readl(0x114000a0) & (~(0xf <<4)))|(0x1 <<4), 0x114000a0);

	writel((readl(0x114000a4) & (~(0x1 <<1)))|(0x1 <<1), 0x114000a4);//output 1
	IIC1_ESetport();

	IIC1_EWrite(BACKLIGHT_ADDR, 0x10,0x00);	// ILED1 & ILED2 are all controlled by Control A PWM and Brightness Registers 
	IIC1_EWrite(BACKLIGHT_ADDR, 0x11,0x12);	//Setting Startup/shutdown Ramp Rate  SHUTDN_RAMP_2p048ms_STEP | START_RAMP_2p048ms_STEP
	IIC1_EWrite(BACKLIGHT_ADDR, 0x12,0x12);	// Run time Ramp Rate  RT_RAMPDN_2p048ms_STEP | RT_RAMPUP_2p048ms_STEP
	IIC1_EWrite(BACKLIGHT_ADDR, 0x13,0x07);	// Control A PWM Register  PWM_CHANNEL_SEL_PWM2 | PWM_INPUT_POLARITY_HIGH | PWM_ZONE_0_EN
	IIC1_EWrite(BACKLIGHT_ADDR, 0x16,0x03);	// Control A Brightness Configuration  I2C_CURRENT_CONTROL | LINEAR_MAPPING | CONTROL_ZONE_TARGET_0
	IIC1_EWrite(BACKLIGHT_ADDR, 0x17,0x13);	// Control A Full-Scale Current    CNTL_FS_CURRENT_20P2mA
	IIC1_EWrite(BACKLIGHT_ADDR, 0x1C,0x03);	// Feedback  FEEDBACK_ENABLE_ILED1 | FEEDBACK_ENABLE_ILED2
	IIC1_EWrite(BACKLIGHT_ADDR, 0x1D,0x01);	// Control Enable  ENABLE_CNTL_A
	IIC1_EWrite(BACKLIGHT_ADDR, 0x70,0xB0);	// Default using Control A Zone 0 Target  
}
