/*  $Id: config.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include "system.hpp"
#include "shark3d.hpp"
#include "config.hpp"

namespace {

bool is_dreamfall_path(const std::string& path)
{
  return !path.empty() && file_exists(path + "bin/res/");
}

} // namespace

std::string find_dreamfall_path()
{
  std::vector<std::string> candidates;

#ifdef _WIN32
  const char* roots[] = { "ProgramFiles(x86)", "ProgramFiles", "ProgramW6432" };
  for(size_t i = 0; i < sizeof(roots)/sizeof(roots[0]); ++i)
    {
      const char* root = getenv(roots[i]);
      if (root && *root)
        {
          std::string r = std::string(root) + "/";
          candidates.push_back(r + "Funcom/Dreamfall/");
          candidates.push_back(r + "Dreamfall/");
          candidates.push_back(r + "GOG Galaxy/Games/Dreamfall - The Longest Journey/");
          candidates.push_back(r + "Steam/steamapps/common/Dreamfall The Longest Journey/");
        }
    }
  candidates.push_back("C:/GOG Games/Dreamfall - The Longest Journey/");
  candidates.push_back("C:/Program Files/Funcom/Dreamfall/");
  candidates.push_back("C:/Program Files/Dreamfall/");
#else
  const char* home_c = getenv("HOME");
  if (home_c && *home_c)
    {
      std::string home = std::string(home_c) + "/";
      candidates.push_back(home + "Games/Heroic/Dreamfall - The Longest Journey/");
      candidates.push_back(home + "games/Heroic/Dreamfall - The Longest Journey/");
      candidates.push_back(home + "GOG Games/Dreamfall - The Longest Journey/");
      candidates.push_back(home + ".local/share/Steam/steamapps/common/Dreamfall The Longest Journey/");
      candidates.push_back(home + ".steam/steam/steamapps/common/Dreamfall The Longest Journey/");
    }
#endif

  for(std::vector<std::string>::iterator i = candidates.begin(); i != candidates.end(); ++i)
    {
      if (is_dreamfall_path(*i))
        return *i;
    }

  return std::string();
}

Config::Config()
{
  use_stdout     = true;
  dreamfall_path = find_dreamfall_path();

  std::string filename = get_config_file("dftoolbox.ini");
  std::ifstream in(filename.c_str());
  if (!in)
    {
      std::cerr << "Couldn't open config file " << filename << std::endl;

      // FIXME: Create file with default values if not present
    }
  else
    {
      Shark3D* shark3d = Shark3D::parse_text(in);
      use_stdout     = shark3d->get_int("use_stdout", use_stdout);
      dreamfall_path = shark3d->get_string("dreamfall_path", dreamfall_path);
    }
}

/* EOF */
