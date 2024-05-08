#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "serialATmega.h"
#include "LCD.h"
#include <Arduino.h>

//TODO: declare variables for cross-task communication

/* You have 5 tasks to implement for this lab */
#define NUM_TASKS 1


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = /* TODO: Calulate GCD of tasks */ 0;

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Define, for each task:
// (1) enums and
// (2) tick functions
//task l 
enum sound_state{sound_init, sound_manual, sound_auto};
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
    DDRC = 0b111100;
    PORTC = 0b000011;
    DDRB = 0b111110;
    PORTB = 0b000001;
    DDRD = 0b11111111;
    PORTD = 0b00000000;
    serial_init(9600);

    // TODO: Initialize tasks here
    //  e.g. tasks[0].period = TASK1_PERIOD
    //  tasks[0].state = ...
    //  tasks[0].timeElapsed = ...
    //  tasks[0].TickFct = &task1_tick_function;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1)
    {
    }

    return 0;
}