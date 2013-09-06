#include <FastSPI_LED2.h>
#include <Animation.h>

#define LED_COUNT 60 // BlinkyTape has 60 LEDs!
struct CRGB leds[LED_COUNT]; // this struct contains 60 CRGB values.  This is where 

#define PIN_BUTTON 10
#define PIN_IO_A   7
#define PIN_IO_B   11
#define PIN_SIGNAL 13
#define PIN_INPUT  10

int color_set = 0;

// first, let's get ready to blink using some FastSPI_LED2 routines
// take a look at the FastSPI_LED2 example called Fast2Dev for more usage info
void setup()
{  
  LEDS.addLeds<WS2811, PIN_SIGNAL, GRB>(leds, LED_COUNT); // this configures the BlinkyBoard - leave as is.
  LEDS.showColor(CRGB(0, 0, 0)); // set the color for the strip all at once.
  LEDS.setBrightness(0); // start out with LEDs off
  LEDS.show(); // you'll always need to call this function to make your changes happen.
}

// set the color for all the LEDs based on the color code
void setcolor(int colorcode) {

  for (int i = 0; i < LED_COUNT; i++) {
    // instead of setting the color all at once we're going to step through each LED to show how to set them individually
    switch(colorcode) { 
      // there are several ways to set colors. We're going to pass a CRGB here, but there are other methods.  
      // See the Fast2Dev example for the others
      case 0: leds[i] = CRGB(i*3,0,0); break; // using the 'i' term will create a brightness gradient in active color 
    
      case 1: leds[i] = CRGB(0,i*3,0); break;
    
      case 2: leds[i] = CRGB(0,0,i*3); break;
    }
  }
  LEDS.show();
}

// we'll make the color fade in and out by setting the brightness
void pulse(int wait_time) {
  // let's fade up by scaling the brightness - in general, brightness shouldn't go above 93, so the strip won't draw too much power.
  // Oh, and 93 is plenty bright!
  for(int scale = 0; scale < 93; scale++) { 
    LEDS.setBrightness(scale);
    LEDS.show();
    delay(wait_time);
  }
  // now let's fade down by scaling the brightness
  for(int scale = 93; scale > 0; scale--) { 
    LEDS.setBrightness(scale);
    LEDS.show();
    delay(wait_time);
  }
}

// this is the main loop where we call the other functions. 
void loop() {
  int waiting_time = 40; //time in ms for color scaling
  
  setcolor(color_set); // we call our color-set routine
  
  pulse(waiting_time); // now we make it fade in and out

  color_set++; // OK - next color!
  color_set = color_set % 3; // modulus, just so we always stay below 2 and our function works.
}
