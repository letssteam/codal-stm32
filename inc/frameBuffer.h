#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <cstdint>
#include <string>

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

  protected:
    unsigned width;
    unsigned height;
    unsigned bufferSize;
    Format format;
    uint8_t pages;
    uint8_t* buffer;
};
}  // namespace codal

#endif