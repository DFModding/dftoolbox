/*  $Id: system.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SYSTEM_HPP
#define HEADER_SYSTEM_HPP

#include <vector>
#include <string>
#include "util.hpp"

std::string get_exe_path();

/** Same as create_dir(), but creates all missing directories in the
    hierachies */
void create_hierachy(const std::string& outfile);
int  create_dir(const std::string& path);
void wait();

struct DirectoryEntry
{
  enum Type { T_DIRECTORY, T_FILE, T_UNKNOWN };

  Type type;
  std::string name;
  std::string fullname;
  DirectoryEntry(Type type_, const std::string& name_,const std::string& fullname_)
    : type(type_), name(name_), fullname(fullname_)
  {}
};

typedef std::vector<DirectoryEntry> Directory;
Directory open_directory(const std::string& pathname);
Directory open_directory(const std::string& pathname, const std::string& suffix);

/** Finds all files under a given directory */
void find_files(const std::string& pathname, std::vector<std::string>& lst);

void file_copy(const std::string& from, const std::string& to);
bool file_exists(const std::string& pathname);
std::vector<char> file_read(const std::string& filename);
std::vector<std::string> file_readlines(const std::string& filename);

Vector            read_vector (std::istream& in);
std::string       read_string0(std::istream& in);
std::string       read_string(std::istream& in, int len);
std::vector<char> read_bytes  (std::istream& in, int len);
float             read_float  (std::istream& in);
float             read_float_reverse(std::istream& in);
uint16_t          read_uint16 (std::istream& in);
int               read_int32  (std::istream& in);
uint32_t          read_uint32 (std::istream& in);

#endif

/* EOF */
