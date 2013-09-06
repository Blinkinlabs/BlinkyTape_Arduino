#include <FastSPI_LED2.h>
#include <Animation.h>

#define LED_COUNT 60
struct CRGB leds[LED_COUNT];

#define PIN_BUTTON 10
#define PIN_IO_A   7
#define PIN_IO_B   11
#define PIN_SIGNAL 13
#define PIN_INPUT  10

void setup()
{  
  LEDS.addLeds<WS2811, PIN_SIGNAL, GRB>(leds, LED_COUNT);
  LEDS.showColor(CRGB(0, 0, 0));
  LEDS.setBrightness(0); // start out with LEDs off
  LEDS.show();
}

void setcolor(int colorcode) {

  for (int i = 0; i < LED_COUNT; i++) {
    switch(colorcode) {
      case 0: leds[i].r = 128; break;
    
      case 1: leds[i].g = 128; break;
    
      case 2: leds[i].b = 128; break;
    }
  }
  LEDS.show();
}

void pulse(int wait_time) {
  // let's fade up by scaling the brightness
  for(int scale = 0; scale < 128; scale++) { 
    LEDS.setBrightness(scale);
    LEDS.show();
    delay(wait_time);
  }
  // now let's fade down by scaling the brightness
  for(int scale = 128; scale > 0; scale--) { 
    LEDS.setBrightness(scale);
    LEDS.show();
    delay(10);
  }
}

void loop() {
  int color_set = 2; // 0 for red, 1 for green, 2 for blue
  int waiting_time = 40; //time in ms for color scaling
  
  setcolor(color_set);
  
  pulse(waiting_time);
}
