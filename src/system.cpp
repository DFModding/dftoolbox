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

#ifndef _WIN32
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <dirent.h>
#  include <unistd.h>
#  include <limits.h>
#else
#  include <windows.h>
#  include <direct.h>
#  include <fcntl.h>
#  include <io.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  define access _access
#  define R_OK 4
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "system.hpp"
#include "util.hpp"

// Empty means "use the platform default", see the resolve_*() functions
std::string g_datadir;
std::string g_configdir;
std::string g_cachedir;
std::string g_user_datadir;
bool g_portable = false;
bool g_no_save  = false;

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

/** A directory and what picked it, for --show-paths */
struct ResolvedPath
{
  std::string path;
  std::string origin;
};

bool is_portable()
{
  return g_portable || file_exists(get_exe_path() + "portable.txt");
}

#ifndef _WIN32
/** Returns $XDG_*_HOME or $HOME/\a fallback, throws if neither is
    available since relative paths are never ok here */
ResolvedPath resolve_xdg(const char* xdg_var, const char* fallback)
{
  std::string dir = get_env(xdg_var, true);
  if (!dir.empty())
    return { with_slash(dir) + "dftoolbox/", xdg_var };

  std::string home = get_env("HOME", true);
  if (home.empty())
    throw std::runtime_error(std::string("Neither $") + xdg_var + " nor $HOME is usable");

  return { with_slash(home) + fallback + "dftoolbox/", "default" };
}
#else
/** Returns %\a var%/dftoolbox/\a category/, throws if it isn't set */
ResolvedPath resolve_known_folder(const char* var, const char* category)
{
  std::string dir = get_env(var);
  if (dir.empty())
    throw std::runtime_error(std::string("%") + var + "% is not set");

  return { with_slash(dir) + "dftoolbox/" + category + "/", "default" };
}
#endif

ResolvedPath resolve_data_path()
{
  if (!g_datadir.empty())
    return { g_datadir, "option" };

#ifdef _WIN32
  return { get_exe_path() + "data/", "executable" };
#else
  std::string relative = get_exe_path() + "../share/dftoolbox/";
  if (file_exists(relative + "df-directory.txt"))
    return { relative, "executable" };

  return { DFTOOLBOX_INSTALL_DATADIR "/", "compiled-in" };
#endif
}

ResolvedPath resolve_config_path()
{
  if (!g_configdir.empty())
    return { g_configdir, "option" };
  if (is_portable())
    return { get_exe_path() + "userdata/config/", "portable" };
#ifdef _WIN32
  return resolve_known_folder("APPDATA", "config");
#else
  return resolve_xdg("XDG_CONFIG_HOME", ".config/");
#endif
}

ResolvedPath resolve_cache_path()
{
  if (!g_cachedir.empty())
    return { g_cachedir, "option" };
  if (is_portable())
    return { get_exe_path() + "userdata/cache/", "portable" };
#ifdef _WIN32
  return resolve_known_folder("LOCALAPPDATA", "cache");
#else
  return resolve_xdg("XDG_CACHE_HOME", ".cache/");
#endif
}

ResolvedPath resolve_user_data_path()
{
  if (!g_user_datadir.empty())
    return { g_user_datadir, "option" };
  if (is_portable())
    return { get_exe_path() + "userdata/data/", "portable" };
#ifdef _WIN32
  return resolve_known_folder("LOCALAPPDATA", "data");
#else
  return resolve_xdg("XDG_DATA_HOME", ".local/share/");
#endif
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

  if (!file_exists(path))
    throw std::runtime_error("Couldn't create directory " + path);
}

/** Portable mode has nothing to fall back to, so it has to find out
    early if the directory next to the executable can't be used */
void check_writable(const std::string& path)
{
  std::string probe = path + ".dftoolbox-write-test";
  std::ofstream out(probe.c_str());
  if (!out)
    throw std::runtime_error("Directory isn't writable: " + path);
  out.close();
  remove(probe.c_str());
}

std::string prepare_directory(const ResolvedPath& resolved)
{
  create_directories(resolved.path);
  if (resolved.origin == "portable")
    check_writable(resolved.path);
  return resolved.path;
}

void print_usage(std::ostream& out, const std::string& program)
{
  out << "Usage: " << program << " [OPTION]...\n"
      << "\n"
      << "  -h, --help            Show this help and exit\n"
      << "  -v, --version         Show the version and exit\n"
      << "      --data-dir DIR    Location of the dftoolbox data files\n"
      << "      --config-dir DIR  Location of the configuration files\n"
      << "      --cache-dir DIR   Location of the cached pak file tables\n"
      << "      --user-data-dir DIR  Location of mods installed by the user\n"
      << "      --portable        Keep everything in userdata/ next to the executable\n"
      << "      --no-save         Don't write any configuration in this run\n"
      << "      --show-paths      Show the directories in use and exit\n";
}

[[noreturn]] void usage_error(const std::string& program, const std::string& message)
{
  std::cerr << program << ": " << message << "\n"
            << "Try '" << program << " --help' for more information." << std::endl;
  exit(EXIT_FAILURE);
}

} // namespace

void parse_args(int argc, char** argv, const std::string& program)
{
  bool show_paths = false;

  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];

    if (arg == "--")
    {
      if (i + 1 < argc)
        usage_error(program, std::string("unexpected argument: ") + argv[i + 1]);
      break;
    }

    std::string value;
    bool has_value = false;
    std::string::size_type equal = arg.find('=');
    if (arg.compare(0, 2, "--") == 0 && equal != std::string::npos)
    {
      value = arg.substr(equal + 1);
      arg.resize(equal);
      has_value = true;
    }

    auto directory_value = [&]() -> std::string
    {
      if (!has_value)
      {
        ++i;
        if (i >= argc)
          usage_error(program, arg + " requires an argument");
        value = argv[i];
      }
      return with_slash(value);
    };

    if (arg == "-h" || arg == "--help")
    {
      print_usage(std::cout, program);
      exit(EXIT_SUCCESS);
    }
    else if (arg == "-v" || arg == "--version")
    {
      std::cout << program << " " << get_version() << std::endl;
      exit(EXIT_SUCCESS);
    }
    else if (arg == "--data-dir" || arg == "--datadir")
    {
      g_datadir = directory_value();
    }
    else if (arg == "--config-dir" || arg == "--configdir")
    {
      g_configdir = directory_value();
    }
    else if (arg == "--cache-dir" || arg == "--cachedir")
    {
      g_cachedir = directory_value();
    }
    else if (arg == "--user-data-dir")
    {
      g_user_datadir = directory_value();
    }
    else if (arg == "--portable")
    {
      g_portable = true;
    }
    else if (arg == "--no-save")
    {
      g_no_save = true;
    }
    else if (arg == "--show-paths")
    {
      show_paths = true;
    }
    else
    {
      usage_error(program, "unknown option: " + arg);
    }
  }

  if (show_paths)
  {
    print_paths(std::cout);
    exit(EXIT_SUCCESS);
  }
}

std::string get_version()
{
  return DFTOOLBOX_VERSION;
}

void print_paths(std::ostream& out)
{
  const struct { const char* name; ResolvedPath resolved; } paths[] = {
    { "data",      resolve_data_path() },
    { "config",    resolve_config_path() },
    { "user data", resolve_user_data_path() },
    { "cache",     resolve_cache_path() }
  };

  for (const auto& entry : paths)
    out << entry.name << ": " << entry.resolved.path
        << " (" << entry.resolved.origin << ")" << std::endl;
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
  throw std::runtime_error("Couldn't find the directory for temporary files");
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
  return resolve_data_path().path;
}

std::string get_config_path()
{
  return prepare_directory(resolve_config_path());
}

std::string get_config_file(const std::string& name)
{
  std::string filename = get_config_path() + name;

#ifdef _WIN32
  // Before the config/ subdirectory the files were in %APPDATA%/dftoolbox/
  if (!file_exists(filename))
    {
      std::string appdata = get_env("APPDATA");
      std::string legacy = with_slash(appdata) + "dftoolbox/" + name;
      if (!appdata.empty() && file_exists(legacy))
        return legacy;
    }
#endif

  return filename;
}

std::string get_cache_path()
{
  return prepare_directory(resolve_cache_path());
}

std::string get_user_data_path()
{
  return prepare_directory(resolve_user_data_path());
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
  GetModuleFileNameA(0, szDllName, _MAX_PATH);
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

void attach_parent_console()
{
#ifdef _WIN32
  if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
      freopen("CONOUT$", "w", stdout);
      freopen("CONOUT$", "w", stderr);
      std::ios::sync_with_stdio();
    }
#endif
}

void set_binary_stdout()
{
#ifdef _WIN32
  _setmode(_fileno(stdout), _O_BINARY);
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
  WIN32_FIND_DATAA FileData;

  std::vector<DirectoryEntry> directory;

  // FindFirstFile() takes a pattern, not a directory
  std::string pattern = pathname + "\\*";

  hList = FindFirstFileA(pattern.c_str(), &FileData);
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

          if (!FindNextFileA(hList, &FileData))
            break;
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

void file_write_atomic(const std::string& filename, const std::string& content)
{
  if (g_no_save)
    return;

  std::string tmpname = filename + ".tmp";
  std::ofstream out(tmpname.c_str(), std::ios::binary);
  out.write(content.data(), content.size());
  out.close();
  if (!out)
    {
      remove(tmpname.c_str());
      throw std::runtime_error("Couldn't write " + filename);
    }

#ifdef _WIN32
  bool renamed = MoveFileExA(tmpname.c_str(), filename.c_str(), MOVEFILE_REPLACE_EXISTING);
#else
  bool renamed = rename(tmpname.c_str(), filename.c_str()) == 0;
#endif
  if (!renamed)
    {
      remove(tmpname.c_str());
      throw std::runtime_error("Couldn't write " + filename);
    }
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

