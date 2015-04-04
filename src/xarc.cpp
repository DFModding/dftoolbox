// Extractor for The Longest Journeys .xarc files

#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "util.hpp"
#include "system.hpp"

struct FileEntry
{
  std::string filename;
  uint32_t size;
  uint32_t version;
};

void xarc_list(std::istream& in)
{
  uint32_t version     = read_uint32(in);
  uint32_t count       = read_uint32(in);
  uint32_t data_offset = read_uint32(in);

  std::cout << "Version:     " << version << std::endl;
  std::cout << "Count:       " << count << std::endl;
  std::cout << "Data_Offset: " << data_offset << std::endl;
  std::cout << std::endl;

  std::vector<FileEntry> entries;
  for(uint32_t i = 0; i < count; ++i)
    {
      FileEntry entry;
      entry.filename = read_string0(in);
      entry.size     = read_uint32(in);
      entry.version  = read_uint32(in);

      std::cout << std::setw(40) << std::left  << entry.filename 
                << std::setw(10) << std::right << entry.size
                << std::setw(10) << std::right << entry.version
                << std::endl;
      entries.push_back(entry);
    }

  std::cout << std::endl;
  std::cout << "End: " << in.tellg() << " vs " << data_offset << std::endl;

  std::vector<char> filebuf;
  for(uint32_t i = 0; i < entries.size(); ++i)
    {
      std::ofstream out((std::string("tlj/") + entries[i].filename).c_str(), std::ios::binary);
      filebuf.resize(entries[i].size);
      in.read(&*filebuf.begin(), filebuf.size());
      out.write(&*filebuf.begin(), filebuf.size());
      
      out.close();
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
          xarc_list(in);
          in.close();
        }
    }
}

/* EOF */
