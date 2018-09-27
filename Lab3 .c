// ***** 0. Documentation Section *****
// TrafficLight.c
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
void PortF_Init();
void Delay();
void SysTick_half(unsigned long delay);

// NEW DEFINES FOR THE TRAFFIC LIGHT
#define LIGHT (*((volatile unsigned long *)0x400053FC))
#define SENSOR (*((volatile unsigned long *)0x4002401C))
#define PLIGHT (*((volatile unsigned long *)0x40025038)) // defining the use for ports F pins 3-1
	
// Linked data structure
struct State {
  unsigned long Out; 
	unsigned long P_out;
  unsigned long Time;  
  unsigned long Next[9];}; 

// define the state names
typedef const struct State STyp;
#define goN  			0
#define waitN			1
#define goE    		2
#define waitE 		3
#define goP		    4
#define waitPon1  5
#define waitPoff1 6
#define waitPon2  7
#define waitPoff2 8
	
// N - PE-2
// E - PE-1
// P - PE-0
	
// Moore state machine with outputs to the traffic light and the pedistrians lights
// time is called from the systic timer for 10ms
STyp FSM[9]={					// N, E, P
								 //     000        001         010       011        100        101        110        111
 {0x21,0x02, 600,{      goN,     waitN,     waitN,     waitN,       goN,     waitN,     waitN,     waitN}}, //goN
 {0x22,0x02, 200,{      goE,       goP,       goE,       goP,       goN,       goP,       goE,       goE}}, //waitN
 {0x0C,0x02, 600,{      goE,     waitE,       goE,     waitE,     waitE,     waitE,     waitE,     waitE}}, //goE
 {0x14,0x02, 200,{      goN,       goP,       goE,       goP,       goN,       goP,       goN,       goP}}, //waitE
 {0x24,0x08, 600,{      goP,       goP,  waitPon1,  waitPon1,  waitPon1,  waitPon1,  waitPon1,  waitPon1}}, //goP
 {0x24,0x02,  50,{waitPoff1, waitPoff1, waitPoff1, waitPoff1, waitPoff1, waitPoff1, waitPoff1, waitPoff1}}, //waitPon1
 {0x24,0x00,  50,{ waitPon2,  waitPon2,  waitPon2,  waitPon2,  waitPon2,  waitPon2,  waitPon2,  waitPon2}}, //waitPoff1
 {0x24,0x02,  50,{waitPoff2, waitPoff2, waitPoff2, waitPoff2, waitPoff2, waitPoff2, waitPoff2, waitPoff2}}, //waitPon2
 {0x24,0x00,  50,{      goN,       goE,       goE,       goE,       goN,       goN,       goN,      goN}}}; //waitPoff2

 
unsigned long S;  // index to the current state 
unsigned long Input; 

// ***** 3. Subroutines Section ****
int main(void){ 
  //TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MH
	
  Init_PortB();
  Init_PortE();
	PortF_Init();
	SysTick_Init();
	
  //EnableInterrupts();
	
	 while(1){
    LIGHT = FSM[S].Out;  // set lights
		PLIGHT = FSM[S].P_out; // set pedestreian lights
    SysTick_Wait10ms(FSM[S].Time);
    Input = SENSOR;     // read sensors
    S = FSM[S].Next[Input];  
  }
}
void Init_PortB(void){ // needs 3 more outputs
	unsigned int delay;
  SYSCTL_RCGC2_R |= 0x00000002;     // 1) B clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTB_CR_R = 0x3F;           // allow changes to PB5-0      
  GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTB_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTB_DIR_R = 0x3F;          // 5) PB5-PB0 output 
  GPIO_PORTB_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTB_PUR_R = 0x00;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTB_DEN_R = 0x3F;          // 7) enable digital pins P5-PB0   
																		//0011.1111
}
void Init_PortE(void){ // needs 3 inputs
	unsigned int delay;
  SYSCTL_RCGC2_R |= 0x00000010;     // 1) E clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTE_CR_R = 0x07;           // allow changes to PE2-PE0         0000.0111   
  GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R = 0xF8;          // 5) PE2-PE0  input                1111.1000
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) no alternate function   
  GPIO_PORTE_DEN_R = 0x07;          // 7) enable digital pins PE2-PE0   0000.0111
																		// 0000.0111 1000
}																		//0 - input, 1- output

// initialize Port F 
void PortF_Init(void){
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x0E;           // allow changes to PF3-1        
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_DEN_R = 0x0E;          // 7) enable digital pins PF3-PF1      0000.1110  
}

void Delay(void){unsigned long volatile time;
	time = (727240*200/91);
  while(time){
		time--;
  }
}
