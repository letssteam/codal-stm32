#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace codal {

class FrameBuffer {
  public:
    enum Format {
        MONO_VLSB,
        // MONO_HLSB,
        // MONO_HMSB,
        // RGB565,
        // GS2_HMSB,
        GS4_HMSB,
        // GS8
    };

    FrameBuffer(unsigned widthPixel, unsigned heightPixel, Format format);
    ~FrameBuffer();

    void fill(uint16_t value);
    void drawPixel(unsigned x, unsigned y, uint16_t color);
    void drawChar(char c, unsigned x, unsigned y, uint16_t color);
    void drawText(std::string str, unsigned x, unsigned y, uint16_t color);
    void setData(uint8_t* data, size_t len) { memcpy(buffer, data, len); }

    uint16_t getPixelColor(uint8_t x, uint8_t y);
    void drawSegment(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t size, uint16_t color);
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t size, uint16_t color);
    void drawBezierCurve(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, uint16_t color);
    void drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool fill, uint16_t color);
    void drawSquare(uint16_t x, uint16_t y, uint16_t lenght, bool fill, uint16_t color);
    void drawEllipse(int width, int height, int xCenter, int yCenter, bool fill, uint16_t color);
    void drawCircle(int x, int y, int radius, bool fill, uint16_t color);
    void drawArc(int x, int y, int radius, int start_angle, int end_angle, uint16_t color);
    void drawPolygon(uint8_t x, uint8_t y, uint8_t line, uint8_t radius, uint8_t size, uint16_t color);
    void drawMatrix(std::vector<std::vector<uint16_t>> matrix, unsigned x, unsigned y);

  protected:
    unsigned width;
    unsigned height;
    unsigned bufferSize;
    Format format;
    uint8_t pages;
    uint8_t* buffer;
};
}  // namespace codal