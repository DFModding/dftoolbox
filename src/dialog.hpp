/*  $Id: dialog.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DIALOG_HPP
#define HEADER_DIALOG_HPP

#include <vector>
#include <fstream>
#include <iostream>

// Dialog can be found in:
// c2d59212e9753be95c6ef8d425a9e14f  dreamfall-extractor/init/dat/0212.dat
// c2d59212e9753be95c6ef8d425a9e14f  dreamfall-extractor/laidback/dat/0003.dat


// 0x12: 14010 (number of text?)
// 0x1B5EA(112106): first text
// 0x1b5f8(112120): second text (14 distantce from 1.)
// 9564 lines of text in the file (index doing double refs -> yes, sort|uniq|wc gives exactly 9564)
// indexs: 0, 14, 19, ..., 580862, 580998
// number1 
// Double refs:
// 869 times: 56
// 1039 times: 85
// 287 times:  578
// longest string: 6508
struct TextEntry
{
  /** offset where the text starts, offset is relative to the first string */
  unsigned int offset; 
  unsigned int id;
};

struct Dialog
{
  std::string            lang_code;
  std::string            language;
  std::vector<TextEntry> entries;
  std::vector<char>      texts;

  std::string get_by_id(unsigned int id) 
  {
    for(unsigned int i = 0; i < entries.size(); ++i)
      {
        if (entries[i].id == id)
          {
            return &*texts.begin() + entries[i].offset;
          }
      } 
    return "";
  }

  void write(const std::string& outfile)
  {
    std::ofstream out(outfile.c_str(), std::ios::binary);
    unsigned int version = 2;


    out.write(reinterpret_cast<char*>(&version), sizeof(version));

    unsigned int language_len = language.size();   

    out.write(lang_code.c_str(), lang_code.size());
    out.write(reinterpret_cast<char*>(&language_len), sizeof(language_len));
    out.write(language.c_str(),  language.size());

    unsigned int entry_count = entries.size();
    out.write(reinterpret_cast<char*>(&entry_count), sizeof(entry_count));

    unsigned int blocksize = texts.size();
    out.write(reinterpret_cast<char*>(&blocksize), sizeof(blocksize));

    for(unsigned int i = 0; i < entries.size(); ++i)
      {
        TextEntry& entry = entries[i];

        out.write(reinterpret_cast<char*>(&entry.id),     sizeof(entry.id));
        out.write(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset)); // regenerate this
      }

    out.write(&*texts.begin(), texts.size());

    out.close();
  }
};

inline void read_dialogs(const std::string& filename, std::vector<Dialog>& dialogs)
{
  // std::cout << "Processing: " << filename << std::endl;
  std::ifstream in(filename.c_str(), std::ios::binary);

  while (true)
    {
      Dialog dialog;

      unsigned int entry_count = 0;  
      unsigned int version = 0;
      in.read(reinterpret_cast<char*>(&version), sizeof(unsigned int));

      if (!in) // End of File found, no more translations available
        {
          std::cout << "End of file found: " << version << std::endl;
          break;
        }
      //std::cout << "." << std::endl;

      if (version == 0)
        { // english file
          in.seekg(0x12, std::ios::beg); 
          in.read(reinterpret_cast<char*>(&entry_count), sizeof(unsigned int));
          //std::cout << "EntryCount: " << entry_count << std::endl;

          unsigned int blocksize = 0;
          in.read(reinterpret_cast<char*>(&blocksize), sizeof(unsigned int));
          dialog.texts.resize(blocksize);
          dialog.lang_code = "en";
          dialog.language  = "English";
        }
      else if (version == 2)
        {
          dialog.lang_code += in.get();
          dialog.lang_code += in.get();

          unsigned int language_len = 0;
          in.read(reinterpret_cast<char*>(&language_len), sizeof(unsigned int));
      
          char language[language_len + 1];
          in.read(reinterpret_cast<char*>(&language), language_len);
          language[language_len] = '\0';
          dialog.language = language;

          //std::cout << "Code:     " << dialog.lang_code << std::endl;
          //std::cout << "Language: " << dialog.language  << std::endl;

          in.read(reinterpret_cast<char*>(&entry_count), sizeof(unsigned int));
          std::cout << "EntryCount: " << entry_count << std::endl;

          unsigned int blocksize = 0;
          in.read(reinterpret_cast<char*>(&blocksize), sizeof(unsigned int));
          dialog.texts.resize(blocksize);
        }
      else
        {
          std::cout << "Error: wrong version: " << version << std::endl;
          exit(EXIT_FAILURE);
        }

      for(unsigned int i = 0; in && (i < entry_count); ++i)
        {
          TextEntry entry;
          in.read(reinterpret_cast<char*>(&entry.id),     sizeof(entry.id));
          in.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));

          // printf("%i: %10d %10d %10d\n", i, entry.offset, entry.number2, entry.number3);
          //printf("%10d\n", entry.number1);
          dialog.entries.push_back(entry);
        }

      in.read(&*dialog.texts.begin(), dialog.texts.size());

      if (0)
        { // debug print
          for(unsigned int i = 0; i < entry_count; ++i)
            {
              TextEntry& entry = dialog.entries[i];
              printf("%5i: %10d %10d  \"%s\"\n", i, 
                     entry.offset, entry.id, &*dialog.texts.begin() + entry.offset);
            }
        }

      dialogs.push_back(dialog);
    } // while(true)
  //std::cout << "EOF" << std::endl;
  in.close();
}

#endif

/* EOF */
