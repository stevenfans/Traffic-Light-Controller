// ***** 0. Documentation Section *****
// Lab3.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
//#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
//void DisableInterrupts(void); // Disable interrupts
//void EnableInterrupts(void);  // Enable interrupts
void Init_PortB();
void Init_PortE();
void Delay();
void SysTick_half(unsigned long delay);

// ***** 3. Subroutines Section ****
int main(void){ 
	unsigned int In, Blink, walk, y;
  //TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MH

  Init_PortB();
  Init_PortE();
	SysTick_Init();
	
	// turn on red led
	GPIO_PORTB_DATA_R |=  0x04;
	GPIO_PORTB_DATA_R &= ~0x03;
	
	//another way to do multiple bits setup
	//GPIO_PORTB_DATA_R &= ~0x07;
	//GPIO_PORTB_DATA_R &=  0x04;
	
  //EnableInterrupts();
	
	// walk variable
	walk = GPIO_PORTE_DATA_R & 0x04; 
	
  while(1){
		//Delay();
			SysTick_half(1);
		
  	// assume use bit 1 on portE for switch 2
		 Blink = GPIO_PORTE_DATA_R & 0x02; //read PE1 into In
		 if (Blink == 0x02){              // zero means SW1 is pressed
			 y = GPIO_PORTB_DATA_R;
			 GPIO_PORTB_DATA_R = GPIO_PORTB_DATA_R ^ GPIO_PORTB_DATA_R; 
			 //Delay();
			 SysTick_half(1);
			 GPIO_PORTB_DATA_R = y;
			 //Delay();
			 SysTick_half(1);
						// zero means SW1 is pressed
			/* GPIO_PORTB_DATA_R >>= 0x01; // changes to yellow then green
			 
			 if (GPIO_PORTB_DATA_R == 0x00){
				 GPIO_PORTB_DATA_R = 0x04;
			 }*/
			 
		 }
		// assume use bit 0 on portE for switch 1
		In = GPIO_PORTE_DATA_R & 0x01; //read PE0 into In
		 if(In == 0x01){              // zero means SW1 is pressed
			GPIO_PORTB_DATA_R >>= 0x01; // changes to yellow then green
			 
			 if (GPIO_PORTB_DATA_R == 0x00){
				 GPIO_PORTB_DATA_R = 0x04;
			 }
		 }
		}	 
}
void Init_PortB(void){
	unsigned int delay;
  SYSCTL_RCGC2_R |= 0x00000002;     // 1) B clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTB_CR_R = 0x07;           // allow changes to PB2-0      
  GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTB_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTB_DIR_R = 0x07;          // 5) PB2-output 
  GPIO_PORTB_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTB_PUR_R = 0x00;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTB_DEN_R = 0x07;          // 7) enable digital pins PB2-PB0        
}
void Init_PortE(void){
	unsigned int delay;
  SYSCTL_RCGC2_R |= 0x00000010;     // 1) E clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTE_CR_R = 0x03;           // allow changes to PE-0       
  GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R = 0xFC;         // 5) PE2-input 
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) no alternate function
 // GPIO_PORTE_PUR_R = 0x00;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTE_DEN_R = 0x03;          // 7) enable digital pins PFE-PF0  
}
void Delay(void){unsigned long volatile time;
  //time = (727240*200/91)/5;  // 0.1sec * 5 = .5 sec
	time = (727240*200/91);
  while(time){
		time--;
  }
}
