#include "frameBuffer.h"

#define F_PI 3.14159265358979323846

#include <cmath>
#include <vector>

#include "font.h"

using namespace codal;
using namespace std;

constexpr float deg_to_rad = F_PI / 180.0;

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

        default:
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
        default:
            break;
    }
}

void FrameBuffer::drawPixel(unsigned x, unsigned y, uint16_t color)
{
    if (x >= width || y >= height) {
        return;
    }

    int shift, index;
    uint8_t* data;
    switch (format) {
        case Format::MONO_VLSB:
            shift = y % 8;
            index = x + ((y / 8) * width);
            data  = &buffer[index];

            if (color > 0) {
                *data |= (0x01 << shift);
            }
            else {
                *data &= ~(0x01 << shift);
            }
            break;

        case Format::GS4_HMSB: {
            uint8_t value = color & 0x000F;
            uint8_t* data = &buffer[y * (width / 2) + (x / 2)];

            if ((x & 0x01) == 1) {
                *data &= 0xF0;
                *data |= value;
            }
            else {
                *data &= 0x0F;
                *data |= (value << 4);
            }

            break;
        }

        default:
            break;
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

uint16_t FrameBuffer::getPixelColor(uint8_t x, uint8_t y)
{
    if (x >= width || y >= height) {
        return 0;
    }

    int shift, index;
    switch (format) {
        case Format::MONO_VLSB:
            shift = y % 8;
            index = x + ((y / 8) * width);
            if ((buffer[index] & (uint8_t)(0x01 << shift)) == 0) {
                return 0;
            }
            else {
                return 1;
            }
            break;

        case Format::GS4_HMSB: {
            uint8_t data = buffer[y * (width / 2) + (x / 2)];

            if ((x & 0x01) == 1) {
                return data & 0x0F;
            }
            else {
                return (data & 0xF0) >> 4;
            }
            break;
        }

        default:
            break;
    }

    return -1;
}

void swapPoint(int16_t& x1, int16_t& y1, int16_t& x2, int16_t& y2)
{
    int16_t tempo = x1;
    x1            = x2;
    x2            = tempo;

    tempo = y1;
    y1    = y2;
    y2    = tempo;
}

void drawSegmentOrLine() {}

void FrameBuffer::drawSegment(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t size, uint16_t color)
{
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;

    if (abs(dy) < abs(dx)) {
        if (x2 < x1) {
            swapPoint(x1, y1, x2, y2);
        }
        for (uint8_t s = 0; s < size; ++s) {
            for (int16_t x = x1; x <= x2; ++x) {
                int16_t y = y1 + dy * (x - x1) / dx + s;
                drawPixel(x, y, color);
            }
        }
    }
    else {
        if (y2 < y1) {
            swapPoint(x1, y1, x2, y2);
        }
        for (uint8_t s = 0; s < size; ++s) {
            for (int16_t y = y1; y <= y2; ++y) {
                int16_t x = x1 + dx * (y - y1) / dy + s;
                drawPixel(x, y, color);
            }
        }
    }
}

void FrameBuffer::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t size, uint16_t color)
{
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;

    if (abs(dy) < abs(dx)) {
        if (x2 < x1) {
            swapPoint(x1, y1, x2, y2);
        }
        for (uint8_t s = 0; s < size; ++s) {
            for (uint8_t x = 0; x < width; x++) {
                int16_t y = y1 + std::ceil(dy * (x - x1)) / dx + s;
                drawPixel(x, y, color);
            }
        }
    }
    else {
        if (y2 < y1) {
            swapPoint(x1, y1, x2, y2);
        }
        for (uint8_t s = 0; s < size; ++s) {
            for (uint8_t y = 0; y < height; y++) {
                int16_t x = x1 + std::ceil(dx * (y - y1)) / dy + s;
                drawPixel(x, y, color);
            }
        }
    }
}

int16_t getPoint(int16_t pt1, int16_t pt2, float percentage)
{
    int16_t diff = pt2 - pt1;
    return pt1 + (diff * percentage);
}

// Pas sur de le garder car pas très utile
void FrameBuffer::drawBezierCurve(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3,
                                  uint16_t color)
{
    int16_t nbrPixel;
    if (x1 < x3 && y1 < y3) {
        nbrPixel = abs(x3 - x1 + y3 - y1);
    }
    else if (x1 > x3 && y1 < y3) {
        nbrPixel = abs(x1 - x3 + y3 - y1);
    }
    else {
        nbrPixel = abs(x3 - x1 + y1 - y3);
    }

    for (float i = 0; i < nbrPixel; ++i) {
        uint16_t xa = getPoint(x1, x2, i / nbrPixel);
        uint16_t ya = getPoint(y1, y2, i / nbrPixel);
        uint16_t xb = getPoint(x2, x3, i / nbrPixel);
        uint16_t yb = getPoint(y2, y3, i / nbrPixel);

        uint16_t x = getPoint(xa, xb, i / nbrPixel);
        uint16_t y = getPoint(ya, yb, i / nbrPixel);

        drawPixel(x, y, color);
    }
}

void FrameBuffer::drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool fill, uint16_t color)
{
    if (fill) {
        uint8_t x = x1;
        if (x2 < x1) {
            x = x2;
        }
        drawSegment(x, y1, x, y2, abs(x2 - x1), color);
        return;
    }

    drawSegment(x1, y1, x2, y1, 1, color);
    drawSegment(x1, y1, x1, y2, 1, color);
    drawSegment(x2, y2, x2, y1, 1, color);
    drawSegment(x2, y2, x1, y2, 1, color);
}

void FrameBuffer::drawSquare(uint16_t x, uint16_t y, uint16_t lenght, bool fill, uint16_t color)
{
    drawRectangle(x, y, (x + lenght), (y + lenght), fill, color);
}

void FrameBuffer::drawEllipse(int width, int height, int xCenter, int yCenter, bool fill, uint16_t color)
{
    int dx, dy, d1, d2, x, y;
    x = 0;
    y = height;

    d1         = (height * height) - (width * width * height) + (0.25 * width * width);
    dx         = 2 * height * height * x;
    dy         = 2 * width * width * y;
    bool start = true;
    while (dx < dy) {
        if (fill) {
            if (start) {
                drawSegment(x + xCenter, y + yCenter, x + xCenter, -y + yCenter, 1, color);
                start = false;
                continue;
            }
            drawSegment(x + xCenter, y + yCenter, x + xCenter, -y + yCenter, 1, color);
            drawSegment(-x + xCenter, y + yCenter, -x + xCenter, -y + yCenter, 1, color);
        }
        else {
            drawPixel(x + xCenter, y + yCenter, color);
            drawPixel(-x + xCenter, y + yCenter, color);
            drawPixel(x + xCenter, -y + yCenter, color);
            drawPixel(-x + xCenter, -y + yCenter, color);
        }

        if (d1 < 0) {
            x++;
            dx = dx + (2 * height * height);
            d1 = d1 + dx + (height * height);
        }
        else {
            x++;
            y--;
            dx = dx + (2 * height * height);
            dy = dy - (2 * width * width);
            d1 = d1 + dx - dy + (height * height);
        }
    }

    d2 = ((height * height) * ((x + 0.5) * (x + 0.5))) + ((width * width) * ((y - 1) * (y - 1))) -
         (width * width * height * height);
    uint8_t lastX = 0;
    while (y >= 0) {
        if (fill) {
            if (lastX != x + xCenter) {
                lastX = x + xCenter;
                drawSegment(x + xCenter, y + yCenter, x + xCenter, -y + yCenter, 1, color);
                drawSegment(-x + xCenter, y + yCenter, -x + xCenter, -y + yCenter, 1, color);
            }
        }
        else {
            drawPixel(x + xCenter, y + yCenter, color);
            drawPixel(-x + xCenter, y + yCenter, color);
            drawPixel(x + xCenter, -y + yCenter, color);
            drawPixel(-x + xCenter, -y + yCenter, color);
        }

        if (d2 > 0) {
            y--;
            dy = dy - (2 * width * width);
            d2 = d2 + (width * width) - dy;
        }
        else {
            y--;
            x++;
            dx = dx + (2 * height * height);
            dy = dy - (2 * width * width);
            d2 = d2 + dx - dy + (width * width);
        }
    }
}

void FrameBuffer::drawCircle(int x, int y, int radius, bool fill, uint16_t color)
{
    drawEllipse(radius * 2, radius * 2, x, y, fill, color);
}

void FrameBuffer::drawArc(int x, int y, int radius, int start_angle, int end_angle, uint16_t color)
{
    float theta_rad = 0;

    for (int theta = start_angle; theta <= end_angle; ++theta) {
        theta_rad = theta * deg_to_rad;
        drawPixel(x + radius * cos(theta_rad), y + radius * sin(theta_rad), color);
    }
}

void FrameBuffer::drawPolygon(uint8_t x, uint8_t y, uint8_t line, uint8_t radius, uint8_t size, uint16_t color)
{
    if (line < 3) return;

    int lastX      = x + radius - 1;
    int lastY      = y;
    int angleEcart = 360 / line;

    for (unsigned i = angleEcart; i < 360; i = i + angleEcart) {
        float radToDeg = i * F_PI / 180;
        int newX       = (cos(radToDeg) * radius) + x;
        int newY       = (sin(radToDeg) * radius) + y;

        drawPixel(newX, newY, color);
        drawSegment(newX, newY, lastX, lastY, size, color);

        lastX = newX;
        lastY = newY;
    }
    drawSegment(lastX, lastY, x + radius - 1, y, size, color);
}

void FrameBuffer::drawMatrix(std::vector<std::vector<uint16_t>> matrix, unsigned x, unsigned y)
{
    for (unsigned i = 0; i < matrix.size(); ++i) {
        for (unsigned j = 0; j < matrix[i].size(); ++j) {
            drawPixel(j + x, i + y, matrix[i][j]);
        }
    }
}