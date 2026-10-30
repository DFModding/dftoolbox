// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stfu.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {

/** The value of bits 7-3 for a pixel without a slope in Y, indexed by
    bits 15-8. Measured from the 1661 different STFU files of the game
    as the most common value, where it comes from is not known, see
    doc/stfu-format.txt. */
const uint8_t y_offset[256] = {
  31, 22, 17, 13, 24, 16, 10, 29, 30, 23, 25, 30, 25, 22, 16,  3,
   2, 30, 28, 28, 31,  0, 30, 26, 28, 29, 29,  2, 28, 22,  9, 24,
  31,  1, 23, 31,  1,  8,  7, 10,  9, 11, 13, 11, 11, 13, 23, 16,
   3,  8,  9,  8, 12,  5,  8,  9,  9, 11, 14, 18, 16, 19, 14, 18,
  22, 31, 30,  1,  6,  5,  9, 11, 15, 13, 16, 30,  0,  1,  5, 12,
  19, 26, 29,  2,  8, 31,  2, 15, 16, 13,  5, 13, 18, 21, 25, 23,
  31,  1,  8, 17, 13,  5,  0,  9, 12, 16, 14, 13,  8,  6,  2,  5,
   3,  6,  8,  1,  0, 17, 19, 22, 19, 17, 20, 16,  5,  6,  3, 15,
  27, 23, 24, 23, 28, 24, 23, 28, 23, 11, 15, 19, 25, 31,  0, 27,
  26, 24, 26,  3,  7,  3, 18, 16, 12, 11,  8,  7,  3,  8,  9, 20,
  18, 23, 23, 31,  1, 25, 26,  0, 10, 22, 20, 26,  0,  8, 10, 15,
  19, 21, 22, 14,  8, 12, 22, 30,  2,  5, 10, 17, 17, 18, 21, 25,
  21, 24, 20, 25, 21, 30,  2,  8,  8, 20, 27, 25, 20, 27, 23, 31,
   0,  8,  1, 28, 30,  1, 31,  0,  9,  7, 15, 13,  9, 12, 16, 21,
  26, 24, 18, 21, 24, 21, 22, 21, 28, 27, 25, 20, 13, 16, 16, 17,
  17, 15, 13, 16,  9,  8, 17,  8, 10,  4, 15, 12, 31,  0, 23, 13,
};

/** Bits 15-8 of a pixel without a slope in X */
const int x_center = 129;

uint32_t get_uint32(const std::vector<char>& data, size_t pos)
{
  if (pos + 4 > data.size())
    throw std::runtime_error("STFU: unexpected end of file");

  const unsigned char* p = reinterpret_cast<const unsigned char*>(data.data()) + pos;
  return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}

uint16_t get_uint16(const std::vector<char>& data, size_t pos)
{
  const unsigned char* p = reinterpret_cast<const unsigned char*>(data.data()) + pos;
  return static_cast<uint16_t>(p[0] | p[1] << 8);
}

/** Same as the alpha channel of DXT5, \a block holds the two endpoints */
uint8_t decode_alpha(uint16_t block, int index)
{
  const int a0 = block & 0xff;
  const int a1 = block >> 8;

  if (index == 0)
    return static_cast<uint8_t>(a0);
  else if (index == 1)
    return static_cast<uint8_t>(a1);
  else if (a0 > a1)
    return static_cast<uint8_t>(((8 - index) * a0 + (index - 1) * a1) / 7);
  else if (index < 6)
    return static_cast<uint8_t>(((6 - index) * a0 + (index - 1) * a1) / 5);
  else
    return index == 6 ? 0 : 255;
}

uint8_t to_byte(float value)
{
  return static_cast<uint8_t>(std::lround((value + 1.0f) * 127.5f));
}

} // namespace

StfuImage
read_stfu(std::istream& in)
{
  const std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

  if (data.size() < 48 || std::memcmp(data.data(), "STFU", 4) != 0)
    throw std::runtime_error("STFU: not a STFU file");

  if (get_uint32(data, 12) != 1)
    throw std::runtime_error("STFU: unknown version");

  // The first mipmap level follows the 32 byte file header
  const uint32_t size   = get_uint32(data, 32);
  const uint32_t width  = get_uint32(data, 36);
  const uint32_t height = get_uint32(data, 40);
  const uint32_t blocks_x = get_uint32(data, 44);

  if (width == 0 || height == 0 || width > 16384 || height > 16384)
    throw std::runtime_error("STFU: invalid image size");

  const uint32_t blocks_y = std::max(1u, height / 4);
  const uint64_t expected = uint64_t(width) * height * 2 + uint64_t(blocks_x) * blocks_y * 2;

  if (blocks_x != std::max(1u, width / 4) || size != expected ||
      height % blocks_y != 0 || width % blocks_x != 0)
    throw std::runtime_error("STFU: inconsistent level header");

  if (48 + expected > data.size())
    throw std::runtime_error("STFU: unexpected end of file");

  StfuImage image;
  image.width  = static_cast<int>(width);
  image.height = static_cast<int>(height);
  image.rgba.resize(size_t(width) * height * 4);

  // A band is one row of blocks followed by the rows of pixels they cover
  const uint32_t rows = height / blocks_y;
  const uint32_t columns = width / blocks_x;
  const size_t band_size = size_t(blocks_x) * 2 + size_t(width) * rows * 2;

  for(uint32_t y = 0; y < height; ++y)
    {
      const size_t band = 48 + (y / rows) * band_size;
      const size_t line = band + size_t(blocks_x) * 2 + size_t(y % rows) * width * 2;

      for(uint32_t x = 0; x < width; ++x)
        {
          const uint16_t pixel = get_uint16(data, line + size_t(x) * 2);
          const uint16_t block = get_uint16(data, band + size_t(x / columns) * 2);

          const int high = pixel >> 8;
          // The difference is circular, as bits 7-3 only hold the low
          // part of it. They are worth 8 steps of bits 15-8.
          const int y_slope = ((((pixel >> 3) & 31) - y_offset[high] + 16) & 31) - 16;

          float nx = static_cast<float>(high - x_center) / 127.0f;
          float ny = static_cast<float>(y_slope * 8) / 127.0f;
          float nz = 0.0f;

          const float length = nx * nx + ny * ny;
          if (length > 1.0f)
            {
              nx /= std::sqrt(length);
              ny /= std::sqrt(length);
            }
          else
            {
              nz = std::sqrt(1.0f - length);
            }

          uint8_t* out = image.rgba.data() + (size_t(y) * width + x) * 4;
          out[0] = to_byte(nx);
          out[1] = to_byte(ny);
          out[2] = to_byte(nz);
          out[3] = decode_alpha(block, pixel & 7);
        }
    }

  return image;
}

/* EOF */
