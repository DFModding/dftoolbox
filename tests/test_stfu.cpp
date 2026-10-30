// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "stfu.hpp"

namespace {

int g_failures = 0;

void check(bool condition, const std::string& what)
{
  if (!condition)
    {
      std::cerr << "FAIL: " << what << std::endl;
      ++g_failures;
    }
}

void put_uint32(std::string& out, uint32_t value)
{
  for(int i = 0; i < 4; ++i)
    out += static_cast<char>((value >> (i * 8)) & 0xff);
}

void put_uint16(std::string& out, uint16_t value)
{
  out += static_cast<char>(value & 0xff);
  out += static_cast<char>(value >> 8);
}

/** A 4x4 file with no slopes, pixel 0 uses \a index for alpha */
std::string make_file(uint16_t block, int index)
{
  std::string out = "STFU";
  put_uint32(out, 0x44229A34);
  put_uint32(out, 0);
  put_uint32(out, 1);
  put_uint32(out, 4);
  put_uint32(out, 4);
  put_uint32(out, 1);
  put_uint32(out, 0);

  put_uint32(out, 34);
  put_uint32(out, 4);
  put_uint32(out, 4);
  put_uint32(out, 1);

  put_uint16(out, block);
  for(int i = 0; i < 16; ++i)
    put_uint16(out, static_cast<uint16_t>(0x81B8 | (i == 0 ? index : 0)));
  return out;
}

bool fails(const std::string& data)
{
  try {
    std::istringstream in(data);
    read_stfu(in);
    return false;
  } catch(const std::runtime_error&) {
    return true;
  }
}

} // namespace

int main()
{
  {
    std::istringstream in(make_file(0x4020, 2));
    const StfuImage image = read_stfu(in);
    check(image.width == 4 && image.height == 4, "size");
    check(image.rgba.size() == 64, "data size");
    check(image.rgba[0] == 128 && image.rgba[1] == 128 && image.rgba[2] == 255, "flat normal");

    // endpoints 0x20 < 0x40 select the 6 value mode
    check(image.rgba[3] == (4 * 0x20 + 1 * 0x40) / 5, "interpolated alpha");
    check(image.rgba[7] == 0x20, "first endpoint");
  }

  const std::string good = make_file(0xffff, 0);
  check(!fails(good), "valid file");
  check(fails(""), "empty file");
  check(fails("DDS " + good.substr(4)), "wrong magic");
  check(fails(good.substr(0, good.size() - 1)), "truncated data");

  std::string bad_size = good;
  bad_size[36] = 8;
  check(fails(bad_size), "level header doesn't match the data");

  return g_failures == 0 ? 0 : 1;
}
