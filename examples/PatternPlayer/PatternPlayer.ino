#include <FastSPI_LED2.h>
#include <avr/pgmspace.h>
#include <Animation.h>
#include "animation.h"

#define LED_COUNT 60
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

int frameDelay = 30; // Number of ms each frame should be displayed.

void setup()
{  
  Serial.begin(57600);

  LEDS.addLeds<WS2811, LED_OUT, GRB>(leds, LED_COUNT);
  LEDS.showColor(CRGB(0, 0, 0));
  LEDS.setBrightness(brightnesSteps[brightness]);
  LEDS.show();
  
  pinMode(BUTTON_IN, INPUT_PULLUP);
}


void serialLoop() {
  static int pixelIndex;
  
  unsigned long lastReceiveTime = millis();

  while(true) {

    if(Serial.available() > 2) {
      lastReceiveTime = millis();

      uint8_t buffer[3]; // Buffer to store three incoming bytes used to compile a single LED color

      for (uint8_t x=0; x<3; x++) { // Read three incoming bytes
        uint8_t c = Serial.read();
        
        if (c < 255) {
          buffer[x] = c; // Using 255 as a latch semaphore
        }
        else {
          LEDS.show();
          pixelIndex = 0;
          break;
        }

        if (x == 2) {   // If we received three serial bytes
          if(pixelIndex == LED_COUNT) break; // Prevent overflow by ignoring the pixel data beyond LED_COUNT
          leds[pixelIndex] = CRGB(buffer[0], buffer[1], buffer[2]);
          pixelIndex++;
        }
      }
    }
    
    // If we haven't received data in 4 seconds, return to playing back our animation
    if(millis() > lastReceiveTime + 4000) {
      // TODO: Somehow the serial port gets trashed here, how to reset it?
      return;
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
  delay(frameDelay);
}

