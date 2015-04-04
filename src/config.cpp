/*  $Id: config.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <iostream>
#include <fstream>
#include "system.hpp"
#include "shark3d.hpp"
#include "config.hpp"

Config::Config()
{
  use_stdout     = true;
  dreamfall_path = "C:/Program Files/Dreamfall/"; // use registry to check this

#ifndef __linux
  /*
HKEY_CLASSES_ROOT
HKEY_CURRENT_CONFIG
HKEY_CURRENT_USER
HKEY_LOCAL_MACHINE
HKEY_USERS

  LONG RegGetValue(
                   HKEY hkey,
                   LPCTSTR lpSubKey, "...funcom/dreamfal/..."
                   LPCTSTR lpValue, "installpath"
                   DWORD dwFlags, // which type should be queried string, int, etc
                   LPDWORD pdwType, // pointer to type enum
                   PVOID pvData, // buffer pointer
                   LPDWORD pcbData // buffer length
);
  */
#endif

  std::string filename = get_exe_path() + "dftoolbox.ini";
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
