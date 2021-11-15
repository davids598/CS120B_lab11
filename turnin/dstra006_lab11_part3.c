/*	Author: David Strathman
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *
 * Link to Vid:
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <string.h>
#include "../header/bit.h"
#include "../header/io.h"
#include "../header/keypad.h"
#include "../header/timer.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


typedef struct task {
  signed char state;
  unsigned long int period;
  unsigned long int elapsedTime;
  int (*TickFct)(int);
} task;
//Globals
unsigned char led0_output = 0x00;
unsigned char led1_output = 0x00;
unsigned char pause = 0;
const char full_string[] = "                CS120B is Legend... wait for it DARY!";
unsigned char i = 0;
unsigned char prev_x = 0;
// END

//--------------------------Example SM Code-------------------------------------
enum pauseButtonSM_States { pauseButton_wait, pauseButton_press, pauseButton_release };

int pauseButtonSMTick(int state) {
  unsigned char press = ~PINA & 0x01;
  switch (state) {
    case pauseButton_wait:
      state = press == 0x01? pauseButton_press: pauseButton_wait; break;
    case pauseButton_press:
      state = pauseButton_release; break;
    case pauseButton_release:
      state = press == 0x00? pauseButton_wait: pauseButton_press; break;
    default: state = pauseButton_wait; break;
  }
  switch (state) {
    case pauseButton_wait: break;
    case pauseButton_press:
      pause = (pause == 0) ? 1 : 0;
      break;
    case pauseButton_release: break;
  }
  return state;
}

enum toggleLED0_States { toggleLED0_wait, togggleLED0_blink };

int toggleLED0SMTick(int state) {
  switch (state) {
    case toggleLED0_wait: state = !pause? togggleLED0_blink: toggleLED0_wait; break;
    case togggleLED0_blink: state = pause? toggleLED0_wait: togggleLED0_blink; break;
    default: state = toggleLED0_wait; break;
  }

  switch (state) {
    case toggleLED0_wait: break;
    case togggleLED0_blink:
      led0_output = (led0_output == 0x00) ? 0x01 : 0x00;
      break;
  }
  return state;
}

enum toggleLED1_States { toggleLED1_wait, toggleLED1_blink };

int toggleLED1SMTick(int state) {
  switch (state) {
    case toggleLED1_wait: state = !pause? toggleLED1_blink: toggleLED1_wait; break;
    case toggleLED1_blink: state = pause? toggleLED1_wait: toggleLED1_blink; break;
    default: state = toggleLED1_wait; break;
  }

  switch (state) {
    case toggleLED1_wait: break;
    case toggleLED1_blink:
      led1_output = (led1_output == 0x00) ? 0x01 : 0x00;
      break;
  }
  return state;
}

enum display_States { display_display };

int displaySMTick(int state) {
  unsigned char output;

  switch (state) {
    case display_display: state = display_display; break;
    default: state = display_display; break;
  }

  switch (state) {
    case display_display:
      output = led0_output | led1_output << 1;
      break;
  }
  PORTB = output;
  return state;
}
//----------------------------Part 1--------------------------------------------
enum keypad_States { keypad_check };

int keypadSMTick(int state) {
  unsigned char x;
  switch (state) {
    case keypad_check: state = keypad_check; break;
    default: state = keypad_check; break;
  }

  switch (state) {
    case keypad_check:
    x = GetKeypadKey();
    switch(x) {
        case '\0': PORTB = 0x1F; break;
        case '1': PORTB = 0x01; break;
        case '2': PORTB = 0x02; break;
        case '3': PORTB = 0x03; break;
        case 'A': PORTB = 0x0A; break;
        case '4': PORTB = 0x04; break;
        case '5': PORTB = 0x05; break;
        case '6': PORTB = 0x06; break;
        case 'B': PORTB = 0x0B; break;
        case '7': PORTB = 0x07; break;
        case '8': PORTB = 0x08; break;
        case '9': PORTB = 0x09; break;
        case 'C': PORTB = 0x0C; break;
        case '*': PORTB = 0x0E; break;
        case '0': PORTB = 0x00; break;
        case '#': PORTB = 0x0F; break;
        case 'D': PORTB = 0x0D; break;
        default: PORTB = 0x1B; break;
    }
  }
  return state;
}
//---------------------------------Part 2---------------------------------------
enum LED_Scrolling_States { LED_LOOP };
  char partial_string[16] = {};
int LCD_Scrolling(int state) {
  switch (state) {
    case LED_LOOP: state = LED_LOOP; break;
    default: state = LED_LOOP; break;
  }

  switch (state) {
    case LED_LOOP:
      memcpy(partial_string, &full_string[i], 16);
      LCD_DisplayString(1, partial_string);
      if (i > 52) {
        i = 0;
      } else {
        i++;
      }
  }
  return state;
}
//----------------------------------Part 3--------------------------------------
enum LCD_KeypadInput_States { Keypad_input };

int LCD_KeypadInputsSMTick(int state) {
  unsigned char x;
  switch (state) {
    case Keypad_input: state = Keypad_input; break;
    default: state = Keypad_input; break;
  }

  switch (state) {
    case Keypad_input:
      x = GetKeypadKey();
      if (x != prev_x && x != '\0') {
        LCD_Cursor(1);
        LCD_WriteData(x);
        LCD_Cursor(2);
        prev_x = x;
      }
  }
  return state;
}

unsigned long int findGCD(unsigned long int a, unsigned long int b) {
  unsigned long int c;
  while (1) {
    c = a%b;
    if (c==0) {return b;}
    a = b;
    b = c;
  }
  return 0;
}

int main(void) {
  DDRA = 0x00; PORTA = 0xFF;
  DDRB = 0xFF; PORTB = 0x00;
  DDRC = 0xF0; PORTC = 0x0F;
  DDRD = 0xFF; PORTD = 0x00;

  static task task1, task2;
  task *tasks[] = { &task1, &task2 };
  const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

  const char start = -1;

  //Task 1
  task1.state = start;
  task1.period = 10;
  task1.elapsedTime = task1.period;
  task1.TickFct = &keypadSMTick;

  task2.state = start;
  task2.period = 50;
  task2.elapsedTime = task2.period;
  task2.TickFct = &LCD_KeypadInputsSMTick;

  unsigned long GCD = tasks[0]->period;
  for (int i = 1; i < numTasks; i++) {
    GCD = findGCD(GCD, tasks[i]->period);
  }
  TimerSet(GCD);
  TimerOn();
  LCD_init();
  LCD_ClearScreen();
  LCD_Cursor(1);

  unsigned short i;
  while (1) {
    for (i = 0; i < numTasks; i++) {
      if (tasks[i]->elapsedTime == tasks[i]->period) {
        tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
        tasks[i]->elapsedTime = 0;
      }
      tasks[i]->elapsedTime += GCD;
    }
    while (!TimerFlag);
    TimerFlag = 0;
  }

  return 0;
}
