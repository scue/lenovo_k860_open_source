#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <power_reason.h>
#include <asm/arch/cpu.h>
#include "../drivers/usb/gadget/usbd-otg-hs.h"

/* #define POWERON_DEBUG */

extern void IIC0_ESetport(void);
extern void IIC0_ERead (unsigned char ChipId, unsigned char IicAddr, unsigned char *IicData);
extern void IIC0_EWrite (unsigned char ChipId, unsigned char IicAddr, unsigned char IicData);

 extern int  max17058_get_soc(void);
 extern int  max17058_get_vcell(void);
 extern void  max17058_init(int charger_on);
 
 
/* int charger_type = CHARGER_UNKNOWN;
 * set charger is usb default
 * we can do charger judge(usb or AC) later
 */
static int charger_type = CHARGER_UNKNOWN;
static  int soc=0,vcell=0;

void do_poweroff(void)
{
	/* write PS_HOLD_CONTROL to output low */
	writel(0x14200, 0x10020000 + 0x330C);
	while(1);
}

void vib(int on)
{
#ifdef POWERON_DEBUG
	unsigned char gpzdata = 0;
	unsigned long gpzcon, gpzpud, gpzdrv;
#endif

	/* config gpz6 to output */
	writel(((readl(GPZCON) & (~(0x0F << 24))) | (0x01 << 24)), GPZCON);

	/* config gpz6 to no pu/pd */
	writel((readl(GPZPUD) & (~(0x03 << 12))), GPZPUD);

	/* config gpz6 to drive 1x */
	writel((readl(GPZDRV) & (~(0x03 << 12))), GPZDRV);

	if (on)	/* set gpz6 output high */
		writel((readl(GPZDAT) | (0x01 << 6)), GPZDAT);
	else	/* set gpz6 output low */
		writel((readl(GPZDAT) & (~(0x01 << 6))), GPZDAT);

#ifdef POWERON_DEBUG
	gpzcon = readl(GPZCON);
	printf("gpzcon=0x%x\n", gpzcon);
	gpzpud = readl(GPZPUD);
	printf("gpzpud=0x%x\n", gpzpud);
	gpzdrv = readl(GPZDRV);
	printf("gpzdrv=0x%x\n", gpzdrv);
	gpzdata = readl(GPZDAT);
	printf("gpzdata = 0x%x\n", gpzdata);
#endif
}

int is_charger_plugged(void)
{
	unsigned char gpx2data;
#ifdef POWERON_DEBUG
	unsigned long gpx2con = 0, gpx2pud = 0, gpx2drv = 0;
#endif

	/* config gpx2_4 to input */
	writel((readl(GPX2CON) & (~(0x0f << 16))), GPX2CON);

	/* config gpx2_4 to no pu/pd */
	writel((readl(GPX2PUD) & (~(0x03 << 8))), GPX2PUD);

	/* config gpx2_4 to drive 1x */
	writel((readl(GPX2DRV) & (~(0x03 << 8))), GPX2DRV);

	/* get input data of gpx2 */
	gpx2data = readl(GPX2DAT);

#ifdef POWERON_DEBUG
	printf("gpx2data=0x%x\n", gpx2data);
	gpx2con = readl(GPX2CON);
	printf("gpx2con=0x%x\n", gpx2con);
	gpx2pud = readl(GPX2PUD);
	printf("gpx2pud=0x%x\n", gpx2pud);
	gpx2drv = readl(GPX2DRV);
	printf("gpx2drv=0x%x\n", gpx2drv);
#endif

	/* check if charger is pluged */
	if (gpx2data & FLAG_CHARGER_ON)
		return 1;

	return 0;
}
#define MAX8971_ADDR 0x6A
#define MAX8971_REG_THM_CNFG 0x9
#define MAX8971_REG_CHG_PROT 0xA
void  max17058_get_info(void)
{     
      max17058_init(is_charger_plugged());
      soc = max17058_get_soc(); 
      vcell = max17058_get_vcell();
       
}

void get_poweron_reason(int *power_reason, int mode, int factory)
{
	unsigned char pmu_poweron;
	unsigned char rtc_pmu_poweron;
	unsigned char pmu_status1;
	unsigned int rst_stat;
	char run_cmd[100];
	int i = 0;
	if(1)
       {
        unsigned char max8971_thm_reg;
        unsigned char read_data;
	IIC0_ESetport();
	/*set charger thermistor Monitor Configuration*/
	IIC0_ERead(MAX8971_ADDR, MAX8971_REG_THM_CNFG, &max8971_thm_reg);
	printf("uboot, firstly, MAX8971_REG_THM_CNFG = 0x%x\n",max8971_thm_reg);
	max8971_thm_reg = 0;
	IIC0_EWrite(MAX8971_ADDR, MAX8971_REG_CHG_PROT, 0xC); //unlock
        IIC0_ERead(MAX8971_ADDR, MAX8971_REG_CHG_PROT, &read_data);
        printf("uboot, MAX8971_REG_CHG_PROT= 0x%x\n",read_data);
	IIC0_EWrite(MAX8971_ADDR, MAX8971_REG_THM_CNFG, max8971_thm_reg);
	max8971_thm_reg = 0xff;
	IIC0_ERead(MAX8971_ADDR, MAX8971_REG_THM_CNFG, &max8971_thm_reg);
	printf("uboot, MAX8971_REG_THM_CNFG = 0x%x\n",max8971_thm_reg);
	}
	else
        /* init i2c0 bus */
	IIC0_ESetport();
    IIC0_ERead(MAX77686_ADDR, 0x01, &rtc_pmu_poweron);
    IIC0_ERead(0x0C, 0x00, &rtc_pmu_poweron);

        /* read the power on source register */
	IIC0_ERead(MAX77686_ADDR, MAX77686_REG_PWRON, &pmu_poweron);
	printf("pmu_poweron = 0x%x\n", pmu_poweron);

	IIC0_ERead(MAX77686_ADDR, MAX77686_REG_STATUS1, &pmu_status1);
	printf("pmu_status1 = 0x%x\n", pmu_status1);

	/* get the reset state register */
	rst_stat = RST_STAT_REG;
	printf("Reset stat = 0x%x\n", rst_stat);

	/* save it to cmdline for debug */
	sprintf(run_cmd, "setenv bootargs ${bootargs} pmu_irq=0x%x pmu_status1=0x%x reset=0x%x", 
		pmu_poweron, pmu_status1, rst_stat);
	run_command(run_cmd, 0);

	if ((rst_stat & (FLAG_RST_STATE_SWRESET | FLAG_RST_STATE_SYS_WDTRESET))){
		printf("System reset detected!\n");
		*power_reason = POWERON_RESET;
	} else if (is_charger_plugged()) {
		printf("Charger detected!\n");
		*power_reason = POWERON_CHARGER;
		vib(1);
		udelay(150*1000);
		vib(0);
	} else if (pmu_poweron & FLAG_POWERON_ALARM1) {
		printf("RTC alarm-1 detected!\n");
		*power_reason = POWERON_RTC_ALARM;
	} else if (pmu_poweron & FLAG_POWERON_JIGONB) {
		printf("JIG ON detected!\n");
		*power_reason = POWERON_JIGON;
	} else if (pmu_poweron & FLAG_POWERON_PWRON) {
		printf("Power key detected, please hold 1s!\n");
		*power_reason = POWERON_ONKEY;
		if (mode) {
			printf("Boot mode = %d\n", mode);
			return;
		}

/*
		i = POWERON_HOLD_TIMES;
		while(i--) {
			udelay(1000);
			IIC0_ERead(MAX77686_ADDR, MAX77686_REG_STATUS1, &pmu_status1);
			if (pmu_status1 & FLAG_PWRON_HOLD_1SEC) {
				printf("PMU status1 = 0x%x\n", pmu_status1);
				break;
			}
		}

		if (i > 0) {
*/
		if (pmu_status1 & FLAG_POWERON_PWRON) {
			printf("Power key hold to 1s, the phone will startup!\n");
			if (factory == 0) {
				vib(1);
				udelay(150*1000);
				vib(0);
			}
		} else {
			printf("Power key not hold to 1s, the phone will power off!\n\n");
			do_poweroff();
		}
	} else {
		printf("No power reason detect, the phone will power off!\n\n");
		do_poweroff();
	}
}

int ac_or_usb_charger(void)
{
	int type = CHARGER_UNKNOWN;
	int usb = 0;
	char run_cmd[100];

	printf("check usb connect status\n");
	usb = s3c_check_usb_connect_status();

	/* save it to cmdline for debug */
	sprintf(run_cmd, "setenv bootargs ${bootargs} usb=%d", usb);
	run_command(run_cmd, 0);

	if(usb == 2) {
		printf("AC Charger detected!\n");
		type = CHARGER_AC;
	} else {
		printf("USB Charger detected!\n");
		type = CHARGER_USB;
	}

	return type;
}


/* jeff, add cpu and pd power_gating for precharge mode*/
extern void precharge_set_ldo(int on);

struct exynos4_power_domain{
	u32 reg_base;
	u32 clk_base;
};

enum exynos4_pd_block{
	PD_CAM,
	PD_TV,
	PD_MFC,
	PD_G3D,
	PD_LCD0,
	PD_ISP,
	PD_MAUDIO,
	PD_GPS,
	PD_GPSALIVE,
};

struct exynos4_power_domain exynos4_pd[] = {
	[PD_CAM] = {
		.reg_base = S5P_PMU_CAM_CONF,
		.clk_base = EXYNOS4_CLKGATE_IP_CAM,
	},
	[PD_TV] = {
		.reg_base = S5P_PMU_TV_CONF,
		.clk_base = EXYNOS4_CLKGATE_IP_TV,
	},
	[PD_MFC] = {
		.reg_base = S5P_PMU_MFC_CONF,
		.clk_base = EXYNOS4_CLKGATE_IP_MFC,
	},
	[PD_G3D] = {
		.reg_base = S5P_PMU_G3D_CONF,
		.clk_base = EXYNOS4_CLKGATE_IP_G3D,
	},
	[PD_LCD0] = {
		.reg_base = S5P_PMU_LCD0_CONF,
		.clk_base = EXYNOS4_CLKGATE_IP_LCD0,
	},
	[PD_ISP] = {
		.reg_base = S5P_PMU_ISP_CONF,
		.clk_base = EXYNOS4_CLKGATE_IP_ISP,
	},
	[PD_GPS] = {
		.reg_base = S5P_PMU_GPS_CONF,
		.clk_base = EXYNOS4_CLKGATE_IP_GPS,
	},
	[PD_MAUDIO] = {
		.reg_base = S5P_PMU_MAUDIO_CONF,
		.clk_base = EXYNOS4_CLKGATE_IP_MAUDIO,
	},
	[PD_GPSALIVE] = {
		.reg_base = S5P_PMU_GPSALIVE_CONF,
		.clk_base = 0,
	},
};

#define EXYNOS4_PD_POWER_EN (0x7)

static int exynos4_pd_enable(struct exynos4_power_domain pd)
{
	u32 tmp;
	u32 timeout = 1000;
	
	if(pd.clk_base)
		tmp = readl(pd.clk_base);

	writel(0xffffffff,pd.clk_base);

	writel(EXYNOS4_PD_POWER_EN, pd.reg_base);

	while( (readl(pd.reg_base + 0x4) & EXYNOS4_PD_POWER_EN ) 
		!= EXYNOS4_PD_POWER_EN){
		if(timeout == 0){
			printf("pd enable err\n");
			return -1;
		}
		timeout--;
		udelay(1);
	}

	if(pd.clk_base)
		writel(tmp,pd.clk_base);
	return 0;
}

static int exynos4_pd_disable(struct exynos4_power_domain pd)
{
	u32 timeout = 1000;

	writel(0x0, pd.reg_base);

	while( (readl(pd.reg_base + 0x4) & EXYNOS4_PD_POWER_EN )){
		if(timeout == 0){
			printf("pd disable err\n");
			return -1;
		}
		timeout--;
		udelay(1);
	}

	return 0;
}

#define EXYNOS4_CPU_POWER_EN (0x3)
static int exynos4_cpu_down(u32 cpu)
{
	u32 timeout = 1000;

	if(cpu > 3 || cpu < 1)
		return -1;

	writel(0x0, S5P_ARM_CORE_CONFIGURATION(cpu));

	while( (readl(S5P_ARM_CORE_CONFIGURATION(cpu) + 0x4) & EXYNOS4_CPU_POWER_EN )){
		if(timeout == 0){
			printf("pd disable err\n");
			return -1;
		}
		timeout--;
		udelay(1);
	}

	return 0;	
}

/* disable PMU unused power domain to save uboot power */
static void exynos4_pmu_set(int on) 
{
	if(on){
		exynos4_pd_enable(exynos4_pd[PD_CAM]);
		exynos4_pd_enable(exynos4_pd[PD_TV]);
		exynos4_pd_enable(exynos4_pd[PD_MFC]);
		exynos4_pd_enable(exynos4_pd[PD_G3D]);
		exynos4_pd_enable(exynos4_pd[PD_ISP]);
		exynos4_pd_enable(exynos4_pd[PD_GPS]);
		exynos4_pd_enable(exynos4_pd[PD_LCD0]);	

	}
	else{

		exynos4_pd_disable(exynos4_pd[PD_CAM]);
		exynos4_pd_disable(exynos4_pd[PD_TV]);
		exynos4_pd_disable(exynos4_pd[PD_MFC]);
		exynos4_pd_disable(exynos4_pd[PD_G3D]);
		exynos4_pd_disable(exynos4_pd[PD_ISP]);
		exynos4_pd_disable(exynos4_pd[PD_GPS]);
		exynos4_pd_disable(exynos4_pd[PD_LCD0]);

		exynos4_cpu_down(1);
		exynos4_cpu_down(2);
		exynos4_cpu_down(3);
	}
}

//mc,cut down uboot start time,comment out II0_Eread() functions for debug.
void do_charger_detect(void)
{
    	unsigned char max8971_thm_reg;
        unsigned char read_data;
	/* init i2c0 bus */
	IIC0_ESetport();
	/*set charger thermistor Monitor Configuration*/
	//IIC0_ERead(MAX8971_ADDR, MAX8971_REG_THM_CNFG, &max8971_thm_reg);
	//printf("uboot, firstly, MAX8971_REG_THM_CNFG = 0x%x\n",max8971_thm_reg);
	//max8971_thm_reg = 0;
	IIC0_EWrite(MAX8971_ADDR, MAX8971_REG_CHG_PROT, 0xC); //unlock the charger 
        //IIC0_ERead(MAX8971_ADDR, MAX8971_REG_CHG_PROT, &read_data);
        //printf("uboot, MAX8971_REG_CHG_PROT= 0x%x\n",read_data);
	//IIC0_EWrite(MAX8971_ADDR, MAX8971_REG_THM_CNFG, max8971_thm_reg);
	//max8971_thm_reg = 0xff;
	 //IIC0_ERead(MAX8971_ADDR, MAX8971_REG_THM_CNFG, &max8971_thm_reg);
	 //printf("uboot, MAX8971_REG_THM_CNFG = 0x%x\n",max8971_thm_reg); 
       
        IIC0_EWrite(MAX8971_ADDR,0x01, 0x0);
        //IIC0_ERead(MAX8971_ADDR,0x01, &read_data);
       // printf("uboot, MAX8971 chg_mask= 0x%x\n",read_data);
       
       if(charger_type != CHARGER_UNKNOWN)	 
       {
	      if (charger_type == CHARGER_AC)
		  {
                  IIC0_EWrite(MAX8971_ADDR,0x06, 0x34);	//AC Charger,chgcc:1A
                  IIC0_EWrite(MAX8971_ADDR,0x07, 0x28); //DCLimit:1A
                  }
	      else if (charger_type == CHARGER_USB)			 
	         { 
                 IIC0_EWrite(MAX8971_ADDR,0x06, 0x2a);//USB charger,chgcc:500mA
   	         IIC0_EWrite(MAX8971_ADDR,0x07, 0x28);// DCLimit:500MA
                 }
                 //for debug 
                 /*
                 IIC0_ERead(MAX8971_ADDR, 0x4, &read_data);
                 printf("uboot, MAX8971 details2=0x%x\n",read_data);
                 IIC0_ERead(MAX8971_ADDR, 0x3, &read_data);
                 printf("uboot, MAX8971 details1=0x%x\n",read_data);
                 IIC0_ERead(MAX8971_ADDR, 0x6, &read_data);
                 printf("uboot, MAX8971 chgcc=0x%x\n",read_data);
                 IIC0_ERead(MAX8971_ADDR, 0xA, &read_data);
                 printf("uboot, MAX8971 prot=0x%x\n",read_data);
                  */
	        if (soc < 1)  
	        {
			printf("Battery capacity is too low, enter precharge mode!\n");
			/* jeff, set external and internal pmu to save power */
			precharge_set_ldo(0);
			exynos4_pmu_set(0);
			
			while(1)
			{
				 
			/*	if ((status & 0x20) == 0)
				{
					printf("The charger has removed, the phone will power off!\n\n");
					do_poweroff();
				}
				if (status &  0x10)
				{
					printf("The battery has removed, the phone will power off!\n\n");
					do_poweroff();
				}
                       */
				
				soc = max17058_get_soc();
				vcell=max17058_get_vcell();
                                printf("The battery voltage: %d mV, capacity: %d%\n", vcell, soc);
				if (soc >= 1||vcell>3600)
					break;
                                if(!is_charger_plugged())
                                     {
                                        printf("charger plugout!Please plugin charger!\n");
                                        break;
                                     }
                                udelay(50000*1000); //Freq reason,about 5s
		         }
			/*jeff, restore power to default state */
			exynos4_pmu_set(1);
			precharge_set_ldo(1);
			
		}
	}
	else if(!is_charger_plugged())	
	{
		if (soc < 2)
		{
			printf("The Battery capacity is too low, the phone will power off!\n");
			printf("Please change battery or plugin charger.\n\n");
			do_poweroff();
		}
	} 
}

void set_poweron_reason_bootargs(int power_reason)
{
	int rets = 0;
	switch(power_reason)
	{
		case POWERON_ONKEY:
		case POWERON_JIGON:
			run_command("setenv bootargs ${bootargs} androidboot.mode=power_key", 0);
			break;

		case POWERON_RTC_ALARM:
			run_command("setenv bootargs ${bootargs} androidboot.mode=rtc_alarm", 0);
			break;

		case POWERON_RESET:
			rets = run_command("setenv bootargs ${bootargs} androidboot.mode=normal", 0);
			break;

		case POWERON_CHARGER:
			charger_type = ac_or_usb_charger();
			if (charger_type == CHARGER_AC)
				run_command("setenv bootargs ${bootargs} androidboot.mode=ac_charger", 0);
			else if (charger_type == CHARGER_USB)
				run_command("setenv bootargs ${bootargs} androidboot.mode=usb_cable", 0);
			break;
	}
}

int do_poweron_reason(int mode, int factory)
{
	int poweron_reason = POWERON_ONKEY;
    max17058_get_info();
	get_poweron_reason(&poweron_reason, mode, factory);
    set_poweron_reason_bootargs(poweron_reason);
    do_charger_detect();
	return 0;
}
