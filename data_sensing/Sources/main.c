// filename ******** Main.C **************
 
//***********************************************************************
// Simple ADC example for the Technological Arts EsduinoXtreme board
// by Carl Barnes, 12/03/2014
//***********************************************************************
 
#include <hidef.h>  	/* common defines and macros */
#include <string.h>
#include "derivative.h"  /* derivative information */
#include "SCI.h"
 
char string[20];
unsigned short xval; 
unsigned short yval;
int num;
int theta;
int theta2;
int scaledX;
int scaledY;
int collecting;
int i;
int j;
int tens;
int ones;
int mode;
void msDelay(unsigned int);
void setClk(void);
//---------------------OutCRLF---------------------
// Output a CR,LF to SCI to move cursor to a new line
// Input: none
// Output: none
// Toggle LED each time through the loop
 
void OutCRLF(void){
  SCI_OutChar(CR);
  SCI_OutChar(LF);
  PTJ ^= 0x20;      	// toggle LED D2
}
 
void main(void) {
// Setup and enable ADC channel 0
// Refer to Chapter 14 in S12G Reference Manual for ADC subsystem details

  setClk();
  
  mode = 0;//start in mode 0	
	ATDCTL0 = 0x06;   //sets wraparound for ADC channel conversion
	ATDCTL1 = 0x2F; 	
	ATDCTL3 = 0x84; 	// right justified, one sample per sequence
	ATDCTL4 = 0x04; 	// prescaler = 4; ATD clock = 2MHz
  ATDCTL5 = 0x30;   //sets channel to start on (0), increments up and converts one by one

  TSCR1 = 0x90;    //Timer System Control Register 1
  TSCR2 = 0x04;    //Timer System Control Register 2             
  TIOS = 0xFA;     //Timer Input Capture or Output capture
  PERT = 0x05;     //Enable Pull-Up resistor on TIC[0] and TIC[2]
  TCTL3 = 0x00;    //TCTL3 & TCTL4 configure which edge(s) to capture (falling edge used)
  TCTL4 = 0x22;    //Configured for falling edge on TIC[0] (channel 0) and TIC[2] (channel 2)
  
  TIE = 0x05;      //Timer Interrupt Enable
	EnableInterrupts; //CodeWarrior's method of enabling interrupts	

 
//Setup LED and SCI
  DDRJ |= 0x01; 	// PortJ bit 0 is output to LED D2 on DIG13
  SCI_Init(9600);
  
  collecting = 0;//start with data collectin off
   
  
  DDR0AD = 0x0F;//configures bits 8-11 of port AD to output
  DDR1AD = 0x0F;//configures bits 0-3 of port AD to output 
                
  for(;;) {
    
	  if(collecting == 1){
	    if(mode ==1){
	      
  	    yval=ATDDR5; //reads from ADC channel
      	theta2 = (int)yval*(180/31);//intial scaling from project requirements 
      	     
        if(theta2>1660){//edge cases from angle calues <0 and >90
         tens = 0;
         ones = 0; 
        } else if(theta2<1335){
         tens = 9;
         ones = 0; 
        }else{
          
          scaledY = (int)(((((theta2-1660)*100)/-340)*90));//scales the data relative to desired 0 angle, converting to a 0-90 angle
          tens = scaledY/1000; //gets the tens column
          ones = (scaledY/100)%10;//gets the oens column
        }
        
        SCI_OutUDec(scaledY/100);//serially communicates both the tens and ones column to matlab
        OutCRLF();

        
	    }else{
      	xval=ATDDR6; //reads from ADC channel
      	
        theta = (int)xval*(180/31); //intial scaling from project requirements 
        
        if(theta>1705){//edge cases from angle calues <0 and >90
         tens = 0;
         ones = 0; 
        } else if(theta<1380){
         tens = 9;
         ones = 0; 
        }else{
          
          scaledX = (int)(((((theta-1705)*100)/-340)*90));//scales the data relative to desired 0 angle, converting to a 0-90 angle
          tens = scaledX/1000; //gets the tens column
          ones = (scaledX/100)%10;//gets the oens column
        }
        SCI_OutUDec(scaledX/100);//serially communicates both the tens and ones column to matlab
        OutCRLF();
	    }
	    
    	PT0AD = tens;
    	PT1AD = ones; //sends tens and ones number to LED output

   	  msDelay(700);	//adds delay 
	  } else{
	    
	    SCI_OutUDec(100);//if not gathering data, sends a known number to matlab so it knows data is not being collected
	    OutCRLF();
	  }
  } 
}


interrupt  VectorNumber_Vtimch0 void ISR_Vtimch0(void)//interrupt to switch between mdoes 0 and 1
{
  unsigned int temp; 
  
  if(mode ==0){
    PTJ = 0x01;
    mode = 1; 
  } else{
    PTJ = 0x00;
    mode = 0; 
  }
  msDelay(700);
  
  temp = TC0;       
  }
  
interrupt  VectorNumber_Vtimch2 void ISR_Vtimch2(void)//interrupt to enable/disable data collection
{
  unsigned int temp;
  
  if(collecting ==0){
    collecting = 1; 
  } else{
    collecting = 0; 
  }
   
  msDelay(700); 
  temp = TC2;     
  }  

   
void msDelay(unsigned int time)
{
	unsigned int j,k;
	for(j=0;j<time;j++)
    	for(k=0;k<1033;k++);
}

void setClk(void){
  CPMUPROT = 0x26; //Allows clock to be configured
  CPMUCLKS = 0x80; //sets source of eclock 
  CPMUOSC = 0x00; //Sets f_IRC to be reference frequency, 1MHz
  CPMUREFDIV = 0x00;    //sets reference to 1MHz, so oscillator reference divisor not needed
  CPMUSYNR = 0x1F; //2*1MHz*(31+1) = 64MHz
  CPMUPOSTDIV=0x03; //64MHz/(3+1) = 16MHz               .
                    //Bus clock speed is F_BUS=F_PLL/2=8MHz  
  
  while (CPMUFLG_LOCK == 0) {}  //Wait for PLL to achieve desired tolerance of target frequency. NOTE: For use when the source clock is PLL. comment out when using external oscillator as source clock 
 
  CPMUPROT = 1; //disables clock configuration to avoid errors
}



