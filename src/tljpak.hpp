/*  $Id: tljpak.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_TLJPAK_HPP
#define HEADER_TLJPAK_HPP

#include <string>
#include <vector>
#include <map>

class ProgressLogger;

struct ChunkDesc
{
  int offset;
  int size;
  int padding;

  ChunkDesc(int offset_, int size_)
    : offset(offset_), size(size_), padding(0)
  {}

  bool operator!=(const ChunkDesc& other)
  {
    return
      this->offset != other.offset ||
      this->size   != other.size;
  }
};

enum FileType {
  FILETYPE_INVALID, // used in case there is an error
  FILETYPE_DDS,
  FILETYPE_STFU4,
  FILETYPE_TLJBONE,
  FILETYPE_SHARK3D,
  FILETYPE_BUNDLE,
  FILETYPE_LOCALISATION,
  FILETYPE_TEXT,
  FILETYPE_WAV,
  FILETYPE_MP3,
  FILETYPE_LIP,
  FILETYPE_UNKNOWN
};

const char* filetype2string(FileType type);

struct FileEntry
{
  std::string pathname;
  std::vector<std::string> guesses;

  int offset;
  int filesize;
  FileType filetype;

  // Those could be analog to:
  // FileCount:    923
  // NumberCount:  371
  // ByteCount:   4598

  // if filesize is != 0 unknown1 is 0
  //                  range  : count
  int nametable_offset; // ~ 0-897  : 289 (like number1, reference to other file?)
  int path_length;      // ~ 0-85   : 78  (always ~100 range)
  int nametable_index;  // ~ 0-4585 : 427 (like number3)

  int chunk_id;

  //FileType filetype;

  FileEntry()
    : filetype(FILETYPE_INVALID), chunk_id(-1)
  {}

  bool is_file() {
    return filesize != 0;
  }

  bool is_terminal() {
    return nametable_index == 0 && path_length == 0 && nametable_index == 0;
  }
};

struct TLJPak
{
  std::string filename;

  int header_padding;

  char magic[12];
  int  file_count;   // file entries
  int  number_count; // number of entries in the second and last section
  int  byte_count;   // a count that is similar to the last number in numbers

  std::vector<FileEntry> files;     // array of FileEntries with length file_count
  std::vector<char>      nametable; // array of bytes with length byte_count, values are never larger then 43
  std::vector<int>       indextable;   // array of int32 with length number_count

  std::vector<std::vector<char> > chunks;

  TLJPak(const std::string& filename_);

  void write(const std::string& outfile);

  std::vector<ChunkDesc> collect_chunk_desc();
  void generate_filetable();
  void read_chunks();
  void insert(int pos, const std::vector<char>& buffer);

  std::string read_nametable(int offset);
  int lookup(const char* pathname, int ptr, int i, int depth);

  FileType get_type(const std::string& pathname);

  int      get_size(int id);
  FileType get_type(int id);

  std::vector<std::string> guess(int entry);
  void guess();

  void guess_lookup(int orig_entry, int source_entry, const std::string& pathname, int path_length,
                    std::vector<std::string>& guesses);

  /** Searches for \a pathname in the pack and returns the FileEntry
      that refers to the pathname */
  int lookup(std::string pathname);

  void extract(int file_entry, const std::string& outfile);

  void print_nametable();
  void print_indextable();
  void print_file_table();
  void print_info();

  /** Scans the pak and generates full pathnames, \a lst is a list of
      filenames to ease the brute force hash guessing */
  void scan(const std::vector<std::string>& lst, ProgressLogger& logger);

  void dump_filetable(std::ostream& out);
  void read_filetable(std::istream& in);
};

#endif

/* EOF */
