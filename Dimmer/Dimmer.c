/*******************************************************
This program was created by the
CodeWizardAVR V3.12 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 01.07.2017
Author  : (Silvia)
Company : 
Comments: 


Chip type               : ATmega8A
Program type            : Application
AVR Core Clock frequency: 16,000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*******************************************************/

#include <mega8.h>
#include <delay.h>
#include <stdio.h>
      
#define LOAD_0   PORTC.2
#define LOAD_1   PORTC.3 
#define LOAD_2   PORTC.4
#define LOAD_3   PORTB.2
#define LOAD_4   PORTB.1
#define LOAD_5   PORTB.0
#define LOAD_6   PORTD.7
#define LOAD_7   PORTD.6  
#define RTS_PIN  PORTD.2

#define TIMERVALUE  0xFF41 //- для таймера-кроку 1% з 255 на 2мгц
#define PERCENT_STEP   1
#define MAXPERCENT     255
#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))
#define resetIn PINB.5
#define powerOff PINC.1

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)

#define delayMax 2000
//
//#define AdrPWM 0x80


unsigned char tmrStart = 0;
unsigned char high = 0;
unsigned int currentPercent = MAXPERCENT;
unsigned char loadPercents[8] = {10,250,245,50,100,140,0,200};
unsigned char bounceTime=30;
unsigned int millisecond;
unsigned int UartSpeed[7]={416,207,103,51,25,16,8}; 
unsigned long int UartSpeedNormal[7]={2400,4800,9600,19200,38400,57600,115200};   
unsigned char speed=3;
unsigned char byteCountStop=5;
unsigned char buffer[256];
unsigned char count=0;
unsigned int timeStop=0;
unsigned char inHoldByte=0;
unsigned int UartSettReg=0;
unsigned char adress=1;
unsigned char UartMode=0;
unsigned char inOld[8]={1,1,1,1,1,1,1,1};
unsigned char bounceTimeButton[8]={0,0,0,0,0,0,0,0};
unsigned char PWMSetValue[8]={0,0,0,0,10,0,0,0};
unsigned char loadPercentsB[8]={0,0,0,0,0,0,0,0};
unsigned long int buttonMsCount[8]={0,0,0,0,0,0,0,0};
unsigned char outStatus[8]={0,0,0,0,0,0,0,0};
eeprom unsigned int eUartSettReg;
eeprom unsigned char ePWMSetValue[8];
eeprom unsigned char eOutStatus[8];

bit start=0;
bit commandDone=1;
unsigned char sp=0;
unsigned int timeToStartSave=500;
unsigned char dir[8]={0,0,0,0,0,0,0,0};
unsigned char distPressed[8]={1,1,1,1,1,1,1,1};

unsigned int TimeStopFunc()
{ 
  long int a=80000*byteCountStop/UartSpeedNormal[speed]; 
  return a%65536;
}
unsigned char in(unsigned char a)
{
 switch (a) {
    case 0:
		return !(!distPressed[0]);
		break;  
    case 1:
		return !(!distPressed[1]);
		break;
    case 2:                   
		return !(!distPressed[2]);
		break;
    case 3:
		return !(!distPressed[3]);
		break;
    case 4:
		return !(!distPressed[4]);
		break;
    case 5:
		return !(!distPressed[5]);
		break;
    case 6:
		return !(!distPressed[6]);
		break;
    case 7:
		return !(!distPressed[7]);
		break;
    }; 
 return 0;
}
unsigned char inHold(unsigned char a)
{
 unsigned char b=((inHoldByte>>a)%2)<<a; 
 unsigned char c=(inHoldByte>>a)%2;
 inHoldByte=inHoldByte^b;
 return c;
 
}


unsigned int holdingReg(unsigned char a)
{
 if(a==0)return UartSettReg;
 else if(a<9)
 {
  int b;
  a--; 
  b=PWMSetValue[a];
  return b; 
 }  
 else if(a==9)
 {
  int b=0; 
  int i;
  for(i=0;i<8;i++)
  { 
   if(outStatus[i]==1)b+=(1<<(15-i));   
   if(distPressed[i]==0)b+=(1<<(7-i));
  }    
  return b; 
 }
 return 0;
}
void holdingRegWrite(unsigned char a, unsigned long int b)
{
 if(a==0)
 {
  UartSettReg=b;
  eUartSettReg=b;
 }
 else if(a<9)
 {
  int c; 
  a--;   
  c=b & 0xFF;
  PWMSetValue[a]=c;
  ePWMSetValue[a]=c; 
 }   
 else if(a==9)
 {
  int c;    
  int i=0;
  c=b>>8;
  for(i=0;i<8;i++)
  { 
   if((c>>(7-i))%2==0x01)
   { 
    if(PWMSetValue[i]==0) PWMSetValue[i]=179;   
    outStatus[i]=1-outStatus[i]; 
   }  
  }
  c=b%256;
  for(i=0;i<8;i++)
  { 
   if((c>>(7-i))%2==0x01) 
   {
    if(distPressed[i]==1)
    {     
     if(buttonMsCount[i]<1500) buttonMsCount[i]=1500;
     distPressed[i]=0;
    } 
    else 
    {
     distPressed[i]=1;
    } 
   }
  }
 }
}

unsigned int crc(unsigned char* data, unsigned char length)
{
int j; 
unsigned int reg_crc=0xFFFF; 
while(length--)
{ 
reg_crc ^= *data++; 
for(j=0;j<8;j++)
{ 
if(reg_crc & 0x01)reg_crc=(reg_crc>>1)^0xA001; 
else reg_crc=reg_crc>>1; 
} 
} 
return reg_crc; 
}

void ReadDiscreteInputs()
{
 if(count==8)
 {
  if(crc(buffer,count-2)==((buffer[7]<<8) + buffer[6]))
  {  
   unsigned int firstInput=buffer[3]+(buffer[2]<<8);
   unsigned int countInput=buffer[5]+(buffer[4]<<8); 
   if(firstInput + countInput <= 0x04 && countInput > 0x00)
   {  
    int i=0; 
    unsigned char data=0;
    for(i=0;i<countInput;i++)
    {  
     data+=(inHold(i+firstInput)<<i);    
    }  
    buffer[0]=adress; 
    buffer[2]=0x01;
    buffer[3]=data; 
    buffer[4]=crc(buffer,4)%256;
    buffer[5]=crc(buffer,4)>>8; 
    for(i=0;i<6;i++)putchar(buffer[i]);
   }
   else if(firstInput>0x07) 
   {  
    int i; 
    buffer[0]=adress;
    buffer[1]+=(1<<7);
    buffer[2]=0x02; 
    buffer[3]=crc(buffer,3)%256;
    buffer[4]=crc(buffer,3)>>8; 
    for(i=0;i<5;i++)putchar(buffer[i]);
   }  
   else
   {
    int i;  
    buffer[0]=adress;
    buffer[1]+=(1<<7);
    buffer[2]=0x03; 
    buffer[3]=crc(buffer,3)%256;
    buffer[4]=crc(buffer,3)>>8; 
    for(i=0;i<5;i++)putchar(buffer[i]);
   }
  }
 }
}
void ReadHoldingRegisters()
{
 if(count==8)
 {
  if(crc(buffer,count-2)==((buffer[7]<<8) + buffer[6]))
  {  
   unsigned int firstReg=buffer[3]+(buffer[2]<<8);
   unsigned int countReg=buffer[5]+(buffer[4]<<8); 
   if(firstReg + countReg <= 0x0A && countReg > 0x00)
   {  
    int i=0;  
    buffer[2]=countReg*2;
    for(i=0;i<countReg;i++)
    {  
     buffer[i*2+3]=(holdingReg(i+firstReg)>>8);   
     buffer[i*2+4]=(holdingReg(i+firstReg)%256);
    }
    i=countReg*2+3;  
    buffer[0]=adress;
    buffer[i]=crc(buffer,i)%256;
    buffer[i+1]=crc(buffer,i)>>8; 
    for(i=0;i<countReg*2+5;i++)putchar(buffer[i]);
   }
   else if(firstReg>0x14) 
   {  
    int i; 
    buffer[0]=adress;
    buffer[1]+=(1<<7);
    buffer[2]=0x02; 
    buffer[3]=crc(buffer,3)%256;
    buffer[4]=crc(buffer,3)>>8; 
    for(i=0;i<5;i++)putchar(buffer[i]);
   }  
   else
   {
    int i; 
    buffer[0]=adress;
    buffer[1]+=(1<<7);
    buffer[2]=0x03; 
    buffer[3]=crc(buffer,3)%256;
    buffer[4]=crc(buffer,3)>>8; 
    for(i=0;i<5;i++)putchar(buffer[i]);
   }
  }
 }
}
void ReadInputRegisters()
{
 if(count==8)
 {
  if(crc(buffer,count-2)==((buffer[7]<<8) + buffer[6]))
  {  
   int firstReg=buffer[3]+(buffer[2]<<8);
   int countReg=buffer[5]+(buffer[4]<<8); 
   if(firstReg + countReg <= 0x04 && countReg > 0x00)
   {  
    int i=0;  
    buffer[2]=countReg*2;
    for(i=0;i<countReg;i++)
    {
     unsigned char a=inHold(i+firstReg);
     if(a==1)a=0xFF;  
     buffer[i*2+3]=a;   
     buffer[i*2+4]=a;
    }
    i=countReg*2+3; 
    buffer[0]=adress; 
    buffer[i]=crc(buffer,i)%256;
    buffer[i+1]=crc(buffer,i)>>8; 
    for(i=0;i<countReg*2+5;i++)putchar(buffer[i]);
   }
   else if(firstReg>0x07) 
   {  
    int i;  
    buffer[0]=adress;
    buffer[1]+=(1<<7);
    buffer[2]=0x02; 
    buffer[3]=crc(buffer,3)%256;
    buffer[4]=crc(buffer,3)>>8; 
    for(i=0;i<5;i++)putchar(buffer[i]);
   }  
   else
   {
    int i;  
    buffer[0]=adress;
    buffer[1]+=(1<<7);
    buffer[2]=0x03; 
    buffer[3]=crc(buffer,3)%256;
    buffer[4]=crc(buffer,3)>>8; 
    for(i=0;i<5;i++)putchar(buffer[i]);
   }
  }
 }
}
void WriteSingleRegister()
{
 if(count==8)
 {
  if(crc(buffer,count-2)==((buffer[7]<<8) + buffer[6]))
  {  
   unsigned int firstReg=buffer[3]+(buffer[2]<<8);
   unsigned int stateReg=buffer[5]+(buffer[4]<<8); 
   if(firstReg <= 0x09)
   {  
    int i=0; 
    holdingRegWrite(firstReg,stateReg);
    buffer[0]=adress;
    buffer[6]=crc(buffer,6)%256;
    buffer[7]=crc(buffer,6)>>8; 
    for(i=0;i<8;i++)putchar(buffer[i]);
   }  
   else
   {  
    int i;
    buffer[0]=adress;
    buffer[1]+=(1<<7);
    buffer[2]=0x02; 
    buffer[3]=crc(buffer,3)%256;
    buffer[4]=crc(buffer,3)>>8; 
    for(i=0;i<5;i++)putchar(buffer[i]);
   }  
  }
 }
}

void ChooseFunction()
{
 switch (buffer[1]) {    
    case 0x01: 
    break;
    case 0x02: 
     ReadDiscreteInputs();
    break;
    case 0x03: 
     ReadHoldingRegisters();
    break; 
    case 0x04: 
     ReadInputRegisters();
    break;     
    case 0x05: 
    break;
    case 0x06: 
     WriteSingleRegister();
    break;
    case 0x0F: 
    break;
    };      
 commandDone=1;
}

interrupt [USART_RXC] void usart_rx_isr(void)
{
	char data,status;
	data=UDR;
	status=UCSRA;
	if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
	{
		if(!start && commandDone && millisecond>timeStop)
		{
			 count=0;
			 if(data==adress || data==0)
			 { 
				start=1;   
			 }
		}
		if(start)
		{
			 buffer[count]=data;
			 count++;
		}
		millisecond=0;
	}
}

//void pwm_init()
//{
//	 PcaWrite(0x01,0x04);  
//	 PcaWrite(0x00,0x10);  
//	 delay_ms(5);
//	 PcaWrite(0xFE,0x03);  
//	 PcaWrite(0x00,0x81);   
//	 delay_ms(5);
//}


void saveData()
{
	 int i=0;   
	 #asm("wdr") 
	 for(i=0;i<8;i++)
	 {
		  ePWMSetValue[i]=PWMSetValue[i]; 
		  #asm("wdr")     
		  eOutStatus[i]=outStatus[i];     
		  #asm("wdr") 
	 }   
}

void ButtonRising(unsigned char a)
{
	 if(bounceTimeButton[a]==0)
	 { 
		  bounceTimeButton[a]=bounceTime; 
		  buttonMsCount[a]=1; 
	 }                                 
}
void ButtonFalling(unsigned char a)
{
	 if(bounceTimeButton[a]==0)
	 { 
		  bounceTimeButton[a]=bounceTime;  
		  if(buttonMsCount[a]<1500)
		  {
			   outStatus[a]=1-outStatus[a]; 
			   if(outStatus[a]==1)dir[a]=0; 
			   buttonMsCount[a]=0;
			   if(outStatus[a]==1 && PWMSetValue[a]==0)PWMSetValue[a]=179;    
		  }
		  else
		  {   
			   unsigned int b=buttonMsCount[a]-1500;  
			   if(dir[a]<2)dir[a]=2;
			   else dir[a]=0;    
			   if(b>5100 && b<=5100+delayMax)b=5100;
			   else if(b>5100)b-=(5100+delayMax),b=5100-b,dir[a]=2-dir[a];
			   b=b/20; 
			   PWMSetValue[a]=b;      
			   outStatus[a]=1;
			   loadPercents[a]=b;
			   buttonMsCount[a]=0;  
		  } 
	 }                   
}

interrupt [TIM2_COMP] void timer2_comp_isr(void)
{
  unsigned char i=0;
  if(timeToStartSave>0)timeToStartSave--;
  #asm("wdr") 
  sp++;
  if(sp>2)sp=0;
  for(i=0;i<8;i++)  
  { 
	  bit c=in(i);
	  if(c==0 && inOld[i]==1)ButtonRising(i);
	  else if(c==1 && inOld[i]==0)ButtonFalling(i); 
	  if(bounceTimeButton[i]>0)bounceTimeButton[i]--;
	  inOld[i]=c;    
	  if(sp==0)
	   {
			if(outStatus[i]==1 && loadPercents[i]<PWMSetValue[i]) 
				loadPercents[i]++;
			else if(outStatus[i]==1 && loadPercents[i]>PWMSetValue[i]) 
				loadPercents[i]--;
			else if(outStatus[i]==0 && loadPercents[i]>0) 
				loadPercents[i]--;  
			if(buttonMsCount[i]<1500)
				loadPercentsB[i]=loadPercents[i];     
	   }
  }                 
  if(millisecond<1000) 
	  millisecond++;  
  if(start && millisecond>timeStop)
  {
	   start=0;
	   commandDone=0;
  }
  for(i=0;i<8;i++) if(buttonMsCount[i]>0)
  {   
	   if(dir[i]<2)
	   {
			if(outStatus[i]==1 && dir[i]==0 && buttonMsCount[i]==1500)
				buttonMsCount[i]=1500+PWMSetValue[i]*20 , dir[i]=1;
			else if(outStatus[i]==0 && dir[i]==0 && buttonMsCount[i]==1500)
				buttonMsCount[i]=1500 , dir[i]=1;
			if(buttonMsCount[i]<6600+delayMax+5100)
				buttonMsCount[i]++;          
			else 
				buttonMsCount[i]=1500;      
		   }        
		   else 
		   {    
			if(buttonMsCount[i]<1500)
				buttonMsCount[i]++;
			else if(buttonMsCount[i]>1500)
				buttonMsCount[i]--;          
			else if(dir[i]==2 && outStatus[i]==1)
				buttonMsCount[i]=1500+PWMSetValue[i]*20 , dir[i]=3;  
			else if(dir[i]==2)
				buttonMsCount[i]=1500 , dir[i]=3;
			else 
				buttonMsCount[i]=6600+delayMax+5100;  
		   }
	  }   
}


void main(void)
{
    int i;

	// Input/Output Ports initialization
	// Port B initialization
	// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=Out Bit1=Out Bit0=Out 
	DDRB=(1<<DDB2) | (1<<DDB1) | (1<<DDB0);
	// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=0 Bit1=0 Bit0=0 
	PORTB= 0;

	// Port C initialization
	// Function: Bit6=In Bit5=In Bit4=Out Bit3=Out Bit2=In Bit1=In Bit0=In 
	DDRC =(1<<DDC4) | (1<<DDC3);
	// State: Bit6=T Bit5=T Bit4=0 Bit3=0 Bit2=T Bit1=T Bit0=T 
	PORTC = 0;

	// Port D initialization
	// Function: Bit7=Out Bit6=Out Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In 
	DDRD=(1<<DDD7) | (1<<DDD6)| (0<<DDD5)| (0<<DDD3);
	// State: INT1 (0 DETECTOR)=P 
	PORTD=(1<<PORTD3) |(1<<PORTD0);

	// Timer/Counter 1 initialization
	// Clock source: System Clock
	// Clock value: 16000,000 kHz
	// Mode: Normal top=0xFFFF
	// OC1A output: Disconnected
	// OC1B output: Disconnected
	// Noise Canceler: Off
	// Input Capture on Falling Edge
	// Timer Period: 0,5 ms
	// Timer1 Overflow Interrupt: On
	// Input Capture Interrupt: Off
	// Compare A Match Interrupt: Off
	// Compare B Match Interrupt: Off
	TCCR1B=(0<<CS12) | (1<<CS11) | (0<<CS10);    //(0<<CS12) | (1<<CS11) | (0<<CS10); - 2MHZ
	TCNT1H= 0;     //????? ???????
	TCNT1L= 0;

		

	// Timer(s)/Counter(s) Interrupt(s) initialization
	TIMSK=(1<<TOIE1);

	// External Interrupt(s) initialization
	// INT0: Off
	// INT1: On
	// INT1 Mode: Falling Edge
	GICR|=(1<<INT1);
	MCUCR=(1<<ISC11);
	GIFR=(1<<INTF1);


	// Analog Comparator initialization
	// Analog Comparator: Off
	// The Analog Comparator's positive input is
	// connected to the AIN0 pin
	// The Analog Comparator's negative input is
	// connected to the AIN1 pin
	ACSR=(1<<ACD);


	///////////////////////////////////////////
	//USART SETUP
	/////////////////////////////////////////// 
	if(!resetIn) eUartSettReg=0xFFFF;

	if(eUartSettReg==0xFFFF)
	{
		int i=0;
		eUartSettReg=0x0103;
		for(i=0;i<8;i++)
		 {
			  ePWMSetValue[i]=0x00;  
			  eOutStatus[i]=0;
		 }
	}
	UartSettReg=eUartSettReg;
	if(((UartSettReg%256)>>4)>2 || ((UartSettReg%256)%16)>6)
	{
		 UartSettReg=0x0103;
		 eUartSettReg=0x0103;
	}
	adress=UartSettReg>>8;
	speed=((UartSettReg%256)%16);
	UartMode=((UartSettReg%256)>>4);
	{
	 int i;
	 for(i=0;i<8;i++)
	 {
		  PWMSetValue[i]=ePWMSetValue[i];
		  outStatus[i]=eOutStatus[i];
	 }
	}
	UCSRA=(0<<RXC) | (0<<TXC) | (0<<UDRE) | (0<<FE) | (0<<DOR) | (0<<UPE) | (0<<U2X) | (0<<MPCM);
	UCSRB=(1<<RXCIE) | (0<<TXCIE) | (0<<UDRIE) | (1<<RXEN) | (1<<TXEN) | (0<<UCSZ2) | (0<<RXB8) | (0<<TXB8);
	if(UartMode==0)UCSRC=(1<<URSEL) | (0<<UMSEL) | (0<<UPM1) | (0<<UPM0) | (0<<USBS) | (1<<UCSZ1) | (1<<UCSZ0) | (0<<UCPOL);
	else if(UartMode==1)UCSRC=(1<<URSEL) | (0<<UMSEL) | (1<<UPM1) | (0<<UPM0) | (0<<USBS) | (1<<UCSZ1) | (1<<UCSZ0) | (0<<UCPOL);
	else UCSRC=(1<<URSEL) | (0<<UMSEL) | (1<<UPM1) | (1<<UPM0) | (0<<USBS) | (1<<UCSZ1) | (1<<UCSZ0) | (0<<UCPOL);
	UBRRH=UartSpeed[speed]>>8;
	UBRRL=UartSpeed[speed]%256;


	// Watchdog Timer initialization
	// Watchdog Timer Prescaler: OSC/64k

	timeStop=TimeStopFunc();

	#pragma optsize-
	WDTCR=(1<<WDCE) | (1<<WDE) | (0<<WDP2) | (1<<WDP1) | (0<<WDP0);
	WDTCR=(0<<WDCE) | (1<<WDE) | (0<<WDP2) | (1<<WDP1) | (0<<WDP0);
	#ifdef _OPTIMIZE_SIZE_
	#pragma optsize+
	#endif
	 //i2c_init();
	#asm("sei")

    while (1)
    {    
        if (tmrStart == 1){
            tmrStart = 0;
            
            TCNT1H= TIMERVALUE >> 8;
            TCNT1L= TIMERVALUE & 0xff;
        }
        
        if (high == 1){
            high = 0;
             
            for (i = 0; i<8;i++){
                if (loadPercents[i] == currentPercent){
                    
                    if      (i== 0){ LOAD_0 = 1; }
                    else if (i== 1){ LOAD_1 = 1; }    
                    else if (i== 2){ LOAD_2 = 1; } 
                    else if (i== 3){ LOAD_3 = 1; }
                    else if (i== 4){ LOAD_4 = 1; }
                    else if (i== 5){ LOAD_5 = 1; }
                    else if (i== 6){ LOAD_6 = 1; } 
                    else if (i== 7){ LOAD_7 = 1; }                
                }           
            }
            delay_us(40);
             LOAD_0 = 0; 
                LOAD_1 = 0;
                LOAD_2 = 0;
                LOAD_3 = 0; 
                LOAD_4 = 0;
                LOAD_5 = 0;
                LOAD_6 = 0; 
                LOAD_7 = 0;
                              
             if (currentPercent <= 1){
                TCNT1H=TIMERVALUE >> 16;
                TCNT1L=TIMERVALUE & 0x00;
                currentPercent = MAXPERCENT;                               
                
            }
            else{
                currentPercent -= 1; 
            }             
        } 
        
              
    }
  }
