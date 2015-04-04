/*  $Id: shark3d_main.cpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>,
**                                 pinkunozou
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

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "config.hpp"
#include "shark3d.hpp"

int main(int argc, char** argv)
{
  try {
    Config config;

    if (argc == 1)
      {
        std::cout << "Usage: " << argv[0] << " FILENAME..." << std::endl;
      }
    else if (argc == 3 && strcmp(argv[1], "--compile") == 0)
      {
        std::ifstream in(argv[2]);

        if(!in)
          {
            throw std::runtime_error("Error: Couldn't open '" + std::string(argv[2]) + "'");
          }
        else
          {
            Shark3D* doc = Shark3D::parse_text(in);

            doc->write_binary(std::cout);

            delete doc;
          }
      }
    else
      {
        for(int i = 1; i < argc; ++i)
          {
            if (argc > 2)
              std::cout << "# " << argv[i] << std::endl;

            std::ifstream in(argv[i]);

            if(!in)
              {
                throw std::runtime_error("Error: Couldn't open '" + std::string(argv[i]) + "'");
              }
            else
              {
                Shark3D* doc = Shark3D::parse_binary(in);
                doc->write_text(std::cout);
                delete doc;

                in.close();
              }
            if (argc > 2)
              std::cout << std::endl;
          }
      }

  } catch(std::exception& err) {
    std::cout << err.what() << std::endl;
    return EXIT_FAILURE;
  }

  //  wait();
}

  /* EOF */
