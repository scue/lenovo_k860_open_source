
#define POWERON_VOLTAGE	3500
#define MAX77686_ADDR 0x12

#define POWERON_HOLD_TIMES		1500
#define MAX77686_REG_STATUS1	6
#define MAX77686_REG_STATUS2	7
#define MAX77686_REG_PWRON		8
/* reset state register flag */
#define FLAG_RST_STATE_SWRESET	0x20000000
#define FLAG_RST_STATE_SYS_WDTRESET 0x00100000

/* TOPSYS status register 1 flag */
#define FLAG_PWRON_HOLD_1SEC	0x08

/* PMIC Power-On Source Register flag */
#define FLAG_POWERON_PWRON		0x01
#define FLAG_POWERON_JIGONB		0x02
#define FLAG_POWERON_ALARM1		0x10

/* gpio address definition */
#define GPX2BASE	0x11000000
#define GPX2CON		(GPX2BASE + 0x0C40)
#define GPX2DAT		(GPX2BASE + 0x0C44)
#define GPX2PUD		(GPX2BASE + 0x0C48)
#define GPX2DRV		(GPX2BASE + 0x0C4C)

#define GPZBASE		0x03860000
#define GPZCON		GPZBASE + 0x0
#define GPZDAT		GPZBASE + 0x4
#define GPZPUD		GPZBASE + 0x8
#define GPZDRV		GPZBASE + 0xC

/* GPX2[4] flag:
 * 1: charger pluged
 * 0: no charger pluged
 */
#define FLAG_CHARGER_ON			0x10

enum {
	POWERON_ONKEY,
	POWERON_CHARGER,
	POWERON_RESET,
	POWERON_RTC_ALARM,
	POWERON_JIGON,
};

enum {
	CHARGER_UNKNOWN,
	CHARGER_USB,
	CHARGER_AC,
};
