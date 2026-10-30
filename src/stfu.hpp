// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADER_STFU_HPP
#define HEADER_STFU_HPP

#include <cstdint>
#include <iosfwd>
#include <vector>

/** The top level of a STFU normal map, see doc/stfu-format.txt */
struct StfuImage
{
  int width;
  int height;

  /** RGBA, one byte per channel: the normal in RGB, as usual for
      normal maps, and the alpha channel of the texture */
  std::vector<uint8_t> rgba;
};

/** Decode the STFU file in \a in, throws on broken data */
StfuImage read_stfu(std::istream& in);

#endif

/* EOF */
