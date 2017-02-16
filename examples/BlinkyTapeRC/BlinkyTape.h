#ifndef BLINKY_TAPE_H
#define BLINKY_TAPE_H

#include <FastLED.h>

const uint16_t MAX_LEDS = 512;       // Maximum number of LEDs that can be controlled

#define BUTTON_PATTERN_SWITCH_TIME    1000   // Time to hold the button down to switch patterns

#define LED_OUT       13
#define BUTTON_IN     10
#define ANALOG_INPUT  A9            // Pin RC-Channel
#define EXTRA_PIN_A    7
#define EXTRA_PIN_B   11

#define RCsupported              // comment to disable RC support
#define buttonSupported          // comment to disable button support

#define maxPulseLenght        1900       // [us] @Servoposition +100% 
#define minPulseLenght        1100       // [us] @Servoposition -100%

#define TimeoutFrequency      27000       // Timeout [us] signal frequency
#define TimeoutPulseLenght     2500      // Timeout [us] pulse length


/*
         RC-position                Function
          
    +100%     +---+                   -+
              |   |                    |
              |   |                    |
              |   |                    |-- BlinkyTape ON, set Brightness
              |   |                    |
              |   |                    |
              |   |  -- max_off_pos   -+
       0%     +---+                    |-- BlinkyTape OFF
              |   |  -- min_off_pos   -+
              |   |                    |
              |   |                    |
              |   |                    |-- BlinkyTape ON, change pattern
              |   |                    |
              |   |                    |
    -100%     +---+                   -+             
*/

       
#define max_off_pos    5  
#define min_off_pos    -40 


#endif

