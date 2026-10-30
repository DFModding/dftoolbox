/*  $Id: dds.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 3
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HEADER_DDSTOOL_HPP
#define HEADER_DDSTOOL_HPP

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "system.hpp"
#include "util.hpp"
#include "rgb.hpp"

#define DDPF_ALPHAPIXELS                0x00000001
#define DDPF_ALPHA                      0x00000002
#define DDPF_FOURCC                     0x00000004
#define DDPF_PALETTEINDEXED4            0x00000008
#define DDPF_PALETTEINDEXEDTO8          0x00000010
#define DDPF_PALETTEINDEXED8            0x00000020
#define DDPF_RGB                        0x00000040
#define DDPF_COMPRESSED                 0x00000080
#define DDPF_RGBTOYUV                   0x00000100
#define DDPF_YUV                        0x00000200
#define DDPF_ZBUFFER                    0x00000400
#define DDPF_PALETTEINDEXED1            0x00000800
#define DDPF_PALETTEINDEXED2            0x00001000
#define DDPF_ZPIXELS                    0x00002000
#define DDPF_STENCILBUFFER              0x00004000
#define DDPF_ALPHAPREMULT               0x00008000
#define DDPF_LUMINANCE                  0x00020000
#define DDPF_BUMPLUMINANCE              0x00040000
#define DDPF_BUMPDUDV                   0x00080000

class DDSCapabilities
{
public:
  // DDS files should always include DDSCAPS_TEXTURE. If the file
  // contains mipmaps, DDSCAPS_MIPMAP should be set. For any .dds file
  // with more than one main surface, such as a mipmaps, cubic
  // environment map, or volume texture, DDSCAPS_COMPLEX should also
  // be set.
  uint32_t dwCaps1;

  // For cubic environment maps, DDSCAPS2_CUBEMAP should be included
  // as well as one or more faces of the map
  // (DDSCAPS2_CUBEMAP_POSITIVEX, DDSCAPS2_CUBEMAP_NEGATIVEX,
  // DDSCAPS2_CUBEMAP_POSITIVEY, DDSCAPS2_CUBEMAP_NEGATIVEY,
  // DDSCAPS2_CUBEMAP_POSITIVEZ, DDSCAPS2_CUBEMAP_NEGATIVEZ). For
  // volume textures, DDSCAPS2_VOLUME should be included.
  uint32_t dwCaps2;

  // Unused
  uint32_t dwReserved[2];

  DDSCapabilities()
  {
    dwCaps1 = 0;
    dwCaps2 = 0;
    dwReserved[0] = 0;
    dwReserved[1] = 0;
  }

  DDSCapabilities(std::istream& in)
  {
    dwCaps1 = read_uint32(in);
    dwCaps2 = read_uint32(in);
    dwReserved[0] = read_uint32(in);
    dwReserved[1] = read_uint32(in);
  }
};

class PixelFormat
{
public:
  // Size of structure. This member must be set to 32.
  uint32_t dwSize;

  // Flags to indicate valid fields. Uncompressed formats will usually
  // use DDPF_RGB to indicate an RGB format, while compressed formats
  // will use DDPF_FOURCC with a four-character code.
  uint32_t dwFlags;

  // This is the four-character code for compressed formats. dwFlags
  // should include DDPF_FOURCC in this case. For DXTn compression,
  // this is set to "DXT1", "DXT2", "DXT3", "DXT4", or "DXT5".
  uint32_t dwFourCC;

  // For RGB formats, this is the total number of bits in the
  // format. dwFlags should include DDPF_RGB in this case. This value
  // is usually 16, 24, or 32. For A8R8G8B8, this value would be 32.
  uint32_t dwRGBBitCount;

  // For RGB formats, these three fields contain the masks for the
  // red, green, and blue channels. For A8R8G8B8, these values would
  // be 0x00ff0000, 0x0000ff00, and 0x000000ff respectively.
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;

  // For RGB formats, this contains the mask for the alpha channel, if
  // any. dwFlags should include DDPF_ALPHAPIXELS in this case. For
  // A8R8G8B8, this value would be 0xff000000.
  uint32_t dwRGBAlphaBitMask;

  PixelFormat()
  {
  }

  PixelFormat(std::istream& in)
  {
    dwSize   = read_uint32(in);
    dwFlags  = read_uint32(in);
    dwFourCC = read_uint32(in);
    dwRGBBitCount = read_uint32(in);
    dwRBitMask = read_uint32(in);
    dwGBitMask = read_uint32(in);
    dwBBitMask = read_uint32(in);
    dwRGBAlphaBitMask = read_uint32(in);
  }
};

enum {
  DDS_DXT1 = 827611204,
  DDS_DXT2 = 844388420,
  DDS_DXT3 = 861165636,
  DDS_DXT4 = 877942852,
  DDS_DXT5 = 894720068
};

enum DDSFlags {
  DDSD_ALL          = 1047022,
  DDSD_ALPHABITDEPTH =    128,
  DDSD_BACKBUFFERCOUNT =   32,
  DDSD_CAPS         =       1,
  DDSD_CKDESTBLT    =   16384,
  DDSD_CKDESTOVERLAY =   8192,
  DDSD_CKSRCBLT     =   65536,
  DDSD_CKSRCOVERLAY =   32768,
  DDSD_HEIGHT       =       2,
  DDSD_LINEARSIZE   =  524288,
  DDSD_LPSURFACE    =    2048,
  DDSD_MIPMAPCOUNT  =  131072,
  DDSD_PITCH        =       8,
  DDSD_PIXELFORMAT  =    4096,
  DDSD_REFRESHRATE  =  262144,
  DDSD_TEXTURESTAGE = 1048576,
  DDSD_WIDTH        =       4,
  DDSD_ZBUFFERBITDEPTH =   64
};

class DDS
{
public:
  uint32_t magic;

  // Size of structure. This member must be set to 124.
  uint32_t dwSize;

  // Flags to indicate valid fields. Always include DDSD_CAPS,
  // DDSD_PIXELFORMAT, DDSD_WIDTH, DDSD_HEIGHT.
  uint32_t flags;

  // Height of the main image in pixels
  uint32_t height;

  // Width of the main image in pixels
  uint32_t width;

  // For uncompressed formats, this is the number of bytes per scan
  // line (uint32_t> aligned) for the main image. dwFlags should include
  // DDSD_PITCH in this case. For compressed formats, this is the
  // total number of bytes for the main image. dwFlags should be
  // include DDSD_LINEARSIZE in this case.
  uint32_t dwPitchOrLinearSize;

  // For volume textures, this is the depth of the volume. dwFlags
  // should include DDSD_DEPTH in this case.
  uint32_t dwDepth;

  // For items with mipmap levels, this is the total number of levels
  // in the mipmap chain of the main image. dwFlags should include
  // DDSD_MIPMAPCOUNT in this case.
  uint32_t dwMipMapCount;

  // Unused
  uint32_t dwReserved1[11];

  // 32-byte value that specifies the pixel format structure.
  PixelFormat pixel_format;

  // 16-byte value that specifies the capabilities structure.
  DDSCapabilities ddsCaps;

  // Unused
  uint32_t dwReserved2;

private:
  // Contains the data of the image in RGBA format, only valid after calling read_data()
  std::vector<char> data;

public:
  DDS(std::istream& in) {
    magic  = read_uint32(in);
    dwSize = read_uint32(in);
    flags  = read_uint32(in);
    height = read_uint32(in);
    width  = read_uint32(in);
    dwPitchOrLinearSize = read_uint32(in);
    dwDepth = read_uint32(in);
    dwMipMapCount = read_uint32(in);

    for(int i = 0; i < 11; ++i)
      dwReserved1[i] = read_uint32(in);

    pixel_format = PixelFormat(in);

    ddsCaps = DDSCapabilities(in);

    dwReserved2 = read_uint32(in);

    read_data(in);
  }

  const std::vector<char>& get_data() {
    return data;
  }

  /** Reads the top level mipmap of the image and converts it to RGBA */
  void read_data(std::istream& in)
  {
    if (width == 0 || height == 0 || width > 16384 || height > 16384)
      {
        std::ostringstream str;
        str << "DDS: invalid image size " << width << "x" << height;
        throw std::runtime_error(str.str());
      }

    data.resize(width * height * 4);

    if (pixel_format.dwFlags & DDPF_FOURCC)
      {
        switch (pixel_format.dwFourCC)
          {
          case DDS_DXT1:
            read_data_dxt(in, 1);
            break;

          case DDS_DXT2:
          case DDS_DXT3:
            read_data_dxt(in, 3);
            break;

          case DDS_DXT4:
          case DDS_DXT5:
            read_data_dxt(in, 5);
            break;

          default:
            {
              std::ostringstream str;
              str << "Format unknown: " << pixel_format.dwFourCC << " " << std::string((char*)&pixel_format.dwFourCC, 4);
              throw std::runtime_error(str.str());
            }
          }
      }
    else if (pixel_format.dwFlags & (DDPF_RGB | DDPF_LUMINANCE | DDPF_ALPHA))
      {
        read_data_uncompressed(in);
      }
    else
      {
        std::ostringstream str;
        str << "DDS: unsupported pixel format flags " << pixel_format.dwFlags;
        throw std::runtime_error(str.str());
      }

    if (!in)
      throw std::runtime_error("DDS: unexpected end of file");
  }

private:
  static unsigned int mask_shift(uint32_t mask)
  {
    unsigned int shift = 0;
    while (mask && !(mask & 1))
      {
        mask >>= 1;
        shift += 1;
      }
    return shift;
  }

  /** Extracts the channel given by \a mask from \a pixel and scales it to 0-255 */
  static unsigned char get_channel(uint32_t pixel, uint32_t mask)
  {
    if (mask == 0)
      return 0;

    unsigned int shift = mask_shift(mask);
    uint32_t max = mask >> shift;
    return static_cast<unsigned char>((((pixel & mask) >> shift) * 255 + max/2) / max);
  }

  void read_data_uncompressed(std::istream& in)
  {
    unsigned int bpp = pixel_format.dwRGBBitCount / 8;
    if (bpp < 1 || bpp > 4)
      {
        std::ostringstream str;
        str << "DDS: unsupported bit count " << pixel_format.dwRGBBitCount;
        throw std::runtime_error(str.str());
      }

    bool has_alpha = (pixel_format.dwFlags & (DDPF_ALPHAPIXELS | DDPF_ALPHA)) && pixel_format.dwRGBAlphaBitMask;

    std::vector<unsigned char> line(width * bpp);
    for(unsigned int y = 0; y < height; ++y)
      {
        in.read(reinterpret_cast<char*>(&*line.begin()), line.size());

        for(unsigned int x = 0; x < width; ++x)
          {
            uint32_t pixel = 0;
            for(unsigned int b = 0; b < bpp; ++b)
              pixel |= uint32_t(line[x*bpp + b]) << (8*b);

            char* out = &data[4*(y*width + x)];
            if (pixel_format.dwFlags & DDPF_LUMINANCE)
              {
                out[0] = out[1] = out[2] = get_channel(pixel, pixel_format.dwRBitMask);
              }
            else if (pixel_format.dwFlags & DDPF_RGB)
              {
                out[0] = get_channel(pixel, pixel_format.dwRBitMask);
                out[1] = get_channel(pixel, pixel_format.dwGBitMask);
                out[2] = get_channel(pixel, pixel_format.dwBBitMask);
              }
            else
              { // alpha only
                out[0] = out[1] = out[2] = 255;
              }
            out[3] = has_alpha ? get_channel(pixel, pixel_format.dwRGBAlphaBitMask) : 255;
          }
      }
  }

  /** Decodes a 4x4 DXT color block into RGBA, \a dxt1 enables
      the one bit alpha mode of DXT1 */
  static void decode_color_block(const unsigned char buf[8], unsigned char out[4*4*4], bool dxt1)
  {
    unsigned short color0 = buf[0] | (buf[1] << 8);
    unsigned short color1 = buf[2] | (buf[3] << 8);

    unsigned int bits = buf[4] | (buf[5] << 8) | (buf[6] << 16) | (unsigned(buf[7]) << 24);

    // RGB stores the 5:6:5 value with blue in 'r' and red in 'b'
    RGB rgb[4];
    unsigned char alpha[4] = { 255, 255, 255, 255 };

    rgb[0] = color0;
    rgb[1] = color1;

    if (color0 > color1 || !dxt1)
      {
        rgb[2] = (2*rgb[0] + rgb[1])/3;
        rgb[3] = (rgb[0] + 2*rgb[1])/3;
      }
    else
      {
        rgb[2] = (rgb[0] + rgb[1])/2;
        rgb[3] = RGB();
        alpha[3] = 0;
      }

    for(int i = 0; i < 16; ++i)
      {
        unsigned int idx = (bits >> (2*i)) & 0x3;

        out[4*i + 0] = rgb[idx].b;
        out[4*i + 1] = rgb[idx].g;
        out[4*i + 2] = rgb[idx].r;
        out[4*i + 3] = alpha[idx];
      }
  }

  /** Decodes a DXT3 explicit alpha block into \a out */
  static void decode_dxt3_alpha(const unsigned char buf[8], unsigned char out[4*4*4])
  {
    for(int i = 0; i < 16; ++i)
      {
        unsigned int a = (buf[i/2] >> (4*(i%2))) & 0xf;
        out[4*i + 3] = static_cast<unsigned char>(a * 17);
      }
  }

  /** Decodes a DXT5 interpolated alpha block into \a out */
  static void decode_dxt5_alpha(const unsigned char buf[8], unsigned char out[4*4*4])
  {
    unsigned int alpha[8];
    alpha[0] = buf[0];
    alpha[1] = buf[1];

    if (alpha[0] > alpha[1])
      {
        for(int i = 1; i < 7; ++i)
          alpha[i+1] = ((7-i)*alpha[0] + i*alpha[1]) / 7;
      }
    else
      {
        for(int i = 1; i < 5; ++i)
          alpha[i+1] = ((5-i)*alpha[0] + i*alpha[1]) / 5;
        alpha[6] = 0;
        alpha[7] = 255;
      }

    uint64_t bits = 0;
    for(int i = 0; i < 6; ++i)
      bits |= uint64_t(buf[2+i]) << (8*i);

    for(int i = 0; i < 16; ++i)
      out[4*i + 3] = static_cast<unsigned char>(alpha[(bits >> (3*i)) & 0x7]);
  }

  /** \a format is 1, 3 or 5 for DXT1, DXT3 and DXT5 */
  void read_data_dxt(std::istream& in, int format)
  {
    unsigned char alpha_buf[8];
    unsigned char color_buf[8];
    unsigned char out[4*4*4];

    for(unsigned int y = 0; y < height; y += 4)
      for(unsigned int x = 0; x < width; x += 4)
        {
          // in DXT3 and DXT5 the alpha block comes before the color block
          if (format != 1)
            in.read(reinterpret_cast<char*>(alpha_buf), 8);
          in.read(reinterpret_cast<char*>(color_buf), 8);

          decode_color_block(color_buf, out, format == 1);
          if (format == 3)
            decode_dxt3_alpha(alpha_buf, out);
          else if (format == 5)
            decode_dxt5_alpha(alpha_buf, out);

          // images smaller than 4x4 or with odd sizes only use part of the block
          for(unsigned int y1 = 0; y1 < 4 && y + y1 < height; y1 += 1)
            for(unsigned int x1 = 0; x1 < 4 && x + x1 < width; x1 += 1)
              for(int c = 0; c < 4; ++c)
                data[4*((y+y1)*width + (x+x1)) + c] = out[4*(4*y1 + x1) + c];
        }
  }
};

#endif

/* EOF */
