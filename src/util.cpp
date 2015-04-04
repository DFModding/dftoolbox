/*  $Id: util.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <fstream>
#include <iostream>
#include "util.hpp"

bool has_suffix(const std::string& str, const std::string& suffix)
{
  if (str.length() >= suffix.length())
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
  else
    return false;
}

std::string tolowercase(const std::string& str)
{
  std::string out;
  for(std::string::const_iterator i = str.begin();  i != str.end(); ++i)
    out += tolower(*i);
  return out;
}

bool file_readlines(const std::string& filename, std::vector<std::string>& lines)
{
  std::ifstream in(filename.c_str());

  if (!in)
    {
      return false;
    }
  else
    {
      std::string line;

      while (std::getline(in, line))
        {
          if (!line.empty())
            {
              lines.push_back(line);
            }
        }

      in.close();

      return true;
    }
}

bool file_readbytes(const std::string& filename, std::vector<char>& bytes)
{
  std::ifstream in(filename.c_str(), std::ios::binary);

  if (!in)
    {
      return false;
    }
  else
    {
      // Get filesize
      in.seekg(0, std::ios::end);
      int size = in.tellg();

      int current_size = bytes.size();

      bytes.resize(current_size + size);
      in.seekg(0, std::ios::beg);
      in.read(reinterpret_cast<char*>(&*(bytes.begin() + current_size)), size);
      in.close();

      return true;
    }
}

bool splitpath(const std::string& filename, std::string& pathpart, std::string& filepart)
{
  if (filename.empty())
    return false;

  std::string::size_type i = filename.size()-1;

  do {
    //std::cout << i << filename << std::endl;
    i = filename.find_last_of('/', i);
  } while(i == filename.size());

  if (i != std::string::npos)
    {
      pathpart = filename.substr(0, i);
      filepart = filename.substr(i+1);
      return true;
    }
  else
    {
      return false;
    }
}

/* EOF */
