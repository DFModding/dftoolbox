#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "util.hpp"
#include "system.hpp"

void xmp_list(std::istream& in)
{
  uint32_t version   = read_uint32(in);
  uint32_t something = read_uint32(in);  // 2x something: 16711935,  269x something: 16776960


  uint32_t width  = read_uint32(in);
  uint32_t height = read_uint32(in);

  uint32_t colors = read_uint32(in); // 33 - 198

  uint32_t zeros = read_uint32(in); // always zero

  uint32_t value = read_uint32(in); // 108 - 9124, 10x 2412

  std::cout << "version:   " << version << std::endl;
  std::cout << "something: " << something << std::endl;
  std::cout << "width:     " << width << std::endl;
  std::cout << "height:    " << height << std::endl;
  std::cout << "colors:    " << colors << std::endl;
  std::cout << "zeros:     " << zeros << std::endl;
  std::cout << "value:     " << value << std::endl;
  std::cout << std::endl;

  std::ofstream out("data.dat", std::ios::binary);

  out << "P6\n"
      << "# CREATOR: The GIMP's PNM Filter Version 1.0\n"
      << width << " " << height/2 << "\n"
      << "255\n" << std::flush;
  
  int y = 0;
  while(in)
    {
      char buffer[3*width+2];
      in.read(buffer, sizeof(buffer));

      for(unsigned int x = 0; x < 3*width; x+=1)
        {
          out.put(buffer[x+3]);
        }

      y += 1;
    }
  out.put(0);
  out.put(0);
  out.close();
}

int main(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i)
    {
      std::ifstream in(argv[i], std::ios::binary);
      if (!in)
        {
          std::cerr << "Couldn't oppen: " << argv[i] << std::endl;
        }
      else
        {
          std::cout << "Filename:  " << argv[i] << std::endl;
          xmp_list(in);
          in.close();
        }
    } 
}

/* EOF */
