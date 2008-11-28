/*  $Id: savegame.cpp,v 1.3 2006/06/16 02:17:53 ingo Exp ingo $
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
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>

#include "config.hpp"
#include "shark3d.hpp"
#include "system.hpp"
#include "tga.hpp"

std::string file_basename(const std::string& name)
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
    throw std::runtime_error("Error: Not a valid .sav pathname: " + name);

  return name.substr(namestart, dotpos - namestart);
}

int main(int argc, char** argv)
{
  try {
    Config config;

    if (argc < 2)
      {
        std::cout << "This program will extract the screenshot from a Dreamfall savegame file." << std::endl;
        std::cout << "Usage: " << argv[0] << " SAVEFILE" << std::endl;
      }
    else if (argc > 2 && strcmp(argv[1], "--build") == 0)
      {
        if (argc == 4)
          {
            std::ifstream in_txt(argv[2]);
            Shark3D* shark3d = Shark3D::parse_text(in_txt);
            in_txt.close();

            std::ifstream in_tga(argv[3], std::ios::binary);
            TGA tga(in_tga);
            in_tga.close();

            // Write 
            if (config.use_stdout)
              {
                shark3d->write_binary(std::cout);
                std::cout.write(tga.get_data(), tga.get_size());
              }
            else
              {
                std::string outfile = get_exe_path() + "saves/" + file_basename(argv[2])  + ".sav";
                std::cout << "Building save and writing to " << outfile << std::endl;
                std::ofstream out(outfile.c_str(), std::ios::binary);
                shark3d->write_binary(out);
                out.write(tga.get_data(), tga.get_size());
                out.close();
              }
          }
        else
          {
            std::cout << "Usage: " << argv[0] << " --build TXTFILE TGAFILE" << std::endl;
          }
      }
    else
      {
        create_dir(get_exe_path() + "saves/");

        for(int i = 1; i < argc; ++i)
          {
            std::vector<char> buffer(256*256*4);
            std::ifstream in(argv[i], std::ios::binary);
            if (!in)
              {
                throw std::runtime_error(std::string("Couldn't open: ") + argv[i]);
              }
            else
              {
                {
                  std::string outfile = get_exe_path() + "saves/" + file_basename(argv[i])  + ".txt";
                  std::cout << "Writing data to: " << outfile << std::endl;
                  std::ofstream out(outfile.c_str());
                  Shark3D::parse_binary(in)->write_text(out);
                }
                
                {
                  // Extract screenshot
                  in.seekg(-(256*256*4), std::ios::end);
                  in.read(&*buffer.begin(), 256*256*4);
                  in.close();

                  std::string outfile = get_exe_path() + "saves/" + file_basename(argv[i])  + ".tga";
                  std::cout << "Writing screenshot to: " << outfile << std::endl;
                  std::ofstream out(outfile.c_str(), std::ios::binary);
                  TGA tga(256,256, buffer);
                  tga.write(out);
                  out.close();
                }
              }
          }
      }
  } catch(std::exception& err) {
    std::cerr << err.what() << std::endl;
  }
}

/* EOF */
