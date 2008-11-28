/*  $Id: extract.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#include <string.h>
#include <assert.h>
#include <iomanip>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>

#include "system.hpp"
#include "util.hpp"
#include "tljpak.hpp"
#include "extract.hpp"

void guess(const std::string& pak_filename)
{
  TLJPak pak(pak_filename);
  pak.guess();
}

void extract(const std::string& pak_filename, const std::string& pathname)
{
  TLJPak pak(pak_filename);
  
  if (0)
    {
      pak.print_file_table();
      pak.print_nametable();
      std::cout << std::endl;
    }

  int i = pak.lookup(pathname);
  std::cout << "Pathname:  " << pathname << " (" << pathname.size() << ")" << std::endl;
  std::cout << "FileEntry: " << i << std::endl;
  if (i != -1)
    {
      std::cout << pak.files[i].offset << " "
                << pak.files[i].filesize << " "
                << pak.files[i].nametable_offset << " "
                << pak.files[i].path_length << " "
                << pak.files[i].nametable_index << std::endl;
    }

  if (1)
    {
      std::ifstream in(pak_filename.c_str(), std::ios::binary);

      std::cout << "Extracting file to: data.dat" << std::endl;
      std::ofstream out("data.dat", std::ios::binary);

      
      std::vector<char> buffer(pak.files[i].filesize);
      
      in.seekg(pak.files[i].offset, std::ios::beg);
      in.read(reinterpret_cast<char*>(&*buffer.begin()),   buffer.size());

      out.write(reinterpret_cast<char*>(&*buffer.begin()), buffer.size());

      out.close();
    }
}

void extract(TLJPak& pak, std::istream& in, const std::string& outpath)
{
  std::string path = outpath;
  std::string prefix = get_exe_path() + "dreamfall-extractor/";

  create_dir(prefix);
  create_dir(prefix + path);
  create_dir(prefix + path + "/fonts");
  create_dir(prefix + path + "/textures");
  create_dir(prefix + path + "/wav");
  create_dir(prefix + path + "/mp3");
  create_dir(prefix + path + "/dat");
  create_dir(prefix + path + "/shader");
  create_dir(prefix + path + "/misc");
  create_dir(prefix + path + "/scripts");

  for(int i = 0; i < int(pak.files.size()); ++i)
    {
      if (pak.files[i].filesize > 0)
        {
          char filename[1024];

          char magic[8];
          in.seekg(pak.files[i].offset, std::ios::beg);
          in.read(magic, 8);

          if (strncmp(magic, "DDS", 3) == 0)
            snprintf(filename, 1024, "textures/%04d.dds", i);
          else if (strncmp(magic, "tljbone", 7) == 0)
            snprintf(filename, 1024, "misc/%04d.tljbone", i);
          else if (strncmp(magic, "ID3", 3) == 0)
            snprintf(filename, 1024, "mp3/%04d.mp3", i);
          else if (strncmp(magic, "vs", 2) == 0 || strncmp(magic, "xvs", 3) == 0)
            snprintf(filename, 1024, "shader/%04d.vs", i);
          else if (strncmp(magic, "ps", 2) == 0 || strncmp(magic, "xps", 3) == 0)
            snprintf(filename, 1024, "shader/%04d.ps", i);
          else if (strncmp(magic, "; ", 2) == 0 || strncmp(magic, "//", 2) == 0)
            snprintf(filename, 1024, "shader/%04d.us", i);
          else if (strncmp(magic, "STFU4", 4) == 0)
            snprintf(filename, 1024, "misc/%04d.stfu4", i);
          else if (strncmp(magic, "RIFF", 4) == 0)
            snprintf(filename, 1024, "wav/%04d.wav", i);
          else if (strncmp(magic, "shark3d", 7) == 0)
            snprintf(filename, 1024, "scripts/%04d.s3dsb", i);
          else if (((unsigned char)magic[0]) == 255 && ((unsigned char)magic[1]) == 0xFB)
            snprintf(filename, 1024, "mp3/%04d.mp3", i);
          //else if (int(magic[0]) == 0x07 && int(magic[1]) == 0x54)
          //  snprintf(filename, 1024, "%04d.directory", i);
          else if (strncmp(magic, "art/gui/", 8) == 0)
            snprintf(filename, 1024, "fonts/%04d.font", i);
          else
            snprintf(filename, 1024, "dat/%04d.dat", i);

          std::cout << prefix + path + "/" + filename << " " << pak.files[i].offset << " " << pak.files[i].filesize << std::endl;
          
          std::ofstream out((prefix + path + "/" + filename).c_str(), std::ios::binary);
          out.write(magic, 8);         
          for(int j = 0; j < pak.files[i].filesize-8; ++j)
            out.put(in.get());
          out.close();
        }
    }
}

std::string directory_name_from_pak(const std::string& name)
{
  std::string::size_type pos1 = name.find_last_of('\\');
  std::string::size_type pos2 = name.find_last_of('/');
  std::string::size_type dotpos = name.find_last_of('.');
  std::string::size_type namestart;

  if (pos1 == std::string::npos && pos2 == std::string::npos)
    namestart = 0;
  else if (pos1 == std::string::npos)
    namestart = pos2 + 1;
  else
    namestart = pos1 + 1;

  if (dotpos == std::string::npos || dotpos <= namestart || namestart > name.size())
    throw std::runtime_error("Error: Not a valid .pak pathname: " + name);

  return name.substr(namestart, dotpos - namestart);
}

void inject(const std::string& pak_filename, 
            const std::string& fileindex, const std::string& file, const std::string& outfile)
{
  std::vector<char> filebuf;

  if (!file_readbytes(file, filebuf))
    {
      throw std::runtime_error("Error: Couldn't open " + file);
    }
  TLJPak pak(pak_filename);
  
  int fileindex_idx = pak.lookup(fileindex);

  if (fileindex_idx == -1)
    {
      throw std::runtime_error("Error: Couldn't find fileindex '" + fileindex + "' in '" + pak_filename + "'");
    }

  std::cout << "Injecting file '" << file << "' with size " << filebuf.size() << " at " << fileindex << std::endl;

  pak.write(outfile);
}

std::vector<std::string>
filter_names(std::vector<std::string>& lst)
{
  std::vector<std::string> ret;
  for(std::vector<std::string>::iterator i = lst.begin(); i != lst.end(); ++i)
    {
      if (i->size() < 5 || (*i)[i->size() - 4] != '.')
        { // not a valid file extension, ignore
        }
      else
        {
          ret.push_back(*i);
        }
    }
  return ret;
}

void 
check_directory(const std::string& pak_filename, const std::vector<std::string>& directory)
{
  std::map<int, std::string> entry2name;
  TLJPak pak(pak_filename);

  for(std::vector<std::string>::const_iterator line = directory.begin(); line != directory.end(); ++line)
    {
      int i = pak.lookup(*line);
      if (i != -1)
        {
          std::map<int, std::string>::iterator it = entry2name.find(i);
          if (it == entry2name.end())
            {
              entry2name[i] = *line;
            }
          else
            {
              if (strcasecmp(entry2name[i].c_str(), line->c_str()) != 0)
                {
                  std::cout << std::setw(5) << i << " " << entry2name[i] << " ### duplicate" << std::endl;
                  std::cout << std::setw(5) << i << " " << *line << " ### duplicate" << std::endl;
                }
            }
        }
    }
 
  // Scan all file entries
  for(int i = 0; i < int(pak.files.size()); ++i)
    {
      if (pak.files[i].is_file())
        {
          std::map<int, std::string>::iterator it = entry2name.find(i);
          if (it != entry2name.end())
            {
              std::cout << std::setw(5) << i << " " << it->second << std::endl;
            }
          else
            {
              if (1)
                {
                  std::vector<std::string> ret = pak.guess(i);
                  ret = filter_names(ret);
              
                  if (ret.size() == 0)
                    {
                      std::cout << i << " ### unknown" << std::endl;
                    }
                  else if (ret.size() > 1)
                    {
                      for(std::vector<std::string>::const_iterator j = ret.begin(); j != ret.end(); ++j)
                        std::cout << std::setw(5) << i << " " << *j << " ### guess" << std::endl;
                    }
                  else
                    {
                      std::cout << std::setw(5) << i << " " << ret.front() << std::endl;
                    }
                }
            }
        }
    }
}

void 
extract_directory(const std::string& pak_filename, const std::string& dir_filename, bool guess = true)
{
  std::map<int, std::string> entry2name;
  TLJPak pak(pak_filename);

  std::string line;
  std::ifstream in(dir_filename.c_str());
  while (std::getline(in, line))
    if (!line.empty())
      {
        int i = pak.lookup(line);
        if (i != -1)
          {
            //std::cout << i << " " << line << std::endl;
            entry2name[i] = line;
          }
      }
  in.close();
 
  for(int i = 0; i < int(pak.files.size()); ++i)
    {
      if (pak.files[i].is_file())
        {
          std::map<int, std::string>::iterator j = entry2name.find(i);
          if (j != entry2name.end())
            {
              std::cout << "Extracting (" << std::setw(3) << (100*(i+1))/pak.files.size() << "%): " << j->second << std::endl;
              pak.extract(i, get_exe_path() + "dreamfall/" + j->second);
            }
          else
            {
              if (guess)
                {
                  std::vector<std::string> guesses = pak.guess(i);
                  assert(guesses.size() > 0);
                  std::cout << "Extracting (" << std::setw(3) << (100*(i+1))/pak.files.size() << "%): " << guesses.front() << std::endl;
                  pak.extract(i, get_exe_path() + "dreamfall/" + guesses.front());
                }
              else
                {
                  //std::cout << "Error: Can't translate entry: " << i << std::endl;
                  char filename[1024];
                  snprintf(filename, 1024, "%s-%04i.dat", directory_name_from_pak(pak_filename).c_str(), i);
                  //std::cout << "Error: Can't translate entry: " << i << ", writing to " << filename << std::endl;
                  pak.extract(i, std::string("dreamfall/unknown/") + filename);
                }
            }
        }
    }
}


int main(int argc, char** argv)
{
  try 
    {
      if (argc == 1)
        {
          std::cout << "Dreamfall Extractor V0.1.2\n"
                    << "==========================\n"
                    << "\n"
                    << "This program allows you to extract the .pak files from\n"
                    << "Dreamfall, which are located in the 'Dreamfall/bin/res/'\n"
                    << "directory. The .pak files contain textures, wav and mp3\n"
                    << "files.\n"
                    << "\n"
                    << "To use this program simply drag&drop the .pak files you\n"
                    << "want to extract onto the dreamfall-extractor.exe, the\n"
                    << "{filename}.pak filewill then be extracted to a directory\n"
                    << "named dreamfall-extractor/{filename}/, located in the same\n"
                    << "directory where the dreamfall-extractor.exe is located\n"
                    << "\n"
                    << "For questions and comments mail Ingo Ruhnke <grumbel@gmx.de>\n"
                    << "or join IRC, server irc.rizon.net, channel #Ragnar\n"
                    << "\n"
                    << "Full source code is available on request.\n"
                    << std::endl;
        }
      else if (strcmp(argv[1], "--guess") == 0)
        {
          if (argc >= 3)
            {
              for(int i = 2; i < argc; ++i)
                guess(argv[i]);
            }
          else
            {
              std::cout << "Usage: extract --guess PAK NUM" << std::endl;
              std::cout << "Usage: extract --guess PAK ALL" << std::endl;
            }                    
        }
      else if (strcmp(argv[1], "--check") == 0)
        {
          if (argc >= 3)
            {
              std::vector<std::string> directory;
              
              if (0)
                {
                  file_readlines("script.index", directory);
                  std::cerr << "Reading Index done " << std::endl;
                  for(int i = 2; i < argc; ++i)
                    {
                      std::cerr << "Checking  " << argv[i] << std::endl;
                      check_directory(argv[i], directory);
                    }
                }

              if (1)
                {
                  for(int i = 3; i < argc; ++i)
                    {
                      file_readlines(argv[i], directory);
                    }
              
                  check_directory(argv[2], directory);
                }
            }
          else
            {
              std::cout << "Usage: extract --check PAK DIRECTORY [DIRECTORY ...]" << std::endl;
            }
        }
      else if (strcmp(argv[1], "--info") == 0)
        {
          if (argc >= 3)
            {
              for(int i = 2; i < argc; ++i)
                {
                  TLJPak pak(argv[i]);
                  pak.print_file_table();
                  std::endl(std::cout);
                  pak.print_nametable();
                  //pak.print_indextable();
                }
            }
          else
            {
              std::cout << "Usage: extract --info PAK..." << std::endl;
            }
        }
      else if (strcmp(argv[1], "--directory") == 0)
        {
          if (argc >= 4)
            {
              extract_directory(argv[2], argv[3]);
            }
          else if (argc >= 3)
            {
              char default_directory[] = "df-directory.txt";
              std::cout << "Using default directory: " << default_directory << std::endl;
              extract_directory(argv[2], get_exe_path() + default_directory);
            }
          else
            {
              std::cout << "Usage: extract --directory PAK DIRECTORY" << std::endl;
            }
        }
      else if (strcmp(argv[1], "--extract") == 0)
        {
          if (argc == 4)
            {
              extract(argv[2], argv[3]);
            }
          else
            {
              std::cout << "Usage: extract --extract PAK PATHNAME" << std::endl;
            }
        }
      else if (strcmp(argv[1], "--inject") == 0)
        {
          if (argc == 6)
            {
              inject(argv[2], argv[3], argv[4], argv[5]);
            }
          else
            {
              std::cout << "Usage: extract --inject PAK FILEINDEX FILE OUTFILE" << std::endl;
            }
        }
      else
        {
          for(int i = 1; i < argc; ++i)
            {
              std::ifstream in(argv[i], std::ios::binary);
              if (!in)
                {
                  std::cerr << "Error: Couldn't open " << argv[i] << std::endl;
                }
              else
                {
                  TLJPak pak(argv[i]);
      
                  std::string outfile = directory_name_from_pak(tolowercase(argv[i]));
                  if (0)
                    {
                      std::cout << "Filename:      " << argv[i] << std::endl;
                      std::cout << "Outfile:       " << outfile << std::endl;
                    }
                    
                  char default_directory[] = "df-directory.txt";
                  std::cout << "Using default directory: " << default_directory << std::endl;
                  extract_directory(argv[i], get_exe_path() + default_directory);
                     
                  //extract(pak, in, outfile);

                  in.close();
                }
            }
        }
    }
  catch(std::exception& err) 
    {
      std::cout << "Error: " << err.what() << std::endl;
    }

  wait();
}

/* EOF */
