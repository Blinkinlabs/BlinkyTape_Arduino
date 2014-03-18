#include <FastLED.h>
#include <Button.h> // http://github.com/virgildisgr4ce/Button

#define LED_COUNT 60
struct CRGB leds[LED_COUNT];

#define LED_OUT      13
#define BUTTON_IN    10
#define ANALOG_INPUT A9
#define IO_A         7
#define IO_B         11

#define COLOR_MODE    0
#define POWERUP_MODE  1
#define PARTY_MODE    2
#define COOLDOWN_MODE 3

uint8_t draw_mode = COLOR_MODE;

uint8_t pixel_index;
long last_time;
long powerup_start;
long powerup_length = 500;
long cooldown_start;
long cooldown_length;
boolean party_on = false;

Button button = Button(BUTTON_IN, BUTTON_PULLUP_INTERNAL, true, 50);

void onPress(Button& b) {
  if(draw_mode == COLOR_MODE) {
    powerup_start = millis();
    draw_mode = POWERUP_MODE;
  }
  if(draw_mode == COOLDOWN_MODE) {
    powerup_start = millis();
    powerup_start = powerup_start - (cooldown_length - (powerup_start - cooldown_start));
    draw_mode = POWERUP_MODE;
  }
  if(draw_mode == PARTY_MODE) {
    draw_mode = COLOR_MODE;
  }
}

void onRelease(Button& b) {
  if(draw_mode == POWERUP_MODE) {
    cooldown_start = millis();
    cooldown_length = cooldown_start - powerup_start;
    draw_mode = COOLDOWN_MODE;
  }
}

void onHold(Button& b) {
//  if(draw_mode == PARTY_MODE)
//    draw_mode = COLOR_MODE;
}

void setup()
{
  LEDS.addLeds<WS2812B, LED_OUT, GRB>(leds, LED_COUNT);
  LEDS.showColor(CRGB(0, 0, 0));
  LEDS.setBrightness(93); // Limit max current draw to 1A
  LEDS.show();
  
  Serial.begin(57600);

  last_time = millis();

  button.pressHandler(onPress);
  button.releaseHandler(onRelease);
  button.holdHandler(onHold, 1000); // must be held for at least 1000 ms to trigger
}


uint8_t i = 0;
int j = 0;
int f = 0;
int k = 0;

int count;

void color_loop() {  
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    uint8_t red =   64*(1+sin(i/2.0 + j/4.0       ));
    uint8_t green = 64*(1+sin(i/1.0 + f/9.0  + 2.1));
    uint8_t blue =  64*(1+sin(i/3.0 + k/14.0 + 4.2));
    
    leds[i] = CRGB(red, green, blue);
    
    if ((millis() - last_time > 15) && pixel_index <= LED_COUNT + 1) {
      last_time = millis();
      count = LED_COUNT - pixel_index;
      pixel_index++; 
    }
  }
  
  LEDS.show();
  
  j = j + random(1,2);
  f = f + random(1,2);
  k = k + random(1,2);
}

void powerup_loop() {
  long elapsed = millis() - powerup_start;
  if(elapsed > powerup_length) {
    draw_mode = PARTY_MODE;
    last_time = millis();
  } else {
    draw_progress(elapsed, powerup_length);
  }
}

void cooldown_loop() {
  long elapsed = millis() - cooldown_start;
  if(elapsed > cooldown_length) {
    draw_mode = COLOR_MODE;
    last_time = millis();
  } else {
    draw_progress(cooldown_length - elapsed, powerup_length);
  }
}

void draw_progress(long elapsed, long powerup_length) {
  float progress = (float) elapsed / (float) powerup_length;
  int count = (int)((float)LED_COUNT * progress);
  for (int x = 0; x < count; x++) {
    leds[x] = CRGB(255,255,255);
  }
  for(int x = count+1; x < LED_COUNT; x++) {
    leds[x] = CRGB(255,0,0);
  }
  LEDS.show();
}

void party_loop() {
  long elapsed = millis() - last_time;

  if(elapsed > 10) {
    party_on = ~party_on;
    last_time = millis();
  }
  if(party_on) {
    for(int x = LED_COUNT - 1; x >= 0; x--) {
      leds[x] = CRGB(255,255,255);
    }
  }
  else {
    for(int x = LED_COUNT - 1; x >= 0; x--) {
      leds[x] = CRGB(0,0,0);
    }
  }

  LEDS.show();
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
  switch(draw_mode) {
    case COLOR_MODE:
      color_loop();
      break;
    case POWERUP_MODE:
      powerup_loop();
      break;
    case COOLDOWN_MODE:
      cooldown_loop();
      break;
    case PARTY_MODE:
      party_loop();
      break;
  }
  button.process();
}



