#ifndef SHIMMER_H
#define SHIMMER_H

#include <FastLED.h>
#include "BlinkyTape.h"

class ShimmerDot {
  public:
    void reset();
    void update();
    uint8_t getValue();

  private:
    uint8_t maxValue;
    uint8_t direction;
    int16_t value;
    uint8_t resetDelay;
};

class Shimmer : public Pattern {
  public:
    Shimmer(float r, float g, float b);
    void reset();
    void draw(CRGB * leds);
  
  private:
    ShimmerDot shimmerDots[LED_COUNT];

    float color_temp_factor_r;
    float color_temp_factor_g;
    float color_temp_factor_b;
};


#endif
