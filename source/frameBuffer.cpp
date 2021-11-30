#include "frameBuffer.h"

#include "font.h"

using namespace codal;
using namespace std;

FrameBuffer::FrameBuffer(unsigned widthPixel, unsigned heightPixel, FrameBuffer::Format format)
    : width(widthPixel), height(heightPixel), format(format)
{
    switch (this->format) {
        case Format::MONO_VLSB:
            pages      = height / 8;
            bufferSize = pages * width;
            break;

        case Format::GS4_HMSB:
            pages      = 0;
            bufferSize = height * (width / 2);
            break;
    }

    buffer = new uint8_t[bufferSize];
}

FrameBuffer::~FrameBuffer()
{
    delete[] buffer;
}

void FrameBuffer::fill(uint16_t color)
{
    switch (format) {
        case Format::MONO_VLSB:
            for (unsigned i = 0; i < bufferSize; ++i) {
                buffer[i] = color > 0 ? 0xFF : 0x00;
            }
            break;

        case Format::GS4_HMSB:
            for (unsigned i = 0; i < bufferSize; ++i) {
                buffer[i] = color > 0 ? 0xFF : 0x00;
            }
            break;
    }
}

void FrameBuffer::drawPixel(unsigned x, unsigned y, uint16_t color)
{
    if (x >= width || y >= height) {
        return;
    }

    switch (format) {
        case Format::MONO_VLSB: {
            int shift     = y % 8;
            int index     = x + ((y / 8) * width);
            uint8_t* data = &buffer[index];

            if (color > 0) {
                *data |= (0x01 << shift);
            }
            else {
                *data &= ~(0x01 << shift);
            }
            break;
        }

        case Format::GS4_HMSB: {
            int shift     = (x & 0x01) == 1 ? 0x00 : 0x04;
            uint8_t value = color & 0x000F;
            uint8_t* data = &buffer[y * height + (x / 2)];

            if (x & 0x01 == 1) {
                *data &= 0xF0;
                *data |= value;
            }
            else {
                *data &= 0x0F;
                *data |= (value << 4);
            }

            break;
        }
    }
}

void FrameBuffer::drawChar(char c, unsigned x, unsigned y, uint16_t color)
{
    for (int i = 0; i < 5; ++i) {
        uint8_t currentChar = ASCII_FONT[c * 5 + i];

        for (int j = 0; j < 8; ++j) {
            if ((currentChar & (0x01 << j)) > 0) {
                drawPixel(x + i, y + j, color);
            }
        }
    }
}

void FrameBuffer::drawText(string str, unsigned x, unsigned y, uint16_t color)
{
    for (size_t i = 0; i < str.length(); ++i) {
        drawChar(str[i], x + i * 6, y, color);
    }
}
