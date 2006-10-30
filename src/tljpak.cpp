/*  $Id: tljpak.cpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "system.hpp"
#include "progress_logger.hpp"
#include "shark3d.hpp"
#include "tljpak.hpp"

char nametabel2ascii_tbl[] = "\0abcdefghijklmnopqrstuvwxyz/\?\?-_'.0123456789"; // 44
char ascii2nametable_tbl[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,32, 0, 0, 0, 0, 0,30,33,27,
  34,35,36,37,38,39,40,41,42,43, 0, 0, 0, 0, 0,28,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26, 0, 0, 0, 0,31,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

inline char ascii2nametable(char c)
{
  return ascii2nametable_tbl[int(c)];
}

inline char  nametable2ascii(char c)
{
  if (size_t(c) < sizeof(nametabel2ascii_tbl))
    {
      return nametabel2ascii_tbl[int(c)];
    }
  else 
    {
      std::cerr << "\nERROR: " << int(c) << " out of table " << sizeof(nametabel2ascii_tbl) << std::endl;
      return 'a';
    }
}

const char* filetype2string(FileType type)
{
  switch(type)
    {
    case FILETYPE_DDS:
      return "DDS";
      
    case FILETYPE_STFU4:
      return "STFU4";
      
    case FILETYPE_TLJBONE:
      return "TLJBONE";

    case FILETYPE_SHARK3D:
      return "SHARK3D";

    case FILETYPE_LOCALISATION:
      return "LOCALISATION";

    case FILETYPE_BUNDLE:
      return "BUNDLE";
            
    case FILETYPE_TEXT:
      return "TEXT";
      
    case FILETYPE_UNKNOWN:
      return "UNKNOWN";

    case FILETYPE_WAV:
      return "WAV";

    case FILETYPE_MP3:
      return "MP3";

    case FILETYPE_LIP:
      return "LIP";

    default:
      return "INVALID";
    }
}

FileType string2filetype(const char* str)
{
  if (strcmp(str, "DDS") == 0)
    return FILETYPE_DDS;
      
  else if (strcmp(str, "STFU4") == 0)
    return FILETYPE_STFU4;
  
  else if (strcmp(str, "TLJBONE") == 0)
    return FILETYPE_TLJBONE;

  else if (strcmp(str, "SHARK3D") == 0)
    return FILETYPE_SHARK3D;

  else if (strcmp(str, "LOCALISATION") == 0)
    return FILETYPE_LOCALISATION;

  else if (strcmp(str, "BUNDLE") == 0)
    return FILETYPE_BUNDLE;
      
  else if (strcmp(str, "TEXT") == 0)
    return FILETYPE_TEXT;
      
  else if (strcmp(str, "UNKNOWN") == 0)
    return FILETYPE_UNKNOWN;

  else if (strcmp(str, "WAV") == 0)
    return FILETYPE_WAV;

  else if (strcmp(str, "MP3") == 0)
    return FILETYPE_MP3;
  
  else if (strcmp(str, "LIP") == 0)
    return FILETYPE_LIP;

  else
    return FILETYPE_INVALID;
}

TLJPak::TLJPak(const std::string& filename_)
  : filename(filename_)
{
  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    {
      throw std::runtime_error("Error: Couldn't open " + filename);
    }

  in.read(reinterpret_cast<char*>(&magic), 12);

  if (strncmp(magic, "tlj_pack0001", 12) != 0)
    {
      throw std::runtime_error("Error: Not a TLJPak file: " + filename);
    }

  in.read(reinterpret_cast<char*>(&file_count), sizeof(int));
  in.read(reinterpret_cast<char*>(&number_count), sizeof(int));
  in.read(reinterpret_cast<char*>(&byte_count), sizeof(int));

  for(int i = 0; i < file_count; ++i)
    {
      FileEntry entry;
      int vals[5];

      in.read(reinterpret_cast<char*>(vals), sizeof(int)*5); 

      entry.offset           = vals[0];
      entry.filesize         = vals[1];
      entry.nametable_offset = vals[2];
      entry.path_length      = vals[3];
      entry.nametable_index  = vals[4];

      files.push_back(entry);
    }

  // Read Nametable
  nametable.resize(byte_count);
  in.read(reinterpret_cast<char*>(&*nametable.begin()), nametable.size());

  for(unsigned int i = 0; i < nametable.size(); ++i)
    { // FIXME: Slow!
      nametable[i] = nametable2ascii(nametable[i]);
    }

  // Read Index Table
  indextable.resize(number_count);
  in.read(reinterpret_cast<char*>(&*indextable.begin()), indextable.size());

  in.close();
}

void
TLJPak::generate_filetable()
{
  //std::cout << "generate_filetable()" << std::endl;

  int pos = 
    12 + 4 + 4 + 4 +       // Header
    files.size() * 4 * 5 + // Filetable
    nametable.size() +     // Nametable
    indextable.size()*4    // Indextable
    ;

  header_padding = pos;

  // Round pos up to a multiple of 2048, not sure if its needed, but
  // thats how the original .paks look like
  if ((pos % 2048) != 0)
    pos = ((pos / 2048) + 1) * 2048; 
  
  //int start_pos = pos;
  header_padding = pos - header_padding;
  
  //std::cout << "Start Pos: " << pos << " " << header_padding << std::endl;

  std::vector<ChunkDesc> chunk_descs;
  for(int i = 0; i < int(chunks.size()); ++i)
    {
      chunk_descs.push_back(ChunkDesc(pos, chunks[i].size()));
      pos += chunks[i].size();
    }

  for(int i = 0; i < int(files.size()); ++i)
    {
      assert(files[i].chunk_id != -1);
      files[i].offset = chunk_descs[files[i].chunk_id].offset;
      
      if (files[i].filesize != 0)
        files[i].filesize = chunk_descs[files[i].chunk_id].size;
    }
}

void
TLJPak::insert(int pos, const std::vector<char>& buffer)
{
  if (chunks.size() == 0)
    read_chunks();

  assert(chunks.size() > 0);

  int chunk_id = files[pos].chunk_id;
  if (chunk_id != -1)
    {
      if (chunk_id >= 0 && chunk_id < int(chunks.size()))
        {
          chunks[chunk_id] = buffer;
        }
      else
        {
          std::cout << "Error: TLJPak::insert(): Couldn't insert file at " << chunk_id << " only have " 
                    << chunks.size() << " entries"
                    << std::endl;
        }
    }
  else
    {
      std::cout << "Error: TLJPak::insert(): " << chunk_id << " not a valid file entry" << std::endl; 
    }
}

void
TLJPak::write(const std::string& outfile)
{
  if (chunks.size() == 0)
    read_chunks();
  assert(chunks.size() > 0);

  generate_filetable();

  std::ofstream out(outfile.c_str(), std::ios::binary);

  if (!out)
    throw std::runtime_error("TLJPak::write(): Couldn't open " + outfile);

  out.write(magic, 12);
  out.write(reinterpret_cast<char*>(&file_count),   sizeof(int));
  out.write(reinterpret_cast<char*>(&number_count), sizeof(int));
  out.write(reinterpret_cast<char*>(&byte_count),   sizeof(int));

  // write file index table
  for(int i = 0; i < file_count; ++i)
    {
      out.write(reinterpret_cast<char*>(&files[i].offset),           sizeof(int)); 
      out.write(reinterpret_cast<char*>(&files[i].filesize),         sizeof(int)); 
      out.write(reinterpret_cast<char*>(&files[i].nametable_offset), sizeof(int)); 
      out.write(reinterpret_cast<char*>(&files[i].path_length),      sizeof(int)); 
      out.write(reinterpret_cast<char*>(&files[i].nametable_index),  sizeof(int)); 
    }

  // write byte block
  for(int i = 0; i < int(nametable.size()); ++i)
    {
      // FIXME: Encode this
      //out.write(reinterpret_cast<char*>(&nametable[i]), sizeof(char));
      out.put(ascii2nametable(nametable[i]));
    }

  // write number block
  for(int i = 0; i < int(indextable.size()); ++i)
    {
      out.write(reinterpret_cast<char*>(&indextable[i]), sizeof(int));
    }

  for(int i = 0; i < header_padding; ++i)
    out.put(0);

  for(int i = 0; i < int(chunks.size()); ++i)
    {
      out.write(reinterpret_cast<char*>(&*chunks[i].begin()), chunks[i].size());
    }

  { // fill in padding bytes at the end of file to reach a total
    // filesize of a multiple of 131072, not sure if these are
    // needed, but we do so to have exactly the same file structure
    // like in the original paks
    out.seekp(0, std::ios::end);
    int end = out.tellp();

    if ((end % 131072) != 0)
      {
        for(int i = 0; i < 131072 - (end % 131072); ++i)
          out.put(0xae);
      }
  } 

  out.close();
}

struct ChunkSort
{
  bool operator()(const ChunkDesc& a, const ChunkDesc& b)
  {
    return a.offset < b.offset;
  }
};

std::vector<ChunkDesc>
TLJPak::collect_chunk_desc()
{
  std::vector<ChunkDesc> firstsweep;

  // Collect chunks
  for(int i = 0; i < int(files.size()); ++i)
    {
      if (files[i].is_file())
        firstsweep.push_back(ChunkDesc(files[i].offset, files[i].filesize));
    }

  std::sort(firstsweep.begin(), firstsweep.end(), ChunkSort());

  std::vector<ChunkDesc> chunk_lst;
  
  // Remove any duplicates
  for(int i = 0; i < int(firstsweep.size()); ++i)
    {
      if (chunk_lst.size() == 0 || chunk_lst.back() != firstsweep[i])
        { 
          chunk_lst.push_back(firstsweep[i]);
        }
    }

  assert(chunk_lst.size() > 0);
  // Check for holes (might not be needed)
  for(int i = 1; i < int(chunk_lst.size()); ++i)
    { // some files themself are '0' padded as well!

      //std::cout << chunk_lst[i].offset << std::endl;
      if (chunk_lst[i].offset != (chunk_lst[i-1].offset + chunk_lst[i-1].size))
        {
          chunk_lst[i].padding = chunk_lst[i].offset - (chunk_lst[i-1].offset + chunk_lst[i-1].size);

          if (0)
            std::cout << "Hole in file: got "
                      << chunk_lst[i].offset
                      << " expected " << chunk_lst[i-1].offset + chunk_lst[i-1].size 
                      << " distance "
                      << chunk_lst[i].padding
                      << std::endl;
        }
    }

  return chunk_lst;
}

void
TLJPak::read_chunks()
{
  //std::cout << "read_chunks()" << std::endl;
  const std::vector<ChunkDesc>& chunk_descs = collect_chunk_desc();

  if (0)
    {
      std::cout << std::setw(10) << "Offset" 
                << std::setw(10) << "Size"
                << std::setw(10) << "Padding"
                << std::endl;

      for(int i = 0; i < int(chunk_descs.size()); ++i)
        {
          std::cout << std::setw(10) << chunk_descs[i].offset   << " " 
                    << std::setw(10) << chunk_descs[i].size     << " "
                    << std::setw(10) << chunk_descs[i].padding  << " "
                    << std::endl;
        }
    }

  // Write chunk ids into the filetable
  std::map<int, int> offset2chunkid;
  for(int i = 0; i < int(chunk_descs.size()); ++i)  
    {
      offset2chunkid[chunk_descs[i].offset] = i;
    }
  
  for(int i = 0; i < int(files.size()); ++i)
    {
      std::map<int, int>::iterator it = offset2chunkid.find(files[i].offset);
      if (it != offset2chunkid.end())
        {
          if (files[i].is_file() &&
              (chunk_descs[it->second].offset != files[i].offset ||
               chunk_descs[it->second].size   != files[i].filesize))
            {
              std::ostringstream str;
              str << "Error: Inconsistent File table at: " 
                  << files[i].offset << ":" << files[i].filesize << " "
                  << chunk_descs[it->second].offset << ":" << chunk_descs[it->second].size;
              throw std::runtime_error(str.str());
            }
          else
            {
              files[i].chunk_id = it->second;          
            }
        }
      else
        {
          std::cout << "Warning: file offset " << files[i].offset << " not found in chunk list" << std::endl;
        }
    }

  // inconsistentcy check to make sure that all chunk_ids are != -1
  for(int i = 0; i < int(files.size()); ++i)
    {
      if (files[i].chunk_id == -1)
        {
          std::ostringstream str;
          str << "Error: Inconsistentcy file table, entry not mapped to chunk: " 
              << files[i].offset << " " << files[i].filesize;
          throw std::runtime_error(str.str());
        }
    }

  
  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    {
      throw std::runtime_error("Error: Couldn't open " + filename);
    }

  chunks.resize(chunk_descs.size());
  for(int i = 0; i < int(chunk_descs.size()); ++i)  
    {
      std::vector<char>& chunk = chunks[i];
      chunk.resize(chunk_descs[i].size + chunk_descs[i].padding, 0);

      in.seekg(chunk_descs[i].offset, std::ios::beg);
      in.read(reinterpret_cast<char*>(&*chunk.begin()), chunk_descs[i].size);
    }

  in.close();
}

std::string
TLJPak::read_nametable(int offset)
{
  std::string str;
  for(int i = 0; nametable[i] != '\0'; ++i)
    str += nametable[offset + i];
  return str;
}

int
TLJPak::lookup(const char* pathname, int ptr, int i, int depth)
{    
  if (depth > 20)
    return -1;
    
  if (i < 0 || i >= int(files.size()))
    {
      std::cerr << "Error: Lookup out of range " << std::endl;
      return -1;
    }

  if (files[i].filesize         == 0 &&
      files[i].path_length      == 0 &&
      files[i].nametable_offset == 0 &&
      files[i].nametable_index  == 0)
    {
      return -1;
    }
  else
    {
      assert(files[i].nametable_index >= 0 && files[i].nametable_index < int(nametable.size()));
        
      char* ntbl_entry     = &*nametable.begin() + files[i].nametable_index;
      int   ntbl_entry_len = strlen(ntbl_entry);

      if (0)
        {
          std::cout << ntbl_entry << " == ";
          std::cout.write(pathname+ptr+1, ntbl_entry_len);
          std::cout << std::endl;
        }

      if (strncmp(ntbl_entry, pathname+ptr+1, ntbl_entry_len) != 0)
        {
          return -1;
        }
      else
        {
          if (files[i].filesize != 0)
            { // Found a file
              //std::cout << files[i].path_length << std::endl;
              return i;
            }
          else
            {
              if (0)
                {
                  std::cout << i << std::endl;
                  std::cout << "Offset:     " << files[i].offset << "\n"
                            << "Filesize:   " << files[i].filesize << "\n"
                            << "NTblOffset: " << files[i].nametable_offset << "\n"
                            << "PathLength: " << files[i].path_length << "\n"
                            << "NTblIndex:  " << files[i].nametable_index << std::endl;
        
                  std::cout << "Pathname:    " << pathname << " " << strlen(pathname) << std::endl;
                  std::cout << "PathPtr:     " << pathname + ptr << " " << strlen(pathname + ptr) << std::endl;
                  std::cout << "Path so far: "; std::cout.write(pathname, files[i].path_length+1); std::cout << std::endl;
                }
              return lookup(pathname, ptr + ntbl_entry_len + 1, 
                            ascii2nametable(pathname[files[i].path_length+1]) + files[i].nametable_offset, depth+1);
            }
        }
    }
}

bool is_good_filename(const std::string& filename)
{
  for(int j = 0; j < int(filename.size()); ++j)
    {
      if (filename[j] == '?')
        return false;
    }
  return true;
}

std::vector<std::string>
filter_filenames(const std::vector<std::string>& guesses)
{
  std::vector<std::string> ret;

  for(std::vector<std::string>::const_iterator i = guesses.begin(); i != guesses.end(); ++i)
    {
      if (is_good_filename(*i))
        ret.push_back(*i);
    }

  return ret;
}

std::vector<std::string>
TLJPak::guess(int entry)
{
  assert(entry >= 0);
  assert(entry < file_count);
  assert(files[entry].is_file());

  std::string pathname = &nametable[files[entry].nametable_index];
  std::vector<std::string> guesses;
  guess_lookup(entry, entry, pathname, files[entry].path_length, guesses);
  return filter_filenames(guesses);
}

void
TLJPak::guess()
{
  for(int entry = 0; entry < file_count; ++entry)
    {
      if (files[entry].is_file()) 
        {
          std::vector<std::string> guesses;
  
          std::string pathname = &nametable[files[entry].nametable_index];
          guess_lookup(entry, entry, pathname, files[entry].path_length, guesses);

          //            if (guesses.size() == 1)
          //              std::cout << guesses[0] << std::endl;
            
          if (1)
            {
              for(int j = 0; j < int(guesses.size()); ++j)
                std::cout << entry << ") " << guesses[j] << std::endl;

              std::cout << std::endl;
            }
        }
    }
}

void
TLJPak::guess_lookup(int orig_entry, int source_entry, const std::string& pathname, int path_length, 
                     std::vector<std::string>& guesses)
{
  if (int(pathname.size()) + 1 == path_length)
    {
      //std::cout 
      //<< orig_entry << " "
      //<< nametable2ascii(source_entry) << pathname << std::endl;
      guesses.push_back(nametable2ascii(source_entry) + pathname);
    }
  else
    {
      for(int entry = 0; entry < file_count; ++entry)
        {
          if (!files[entry].is_file()) 
            {
              for(int j = 1; j < 44; ++j)
                {
                  if (files[entry].nametable_offset + j == source_entry
                      && files[entry].path_length + int(pathname.length()) + 2 == path_length)
                    {
                      std::ostringstream str;
                      str << &nametable[files[entry].nametable_index] << nametable2ascii(j) << pathname;
                      guess_lookup(orig_entry, entry, str.str(), path_length, guesses);
                    }
                }
            }
        }
    }
}

/** Searches for \a pathname in the pack and returns the FileEntry
    that refers to the pathname */
int
TLJPak::lookup(std::string pathname)
{
  for(unsigned int i = 0; i < pathname.size(); ++i)
    pathname[i] = tolower(pathname[i]);
  //std::cout << "Lookup: " << pathname << std::endl;
  int i = ascii2nametable(pathname[0]);
  return lookup(pathname.c_str(), 0, i, 0);
}

void
TLJPak::extract(int file_entry, const std::string& outfile)
{
  assert(file_entry >= 0 && file_entry < int(files.size()));
    
  create_hierachy(outfile);

  if (files[file_entry].is_file())
    {
      std::ifstream in(filename.c_str(), std::ios::binary);
      if (!in)
        throw std::runtime_error("Error: Couldn't open " + filename);

      std::vector<char> filebuf;
      filebuf.resize(files[file_entry].filesize);
      in.seekg(files[file_entry].offset, std::ios::beg);
      in.read(reinterpret_cast<char*>(&*filebuf.begin()), filebuf.size());

      std::ofstream out(outfile.c_str(), std::ios::binary);
      if (!out)
        throw std::runtime_error("Error: Couldn't open " + outfile);
      out.write(reinterpret_cast<char*>(&*filebuf.begin()), filebuf.size());
      out.close();

      in.close();
    }
}

void
TLJPak::print_nametable()
{ 
  for(int i = 0; i < int(nametable.size()); ++i)
    {
      std::cout << i << ") ";
      while(nametable[i] != '\0' && i < int(nametable.size()))
        std::cout << nametable[i++];
      std::cout << std::endl;
    }
}

void
TLJPak::print_indextable()
{
  for(int i = 0; i < int(indextable.size()); ++i)
    std::cout << indextable[i] << std::endl;
}

void
TLJPak::print_file_table()
{
  if (1)
    {
      std::cout << "  Nr.     offset   filesize   NTblOffset  PathLength NTblIndex      Name" << std::endl;
      std::cout << "=================================================================================" << std::endl;
    }

  for(int i = 0; i < int(files.size()); ++i)
    printf("%4d) %10d %10d %10d %10d %10d        %s\n", 
           i,
           files[i].offset, 
           files[i].filesize,
           files[i].nametable_offset,
           files[i].path_length,
           files[i].nametable_index,
           (files[i].filesize + files[i].nametable_offset + files[i].path_length + files[i].nametable_index == 0)
           ? "(null)" : ("\"" + std::string(&nametable[files[i].nametable_index]) + "\"").c_str());
}

void
TLJPak::print_info()
{
  //std::cout << "Filename: " << filename << std::endl;
  //std::cout << "Filesize: " << st.st_size/1024/1024 << "mb (" << st.st_size << " nametable)" << std::endl;
  if (0)
    {
      std::cout << "Magic:         ";
      std::cout.write(magic, 12);
      std::cout << std::endl;

      std::cout << "File_Count:    " << file_count << std::endl;
      std::cout << "Number_Count:  " << number_count << std::endl;
      std::cout << "Nametable Count:   " << byte_count << std::endl;


      int sum = 0;
      for(int i = 0; i < int(files.size()); ++i)
        if (files[i].filesize != 0)
          sum += 1;
      std::cout << "Real Files:    " << sum << " (files with non-null filesize)" << std::endl;

      std::cout << std::endl;

      std::cout << "relative: " << indextable.back() << " < " << byte_count << std::flush;
      if (indextable.back() >= byte_count) std::cout << "!!!WRONG!!!" << std::endl; 
      else std::cout << std::endl;
    
    }
  print_nametable();
  print_file_table();
}

FileType
TLJPak::get_type(const std::string& pathname)
{
  std::cout << "Pathname: " << pathname << std::endl;
  int i = lookup(pathname);
  if (i == -1)
    {
      return FILETYPE_INVALID;
    }
  else
    {
      return get_type(i);
    }
}

FileType
TLJPak::get_type(int i)
{
  // FIXME: add some assert/exceptions here
  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    {
      throw std::runtime_error("Error: Couldn't open " + filename);
    }
  else
    {
      char magic[8];
      in.seekg(files[i].offset, std::ios::beg);
      in.read(magic, sizeof(magic));
      in.close();

      if (strncmp(magic, "DDS", 3) == 0)
        return FILETYPE_DDS;
      else if (strncmp(magic, "tljbone", 7) == 0)
        return FILETYPE_TLJBONE;
      else if (strncmp(magic, "STFU4", 4) == 0)
        return FILETYPE_STFU4;
      else if (strncmp(magic, "RIFF", 4) == 0)
        return FILETYPE_WAV;
      else if (strncmp(magic, "shark3d", 7) == 0)
        return FILETYPE_SHARK3D;
      else if (has_suffix(files[i].pathname, ".bun"))
        return FILETYPE_BUNDLE;
      else if (has_suffix(files[i].pathname, ".mp3"))
        return FILETYPE_MP3;
      else if (has_suffix(files[i].pathname, ".wav"))
        return FILETYPE_WAV;
      else if (has_suffix(files[i].pathname, ".lip"))
        return FILETYPE_LIP;
      else if (files[i].pathname, "art/generated/config/universe/localization.dat")
        return FILETYPE_LOCALISATION;
      // FIXME: This is mostly guessing
      else if (strncmp(magic, "ID3", 3) == 0)
        return FILETYPE_MP3;
      else if (strncmp(magic, "vs", 2) == 0 || strncmp(magic, "xvs", 3) == 0)
        return FILETYPE_TEXT;
      else if (strncmp(magic, "ps", 2) == 0 || strncmp(magic, "xps", 3) == 0)
        return FILETYPE_TEXT;
      else if (strncmp(magic, "; ", 2) == 0 || strncmp(magic, "//", 2) == 0)
        return FILETYPE_TEXT;
      else          
        return FILETYPE_UNKNOWN;
    }
}

int
TLJPak::get_size(int id)
{
  return files[id].filesize;
}

void
TLJPak::scan(const std::vector<std::string>& lst, ProgressLogger& logger)
{
  std::map<int, std::string> entry2name;
  
  for(std::vector<std::string>::const_iterator i = lst.begin(); i != lst.end(); ++i)
    {
      int j = lookup(*i);
      if (j != -1)
        files[j].pathname = *i;
    }

  logger.set_task_size(files.size());
  for(int i = 0; i < int(files.size()); ++i)
    {
      if (files[i].is_file() && files[i].pathname.empty())
        {
          // FIXME: Add filtering somewhere
          files[i].guesses = guess(i);
          assert(files[i].guesses.size() > 0);
          files[i].pathname = files[i].guesses.front();
          logger.println("found " + files[i].pathname + " (guessed)");
        }
      /* to much of an speed impact 
      else if (!files[i].pathname.empty())
        {
        logger.println("found " + files[i].pathname);
        }
      */

      files[i].filetype = get_type(i);

      logger.set_task_status(i);
      logger.sync();
    }
}

void
TLJPak::dump_filetable(std::ostream& out)
{
  std::string pathpart, filepart;
  splitpath(filename, pathpart, filepart);

  out << "filename \"" << filepart << "\"" << std::endl;
  out << "files" << std::endl;
  for(int i = 0; i < int(files.size()); ++i)
    {
      if (files[i].is_file())
        {
          out << "{" << std::endl;
          out << "    index      " << i << std::endl;
          //out << "    pathname \"" << files[i].pathname << "\"" << std::endl;
          out << "    pathnames  ";
          if (files[i].guesses.empty())
            {
              out << "\"" << files[i].pathname << "\"" << std::endl;
            }
          else
            {
              for(unsigned int j = 0; j < files[i].guesses.size(); ++j)
                out << "\"" << files[i].guesses[j] << "\" ";
              out << std::endl;
            }
          out << "    filesize   " << files[i].filesize << std::endl;
          out << "    filetype   \"" << filetype2string(files[i].filetype) << "\"" << std::endl;
          out << "}" << std::endl;
        }
    }
  out << "$" << std::endl;
}

void
TLJPak::read_filetable(std::istream& in)
{
  Shark3D* shark = Shark3D::parse_text(in);
  if (!has_suffix(filename, shark->get_string("filename", "")))
    {
      std::cout << "TLJPak::read_filetable: filename missmatch in " << filename << std::endl;
    }

  SectionNodes* nodes = shark->get_sections("files");
  for(std::vector<SectionNode*>::iterator i = nodes->sections.begin(); 
      i != nodes->sections.end(); ++i)
    {
      int index = (*i)->get_ints("index").front();
      
      assert(index >= 0 && index < int(files.size()) && files[index].is_file());

      files[index].guesses  = (*i)->get_strings("pathnames");
      files[index].pathname = files[index].guesses.front();
      files[index].filetype = string2filetype((*i)->get_strings("filetype").front().c_str());
      //files[i].guesses = (*i)->get_strings("pathnames");
    }
}

/* EOF */
