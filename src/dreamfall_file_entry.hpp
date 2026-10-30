/*  $Id: dreamfall_file_entry.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DREAMFALL_FILE_ENTRY_HPP
#define HEADER_DREAMFALL_FILE_ENTRY_HPP

#include <assert.h>
#include <string>
#include <vector>
#include "tljpak.hpp"

class DreamfallFileEntry;

/** What a file gets written out as */
enum ExportFormat {
  /** The data as it is stored, for a savegame that is the whole .sav */
  EXPORT_ORIGINAL,
  EXPORT_PNG,
  EXPORT_TEXT
};

struct SaveFileEntry
{
  enum Type { IMAGE_DATA, SHARK3D_DATA, TEXT_DATA, MESSAGELOG_DATA, JOURNAL_DATA };

  /** Select Image Data or Shark3D tree */
  Type type;

  /** Name of the save (savegame_001.sav) */
  std::string name;

  std::string fullname;

  SaveFileEntry() {}

  SaveFileEntry(Type type_, const std::string& name_, const std::string& fullname_)
    : type(type_), name(name_), fullname(fullname_)
  {}

  std::string get_url() const;
};

struct PakFileEntry
{
  FileType type;

  /** If empty the filename will be locked up in all paks, else only
      in the pak given by \a pakfile */
  std::string pakfile;

  std::string filename;
  std::string fullname;
  int filesize;

  // bool guessed;
  std::vector<std::string> guesses;

  // pakfiles
  // thumbnails

  PakFileEntry()
    : type(FILETYPE_UNKNOWN),
      filesize(0)
  {}

  std::string get_url() const;
};

class DirectoryFileEntry
{
public:
  std::string name;
  std::string fullname;
  std::vector<DreamfallFileEntry*> children;
  DreamfallFileEntry* parent;

  DirectoryFileEntry() :parent(0) {}
};

class DreamfallFileEntry
{
public:
  enum Type {
    UNKNOWN_FILE_ENTRY,
    DIRECTORY_FILE_ENTRY,
    SAVE_FILE_ENTRY,
    PAK_FILE_ENTRY
  };

private:
  Type type;

  PakFileEntry       pak;
  SaveFileEntry      save;
  DirectoryFileEntry dir;

public:
  DreamfallFileEntry()
    : type(UNKNOWN_FILE_ENTRY)
  {}

  DreamfallFileEntry(Type type_)
    : type(type_)
  {}

  DreamfallFileEntry(DirectoryFileEntry dir_)
    : type(DIRECTORY_FILE_ENTRY), dir(dir_)
  {}

  DreamfallFileEntry(PakFileEntry pak_)
    : type(PAK_FILE_ENTRY), pak(pak_)
  {}

  DreamfallFileEntry(SaveFileEntry save_)
    : type(SAVE_FILE_ENTRY), save(save_)
  {}

  bool is_save() const { return type == SAVE_FILE_ENTRY; }
  bool is_pak() const { return type == PAK_FILE_ENTRY; }
  bool is_dir() const { return type == DIRECTORY_FILE_ENTRY; }

  Type get_type() const { return type; }
  SaveFileEntry&       get_save() { assert(type == SAVE_FILE_ENTRY);      return save; }
  PakFileEntry&        get_pak()  { assert(type == PAK_FILE_ENTRY);       return pak;  }
  DirectoryFileEntry&  get_dir()  { assert(type == DIRECTORY_FILE_ENTRY); return dir;  }
  std::string get_url() const;

  /** The formats this entry can be exported to, EXPORT_ORIGINAL
      first. Empty for directories. */
  std::vector<ExportFormat> get_export_formats() const;

  /** The filename for an export to \a format, Dreamfall stores
      textures that really are DDS as .png, so the original of foo.png
      is foo.png.dds and only its conversion is foo.png, the same goes
      for STFU4 textures */
  std::string get_export_name(ExportFormat format) const;

  // Function for placing the file entry in the tree
  std::string get_label() const;
  std::string get_path() const;
};

#endif

/* EOF */
