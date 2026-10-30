/*  $Id: system.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SYSTEM_HPP
#define HEADER_SYSTEM_HPP

#include <iosfwd>
#include <vector>
#include <string>
#include <stdint.h>
#include "util.hpp"

/** Handles the command line of the GUI programs, \a program is the
    name shown by --help and --version. Exits for --help, --version,
    --show-paths and on invalid arguments. */
void parse_args(int argc, char** argv, const std::string& program);

/** The full version string, see VERSION */
std::string get_version();

/** Directory of the shipped data: --data-dir, relative to the
    executable (../share/dftoolbox/ on Linux, data/ on Windows) or the
    compiled-in install path */
std::string get_data_path();

/** Directory for configuration files: $XDG_CONFIG_HOME/dftoolbox/ on
    Linux, %APPDATA%/dftoolbox/config/ on Windows, created if missing */
std::string get_config_path();

/** Path of the configuration file \a name for reading: falls back to
    where older versions kept it if it isn't in get_config_path() */
std::string get_config_file(const std::string& name);

/** Directory for regeneratable data: $XDG_CACHE_HOME/dftoolbox/ on
    Linux, %LOCALAPPDATA%/dftoolbox/cache/ on Windows, created if missing */
std::string get_cache_path();

/** Directory for mods installed by the user: $XDG_DATA_HOME/dftoolbox/
    on Linux, %LOCALAPPDATA%/dftoolbox/data/ on Windows, created if
    missing */
std::string get_user_data_path();
std::string get_temp_path();
std::string get_exe_path();

/** Prints every resolved directory and where it came from */
void print_paths(std::ostream& out);

/** Windows only: makes the output of a GUI program visible in the
    console it was started from */
void attach_parent_console();

/** Windows only: stops stdout from turning \n into \r\n, needed
    before writing binary data to it */
void set_binary_stdout();

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

/** Returns the size and modification time of \a pathname, false on error */
bool file_stat(const std::string& pathname, int64_t& size, int64_t& mtime);

/** Replaces \a filename in one step, so that a crash can't leave a
    half written file behind. Throws on failure, does nothing with
    --no-save. */
void file_write_atomic(const std::string& filename, const std::string& content);
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
