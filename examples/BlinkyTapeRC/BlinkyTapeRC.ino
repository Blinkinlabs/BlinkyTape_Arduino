/*
 * BlinkyTapeRC 
 * V1.0
 *

Erweiterte BlinkyTape Firmware für RC Modelle.
Die Firmware unterstütz ein normales PWM Servosignal, und sollte mit jedem RC-Hersteller kompatibel sein.


Releasnotes:

V1.0     15.01.17
- PWM Servosignal wird unterstütz
- Framedelay optimiert



Original Sketch von Blinkinlabs:

// Pattern Player Sketch
// Designed to work with PatternPaint and the BlinkyTape controller
//
// Tested with the following software:
// Arduino 1.6.12 (https://www.arduino.cc/en/Main/Software)
// FastLED 3.1.3 (https://github.com/FastLED/FastLED/releases/tag/v3.1.3)
// BlinkyTape 2.1.0 (https://github.com/Blinkinlabs/BlinkyTape_Arduino/releases/tag/2.1.0)

*/



#include <animation.h>

#include <EEPROM.h>
#include <FastLED.h>
#include <avr/pgmspace.h>

#include "BlinkyTape.h"
#include "SerialLoop.h"

// Pattern table definitions
#define PATTERN_TABLE_ADDRESS  (0x7000 - 0x80)   // Location of the pattern table in the flash memory
#define PATTERN_TABLE_HEADER_LENGTH     11       // Length of the header, in bytes
#define PATTERN_TABLE_ENTRY_LENGTH      7        // Length of each entry, in bytes

// Header data sections
#define PATTERN_COUNT_OFFSET    0    // Number of patterns in the pattern table (1 byte)
#define LED_COUNT_OFFSET        1    // Number of LEDs in the pattern (2 bytes)
#define BRIGHTNESS_OFFSET       3    // Brightness table (8 bytes)

// Entry data sections
#define ENCODING_TYPE_OFFSET    0    // Encoding (1 byte)
#define FRAME_DATA_OFFSET       1    // Memory location (2 bytes)
#define FRAME_COUNT_OFFSET      3    // Frame count (2 bytes)
#define FRAME_DELAY_OFFSET      5    // Frame delay (2 bytes)

// RC signal
#ifdef RCsupported
bool patternChanged = false;
bool RCavailable = false;
const int TimeoutFrequency_COUNTS = TimeoutFrequency / 4.19;
const int TimeoutPulseLenght_COUNTS = TimeoutPulseLenght / 4.19;
const int maxPulseLenght_COUNTS = maxPulseLenght / 4.19;
const int minPulseLenght_COUNTS = minPulseLenght  / 4.19;
#endif

// LED data array
struct CRGB leds[MAX_LEDS];   // Space to hold the current LED data
CLEDController* controller;   // LED controller

// Pattern information
uint8_t patternCount;         // Number of available patterns
uint8_t currentPattern;       // Index of the current patter
Animation pattern;            // Current pattern

uint16_t ledCount;            // Number of LEDs used in the current sketch

const uint8_t brightnessCount = 8;
uint8_t brightnesSteps[brightnessCount];

uint8_t currentBrightness;
uint8_t lastBrightness;

// Button interrupt variables and Interrupt Service Routine
#ifdef buttonSupported 
bool buttonDebounced;
uint16_t buttonCounter;
const uint16_t BUTTON_PATTERN_SWITCH_COUNTS = BUTTON_PATTERN_SWITCH_TIME / 1.42;
#endif

#define EEPROM_START_ADDRESS  0
#define EEPROM_MAGIG_BYTE_0   0x12
#define EEPROM_MAGIC_BYTE_1   0x34
#define PATTERN_EEPROM_ADDRESS EEPROM_START_ADDRESS + 2
#define BRIGHTNESS_EEPROM_ADDRESS EEPROM_START_ADDRESS + 3

// Read the pattern data from the end of the program memory, and construct a new Pattern from it.
void setPattern(uint8_t newPattern) {
  currentPattern = newPattern % patternCount;

  if (EEPROM.read(PATTERN_EEPROM_ADDRESS) != newPattern) {
    EEPROM.write(PATTERN_EEPROM_ADDRESS, newPattern);
  }

  uint16_t patternEntryAddress =
    PATTERN_TABLE_ADDRESS
    + PATTERN_TABLE_HEADER_LENGTH
    + currentPattern * PATTERN_TABLE_ENTRY_LENGTH;


  Animation::Encoding encodingType = (Animation::Encoding)pgm_read_byte(patternEntryAddress + ENCODING_TYPE_OFFSET);

  PGM_P frameData =  (PGM_P)pgm_read_word(patternEntryAddress + FRAME_DATA_OFFSET);
  uint16_t frameCount = pgm_read_word(patternEntryAddress + FRAME_COUNT_OFFSET);
  uint16_t frameDelay = pgm_read_word(patternEntryAddress + FRAME_DELAY_OFFSET);

  pattern.init(frameCount, frameData, encodingType, ledCount, frameDelay);
}

void setBrightness(uint8_t newBrightness) {
  currentBrightness = newBrightness % brightnessCount;

  if (EEPROM.read(BRIGHTNESS_EEPROM_ADDRESS) != currentBrightness) {
    EEPROM.write(BRIGHTNESS_EEPROM_ADDRESS, currentBrightness);
  }

  LEDS.setBrightness(brightnesSteps[currentBrightness]);
}

void changePattern(){
  // first unroll the brightness!
  setBrightness(lastBrightness);
  setPattern(currentPattern + 1);
}


ISR(PCINT0_vect) {
  // check if pwm signal available
  #ifdef RCsupported
  uint8_t pwmState = !(PINB & (1 << PINB5)); // Reading state of the PB5
  if (!pwmState) { // at rising edge
    RCavailable = true;
  }
  #endif

  #ifdef buttonSupported
  // Called when the button is both pressed and released.
  uint8_t buttonState = !(PINB & (1 << PINB6)); // Reading state of the PB6 (remember that HIGH == released)
  if (buttonState) {
    if (buttonDebounced == false) {
      buttonDebounced = true;
      // if the button pressed once, then change the brightness
      lastBrightness = currentBrightness;
      setBrightness(currentBrightness + 1);

      // And configure and start timer4 interrupt.
      TCCR4B = 0x06; // prescaler: 32
      TCNT4 = 0;  // Reset the counter
      TIMSK4 = _BV(TOV4);  // turn on the interrupt
      // reset overflow counter
      buttonCounter = 0;
    }
  }
  else {
    buttonDebounced = false;
    TIMSK4 = 0;  // turn off the interrupt
  }
  #endif
}


// This is called every 1.42 ms while the button is being held down
#ifdef buttonSupported
ISR(TIMER4_OVF_vect) {
  // If we've waited long enough, switch the pattern
  ++buttonCounter;
  if (buttonCounter == BUTTON_PATTERN_SWITCH_COUNTS) {
    changePattern();
    // reset overflow counter
    buttonCounter = 0;
  }
}
#endif


void setup()
{
  Serial.begin(57600);

  pinMode(BUTTON_IN, INPUT_PULLUP);
  #ifdef pwmSupported
  pinMode(ANALOG_INPUT, INPUT_PULLUP);
  #endif

  // Interrupt set-up; see Atmega32u4 datasheet section 11
  PCIFR  |= (1 << PCIF0);  // Just in case, clear interrupt flag
  #ifdef buttonSupported
  PCMSK0 |= (1 << PCINT6); // Set interrupt mask to the button pin (PCINT6)
  #endif
  #ifdef RCsupported
  PCMSK0 |= (1 << PCINT5); // Set interrupt mask to the analog pin (PCINT5)
  #endif
  PCICR  |= (1 << PCIE0);  // Enable interrupt

  // setup Timer1 for frame delay
  TCCR1A = 0;
  TCCR1B = 0x04; // prescaler: 256
  TIFR1 = 0xFF;

  // First, load the pattern count and LED geometry from the pattern table
  patternCount = pgm_read_byte(PATTERN_TABLE_ADDRESS + PATTERN_COUNT_OFFSET);
  ledCount     = pgm_read_word(PATTERN_TABLE_ADDRESS + LED_COUNT_OFFSET);

  // Next, read the brightness table.
  for (uint8_t i = 0; i < brightnessCount; i++) {
    brightnesSteps[i] = pgm_read_byte(PATTERN_TABLE_ADDRESS + BRIGHTNESS_OFFSET + i);
  }

  // Bounds check for the LED count
  // Note that if this is out of bounds,the patterns will be displayed incorrectly.
  if (ledCount > MAX_LEDS) {
    ledCount = MAX_LEDS;
  }

  // If the EEPROM hasn't been initialized, do so now
  if ((EEPROM.read(EEPROM_START_ADDRESS) != EEPROM_MAGIG_BYTE_0)
      || (EEPROM.read(EEPROM_START_ADDRESS + 1) != EEPROM_MAGIC_BYTE_1)) {
    EEPROM.write(EEPROM_START_ADDRESS, EEPROM_MAGIG_BYTE_0);
    EEPROM.write(EEPROM_START_ADDRESS + 1, EEPROM_MAGIC_BYTE_1);
    EEPROM.write(PATTERN_EEPROM_ADDRESS, 0);
    EEPROM.write(BRIGHTNESS_EEPROM_ADDRESS, 0);
  }

  // Read in the last-used pattern and brightness
  currentPattern = EEPROM.read(PATTERN_EEPROM_ADDRESS);
  currentBrightness = EEPROM.read(BRIGHTNESS_EEPROM_ADDRESS);

  // Now, read the first pattern from the table
  setPattern(currentPattern);
  setBrightness(currentBrightness);

  controller = &(LEDS.addLeds<WS2811, LED_OUT, GRB>(leds, ledCount));
  LEDS.show();

}



void loop()
{

  // load timer1 for frame delay
  TCNT1 = 65535 - pattern.getFrameDelay() * 63;
  TIFR1 = 0x01; //reset overflow flag

  // If'n we get some data, switch to passthrough mode
  if (Serial.available() > 0) {
    serialLoop(leds);
  }

  // RC signal
  #ifdef RCsupported
  if (RCavailable == true) {

    // get pulse
    
    noInterrupts();
    // first sync with signal
    int rcPulse = 0;
    bool PulseEdge;
    bool lastPulseEdge = digitalRead(ANALOG_INPUT);  
    // wait for rising edge  
    while(rcPulse<TimeoutFrequency_COUNTS){
      PulseEdge = digitalRead(ANALOG_INPUT);
      if(PulseEdge != lastPulseEdge){
        if(PulseEdge==HIGH){
          break;
        }
      }
      lastPulseEdge = PulseEdge;
      rcPulse++;
    }

    // then measure pulse length
    rcPulse = 0;
    while(digitalRead(ANALOG_INPUT)==HIGH){
      rcPulse++;
      __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
      if(rcPulse>TimeoutPulseLenght_COUNTS){
        rcPulse = 0;
        break;
      }
    }
    interrupts();

    if (rcPulse == 0) {
      RCavailable = false;
      setBrightness(currentBrightness);
      goto rcTimeOut;
    }

    // pwm to %
    rcPulse = rcPulse - minPulseLenght_COUNTS - 100;

    if (rcPulse < min_off_pos) {
      // change pattern
      if (patternChanged == false) {
        changePattern();
        patternChanged = true;
      }

    } else if (rcPulse > max_off_pos) {
      // run
      patternChanged = false;
      LEDS.setBrightness(rcPulse);

    } else {
      // stop
      patternChanged = false;
      LEDS.setBrightness(0);
    }
  }
  rcTimeOut:
  #endif

  // draw pattern
  pattern.draw(leds);

  // wait until timer1 overflow flag is set
  while (!(TIFR1 & 1)) {
  }

}


