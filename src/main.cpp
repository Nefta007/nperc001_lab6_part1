#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "serialATmega.h"
#include "LCD.h"
#include <Arduino.h>

//TODO: declare variables for cross-task communication
unsigned char turn_sound;
unsigned char cnt;
unsigned char motor_man;
/* You have 5 tasks to implement for this lab */
#define NUM_TASKS 2


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long display_period = 100;
const unsigned long Sound_period = 100;
const unsigned long GCD_PERIOD = findGCD(display_period, Sound_period);

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Define, for each task:
// (1) enums and
// (2) tick functions

//task l Display
enum display_state{display_init, display_manual, display_auto};
int TickFtn_Display(int state);

// task 2 Sound
enum sound_state{sound_init, sound_off, sound_on_pressed, sound_up, sound_down};
int TickFtn_Sound(int state);

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}

int main(void)
{
    // TODO: initialize all your inputs and ouputs

    ADC_init();   // initializes ADC
    init_sonar(); // initializes sonar
    lcd_init(); // initialize lcd 
    //  Output: DDR = 1, PORT = 0
    //  Input: DDR = 0, PORT = 1
    DDRC = 0b111000; PORTC = 0b000111;
    DDRB = 0b111111; PORTB = 0b000000;
    DDRD = 0b11111111; PORTD = 0b00000000;
    serial_init(9600);

    // TODO: Initialize tasks here
    //  e.g. tasks[0].period = TASK1_PERIOD
    //  tasks[0].state = ...
    //  tasks[0].timeElapsed = ...
    //  tasks[0].TickFct = &task1_tick_function;

    // Task 1
    unsigned char i  = 0;
    tasks[i].state = display_init;
    tasks[i].period = display_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_Display;
    i++;

    // Task 2
    tasks[i].state = sound_init;
    tasks[i].period = Sound_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_Sound;
    // i++;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1)
    {
    }

    return 0;
}
int TickFtn_Display(int state){ //transition
    switch (state)
    {
    case display_init:
        if(!((PINC >> 2)&0x01)){
            cnt = 0;
            turn_sound = 1;
            motor_man = 1;
            state = display_manual;
        }
        break;
    
    case display_manual:
        if(cnt > 0){
            state = display_manual;
        }
        else if(!((PINC >> 2)&0x01)){
            cnt = 0;
            turn_sound = 1;
            motor_man = 0;
            state = display_auto;
        }
        break;
    
    case display_auto:
        if(cnt > 0){
            state = display_auto;
        }
        else if(!((PINC >> 2)&0x01)){
            cnt = 0;
            turn_sound = 1;
            motor_man = 1;
            state = display_manual;
        }
        break;
    default:
        break;
    }

    switch(state){
        case display_init:
        turn_sound = 0;
        break;
        
        case display_manual:
        if(!((PINC >> 2)&0x01)){
            cnt++;
            // turn_sound = 0;
        }
        else if((PINC >> 2)&0x01){
            cnt = 0;
        }
        //turn_sound = 0;
        lcd_clear();
        lcd_write_str("Mode: Manual");
        break;

        case display_auto:
        if(!((PINC >> 2)&0x01)){
            // turn_sound = 0;
            cnt ++;
        }
        else if((PINC >> 2)&0x01){
            cnt = 0;
        }
        //turn_sound = 0;
        lcd_clear();
        lcd_write_str("Mode: Auto");
        break;

        default:
        break;
    }
    return state;
}

// enum sound_state{sound_init, sound_off_pressed, sound_on_pressed, sound_up, sound_down};
int TickFtn_Sound(int state){
    switch (state)
    {
     case sound_init:
        state = sound_off;
        break;

    case sound_off:
        if(!((PINC >> 2)&0x01)){
            PORTB = SetBit(PORTB,0,1);
            state = sound_on_pressed;
        }
        else if(ADC_read(1) >= 800){
            PORTB = SetBit(PORTB,0,1);
            state = sound_up;
        }
        else if(ADC_read(1) <= 200){
            PORTB = SetBit(PORTB,0,1);
            state = sound_down;
        }
        else{
            state = sound_off;
        }
        break;
    
    case sound_on_pressed:
    if(!((PINC >> 2)&0x01)){
        PORTB = SetBit(PORTB,0,0);
        state = sound_on_pressed;
    }
    else{
        state = sound_off; 
    }
    break;

    case sound_up:
    if(ADC_read(1)>=800){
        PORTB = SetBit(PORTB,0,0);
        state = sound_up;
    }
    else{
        state = sound_off;
    }  
    break;

    case sound_down:
    if(ADC_read(1) <=200){
       PORTB = SetBit(PORTB,0,0);
        state = sound_down;
    }
    else{
        state = sound_off;
    }
    break;

    default:
        break;
    }

    switch (state)
    {
    case sound_init:
        break;
    
    case sound_off:
        // PORTB = SetBit(PORTB,0,0);
        break;
    
    case sound_on_pressed:
        // PORTB = SetBit(PORTB,0,0);
        break;
    
    case sound_up:
        // PORTB = SetBit(PORTB,0,0);
        break;

    case sound_down:
        // PORTB = SetBit(PORTB,0,0);
        break;
    
    default:
        break;
    }
    return state;
}