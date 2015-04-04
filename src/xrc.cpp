#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "util.hpp"
#include "system.hpp"

void xrc_list(std::istream& in)
{
  while(in)
    {
  uint16_t something1 = read_uint16(in);
  uint16_t something2 = read_uint16(in);

  uint16_t len = read_uint16(in);

  std::string str = read_string(in, len);

  /*uint32_t something3 =*/ read_uint32(in);
  /*uint32_t something4 =*/ read_uint32(in);

  std::cout << something1 << " " << something2 << " " << str << std::endl;
    }
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
          xrc_list(in);
          in.close();
          std::cout << std::endl;
        }
    }
}

/* EOF */
