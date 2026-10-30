/*  $Id: system.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef _WIN32
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <dirent.h>
#  include <unistd.h>
#  include <limits.h>
#else
#  include <windows.h>
#  include <direct.h>
#  include <io.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  define access _access
#  define R_OK 4
#endif

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "system.hpp"
#include "util.hpp"

// Empty means "use the platform default", see the get_*_path() functions
std::string g_datadir;
std::string g_configdir;
std::string g_cachedir;

namespace {

std::string with_slash(std::string path)
{
  if (!path.empty() && path[path.size()-1] != '/' && path[path.size()-1] != '\\')
    path += "/";
  return path;
}

/** Returns the environment variable \a name or an empty string, XDG
    requires relative paths to be ignored, so \a absolute_only can be
    used to enforce that */
std::string get_env(const char* name, bool absolute_only = false)
{
  const char* value = getenv(name);
  if (!value || !*value)
    return std::string();

#ifndef _WIN32
  if (absolute_only && value[0] != '/')
    return std::string();
#endif

  return value;
}

/** Returns $XDG_*_HOME or $HOME/\a fallback, empty if neither is available */
std::string get_xdg_dir(const char* xdg_var, const char* fallback)
{
  std::string dir = get_env(xdg_var, true);
  if (!dir.empty())
    return with_slash(dir);

  std::string home = get_env("HOME");
  if (!home.empty())
    return with_slash(home) + fallback;

  return std::string();
}

/** Creates \a path including all its parent directories */
void create_directories(const std::string& path)
{
  for(std::string::size_type i = 1; i <= path.size(); ++i)
    {
      if (i == path.size() || path[i] == '/' || path[i] == '\\')
        {
          std::string dir = path.substr(0, i);
          if (!dir.empty() && !file_exists(dir))
            create_dir(dir);
        }
    }
}

} // namespace

void parse_args(int argc, char** argv)
{
  for (int i = 1; i < argc; ++i)
  {
    if (strcmp(argv[i], "--datadir") == 0)
    {
      ++i;
      if (i >= argc)
      {
        throw std::runtime_error("--datadir requires an argument");
      }
      else
      {
        g_datadir = with_slash(argv[i]);
      }
    }
    else if (strcmp(argv[i], "--configdir") == 0)
    {
      ++i;
      if (i >= argc)
      {
        throw std::runtime_error("--configdir requires an argument");
      }
      else
      {
        g_configdir = with_slash(argv[i]);
      }
    }
    else if (strcmp(argv[i], "--cachedir") == 0)
    {
      ++i;
      if (i >= argc)
      {
        throw std::runtime_error("--cachedir requires an argument");
      }
      else
      {
        g_cachedir = with_slash(argv[i]);
      }
    }
    else
    {
      std::cout << "Usage: " << argv[0] << " [OPTION]...\n"
                << "\n"
                << "  --datadir PATH    Location for dftoolbox data files\n"
                << "  --configdir PATH  Location for configuration files\n"
                << "  --cachedir PATH   Location for cached pak file tables\n"
                << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  std::cout << "Directories:" << std::endl;
  std::cout << "    data: " << get_data_path() << std::endl;
  std::cout << "  config: " << get_config_path() << std::endl;
  std::cout << "   cache: " << get_cache_path() << std::endl;
  std::cout << "    temp: " << get_temp_path() << std::endl;
}

std::string get_temp_path()
{
#ifdef _WIN32
  char buffer[MAX_PATH+1];
  DWORD len = GetTempPathA(sizeof(buffer), buffer);
  if (len > 0 && len < sizeof(buffer))
    {
      std::string tmpdir(buffer, len);
      for(std::string::size_type i = 0; i < tmpdir.size(); ++i)
        if (tmpdir[i] == '\\')
          tmpdir[i] = '/';
      return with_slash(tmpdir);
    }
  return get_exe_path();
#else
  std::string tmpdir = get_env("TMPDIR");
  if (!tmpdir.empty())
    return with_slash(tmpdir);
  else
    return "/tmp/";
#endif
}

std::string get_data_path()
{
  if (!g_datadir.empty())
    return g_datadir;

#ifdef _WIN32
  return get_exe_path();
#else
  return "./";
#endif
}

std::string get_config_path()
{
  if (g_configdir.empty())
    {
#ifdef _WIN32
      std::string appdata = get_env("APPDATA");
      if (!appdata.empty())
        g_configdir = with_slash(appdata) + "dftoolbox/";
      else
        g_configdir = get_exe_path();
#else
      std::string config_home = get_xdg_dir("XDG_CONFIG_HOME", ".config/");
      if (!config_home.empty())
        g_configdir = config_home + "dftoolbox/";
      else
        g_configdir = "./";
#endif
      create_directories(g_configdir);
    }

  return g_configdir;
}

std::string get_cache_path()
{
  if (g_cachedir.empty())
    {
#ifdef _WIN32
      std::string localappdata = get_env("LOCALAPPDATA");
      if (!localappdata.empty())
        g_cachedir = with_slash(localappdata) + "dftoolbox/";
      else
        g_cachedir = get_temp_path() + "dftoolbox/";
#else
      std::string cache_home = get_xdg_dir("XDG_CACHE_HOME", ".cache/");
      if (!cache_home.empty())
        g_cachedir = cache_home + "dftoolbox/";
      else
        g_cachedir = get_temp_path() + "dftoolbox/";
#endif
      create_directories(g_cachedir);
    }

  return g_cachedir;
}

std::string get_exe_path()
{
#ifdef _WIN32
  // Get path to executable:
  char szDllName[_MAX_PATH];
  char szDrive[_MAX_DRIVE];
  char szDir[_MAX_DIR];
  char szFilename[_MAX_FNAME];
  char szExt[_MAX_EXT];
  GetModuleFileName(0, szDllName, _MAX_PATH);
  _splitpath(szDllName, szDrive, szDir, szFilename, szExt);

  return std::string(szDrive) + std::string(szDir);
#else
  char buffer[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (len <= 0)
    return "./";

  std::string exe(buffer, len);
  return exe.substr(0, exe.rfind('/') + 1);
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
#ifndef _WIN32
  return mkdir(path.c_str(), 0755);
#else
  return _mkdir(path.c_str());
#endif
}

void wait()
{
#ifdef _WIN32
  // Wait for a keypress so that the user can look at the textual
  // output in Windows
  std::cout << "\nPress Enter to continue..." << std::endl;
  getchar();
#endif
}

#ifdef _WIN32
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

bool file_stat(const std::string& pathname, int64_t& size, int64_t& mtime)
{
  struct stat buf;
  if (stat(pathname.c_str(), &buf) != 0)
    return false;

  size  = buf.st_size;
  mtime = buf.st_mtime;
  return true;
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

