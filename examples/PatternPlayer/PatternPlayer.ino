#include <FastLED.h>
#include <animation.h>

#include "samplePattern.h"

#define LED_COUNT 60*5
struct CRGB leds[LED_COUNT];

#define LED_OUT      13 
#define BUTTON_IN    10
#define ANALOG_INPUT A9
#define IO_A         7
#define IO_B         11

#define BRIGHT_STEP_COUNT 5
uint8_t brightnesSteps[BRIGHT_STEP_COUNT] = {5,15,40,70,93};
uint8_t brightness = 4;
uint8_t lastButtonState = 1;

void setup()
{  
  Serial.begin(57600);

  LEDS.addLeds<WS2812B, LED_OUT, GRB>(leds, LED_COUNT);
  LEDS.showColor(CRGB(0, 0, 0));
  LEDS.setBrightness(brightnesSteps[brightness]);
  LEDS.show();
  
  pinMode(BUTTON_IN, INPUT_PULLUP);
}


void serialLoop() {
  
  static int pixelIndex;
  uint8_t idx = 0;
  uint8_t buffer[3];
  uint8_t c;
  
  while(true) {
    if (Serial.available() > 0) {
      c = Serial.read();
      if (c == 255) {
    LEDS.show();
    pixelIndex = 0;
    idx = 0;   
    // BUTTON_IN (D10):   07 - 0111
    // EXTRA_PIN_A(D7):          11 - 1011
    // EXTRA_PIN_B (D11):        13 - 1101
    // ANALOG_INPUT (A9): 14 - 1110
      } else {        
        buffer[idx++] = c;
        if (idx == 3) {
          if(pixelIndex == LED_COUNT) break; // Prevent overflow by ignoring the pixel data beyond LED_COUNT
          leds[pixelIndex] = CRGB(buffer[0], buffer[1], buffer[2]);
          pixelIndex++;
          idx = 0;
        }
      }
    }
  }
}

void loop()
{
  // If'n we get some data, switch to passthrough mode
  if(Serial.available() > 2) {
    serialLoop();
  }
  
  // Check if the brightness should be adjusted
  uint8_t buttonState = digitalRead(BUTTON_IN);
  if((buttonState != lastButtonState) && (buttonState == 0)) {
    brightness = (brightness + 1) % BRIGHT_STEP_COUNT;
    LEDS.setBrightness(brightnesSteps[brightness]);
  }
  lastButtonState = buttonState;
  
  animation.draw(leds);
  
  // TODO: More sophisticated wait loop to get constant framerate.
  delay(animation.getFrameDelay());
}

