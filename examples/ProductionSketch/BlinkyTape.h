#ifndef BLINKY_TAPE_H
#define BLINKY_TAPE_H

#include <FastLED.h>

const uint16_t LED_COUNT = 64;       // Number of LEDs to display the patterns on
const uint16_t MAX_LEDS = 500;       // Maximum number of LEDs that can be controlled

#define LED_OUT       13
#define BUTTON_IN     10
#define ANALOG_INPUT  A9
#define EXTRA_PIN_A    7
#define EXTRA_PIN_B   11

class Pattern {
  public:
    virtual void draw(CRGB * leds);
    virtual void reset();
};

#endif
