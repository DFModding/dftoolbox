/*  $Id: system.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifdef __linux__
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <dirent.h>
#else
#  include <windows.h>
#  include <direct.h>
#endif

#include <string.h>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "system.hpp"
#include "util.hpp"

std::string get_exe_path()
{
#ifdef __linux__
  return "./";
#else
  // Get path to executable:
  char szDllName[_MAX_PATH];
  char szDrive[_MAX_DRIVE];
  char szDir[_MAX_DIR];
  char szFilename[_MAX_FNAME];
  char szExt[_MAX_EXT];
  GetModuleFileName(0, szDllName, _MAX_PATH);
  _splitpath(szDllName, szDrive, szDir, szFilename, szExt);

  return std::string(szDrive) + std::string(szDir);
#endif
}

void create_hierachy(const std::string& outfile)
{
  for(std::string::size_type i = 0; i < outfile.size(); ++i)
    {
      if (outfile[i] == '/')
        {
          std::string dir = outfile.substr(0, i);
          //std::cout << "Create Dir: " << dir << std::endl;
          if (!file_exists(dir))
            create_dir(dir);
        }
    }
}

int create_dir(const std::string& path)
{
#ifdef __linux__
  return mkdir(path.c_str(), 0755);
#else
  return _mkdir(path.c_str());
#endif
}

void wait()
{
#ifndef __linux__
  // Wait for a keypress so that the user can look at the textual
  // output in Windows
  std::cout << "\nPress Enter to continue..." << std::endl;
  getchar();
#endif
}

#ifndef __linux__
std::vector<DirectoryEntry>
open_directory(const std::string& pathname)
{
  HANDLE          hList;
  TCHAR           szDir[MAX_PATH+1];
  WIN32_FIND_DATA FileData;

  std::vector<DirectoryEntry> directory;  
  
  // What is this good for?!
  sprintf(szDir, "%s\\*", pathname.c_str());

  hList = FindFirstFile(szDir, &FileData);
  if (hList == INVALID_HANDLE_VALUE)
    { 
      throw std::runtime_error("open_directory(): Couldn't open " + pathname);
    }
  else
    {
      while (true)
        {
          std::string fullname = pathname + FileData.cFileName;

          if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              if (strcmp(FileData.cFileName, ".")  != 0 &&
                  strcmp(FileData.cFileName, "..") != 0)
                directory.push_back(DirectoryEntry(DirectoryEntry::T_DIRECTORY, FileData.cFileName, fullname + "/"));            
            }
          else
            {
              directory.push_back(DirectoryEntry(DirectoryEntry::T_FILE, FileData.cFileName, fullname));
            }
            
          if (!FindNextFile(hList, &FileData))
            {
              if (GetLastError() == ERROR_NO_MORE_FILES)
                {
                  break;
                }
            }

        }
      FindClose(hList);

      return directory;
    }
}
#else 
std::vector<DirectoryEntry>
open_directory(const std::string& pathname)
{
  std::vector<DirectoryEntry> directory;

  DIR* dir = opendir(pathname.c_str());
  
  if (!dir)
    throw std::runtime_error("Couldn't open " + pathname);

  struct dirent* dp;
  while((dp = readdir(dir)) != 0)
    {
      if (strcmp(dp->d_name, "..") != 0 &&
          strcmp(dp->d_name, ".")  != 0)
        {
          struct stat buf;
          
          std::string fullname = pathname + dp->d_name;
          if (stat(fullname.c_str(), &buf) == 0)
            {
              if (S_ISDIR(buf.st_mode))
                directory.push_back(DirectoryEntry(DirectoryEntry::T_DIRECTORY, dp->d_name, fullname + "/"));
              else if (S_ISREG(buf.st_mode))
                directory.push_back(DirectoryEntry(DirectoryEntry::T_FILE, dp->d_name, fullname));
              else
                directory.push_back(DirectoryEntry(DirectoryEntry::T_UNKNOWN, dp->d_name, fullname));
            }
        }
    }
  closedir(dir);

  return directory;
}
#endif 

Directory 
open_directory(const std::string& pathname, const std::string& suffix)
{
  Directory res;
  Directory dir = open_directory(pathname);

  for(Directory::iterator i = dir.begin(); i != dir.end(); ++i)
    {
      if (i->type == DirectoryEntry::T_FILE && has_suffix(i->name, suffix))
        res.push_back(*i);
    }
  return res;
}

void find_files(const std::string& pathname, std::vector<std::string>& lst)
{
  Directory dir = open_directory(pathname);
  for(Directory::iterator i = dir.begin(); i != dir.end(); ++i)
    {
      if (i->type == DirectoryEntry::T_DIRECTORY)
        {
          find_files(i->fullname, lst);
        }
      else if (i->type == DirectoryEntry::T_FILE)
        {
          lst.push_back(i->fullname);
        }
    }
}

void file_copy(const std::string& from, const std::string& to)
{
  std::ifstream in(from.c_str(), std::ios::binary);
  if (!in)
    throw std::runtime_error("Couldn't open " + from);
  
  std::ofstream out(to.c_str(), std::ios::binary);
  if (!out)
    throw std::runtime_error("Couldn't open " + to);

  char buffer[4096];
  
  while(in)
    {
      int read = in.read(buffer, 4096).gcount();
      out.write(buffer, read);
    }

  out.close();
  in.close();
}

bool file_exists(const std::string& pathname)
{
  return access(pathname.c_str(), R_OK) == 0;
}

std::vector<char>
file_read(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    throw std::runtime_error("file_read: couldn't open " + filename);

  char buffer[4096];
  std::vector<char> ret;
  while(in)
    {
      int read = in.read(buffer, 4096).gcount();
      ret.insert(ret.end(), buffer, buffer + read);
    }

  return ret;
}

std::string read_string0(std::istream& in)
{
  std::string str;
  int c;
  while((c = in.get()) != 0)
    str += c;
  return str;
}

std::string read_string(std::istream& in, int len)
{
  std::string buf;
  for(int i = 0; i < len; ++i)
    {
      buf += in.get();
    }
  return buf; 
}

std::vector<char> read_bytes(std::istream& in, int len)
{
  std::vector<char> buf;
  for(int i = 0; i < len; ++i)
    {
      buf.push_back(in.get());
    }
  return buf;
}

float read_float(std::istream& in)
{
  float v;
  in.read(reinterpret_cast<char*>(&v), sizeof(v));
  return v;
}

float read_float_reverse(std::istream& in)
{
  char v[4];
  v[3] = in.get();
  v[2] = in.get();
  v[1] = in.get();
  v[0] = in.get();
  return *((float*)v);
}

Vector read_vector(std::istream& in)
{
  Vector v;
  v.x = read_float(in);
  v.y = read_float(in);
  v.z = read_float(in);
  return v;
}

uint16_t read_uint16(std::istream& in)
{
  uint16_t v;
  in.read(reinterpret_cast<char*>(&v), sizeof(v));
  return v;
}

int read_int32(std::istream& in)
{
  int v;
  in.read(reinterpret_cast<char*>(&v), sizeof(v));
  return v;
}

uint32_t read_uint32(std::istream& in)
{
  uint32_t v;
  in.read(reinterpret_cast<char*>(&v), sizeof(v));
  return v;
}

/* EOF */

