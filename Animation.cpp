#include "Animation.h"

Animation::Animation() {
  init(0, NULL, ENCODING_RGB24, 0);
}

Animation::Animation(uint16_t frameCount_,
                     const prog_uint8_t* frameData_,
                     const uint8_t encoding_,
                     const uint8_t ledCount_)
{
  init(frameCount_, frameData_, encoding_, ledCount_);
  reset();
}

void Animation::init(uint16_t frameCount_,
                     const prog_uint8_t* frameData_,
                     const uint8_t encoding_,
                     const uint8_t ledCount_)
{
  frameCount = frameCount_;
  frameData = const_cast<prog_uint8_t*>(frameData_);
  encoding = encoding_;
  ledCount = ledCount_;

  switch(encoding) {
    case ENCODING_RGB24:
    case ENCODING_RGB565_RLE:
      // Nothing to preload.
      break;
    case ENCODING_INDEXED:
    case ENCODING_INDEXED_RLE:
      // Load the color table into memory
      // TODO: Free this memory somewhere?
      colorTableEntries = pgm_read_byte(frameData) + 1;
      colorTable = (CRGB *)malloc(colorTableEntries*sizeof(CRGB));

      for(int i = 0; i < colorTableEntries; i++) {
        colorTable[i] = CRGB(pgm_read_byte(frameData + 1 + i*3    ),
                             pgm_read_byte(frameData + 1 + i*3 + 1),
                             pgm_read_byte(frameData + 1 + i*3 + 2));
      }
      break;
  }

  reset();
}
 
void Animation::reset() {
  frameIndex = 0;
}

void Animation::draw(struct CRGB strip[]) {
  switch(encoding) {
    case ENCODING_RGB24:
      drawRgb24(strip);
      break;
    case ENCODING_RGB565_RLE:
      drawRgb16_RLE(strip);
      break;
    case ENCODING_INDEXED:
      drawIndexed(strip);
      break;
    case ENCODING_INDEXED_RLE:
      drawIndexed_RLE(strip);
      break;
  }

  LEDS.show();
  
  frameIndex = (frameIndex + 1)%frameCount;
};

void Animation::drawRgb24(struct CRGB strip[]) {
  currentFrameData = frameData
    + frameIndex*ledCount*3;  // Offset for current frame
  
  for(uint8_t i = 0; i < ledCount; i++) {
    strip[i] = CRGB(pgm_read_byte(currentFrameData + i*3    ),
                    pgm_read_byte(currentFrameData + i*3 + 1),
                    pgm_read_byte(currentFrameData + i*3 + 2));
  }
}

void Animation::drawRgb16_RLE(struct CRGB strip[]) {
  if(frameIndex == 0) {
    currentFrameData = frameData;
  }

  // Read runs of RLE data until we get enough data.
  uint8_t count = 0;
  while(count < ledCount) {
    uint8_t run_length = 0x7F & pgm_read_byte(currentFrameData);
    uint8_t upperByte = pgm_read_byte(currentFrameData + 1);
    uint8_t lowerByte = pgm_read_byte(currentFrameData + 2);
    
    uint8_t r = ((upperByte & 0xF8)     );
    uint8_t g = ((upperByte & 0x07) << 5)
              | ((lowerByte & 0xE0) >> 3);
    uint8_t b = ((lowerByte & 0x1F) << 3);
    
    for(uint8_t i = 0; i < run_length; i++) {
      strip[count + i] = CRGB(r,g,b);
    }
    
    count += run_length;
    currentFrameData += 3;
  }
};

void Animation::drawIndexed(struct CRGB strip[]) {
  currentFrameData = frameData
    + 1 + 3*colorTableEntries   // Offset for color table
    + frameIndex*ledCount;      // Offset for current frame
  
  for(uint8_t i = 0; i < ledCount; i++) {
    strip[i] = colorTable[pgm_read_byte(currentFrameData + i)];
  }
}

void Animation::drawIndexed_RLE(struct CRGB strip[]) {
  if(frameIndex == 0) {
    currentFrameData = frameData
      + 1 + 3*colorTableEntries;   // Offset for color table
  }

  // Read runs of RLE data until we get enough data.
  uint8_t count = 0;
  while(count < ledCount) {
    uint8_t run_length = pgm_read_byte(currentFrameData++);
    uint8_t colorIndex = pgm_read_byte(currentFrameData++);
    
    for(uint8_t i = 0; i < run_length; i++) {
      strip[count++] = colorTable[colorIndex];
    }
  }
};

