//2012.mc add for max17058 chip.

#include <common.h>
#include <asm/arch/pmic.h>
#include <config.h>
#include <asm/io.h>
#include "i2c.h" 
#include <power_reason.h>
#include <asm/arch/cpu.h>
#include "../drivers/usb/gadget/usbd-otg-hs.h"


//set GPD1 port register here
#define GPIO_LB_BASE                    0x11400000
#define GPD1CON_OFFSET 			0x0C0 
#define GPD1DAT_OFFSET 			0x0C4 
#define GPD1PUD_OFFSET 			0x0C8 
#define GPD1DRV_SR_OFFSET 		         0x0CC 
#define GPD1CONPDN_OFFSET 		0x0D0 
#define GPD1PUDPDN_OFFSET 		0x0D4 

//Rigister name and Address
#define MAX17058_VCELL_MSB	0x02
#define MAX17058_VCELL_LSB	0x03
#define MAX17058_SOC_MSB	         0x04
#define MAX17058_SOC_LSB	         0x05
#define MAX17058_MODE_MSB	0x06
#define MAX17058_MODE_LSB	0x07
#define MAX17058_VER_MSB	         0x08
#define MAX17058_VER_LSB	         0x09
#define MAX17058_CONFIG_MSB	0x0C
#define MAX17058_CONFIG_LSB	0x0D
#define MAX17058_VRESET_MSB	0x18
#define MAX17058_VRESET_LSB	0x19
#define MAX17058_TABLE_MSB	0x40
#define MAX17058_TABLE_LSB	0x7F
#define MAX17058_CMD_MSB	         0xFE
#define MAX17058_CMD_LSB	         0xFF

#define  MAX17058_ADDR	         0x6C



#define readl(addr) (*(volatile unsigned int*)(addr))
#define writel(b,addr) ((*(volatile unsigned int *) (addr)) = (b))
#define s5p_iic0_scl_set(val) \
    writel((readl(GPIO_LB_BASE + GPD1DAT_OFFSET) & (~(1 << 1))) |\
       ((val ? 1 : 0) << 1), GPIO_LB_BASE + GPD1DAT_OFFSET)  
#define s5p_iic0_sda_set(val) \
    writel((readl(GPIO_LB_BASE + GPD1DAT_OFFSET) & (~(1 << 0))) |\
       ((val ? 1 : 0) << 0), GPIO_LB_BASE + GPD1DAT_OFFSET)  
#define s5p_iic0_sda_get(val) \
(readl(GPIO_LB_BASE + GPD1DAT_OFFSET) & 0x1) 

typedef enum { 
    INPUT = 0, OUTPUT 
} io_dir;
static int  is_custom_model=0; //0:not load custom model,1:load custom model
static void s5p_iic0_sda_as(io_dir d) 
{
	writel((readl(GPIO_LB_BASE + GPD1CON_OFFSET) & (~0xF)) |\
		(d & (0x1)), GPIO_LB_BASE + GPD1CON_OFFSET);
} 

static void s5p_iic0_init() 
{
	writel((readl(GPIO_LB_BASE + GPD1CON_OFFSET) & (~0xFF)) | 0x11,\
		GPIO_LB_BASE + GPD1CON_OFFSET);
	writel(readl(GPIO_LB_BASE + GPD1PUD_OFFSET) & (~0xF),\
		GPIO_LB_BASE + GPD1PUD_OFFSET);
	writel((readl(GPIO_LB_BASE + GPD1CONPDN_OFFSET) & (~0xF)) | 0x5,\
		GPIO_LB_BASE + GPD1CONPDN_OFFSET);
	writel(readl(GPIO_LB_BASE + GPD1PUDPDN_OFFSET) & (~0xF),\
		GPIO_LB_BASE + GPD1PUDPDN_OFFSET);
	s5p_iic0_sda_set(1);
	s5p_iic0_scl_set(1);
	udelay(5);
} 

static void s5p_iic0_start() 
{
	s5p_iic0_sda_set(1);
	s5p_iic0_scl_set(1);
	udelay(5);
	s5p_iic0_sda_set(0);
	udelay(10);
	s5p_iic0_scl_set(0);
	udelay(10);
} 

static void s5p_iic0_stop() 
{
	s5p_iic0_scl_set(0);
	s5p_iic0_sda_set(0);
	udelay(5);
	s5p_iic0_scl_set(1);
	udelay(10);
	s5p_iic0_sda_set(1);
	udelay(10);
} 

static void s5p_iic0_send_ack() 
{
	s5p_iic0_sda_set(0);
	udelay(10);
	s5p_iic0_scl_set(0);
	udelay(10);
	s5p_iic0_scl_set(1);
	udelay(30);
	s5p_iic0_scl_set(0);
	udelay(20);
} 

//when IIC read,the last byte need no ack.
static void s5p_iic0_send_nack() 
{
	s5p_iic0_sda_set(1);
	udelay(10);
	s5p_iic0_scl_set(0);
	udelay(10);
	s5p_iic0_scl_set(1);
	udelay(30);
	s5p_iic0_scl_set(0);
	udelay(20);
} 

static int s5p_iic0_rcv_ack() 
{
	int i, ret = 0;
	s5p_iic0_sda_as(INPUT);
	s5p_iic0_scl_set(0);
	udelay(20);
	s5p_iic0_scl_set(1);
	udelay(20);
	for (i = 0; i < 5; i++)
		 {
		if (!s5p_iic0_sda_get())
			 {
			ret = 1;
			break;
			}
		}
	udelay(10);
	s5p_iic0_scl_set(0);
	udelay(10);
	s5p_iic0_sda_set(1);
	s5p_iic0_sda_as(OUTPUT);
	return ret;
}

static int s5p_iic0_send_byte(unsigned char val) 
{
	int i;
	for (i = 0; i < 8; i++)
		 {
		s5p_iic0_scl_set(0);
		udelay(10);
		s5p_iic0_sda_set((val & (0x80 >> i)) ? 1 : 0);
		udelay(10);
		s5p_iic0_scl_set(1);
		udelay(30);
		s5p_iic0_scl_set(0);
		udelay(10);
		}
	return s5p_iic0_rcv_ack();
}

static unsigned char s5p_iic0_rcv_byte() 
{
	int i;
	unsigned char rdata = 0;
	s5p_iic0_sda_as(INPUT);
	for (i = 0; i < 8; i++)
		 {
		s5p_iic0_scl_set(0);
		udelay(10);
		s5p_iic0_scl_set(1);
		udelay(20);
		rdata |= (s5p_iic0_sda_get() << (7 - i));
		udelay(10);
		s5p_iic0_scl_set(0);
		udelay(20);
		}
	s5p_iic0_sda_as(OUTPUT);
	return rdata;
}


static int s5p_iic0_write_byte(unsigned char chip, unsigned char reg,
			    unsigned char val) 
{
	s5p_iic0_start();
	if (!s5p_iic0_send_byte(chip))
		return -1;
	if (!s5p_iic0_send_byte(reg))
		return -1;
	if (!s5p_iic0_send_byte(val))
		return -1;
	s5p_iic0_stop();
	return 0;
}

static int s5p_iic0_write_word(unsigned char chip, unsigned char reg,
			    unsigned char low_data,unsigned char high_data) 
{
	s5p_iic0_start();
	if (!s5p_iic0_send_byte(chip))
		return -1;
	if (!s5p_iic0_send_byte(reg))
		return -1;
	if (!s5p_iic0_send_byte(low_data))
		return -1;
	if (!s5p_iic0_send_byte(high_data))
		return -1;
	s5p_iic0_stop();
	        return 0;
}
static int s5p_iic0_write_block(unsigned char chip, unsigned char reg,
					  int  len,unsigned char *value) 
 {
            int i=0;
	   unsigned char reg_value;
	  s5p_iic0_start();
	 if (!s5p_iic0_send_byte(chip))
		 return -1;
	 if (!s5p_iic0_send_byte(reg))
		return -1;
	 for(i=0;i<len;i++)
	 {
                    reg_value=*(value+i);
		 //printf("0x%.2x,",reg_value);
		 if (!s5p_iic0_send_byte(reg_value))
		         return -1;
	 }
	 s5p_iic0_stop();
		 return 0;
	  }

static int s5p_iic0_read_byte(unsigned char chip, unsigned char reg) 
{
	unsigned char datal;
	s5p_iic0_start();
	if (!s5p_iic0_send_byte(chip))
		return -1;
	if (!s5p_iic0_send_byte(reg))
		return -1;
	
//      s5p_iic0_stop();
	s5p_iic0_start();
	if (!s5p_iic0_send_byte(chip + 1))
		return -1;
	datal = s5p_iic0_rcv_byte();
	
    //s5p_iic0_send_ack();
	    
	s5p_iic0_send_nack() ;//when read, last byte need no ack
	s5p_iic0_stop();
        //printf("~~~~~~~~~chip:0x%x,reg:0x%x,datal:0x%x\n",chip,reg,datal);
	return (int)(datal);
} 

static int s5p_iic0_read_word(unsigned char chip, unsigned char reg) 
{
	unsigned char datal, datah;
	s5p_iic0_start();
	if (!s5p_iic0_send_byte(chip))
		return -1;
	if (!s5p_iic0_send_byte(reg))
		return -1;
	
//      s5p_iic0_stop();
	s5p_iic0_start();
	if (!s5p_iic0_send_byte(chip + 1))
		return -1;
	datal = s5p_iic0_rcv_byte();
	
         s5p_iic0_send_ack();   
         datah = s5p_iic0_rcv_byte();
       //s5p_iic0_send_ack();
	    
	s5p_iic0_send_nack() ; //mc,when read, last byte need no ack
	s5p_iic0_stop();
        // printf("chip:0x%x,reg:0x%x,datal:0x%x,datah:0x%x,data:0x%x\n",chip,reg,datal,datah,(datah << 8 | datal));
	return (int)(datah << 8 | datal);
} 


static const unsigned char custom_model[4][16] = 
{
    {0xA6,0xD0,0xB5,0x00,0xB8,0x20,0xB9,0xB0,0xBA,0xC0,0xBB,0xD0,0xBC,0x40,0xBC,0x70},
    {0xBC,0xB0,0xBD,0x90,0xBE,0x80,0xC0,0x80,0xC3,0xE0,0xC7,0x70,0xCB,0x90,0xCF,0xB0},
    {0x01,0xC0,0x13,0xE0,0x13,0x40,0x27,0x40,0x27,0x40,0x7F,0x00,0x94,0x40,0x6F,0x20},
    {0x2A,0x20,0x27,0x60,0x25,0xE0,0x17,0x80,0x1A,0x00,0x14,0x20,0x14,0x20,0x14,0x20}
};

static void max17058_load_lenovo_custom_model(void)
{
    int i=0,count=0;
    int SOC1;
    int SOC;
    u8  msb,lsb;
    u16 original_RCOMP,original_OCV;
  while(1)   
  {
     count++;
   //1. Unlock Model Access 
    s5p_iic0_write_word(MAX17058_ADDR, 0x3E, 0x4A,0x57);

    //2. Read RCOMP and OCV
     original_RCOMP= s5p_iic0_read_word(MAX17058_ADDR, 0x0C);
     original_OCV     = s5p_iic0_read_word(MAX17058_ADDR, 0x0E);
	//Verify Model Acess Unlocked
    if(original_OCV!=0xFFFF)
	{
	//printf("MAX17058 Unlock Model Access sucessfully.\n ");
	break;
    	}
     if(count==2)
	{
	printf("Test2 times,MAX17058 Unlock Model Access Failed.\n ");
	break;
    	}
  }

    //3.  Write OCV OCVtest=0xD9B0
     s5p_iic0_write_word(MAX17058_ADDR, 0x0E, 0xD9,0xB0);
   
    //4. Write RCOMP to a Maximum value of 0xFF00h
    s5p_iic0_write_word(MAX17058_ADDR, 0x0C, 0xFF,0x0); 
        count=0;
while(1)
{
    count++;
   //5. Write the Model
    for(i = 0; i < 4; i++)
       {
        s5p_iic0_write_block(MAX17058_ADDR, 0x40 + i * 0x10, 16, custom_model[i]); 

        // printf("===\n");
    	}

    //6. Delay at least 150mS
    udelay(150*1000); 
    
      //7.2 lock Model Access (max17048/49/58/59 only)
   s5p_iic0_write_word(MAX17058_ADDR, 0x3E, 0x0,0x0);

    //8. Delay between 150mS and 600mS
    udelay(200*1000); 
   

    //9. Read SOC Register and Compare to expected result
    SOC=s5p_iic0_read_word(MAX17058_ADDR, MAX17058_SOC_MSB);
    SOC1 =  SOC&0xFF;
     //SOCCheckA = 249,SOCCheckB = 251
    if(SOC1 >= 0xF9 && SOC1 <= 0xFB)
    {
         is_custom_model = 1;
	 printf( "[Max17058]Load  Custom Model Sucessfully.\n");
	 break;
    }
   if(count==2)
   {
	 printf("[Max17058]Test 2 times,Load Custom Model Failed.\n.");
	 break;
    }
} 

//9.1 unlock Model Access (max17048/49/58/59 only)
     s5p_iic0_write_word(MAX17058_ADDR, 0x3E, 0x4A,0x57);

    //10. Restore RCOMP and OCV,starting RCOMP=0x6F
     s5p_iic0_write_word(MAX17058_ADDR, 0x0C, 0x6F,(u8)(original_RCOMP&0xff00)>>8); 
     msb=(u8)(original_OCV&0xff);
     lsb=(u8)(original_OCV&0xff00)>>8;
     s5p_iic0_write_word(MAX17058_ADDR, 0x0E, msb,lsb);
    //11. Lock Model Access
     s5p_iic0_write_word(MAX17058_ADDR, 0x3E, 0x0,0x0);
     udelay(150*1000);
}

static void  max17058_correct_init_OCV(int charger_on)
{
 //1.needed variables
        int  count=0;
        int  read_data=0,vcell1,vcell2,OCV,Desired_OCV;
 //2.read vcell1 sample
	read_data = s5p_iic0_read_word(MAX17058_ADDR, MAX17058_VCELL_MSB);
	vcell1=((read_data<<8)&0xff00)|((read_data>>8)&0xff);
	//printf("read_data+++0x%.4x\n",read_data);
	//printf("vcell1+++0x%.4x\n",vcell1);
	udelay(125*1000);
 //3.read vcell2 sample
        read_data = s5p_iic0_read_word(MAX17058_ADDR, MAX17058_VCELL_MSB);
	vcell2=((read_data<<8)&0xff00) |((read_data>>8)&0xff);
	//printf("read_data+++0x%.4x\n",read_data);
	//printf("vcell2+++0x%.4x\n",vcell1);
     while(1)   
  {
      count++;
 //4. Unlock Model Access 
      s5p_iic0_write_word(MAX17058_ADDR, 0x3E, 0x4A,0x57);

 //5.Read  OCV
      read_data = s5p_iic0_read_word(MAX17058_ADDR, 0x0E);
      OCV  =((read_data<<8)&0xff00)|((read_data>>8)&0xff);
      //printf("read_data+++0x%.4x\n",read_data);
      //printf("OCV+++0x%.4x\n",OCV);
 //6.Verify Model Acess Unlocked
    if(read_data!=0xFFFF)
	{
	//printf("[Max17058 ]Correct OCV:Unlock Model Access sucessfully.\n ");
	break;
    	}
     if(count==2)
	{
	//printf("[Max17058] Test 2 times,Correct OCV: Unlock Model Access Failed.\n ");
	break;
    	}
  }

    if(!charger_on){//without charger
 //7. determin maxinum for desired OCV
	    if(max(vcell1,vcell2)>=OCV)
		{
		   Desired_OCV = max(vcell1,vcell2) + 640;//50mV
		   //printf("VCELL1+++0x%x\n",Desired_OCV);
		 }
	    else if(OCV>max(vcell1,vcell2)+128*8){
		   Desired_OCV = max(vcell1,vcell2) + 640;
		 }
	    else
		{
		   Desired_OCV=OCV + 128;//10mV
		   //printf("OCV+++0x%x\n",Desired_OCV);
		}
   }
   else{//with charger
	if(OCV > max(vcell1,vcell2)){ //maybe the charger is plugged in firstly and then battery,0xD100=4180mV
		Desired_OCV = min(vcell1,vcell2)-128*2;//256:20mV
	}
	else if(OCV+128*5 < min(vcell1,vcell2)){//maybe the OCV value is last low battery value
		Desired_OCV = min(vcell1,vcell2)-128*2;//256:20mV
	}
	else
		Desired_OCV=OCV + 128;
   }
 //8.write OCV
      s5p_iic0_write_word(MAX17058_ADDR, 0x0E,(Desired_OCV>>8)&0xff, Desired_OCV&0xff);
      //printf("read_data+++0x%x\n",s5p_iic0_read_word(MAX17058_ADDR, 0x0E));
      //printf("write_data+++0x%x,0x%x\n",Desired_OCV&0xff,(Desired_OCV>>8)&0xff);
 //9.lock Model Access 
      s5p_iic0_write_word(MAX17058_ADDR, 0x3E, 0x0,0x0);
      udelay(125*1000);
	 
}
 void max17058_init(int charger_on) 
{
	s5p_iic0_init();
	int  version;
	
	 //reset and get verion
	
	//s5p_iic0_write_word(MAX17058_ADDR,MAX17058_CMD_MSB,0x54,0x00);	//compeletly reset
	//udelay(100*1000);
	//version=s5p_iic0_read_word(MAX17058_ADDR, MAX17058_VER_MSB);//version
	//version=((version>>8)&0xff)|((version<<8)&(0xff00));
	//printf("[Max17058] version is 0x%.4x\n",version );
	 //s5p_iic0_read_word(0x6c, 0xfe);
	s5p_iic0_write_word(MAX17058_ADDR,MAX17058_VRESET_MSB,0x0,0xaf); //set the voltage3.48v below which ICs resets
          //s5p_iic0_read_word(0x6c, 0x18);
		  
	s5p_iic0_write_word(MAX17058_ADDR,MAX17058_CONFIG_MSB,0x6f,0x1f); //RCOMP,SLEEP,ALERT set
         // s5p_iic0_read_word(0x6c, 0x0c);

	 max17058_load_lenovo_custom_model();  //Load Custom Model
         max17058_correct_init_OCV(charger_on);    //Correct Corrupted Initial Voltage Reading

}


int max17058_get_soc(void) 
{
        int read_data,soc=0;
	 s5p_iic0_init();
	 read_data = s5p_iic0_read_word(MAX17058_ADDR, MAX17058_SOC_MSB);
         // printf("soc====%.4x\n", read_data);
	if(!is_custom_model)
	          soc=min((((read_data&0xff)<<8)|((read_data>>8)&0xff))/256,100);
	else
	          soc=min((((read_data&0xff)<<8)|((read_data>>8)&0xff))/512,100);
        if(soc>=60) 
                  soc=min((soc*11-56)/10,100);     //set 96% for full(100%),the same as kernel         
	
	printf("[Max17058] The Battery SOC is %d\n",soc);//soc is 1%/256cells
	return  soc;
}

int max17058_get_vcell(void)
{
	int read_data,vcell;

	s5p_iic0_init(); 
	read_data = s5p_iic0_read_word(MAX17058_ADDR, MAX17058_VCELL_MSB);
	vcell=(read_data&0xff)*20+((read_data>>8)&0xff)*10/128;//78.125uV/cell  78.125uV*2^8=20mV
        if(vcell>4200)
	  printf("[MAX17058]Please check!The Battery maybe not inserted into.  \n" );	  
	else
	  printf("[MAX17058]The Battery Vcell is %dmV \n",vcell );
	return vcell;
}



