#include "Animation.h"

Animation::Animation() {
  init(0, NULL, ENCODING_RGB24, 0);
}

Animation::Animation(uint16_t frameCount,
                     const prog_uint8_t* frameData,
                     const uint8_t encoding,
                     const uint8_t ledCount)
{
  init(frameCount, frameData, encoding, ledCount);
  reset();
}

void Animation::init(uint16_t frameCount,
                     const prog_uint8_t* frameData,
                     const uint8_t encoding,
                     const uint8_t ledCount)
{
  m_frameCount = frameCount;
  m_frameData = const_cast<prog_uint8_t*>(frameData);
  m_encoding = encoding;
  m_ledCount = ledCount;

  switch(m_encoding) {
    case ENCODING_RGB24:
    case ENCODING_RGB565_RLE:
      // Nothing to preload.
      break;
    case ENCODING_INDEXED:
      // Load the color table into memory
      // TODO: Free this memory somewhere?
      colorTableEntries = pgm_read_byte(m_frameData);
      colorTable = (CRGB *)malloc(colorTableEntries*sizeof(CRGB));

      for(int i = 0; i < colorTableEntries; i++) {
        colorTable[i] = CRGB(pgm_read_byte(m_frameData + 1 + i*3    ),
                             pgm_read_byte(m_frameData + 1 + i*3 + 1),
                             pgm_read_byte(m_frameData + 1 + i*3 + 2));
      }
      break;
  }

  reset();
}
 
void Animation::reset() {
  m_frameIndex = 0;
}

void Animation::draw(struct CRGB strip[]) {
  switch(m_encoding) {
    case ENCODING_RGB24:
      drawRgb24(strip);
      break;
    case ENCODING_RGB565_RLE:
      drawRgb16_RLE(strip);
      break;
    case ENCODING_INDEXED:
      drawIndexed(strip);
      break;
  }
};

void Animation::drawRgb24(struct CRGB strip[]) {
  currentFrameData = m_frameData
    + m_frameIndex*m_ledCount*3;  // Offset for current frame
  
  for(uint8_t i = 0; i < m_ledCount; i+=1) {
    strip[i] = CRGB(pgm_read_byte(currentFrameData + i*3    ),
                    pgm_read_byte(currentFrameData + i*3 + 1),
                    pgm_read_byte(currentFrameData + i*3 + 2));
  }
  
  LEDS.show();
  
  m_frameIndex = (m_frameIndex + 1)%m_frameCount;
}

void Animation::drawIndexed(struct CRGB strip[]) {
  currentFrameData = m_frameData
    + 1 + 3*colorTableEntries   // Offset for color table
    + m_frameIndex*m_ledCount;  // Offset for current frame
  
  for(uint8_t i = 0; i < m_ledCount; i+=1) {
    strip[i] = colorTable[pgm_read_byte(currentFrameData + i)];
  }
  
  LEDS.show();
  
  m_frameIndex = (m_frameIndex + 1)%m_frameCount;
}

void Animation::drawRgb16_RLE(struct CRGB strip[]) {
  if(m_frameIndex == 0) {
    currentFrameData = m_frameData;
  }

  // Read runs of RLE data until we get enough data.
  uint8_t count = 0;
  while(count < m_ledCount) {
    uint8_t run_length = 0x7F & pgm_read_byte(currentFrameData);
    uint8_t upperByte = pgm_read_byte(currentFrameData + 1);
    uint8_t lowerByte = pgm_read_byte(currentFrameData + 2);
    
    uint8_t r = ((upperByte & 0xF8)     );
    uint8_t g = ((upperByte & 0x07) << 5)
              | ((lowerByte & 0xE0) >> 3);
    uint8_t b = ((lowerByte & 0x1F) << 3);
    
    for(uint8_t i = 0; i < run_length; i+=1) {
      strip[count + i] = CRGB(r,g,b);
    }
    
    count += run_length;
    currentFrameData += 3;
  }
  
  LEDS.show();

  m_frameIndex = (m_frameIndex + 1)%m_frameCount;
};

