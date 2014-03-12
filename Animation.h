#ifndef ANIMATION_H
#define ANIMATION_H

#include <Arduino.h>
#include <FastLED.h>

#define ENCODING_RGB24       0
#define ENCODING_RGB565_RLE  1
#define ENCODING_INDEXED     2
#define ENCODING_INDEXED_RLE 3

class Animation {
 private:
  uint8_t ledCount;               // Number of LEDs in the strip (max 254)
  uint16_t frameCount;            // Number of frames in this animation (max 65535)
  uint8_t encoding;               // Encoding type
  prog_uint8_t* frameData;        // Pointer to the begining of the frame data
  
  uint16_t frameIndex;            // Current animation frame
  prog_uint8_t* currentFrameData; // Pointer to the current position in the frame data

  uint8_t colorTableEntries;      // Number of entries in the color table, minus 1 (max 255)
  struct CRGB* colorTable;        // Pointer to color table, if used by the encoder

  void drawRgb24(struct CRGB strip[]);
  void drawRgb16_RLE(struct CRGB strip[]);
  void drawIndexed(struct CRGB strip[]);
  void drawIndexed_RLE(struct CRGB strip[]);

 public:
  // Initialize the animation with no data. This is intended for the case
  // where the animation will be re-initialized from a memory structure in ROM
  // after the sketch starts.
  Animation();

  // Initialize the animation
  // @param frameCount Number of frames in this animation
  // @param frameData Pointer to the frame data. Format of this data is encoding-specficic
  // @param encoding Method used to encode the animation data
  // @param ledCount Number of LEDs in the strip
  Animation(uint16_t frameCount,
            const prog_uint8_t* frameData,
            const uint8_t encoding,
            const uint8_t ledCount);

  // Re-initialize the animation with new information
  // @param frameCount Number of frames in this animation
  // @param frameData Pointer to the frame data. Format of this data is encoding-specficic
  // @param encoding Method used to encode the animation data
  // @param ledCount Number of LEDs in the strip
  void init(uint16_t frameCount,
            const prog_uint8_t* frameData,
            const uint8_t encoding,
            const uint8_t ledCount);
 
  // Reset the animation, causing it to start over from frame 0.
  void reset();
  
  // Draw the next frame of the animation
  // @param strip[] LED strip to draw to.
  void draw(struct CRGB strip[]);
};

#endif
