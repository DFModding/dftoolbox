#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include "system.hpp"
#include "util.hpp"

void stuf4(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::binary);

  uint32_t magic   = read_uint32(in); // always: STFU
  uint32_t num1    = read_uint32(in); // always: 1143118388
  uint32_t num2    = read_uint32(in); // always: 0
  uint32_t num3    = read_uint32(in); // always: 1
  uint32_t width   = read_uint32(in);
  uint32_t height  = read_uint32(in);
  uint32_t mipmaps = read_uint32(in);// variable, never larger then 12
  uint32_t num5    = read_uint32(in); // 0 - 4
  uint32_t num6    = read_uint32(in); // 2176 - 8912896 // block size: 2*(width*height) + (width*height)/8)
  uint32_t width2  = read_uint32(in); // width, power of 2
  uint32_t height2 = read_uint32(in); // height, power of 2
  uint32_t num9    = read_uint32(in); // 8-512, power of 2, 'skip', width/4

  if ((width != width2) || (height != height2))
    {
      std::cout << "ERROR: WIDTH HEIGHT MISSMATCH" << std::endl;
    }

  if (num6 != 2*(width*height) + (width*height)/8)
    {
      std::cout << "ERROR: Nummer6: " << num6 << " vs "
                << 2*(width*height) + (width*height)/8 << std::endl;
    }

  if (num9 != width/4)
    {
      std::cout << "ERROR: SKIP: " << num9 << " " << width/4 << std::endl;
    }

  std::cout << "magic: " << std::string((char*)&magic, 4) << std::endl;
  std::cout << "num1: " << num1 << std::endl;
  std::cout << "num2: " << num2 << std::endl;
  std::cout << "num3: " << num3 << std::endl;
  std::cout << "Width:  " << width << std::endl;
  std::cout << "Height: " << height << std::endl;
  std::cout << "mipmaps: " << mipmaps << std::endl;
  std::cout << "num5: " << num5 << std::endl;
  std::cout << "num6: " << num6 << std::endl;
  std::cout << "width2: " << width2 << std::endl;
  std::cout << "heigh2t: " << height2 << std::endl;
  std::cout << "num9 " << num9 << std::endl;

  std::ofstream out("data.dat", std::ios::binary);

  if (1)
    out << "P6\n"
        << "# CREATOR: The GIMP's PNM Filter Version 1.0\n"
        << width << " " << height << "\n"
        << "255\n" << std::flush;
  else
    out << "P6\n"
        << "# CREATOR: The GIMP's PNM Filter Version 1.0\n"
        << width/4 << " " << height/4 << "\n"
        << "255\n" << std::flush;

  std::cout << "Snoknow" << std::endl;
  for(uint32_t y = 0; y < height/4; ++y)
    {
      unsigned char bumpmap[width/8*4];
      in.read  ((char*)bumpmap, sizeof(bumpmap));

      if (0)
        for(unsigned int i = 0; i < sizeof(bumpmap); i+=2)
          {
            //if (bumpmap[i] > 64)
            //std::cout << int(bumpmap[i]) << std::endl;
            if (0)
              {
                out.put((bumpmap[i+0] + bumpmap[i+1])/2);
                out.put((bumpmap[i+0] + bumpmap[i+1])/2);
                out.put((bumpmap[i+0] + bumpmap[i+1])/2);
              }
            else
              {
                out.put(bumpmap[i+1]);
                out.put(bumpmap[i+1]);
                out.put(bumpmap[i+1]);
              }

            //out.put(bumpmap[i+1]*4);
            //out.put(bumpmap[i+1]*4);
            //out.put(bumpmap[i+1]*4);

          }

      for(int j = 0; j < 4; ++j)
        {
          unsigned char nmlmap[width/2*4];
          in.read  ((char*)nmlmap, sizeof(nmlmap));

          if (1)
            {
              for(unsigned int i = 0; i < sizeof(nmlmap); i += 2)
                {
                  //out.put((nmlmap[i+1]&3)<<6);
                  //out.put((((nmlmap[i+1]&3)<<6) | ((nmlmap[i+0]&0xE0)>>5)) << 3);
                  out.put(128+((nmlmap[i+0]&0xFF)<<0)); // 6bits are enough
                  out.put(0);
                  out.put(0);
                  //out.put(nmlmap[i+1]& (~3)); // 6bits are enough
                  //out.put((((nmlmap[i+1]&3)<<6) | ((nmlmap[i+0]&0xF0)>>2)));
                }
            }
          else
            {
            }
        }

      /*

      out.write(bumpmap, sizeof(bumpmap));
      out.write(nmlmap, sizeof(nmlmap));


      for(int i = 0; i < 3; ++i)
      for(uint32_t x = 0; x < width; ++x)
      {
      out.put(buffer1[2*x/4 + 0]);
      out.put(byyuffer1[2*x/4 + 1]);
      }*/
    }
  out.close();

  in.close();
}

int main(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i)
    {
      std::cout << "File: " << argv[i] << std::endl;
      stuf4(argv[i]);
      std::cout << std::endl;
    }
}

/* EOF */
