
Image::Image(XGraphics* parent)
    : XGraphics(parent) {}

void Image::setImagePosition(int16_t x, int16_t y, int16_t w, int16_t h) {
    _imageBBox.pivot = XVector2<int16_t>(x, y);
    _imageBBox.width = w;
    _imageBBox.height = h;
}

void Image::setBitmap(uint8_t* bitmapBuffer, uint16_t bitmapWidth, uint16_t bitmapHeight) {
    _bitmapBuffer = bitmapBuffer;
    _bitmapWidth = bitmapWidth;
    _bitmapHeight = bitmapHeight;
}

void Image::renderScanline(XRenderer* renderer, int16_t scanline, uint16_t* buffer, size_t bufferSize) {

    if (scanline < _imageBBox.pivot.y || scanline > _imageBBox.pivot.y + _imageBBox.height - 1)
        return;

    // Calculating the line of a bitmap taking the vertical tiling into account.
    int16_t line = (scanline - (_imageBBox.pivot.y + _bitmapHeight * ((scanline - _imageBBox.pivot.y) / _bitmapHeight))) * _bitmapWidth * 2;

    for (int16_t x = _imageBBox.pivot.x, c = 0; x < _imageBBox.pivot.x + _imageBBox.width; x++, c = c + 2) {

        // Tile horizontally.
        if (c == _bitmapWidth * 2) c = 0;
        
        uint8_t b0 = pgm_read_byte(&_bitmapBuffer[line + c]);
        uint8_t b1 = pgm_read_byte(&_bitmapBuffer[line + c + 1]);
        
        uint16_t color = ((uint16_t)b0 << 8) | b1;

        if (x >= 0 && x < bufferSize)
            buffer[x] = color;

    }

}
