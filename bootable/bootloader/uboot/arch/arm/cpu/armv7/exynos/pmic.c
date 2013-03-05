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

#include <common.h>
#include <asm/arch/pmic.h>

void Delay(void)
{
	unsigned long i;
	for(i=0;i<DELAY;i++);
}

void IIC0_SCLH_SDAH(void)
{
	IIC0_ESCL_Hi;
	IIC0_ESDA_Hi;
	Delay();
}

void IIC0_SCLH_SDAL(void)
{
	IIC0_ESCL_Hi;
	IIC0_ESDA_Lo;
	Delay();
}

void IIC0_SCLL_SDAH(void)
{
	IIC0_ESCL_Lo;
	IIC0_ESDA_Hi;
	Delay();
}

void IIC0_SCLL_SDAL(void)
{
	IIC0_ESCL_Lo;
	IIC0_ESDA_Lo;
	Delay();
}

void IIC1_SCLH_SDAH(void)
{
	IIC1_ESCL_Hi;
	IIC1_ESDA_Hi;
	Delay();
}

void IIC1_SCLH_SDAL(void)
{
	IIC1_ESCL_Hi;
	IIC1_ESDA_Lo;
	Delay();
}

void IIC1_SCLL_SDAH(void)
{
	IIC1_ESCL_Lo;
	IIC1_ESDA_Hi;
	Delay();
}

void IIC1_SCLL_SDAL(void)
{
	IIC1_ESCL_Lo;
	IIC1_ESDA_Lo;
	Delay();
}

#if defined(CONFIG_SMDKC220)||defined(CONFIG_STUTTGART)
void IIC3_SCLH_SDAH(void)
{
	IIC3_ESCL_Hi;
	IIC3_ESDA_Hi;
	Delay();
}

void IIC3_SCLH_SDAL(void)
{
	IIC3_ESCL_Hi;
	IIC3_ESDA_Lo;
	Delay();
}

void IIC3_SCLL_SDAH(void)
{
	IIC3_ESCL_Lo;
	IIC3_ESDA_Hi;
	Delay();
}

void IIC3_SCLL_SDAL(void)
{
	IIC3_ESCL_Lo;
	IIC3_ESDA_Lo;
	Delay();
}
#endif

void IIC0_ELow(void)
{
	IIC0_SCLL_SDAL();
	IIC0_SCLH_SDAL();
	IIC0_SCLH_SDAL();
	IIC0_SCLL_SDAL();
}

void IIC0_EHigh(void)
{
	IIC0_SCLL_SDAH();
	IIC0_SCLH_SDAH();
	IIC0_SCLH_SDAH();
	IIC0_SCLL_SDAH();
}

void IIC1_ELow(void)
{
	IIC1_SCLL_SDAL();
	IIC1_SCLH_SDAL();
	IIC1_SCLH_SDAL();
	IIC1_SCLL_SDAL();
}

void IIC1_EHigh(void)
{
	IIC1_SCLL_SDAH();
	IIC1_SCLH_SDAH();
	IIC1_SCLH_SDAH();
	IIC1_SCLL_SDAH();
}

#if defined(CONFIG_SMDKC220)||defined(CONFIG_STUTTGART)
void IIC3_ELow(void)
{
	IIC3_SCLL_SDAL();
	IIC3_SCLH_SDAL();
	IIC3_SCLH_SDAL();
	IIC3_SCLL_SDAL();
}

void IIC3_EHigh(void)
{
	IIC3_SCLL_SDAH();
	IIC3_SCLH_SDAH();
	IIC3_SCLH_SDAH();
	IIC3_SCLL_SDAH();
}
#endif


void IIC0_EStart(void)
{
	IIC0_SCLH_SDAH();
	IIC0_SCLH_SDAL();
	Delay();
	IIC0_SCLL_SDAL();
}

void IIC0_EEnd(void)
{
	IIC0_SCLL_SDAL();
	IIC0_SCLH_SDAL();
	Delay();
	IIC0_SCLH_SDAH();
}

void IIC0_EAck_write(void)
{
	unsigned long ack;

	IIC0_ESDA_INP;			// Function <- Input

	IIC0_ESCL_Lo;
	Delay();
	IIC0_ESCL_Hi;
	Delay();
	ack = GPD1DAT;
	IIC0_ESCL_Hi;
	Delay();
	IIC0_ESCL_Hi;
	Delay();

	IIC0_ESDA_OUTP;			// Function <- Output (SDA)

#ifdef CONFIG_INVERSE_PMIC_I2C
	ack = (ack>>1)&0x1;
#else
	ack = (ack>>0)&0x1;
#endif
//	while(ack!=0);

	IIC0_SCLL_SDAL();
}

void IIC0_EAck_read(void)
{
	IIC0_ESDA_OUTP;			// Function <- Output

	IIC0_ESCL_Lo;
	IIC0_ESCL_Lo;
	IIC0_ESDA_Lo;
	IIC0_ESCL_Hi;
	IIC0_ESCL_Hi;

	IIC0_ESDA_INP;			// Function <- Input (SDA)

	IIC0_SCLL_SDAL();
}

void IIC1_EStart(void)
{
	IIC1_SCLH_SDAH();
	IIC1_SCLH_SDAL();
	Delay();
	IIC1_SCLL_SDAL();
}

void IIC1_EEnd(void)
{
	IIC1_SCLL_SDAL();
	IIC1_SCLH_SDAL();
	Delay();
	IIC1_SCLH_SDAH();
}

void IIC1_EAck(void)
{
	unsigned long ack;

	IIC1_ESDA_INP;			// Function <- Input

	IIC1_ESCL_Lo;
	Delay();
	IIC1_ESCL_Hi;
	Delay();
	ack = GPD1DAT;
	IIC1_ESCL_Hi;
	Delay();
	IIC1_ESCL_Hi;
	Delay();

	IIC1_ESDA_OUTP;			// Function <- Output (SDA)

	ack = (ack>>2)&0x1;
//	while(ack!=0);

	IIC1_SCLL_SDAL();
}

#if defined(CONFIG_SMDKC220)||defined(CONFIG_STUTTGART)
void IIC3_EStart(void)
{
	IIC3_SCLH_SDAH();
	IIC3_SCLH_SDAL();
	Delay();
	IIC3_SCLL_SDAL();
}

void IIC3_EEnd(void)
{
	IIC3_SCLL_SDAL();
	IIC3_SCLH_SDAL();
	Delay();
	IIC3_SCLH_SDAH();
}

void IIC3_EAck(void)
{
	unsigned long ack;

	IIC3_ESDA_INP;			// Function <- Input

	IIC3_ESCL_Lo;
	Delay();
	IIC3_ESCL_Hi;
	Delay();
	ack = GPA1DAT;
	IIC3_ESCL_Hi;
	Delay();
	IIC3_ESCL_Hi;
	Delay();

	IIC3_ESDA_OUTP;			// Function <- Output (SDA)

	ack = (ack>>2)&0x1;
//	while(ack!=0);

	IIC3_SCLL_SDAL();
}
#endif


void IIC0_ESetport(void)
{
	GPD1PUD &= ~(0xf<<0);	// Pull Up/Down Disable	SCL, SDA

	IIC0_ESCL_Hi;
	IIC0_ESDA_Hi;

	IIC0_ESCL_OUTP;		// Function <- Output (SCL)
	IIC0_ESDA_OUTP;		// Function <- Output (SDA)

	Delay();
}

void IIC1_ESetport(void)
{
	GPD1PUD &= ~(0xf<<4);	// Pull Up/Down Disable	SCL, SDA

	IIC1_ESCL_Hi;
	IIC1_ESDA_Hi;

	IIC1_ESCL_OUTP;		// Function <- Output (SCL)
	IIC1_ESDA_OUTP;		// Function <- Output (SDA)

	Delay();
}

#if defined(CONFIG_SMDKC220)||defined(CONFIG_STUTTGART)
void IIC3_ESetport(void)
{
	GPA1PUD &= ~(0xf<<4);	// Pull Up/Down Disable	SCL, SDA

	IIC3_ESCL_Hi;
	IIC3_ESDA_Hi;

	IIC3_ESCL_OUTP;		// Function <- Output (SCL)
	IIC3_ESDA_OUTP;		// Function <- Output (SDA)

	Delay();
}
#endif

void IIC0_EWrite (unsigned char ChipId, unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i;

	IIC0_EStart();

////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> i) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_ELow();	// write

	IIC0_EAck_write();	// ACK

////////////////// write reg. addr. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicAddr >> (i-1)) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_EAck_write();	// ACK

////////////////// write reg. data. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicData >> (i-1)) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_EAck_write();	// ACK

	IIC0_EEnd();
}

void IIC0_ERead (unsigned char ChipId, unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i, reg;
	unsigned char data = 0;

	IIC0_EStart();

////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> i) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_ELow();	// write

	IIC0_EAck_write();	// ACK

////////////////// write reg. addr. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicAddr >> (i-1)) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_EAck_write();	// ACK

	IIC0_EStart();

////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> i) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_EHigh();	// read

	IIC0_EAck_write();	// ACK

////////////////// read reg. data. //////////////////
	IIC0_ESDA_INP;

	for(i = 8; i>0; i--)
	{
		IIC0_ESCL_Lo;
		Delay();
		IIC0_ESCL_Hi;
		Delay();
		reg = GPD1DAT;
		IIC0_ESCL_Hi;
		Delay();
		IIC0_ESCL_Hi;
		Delay();

#ifdef CONFIG_INVERSE_PMIC_I2C
		reg = (reg >> 1) & 0x1;
#else
		reg = (reg >> 0) & 0x1;
#endif
		data |= reg << (i-1);
	}

	IIC0_EAck_read();	// ACK
	IIC0_ESDA_OUTP;

	IIC0_EEnd();

	*IicData = data;
}


void IIC1_EWrite (unsigned char ChipId, unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i;

	IIC1_EStart();

////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> i) & 0x0001)
			IIC1_EHigh();
		else
			IIC1_ELow();
	}

	IIC1_ELow();	// write 'W'

	IIC1_EAck();	// ACK

////////////////// write reg. addr. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicAddr >> (i-1)) & 0x0001)
			IIC1_EHigh();
		else
			IIC1_ELow();
	}

	IIC1_EAck();	// ACK

////////////////// write reg. data. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicData >> (i-1)) & 0x0001)
			IIC1_EHigh();
		else
			IIC1_ELow();
	}

	IIC1_EAck();	// ACK

	IIC1_EEnd();
}

#if defined(CONFIG_SMDKC220)||defined(CONFIG_STUTTGART)
void IIC3_EWrite (unsigned char ChipId, unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i;

	IIC3_EStart();

////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> i) & 0x0001)
			IIC3_EHigh();
		else
			IIC3_ELow();
	}

	IIC3_ELow();	// write 'W'

	IIC3_EAck();	// ACK

////////////////// write reg. addr. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicAddr >> (i-1)) & 0x0001)
			IIC3_EHigh();
		else
			IIC3_ELow();
	}

	IIC3_EAck();	// ACK

////////////////// write reg. data. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicData >> (i-1)) & 0x0001)
			IIC3_EHigh();
		else
			IIC3_ELow();
	}

	IIC3_EAck();	// ACK

	IIC3_EEnd();
}
#endif


void I2C_MAX8997_VolSetting(PMIC_RegNum eRegNum, u8 ucVolLevel, u8 ucEnable)
{
	u8 reg_addr, reg_bitpos, reg_bitmask, vol_level;
	u8 read_data;

	reg_bitpos = 0;
	reg_bitmask = 0x3F;
	if(eRegNum == 0)
	{
		reg_addr = 0x19;
	}
	else if(eRegNum == 1)
	{
		reg_addr = 0x22;
	}
	else if(eRegNum == 2)
	{
		reg_addr = 0x2B;
	}
	else if(eRegNum == 3)
	{
		reg_addr = 0x2D;
	}
	else if(eRegNum == 4)
	{
		reg_addr = 0x48;
	}
	else if(eRegNum == 5)
	{
		reg_addr = 0x44;
	}
	else
		while(1);

	vol_level = ucVolLevel&reg_bitmask;

	IIC0_ERead(MAX8997_ADDR, reg_addr, &read_data);

	read_data = (read_data & (~(reg_bitmask<<reg_bitpos))) | (vol_level<<reg_bitpos);

	IIC0_EWrite(MAX8997_ADDR, reg_addr, read_data);

	I2C_MAX8997_EnableReg(eRegNum, ucEnable);
}

void I2C_MAX8997_EnableReg(PMIC_RegNum eRegNum, u8 ucEnable)
{
	u8 reg_addr, reg_bitpos;
	u8 read_data;

	reg_bitpos = 0;
	if(eRegNum == 0)
	{
		reg_addr = 0x18;
	}
	else if(eRegNum == 1)
	{
		reg_addr = 0x21;
	}
	else if(eRegNum == 2)
	{
		reg_addr = 0x2A;
	}
	else if(eRegNum == 3)
	{
		reg_addr = 0x2C;
	}
	else if(eRegNum == 4)
	{
		reg_addr = 0x48;
		reg_bitpos = 0x6;
	}
	else if(eRegNum == 5)
	{
		reg_addr = 0x44;
		reg_bitpos = 0x6;
	}

	else
		while(1);

	IIC0_ERead(MAX8997_ADDR, reg_addr, &read_data);

	read_data = (read_data & (~(1<<reg_bitpos))) | (ucEnable<<reg_bitpos);

	IIC0_EWrite(MAX8997_ADDR, reg_addr, read_data);
}

void pmic_init(void)
{
	float vdd_arm, vdd_int, vdd_g3d;

#if defined(CONFIG_SMDKC220) || defined(CONFIG_ARCH_EXYNOS5) || defined(CONFIG_STUTTGART)
	float vdd_mif;
	float vdd_ldo14;
#if defined(CONFIG_PM_VDD_LDO10)
	float vdd_ldo10;
#endif
#endif

	u8 read_data;

	vdd_arm = CONFIG_PM_VDD_ARM;
	vdd_int = CONFIG_PM_VDD_INT;
	vdd_g3d = CONFIG_PM_VDD_G3D;
#if defined(CONFIG_SMDKC220) || defined(CONFIG_ARCH_EXYNOS5) || defined(CONFIG_STUTTGART)
	vdd_mif = CONFIG_PM_VDD_MIF;
#if defined(CONFIG_PM_VDD_LDO10)
	vdd_ldo10 = CONFIG_PM_VDD_LDO10;
#endif
	vdd_ldo14 = CONFIG_PM_VDD_LDO14;
#endif
	IIC0_ESetport();
	IIC1_ESetport();
#if defined(CONFIG_SMDKC220)||defined(CONFIG_STUTTGART)
	IIC3_ESetport();
#endif
	/* read ID */
	IIC0_ERead(MAX8997_ADDR, 0, &read_data);
	if (read_data == 0x77) {
		I2C_MAX8997_VolSetting(PMIC_BUCK1, CALC_MAXIM_BUCK1245_VOLT(vdd_arm * 1000), 1);
		I2C_MAX8997_VolSetting(PMIC_BUCK2, CALC_MAXIM_BUCK1245_VOLT(vdd_int * 1000), 1);
		I2C_MAX8997_VolSetting(PMIC_BUCK3, CALC_MAXIM_BUCK37_VOLT(vdd_g3d * 1000), 1);
#if defined(CONFIG_ARCH_EXYNOS5)
		I2C_MAX8997_VolSetting(PMIC_BUCK4, CALC_MAXIM_BUCK1245_VOLT(vdd_mif * 1000), 1);
#if defined(CONFIG_PM_VDD_LDO10)
		I2C_MAX8997_VolSetting(PMIC_LDO10, CALC_MAXIM_ALL_LDO(vdd_ldo10 * 1000), 3);
#endif
#endif

#if defined(CONFIG_SMDKC220) || defined(CONFIG_CPU_EXYNOS5210) ||defined(CONFIG_STUTTGART)
		/* LDO14 config */
		I2C_MAX8997_VolSetting(PMIC_LDO14, CALC_MAXIM_ALL_LDO(vdd_ldo14 * 1000), 3);
#endif
	} else {
#if !defined(CONFIG_ARCH_EXYNOS5)
		/* VDD_ARM, mode 3 register */
		IIC0_EWrite(MAX8952_ADDR, 0x03, 0x80 | (((unsigned char)(vdd_arm * 100))-77));
		/* VDD_INT, mode 2 register */
		IIC1_EWrite(MAX8649_ADDR, 0x02, 0x80 | (((unsigned char)(vdd_int * 100))-75));
		/* VDD_G3D, mode 2 register */
		IIC0_EWrite(MAX8649A_ADDR, 0x02, 0x80 | (((unsigned char)(vdd_g3d * 100))-75));
#endif
	}

#if defined( CONFIG_SMDKC220)||defined(CONFIG_STUTTGART)
	/* VDD_MIF, mode 1 register */
	IIC3_EWrite(MAX8952_ADDR, 0x01, 0x80 | (((unsigned char)(vdd_mif * 100))-77));
	GPA1PUD |= (0x5<<4);	// restore reset value: Pull Up/Down Enable SCL, SDA
#endif
}

#define CALC_S5M8767_VOLT1(x)  ( (x<600) ? 0 : ((x-600)/6.25) )
#define CALC_S5M8767_VOLT2(x)  ( (x<650) ? 0 : ((x-650)/6.25) )

void I2C_S5M8767_VolSetting(PMIC_RegNum eRegNum, u8 ucVolLevel, u8 ucEnable)
{
	u8 reg_addr, reg_bitpos, reg_bitmask, vol_level;
	u8 read_data;

	reg_bitpos = 0;
	reg_bitmask = 0xFF;
	if(eRegNum == 0)
	{
		reg_addr = 0x33;
	}
	else if(eRegNum == 1)
	{
		reg_addr = 0x35;
	}
	else if(eRegNum == 2)
	{
		reg_addr = 0x3E;
	}
	else if(eRegNum == 3)
	{
		reg_addr = 0x47;
	}
	else if(eRegNum == 4)
	{
		reg_addr = 0x48;
	}
	else
		while(1);

	vol_level = ucVolLevel&reg_bitmask;
    IIC0_EWrite(MAX8997_ADDR, reg_addr, vol_level);	

}

void pmic8767_init(void)
{
	float vdd_arm, vdd_int, vdd_g3d;
	float vdd_mif;
	u8 read_data;

	vdd_arm = CONFIG_PM_VDD_ARM;
	vdd_int = CONFIG_PM_VDD_INT;
	vdd_g3d = CONFIG_PM_VDD_G3D;
	vdd_mif = CONFIG_PM_VDD_MIF;

	IIC0_ESetport();
	IIC1_ESetport();

	I2C_S5M8767_VolSetting(PMIC_BUCK1, CALC_S5M8767_VOLT2(vdd_mif * 1000), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK2, CALC_S5M8767_VOLT1(vdd_arm * 1000), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK3, CALC_S5M8767_VOLT1(vdd_int * 1000), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK4, CALC_S5M8767_VOLT1(vdd_g3d * 1000), 1);
}
#define MAX77686_ADDR (0x12)

enum max77686_regulators{
	MAX77686_LDO1 = 0,
	MAX77686_LDO2,
	MAX77686_LDO3,
	MAX77686_LDO4,
	MAX77686_LDO5,
	MAX77686_LDO6,
	MAX77686_LDO7,
	MAX77686_LDO8,
	MAX77686_LDO9,
	MAX77686_LDO10,
	MAX77686_LDO11,
	MAX77686_LDO12,
	MAX77686_LDO13,
	MAX77686_LDO14,
	MAX77686_LDO15,
	MAX77686_LDO16,
	MAX77686_LDO17,
	MAX77686_LDO18,
	MAX77686_LDO19,
	MAX77686_LDO20,
	MAX77686_LDO21,
	MAX77686_LDO22,
	MAX77686_LDO23,
	MAX77686_LDO24,
	MAX77686_LDO25,
	MAX77686_LDO26,
	MAX77686_BUCK1,
	MAX77686_BUCK2,
	MAX77686_BUCK3,
	MAX77686_BUCK4,
	MAX77686_BUCK5,
	MAX77686_BUCK6,
	MAX77686_BUCK7,
	MAX77686_BUCK8,
	MAX77686_BUCK9,
};

enum max77686_pmic_reg {
	MAX77686_REG_DEVICE_ID		= 0x00,
	MAX77686_REG_INTSRC		= 0x01,
	MAX77686_REG_INT1		= 0x02,
	MAX77686_REG_INT2		= 0x03,

	MAX77686_REG_INT1MSK		= 0x04,
	MAX77686_REG_INT2MSK		= 0x05,

	MAX77686_REG_STATUS1		= 0x06,
	MAX77686_REG_STATUS2		= 0x07,

	MAX77686_REG_PWRON		= 0x08,
	MAX77686_REG_ONOFF_DELAY	= 0x09,
	MAX77686_REG_MRSTB		= 0x0A,
	/* Reserved: 0x0B-0x0F */

	MAX77686_REG_BUCK1CTRL		= 0x10,
	MAX77686_REG_BUCK1OUT		= 0x11,
	MAX77686_REG_BUCK2CTRL1		= 0x12,
	MAX77686_REG_BUCK234FREQ	= 0x13,
	MAX77686_REG_BUCK2DVS1		= 0x14,
	MAX77686_REG_BUCK2DVS2		= 0x15,
	MAX77686_REG_BUCK2DVS3		= 0x16,
	MAX77686_REG_BUCK2DVS4		= 0x17,
	MAX77686_REG_BUCK2DVS5		= 0x18,
	MAX77686_REG_BUCK2DVS6		= 0x19,
	MAX77686_REG_BUCK2DVS7		= 0x1A,
	MAX77686_REG_BUCK2DVS8		= 0x1B,
	MAX77686_REG_BUCK3CTRL1		= 0x1C,
	/* Reserved: 0x1D */
	MAX77686_REG_BUCK3DVS1		= 0x1E,
	MAX77686_REG_BUCK3DVS2		= 0x1F,
	MAX77686_REG_BUCK3DVS3		= 0x20,
	MAX77686_REG_BUCK3DVS4		= 0x21,
	MAX77686_REG_BUCK3DVS5		= 0x22,
	MAX77686_REG_BUCK3DVS6		= 0x23,
	MAX77686_REG_BUCK3DVS7		= 0x24,
	MAX77686_REG_BUCK3DVS8		= 0x25,
	MAX77686_REG_BUCK4CTRL1		= 0x26,
	/* Reserved: 0x27 */
	MAX77686_REG_BUCK4DVS1		= 0x28,
	MAX77686_REG_BUCK4DVS2		= 0x29,
	MAX77686_REG_BUCK4DVS3		= 0x2A,
	MAX77686_REG_BUCK4DVS4		= 0x2B,
	MAX77686_REG_BUCK4DVS5		= 0x2C,
	MAX77686_REG_BUCK4DVS6		= 0x2D,
	MAX77686_REG_BUCK4DVS7		= 0x2E,
	MAX77686_REG_BUCK4DVS8		= 0x2F,
	MAX77686_REG_BUCK5CTRL		= 0x30,
	MAX77686_REG_BUCK5OUT		= 0x31,
	MAX77686_REG_BUCK6CTRL		= 0x32,
	MAX77686_REG_BUCK6OUT		= 0x33,
	MAX77686_REG_BUCK7CTRL		= 0x34,
	MAX77686_REG_BUCK7OUT		= 0x35,
	MAX77686_REG_BUCK8CTRL		= 0x36,
	MAX77686_REG_BUCK8OUT		= 0x37,
	MAX77686_REG_BUCK9CTRL		= 0x38,
	MAX77686_REG_BUCK9OUT		= 0x39,
	/* Reserved: 0x3A-0x3F */

	MAX77686_REG_LDO1CTRL1		= 0x40,
	MAX77686_REG_LDO2CTRL1		= 0x41,
	MAX77686_REG_LDO3CTRL1		= 0x42,
	MAX77686_REG_LDO4CTRL1		= 0x43,
	MAX77686_REG_LDO5CTRL1		= 0x44,
	MAX77686_REG_LDO6CTRL1		= 0x45,
	MAX77686_REG_LDO7CTRL1		= 0x46,
	MAX77686_REG_LDO8CTRL1		= 0x47,
	MAX77686_REG_LDO9CTRL1		= 0x48,
	MAX77686_REG_LDO10CTRL1		= 0x49,
	MAX77686_REG_LDO11CTRL1		= 0x4A,
	MAX77686_REG_LDO12CTRL1		= 0x4B,
	MAX77686_REG_LDO13CTRL1		= 0x4C,
	MAX77686_REG_LDO14CTRL1		= 0x4D,
	MAX77686_REG_LDO15CTRL1		= 0x4E,
	MAX77686_REG_LDO16CTRL1		= 0x4F,
	MAX77686_REG_LDO17CTRL1		= 0x50,
	MAX77686_REG_LDO18CTRL1		= 0x51,
	MAX77686_REG_LDO19CTRL1		= 0x52,
	MAX77686_REG_LDO20CTRL1		= 0x53,
	MAX77686_REG_LDO21CTRL1		= 0x54,
	MAX77686_REG_LDO22CTRL1		= 0x55,
	MAX77686_REG_LDO23CTRL1		= 0x56,
	MAX77686_REG_LDO24CTRL1		= 0x57,
	MAX77686_REG_LDO25CTRL1		= 0x58,
	MAX77686_REG_LDO26CTRL1		= 0x59,
	/* Reserved: 0x5A-0x5F */
	MAX77686_REG_LDO1CTRL2		= 0x60,
	MAX77686_REG_LDO2CTRL2		= 0x61,
	MAX77686_REG_LDO3CTRL2		= 0x62,
	MAX77686_REG_LDO4CTRL2		= 0x63,
	MAX77686_REG_LDO5CTRL2		= 0x64,
	MAX77686_REG_LDO6CTRL2		= 0x65,
	MAX77686_REG_LDO7CTRL2		= 0x66,
	MAX77686_REG_LDO8CTRL2		= 0x67,
	MAX77686_REG_LDO9CTRL2		= 0x68,
	MAX77686_REG_LDO10CTRL2		= 0x69,
	MAX77686_REG_LDO11CTRL2		= 0x6A,
	MAX77686_REG_LDO12CTRL2		= 0x6B,
	MAX77686_REG_LDO13CTRL2		= 0x6C,
	MAX77686_REG_LDO14CTRL2		= 0x6D,
	MAX77686_REG_LDO15CTRL2		= 0x6E,
	MAX77686_REG_LDO16CTRL2		= 0x6F,
	MAX77686_REG_LDO17CTRL2		= 0x70,
	MAX77686_REG_LDO18CTRL2		= 0x71,
	MAX77686_REG_LDO19CTRL2		= 0x72,
	MAX77686_REG_LDO20CTRL2		= 0x73,
	MAX77686_REG_LDO21CTRL2		= 0x74,
	MAX77686_REG_LDO22CTRL2		= 0x75,
	MAX77686_REG_LDO23CTRL2		= 0x76,
	MAX77686_REG_LDO24CTRL2		= 0x77,
	MAX77686_REG_LDO25CTRL2		= 0x78,
	MAX77686_REG_LDO26CTRL2		= 0x79,
	MAX77686_REG_BBAT_CHG		= 0x7E,
	MAX77686_REG_32KHZ		= 0x7F,
	MAX77686_REG_PMIC_END		= 0x80,
};

#define CALC_MAX77686_LDO_VOL(x)      ( (x<800000 || x>3950000) ? 0 : ((x-800000)/50000) )
#define CALC_MAX77686_LDO_LOW_VOL(x)  ( (x<800000 || x>2375000) ? 0 : ((x-800000)/25000) )
#define CALC_MAX77686_BUCKDVS_VOL(x)  ( (x<600000 || x>3787500) ? 0 : ((x-600000)/12500))
#define CALC_MAX77686_BUCK_VOL(x)     ( (x<750000 || x>3900000) ? 0 : ((x-750000)/50000) )

/*jeff, pmic set for Lenovo S3, max77686 */
void pmic_max77686_init(void)
{
//	printf("pmic_max77686_init\n");

	float vdd_arm, vdd_int, vdd_g3d;
	float vdd_mif;
	u8 read_data;

        /* as smdk4412.h define*/
	vdd_arm = CONFIG_PM_VDD_ARM;
	vdd_int = CONFIG_PM_VDD_INT;
	vdd_g3d = CONFIG_PM_VDD_G3D;
	vdd_mif = CONFIG_PM_VDD_MIF;

	IIC0_ESetport();

	/* BUCK2. VDD_ARM_DVS1, 1350mV for 1.4Ghz, 1125mv for 1Ghz, SET 1.25v as usual*/

	IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_BUCK2DVS1, CALC_MAX77686_BUCKDVS_VOL(vdd_arm * 1000000));
	/* BUCK3. VDD_INT_DVS1, 1.1V for 400Mhz, 1.0v for 200Mhz*/
	IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_BUCK3DVS1, CALC_MAX77686_BUCKDVS_VOL(vdd_int * 1000000));
	/* BUCK4. VDD_G3D_DVS1, 1.2v or 1.1v for mali400 ?*/
	IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_BUCK4DVS1, CALC_MAX77686_BUCKDVS_VOL(vdd_g3d * 1000000));

	/* BUCK1. VDD_MIF_AP, 1.1v */
	IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_BUCK1OUT, CALC_MAX77686_BUCK_VOL(vdd_mif * 1000000) );
/*disable ldo23, and then enable it*/
	/*
	IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO23CTRL1, &read_data);
	IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO23CTRL1,  (read_data & ~(0x3 << 6)));

	IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO8CTRL1, &read_data);
	IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO8CTRL1,  (read_data & ~(0x3 << 6)));
	*/

	//IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO23CTRL1, &read_data);
	//IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO23CTRL1,  (read_data | (0x3 << 6)));
}
/*jeff, set ldo if uboot enter precharge */
void precharge_set_ldo(int on)
{
	u8 read_data;

	if(on){
		/* disable unused LDO for optmize uboot power */
		/* BUCK4 G3D OPMOD: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_BUCK4CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_BUCK4CTRL1,  (read_data | (0x3 << 4)));

		/* LDO5 VDD18_CODEC OPMODE: On*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO5CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO5CTRL1,  (read_data | (0x3 << 6)));

		/* LDO8 MIPI_HDMI_AP: On*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO8CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO8CTRL1,  (read_data | (0x3 << 6)));

		/* LDO10 MIPI_HDMI: On*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO10CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO10CTRL1,  (read_data | (0x3 << 6)));

		/* LDO13 VDDQ_C2C OPMODE: On*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO13CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO13CTRL1,  (read_data | (0x3 << 6)));

		/* LDO12 VDD33_OTG_AP OPMODE: On*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO12CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO12CTRL1,  (read_data | (0x3 << 6)));
	
		/* LDO15 VDD10_HSIC_AP OPMODE: On*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO15CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO15CTRL1,  (read_data | (0x3 << 6)));
	
		/* LDO16 VDDQ_HSIC OPMODE: On*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO16CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO16CTRL1,  (read_data | (0x3 << 6)));
	
	}
	else{
		/* LDO8 MIPI_HDMI_AP: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO8CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO8CTRL1,  (read_data & ~(0x3 << 6)));

		/* LDO10 MIPI_HDMI: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO10CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO10CTRL1,  (read_data & ~(0x3 << 6)));

		/* disable unused LDO for optmize uboot power */
		/* BUCK4 G3D OPMOD: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_BUCK4CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_BUCK4CTRL1,  (read_data & ~(0x3 << 4)));

		/* LDO5 VDD18_CODEC OPMODE: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO5CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO5CTRL1,  (read_data & ~(0x3 << 6)));

		/* LDO13 VDDQ_C2C OPMODE: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO13CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO13CTRL1,  (read_data & ~(0x3 << 6)));

		/* LDO12 VDD33_OTG_AP OPMODE: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO12CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO12CTRL1,  (read_data & ~(0x3 << 6)));
	
		/* LDO15 VDD10_HSIC_AP OPMODE: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO15CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO15CTRL1,  (read_data & ~(0x3 << 6)));
	
		/* LDO16 VDDQ_HSIC OPMODE: OFF*/
		IIC0_ERead(MAX77686_ADDR, MAX77686_REG_LDO16CTRL1, &read_data);
		IIC0_EWrite(MAX77686_ADDR, MAX77686_REG_LDO16CTRL1,  (read_data & ~(0x3 << 6)));
	}
}

