
#ifndef IMAGE_H
#define IMAGE_H

#ifndef PGM_SPACE
#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#endif // __AVR__
#endif // PGM_SPACE

/*
 * Not all non-AVR boards installs define macros
 * for compatibility with existing PROGMEM-reading AVR code.
 */
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#include "XGraphics.h"

class Image : public XGraphics {
private:
    BBox _imageBBox;

    uint8_t* _bitmapBuffer;
    uint16_t _bitmapWidth;
    uint16_t _bitmapHeight;

public:
    Image(XGraphics* parent);

    void setImagePosition(int16_t x, int16_t y, int16_t w, int16_t h);
    void setBitmap(uint8_t* bitmapBuffer, uint16_t bitmapWidth, uint16_t bitmapHeight);

    void renderScanline(XRenderer* renderer, int16_t scanline, uint16_t* buffer, size_t bufferSize);
};

#include "Image.inl"

#endif // BITMAP_H
