/*  $Id: util.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_UTIL_HPP
#define HEADER_UTIL_HPP

#include <sstream>
#include <string>
#include <vector>

/* Check if the last characters of \a str match \a suffix */
bool has_suffix(const std::string& str, const std::string& suffix);

/** Convert a string to lowercase */
std::string tolowercase(const std::string& str);

/** Read a file line by line and append all lines to \a lines */
bool file_readlines(const std::string& filename, std::vector<std::string>& lines);

/** Read a file byte by byte and append bytes to \a bytes */
bool file_readbytes(const std::string& filename, std::vector<char>& bytes);

template<class T>
std::string to_string(const T& data)
{
  std::ostringstream os;
  os << data;
  return os.str();
}

template <class T>
bool from_string(const std::string& rep, T& x)
{
 // this is necessary so that if "x" is not modified if the conversion fails
  T temp;
  std::istringstream iss(rep);

  iss >> temp;

  if (iss.fail())
    {
      return false;
    }
  else
    {
      x = temp;
      return true;
    }
}

struct Vector
{
  float x, y, z;
};

bool splitpath(const std::string& filename, std::string& pathpart, std::string& filepart);

#endif

/* EOF */
