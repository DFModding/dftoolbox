#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "util.hpp"
#include "system.hpp"

void cir_list(std::istream& in)
{
  uint32_t version = read_uint32(in);

  uint32_t count1 = read_uint32(in);

  std::cout << "version: " << version << std::endl;
  std::cout << "count1:  " << count1 << std::endl;

  std::cout << read_float(in) << std::endl;
  std::cout << read_float(in) << std::endl;
  std::cout << read_float(in) << std::endl;

  uint32_t count2 = read_uint32(in);
  std::cout << "count2:  " << count2 << std::endl;

  uint32_t len;

  for(uint32_t i = 0; i < count2; ++i)
    {
      std::cout << std::endl;
      len = read_uint32(in);
      std::cout << read_string(in, len) << std::endl;

      std::cout << "Zero: " << read_uint32(in) << std::endl;
      len = read_uint32(in);
      std::cout << '"' << read_string(in, len) << '"' << std::endl;
      std::cout << read_float(in) << std::endl;
      std::cout << read_float(in) << std::endl;
      std::cout << read_float(in) << std::endl;
    }

  std::cout << "\nSome: " << read_uint32(in) << std::endl;
  uint32_t countn;
  std::cout << "\nSome: " << (countn  = read_uint32(in)) << std::endl;
  for(uint32_t i = 0; i < countn; ++i)
    {
      std::cout << read_uint32(in) << std::endl;
      std::cout << read_uint32(in) << std::endl;
      std::cout << read_uint32(in) << std::endl;
      std::cout << read_uint32(in) << std::endl;
      //std::cout << std::endl;

    }

  len = read_uint32(in);
  std::cout << '"' << read_string(in, len) << '"' << std::endl;
  std::cout << read_uint32(in) << std::endl;
  std::cout << read_uint32(in) << std::endl;
  uint32_t c;
  std::cout << (c = read_uint32(in)) << std::endl;

  for(uint32_t i = 0; i < 11; ++i)
    std::cout << read_float(in) << std::endl;
  std::cout << "INT: " << read_uint32(in) << std::endl;
  std::cout << "INT: " << read_uint32(in) << std::endl;

  for(uint32_t i = 0; i < c-1; ++i)
    {
      std::cout << std::endl;

      for(uint32_t j = 0; j < 12; ++j)
        std::cout << read_float(in) << std::endl;

      std::cout << "INT: " << read_uint32(in) << std::endl;
      std::cout << "INT: " << read_uint32(in) << std::endl;
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
          std::cout << "#### Filename: " << argv[i] << std::endl;
          cir_list(in);
          in.close();
        }
    }
}

/* EOF */
