/*  $Id: dialog.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <string.h>
#include <assert.h>
#include <vector>
#include <iostream>
#include <fstream>

#include "dialog.hpp"
#include "tinygettext.hpp"
#include "system.hpp"

std::string escape(const std::string& text)
{
  std::string res;
  for(std::string::size_type i = 0; i < text.length(); ++i)
    {
      if (text[i] == '"')
        res += "\\\"";
      else
        res += text[i];
    }
  return res;
}

void write_po2(const Dialog& dialog, const Dialog& dialog_en)
{
  std::map<int, std::vector<int> > offset2entry;

  for(unsigned int i = 0; i < dialog.entries.size(); ++i)
    {
      offset2entry[dialog.entries[i].offset].push_back(i);
    }

  std::string filename = get_exe_path() + "dreamfall-" + dialog.lang_code + ".po";
  std::ofstream out(filename.c_str(), std::ios::binary);
  out << "# Dreamfall Dialog Extractor V0.0\n"
      << "msgid \"\"\n"
      << "msgstr \"\"\n"
      << "\"MIME-Version: 1.0\\n\"\n"
      << "\"Content-Type: text/plain; charset=UTF-8\\n\"\n"
      << "\"Content-Transfer-Encoding: 8bit\\n\"\n"
      << std::endl;        

  const char* texts = &*dialog.texts.begin();
  int   size  = dialog.texts.size();
  for(int i = 0; i < size; ++i)
    {
      std::map<int, std::vector<int> >::iterator j = offset2entry.find(i);
      if (j == offset2entry.end())
        {
          std::cout << "Error: localization.dat looks inconsistent" << std::endl;
        }
      else
        {
          out << "# offset: " << i << ", entries: ";
          for(int k = 0; k < int(j->second.size()); ++k)
            out << j->second[k] << " ";
          out << std::endl;      
          if (dialog_en.entries.size() != 0)
            {
              out << "msgid \"" << escape((&*dialog_en.texts.begin()) + dialog_en.entries[j->second.front()].offset) << "\"" << std::endl;
              out << "msgstr \"" << escape(texts+i) << "\"" << std::endl;
            }
          else
            {
              out << "msgid \"" << escape(texts+i) << "\"" << std::endl;
              out << "msgstr \"\"" << std::endl;
            }
          out << std::endl;
        }

      i += strlen(texts+i);
    }

  //out << "\n# EOF #\n";
  out.close(); 
}

void write_po(const Dialog& dialog, const Dialog& dialog_en)
{
  bool use_en = true;
  if (dialog_en.entries.size() == 0)
    {
      use_en = false;
    }
  else
    {
      assert(dialog_en.entries.size() == dialog.entries.size());
    }

  std::string filename = get_exe_path() + "dreamfall-" + dialog.lang_code + ".po";
  std::ofstream out(filename.c_str(), std::ios::binary);
  out << "# Dreamfall Dialog Extractor V0.0\n"
      << "msgid \"\"\n"
      << "msgstr \"\"\n"
      << "\"MIME-Version: 1.0\\n\"\n"
      << "\"Content-Type: text/plain; charset=UTF-8\\n\"\n"
      << "\"Content-Transfer-Encoding: 8bit\\n\"\n"
      << std::endl;        

  for(unsigned int i = 0; i < dialog.entries.size(); ++i)
    {
      if (use_en)
        {
          const TextEntry& entry    = dialog.entries[i];
          const TextEntry& entry_en = dialog_en.entries[i];
          
          out << "# " << std::endl;
          out << "msgid \""  << escape(&*dialog_en.texts.begin() + entry_en.offset) << "\"" << std::endl;
          out << "msgstr \"" << escape(&*dialog.texts.begin() + entry.offset) << "\"\n" << std::endl;
        }
      else
        {
          const TextEntry& entry = dialog.entries[i];
          
          out << "# " << std::endl;
          out << "msgid \"" << escape(&*dialog.texts.begin() + entry.offset) << "\"" << std::endl;
          out << "msgstr \"\"\n" << std::endl;          
        }
    }

  //out << "\n# EOF #\n";
  out.close();
}


void extract(const char* filename)
{
  std::vector<Dialog> dialogs;
  read_dialogs(filename, dialogs);

  Dialog dialog_en;
  if (dialogs.size() == 1)
    {
      write_po2(dialogs.front(), Dialog());
    }
  else
    {
      for(std::vector<Dialog>::iterator i = dialogs.begin(); i != dialogs.end(); ++i)
        {
          if (i->lang_code == "en")
            {
              dialog_en = *i;
            }
        }
  
      if (dialog_en.entries.size() == 0)
        {
          std::cout << "Couldn't find english dialogs" << std::endl;
        }
      else
        {
          for(std::vector<Dialog>::iterator i = dialogs.begin(); i != dialogs.end(); ++i)
            {
              std::cout << "Writing: " << i->language << std::endl;
              if (i->lang_code != "en")
                {
                  write_po2(*i, dialog_en);
                }
              else
                {
                  write_po2(*i, Dialog());
                }
            }
        }
    }
}

void generate(const std::string& localisation, const std::string& po_filename,
              const std::string& outfile)
{
  TinyGetText::Dictionary dictionary;
  std::ifstream po_in(po_filename.c_str());
  read_po_file(dictionary, po_in);
  
  std::vector<Dialog> dialogs;
  read_dialogs(localisation.c_str(), dialogs);

  Dialog dialog;
  for(std::vector<Dialog>::iterator i = dialogs.begin(); i != dialogs.end(); ++i)
    {
      if (i->lang_code == "en")
        {
          dialog = *i;
          std::cout << "Found english file" << std::endl;
          break;
        }
    }

  Dialog new_dialog;
  new_dialog.lang_code = "nw";
  new_dialog.language  = "newlanguage";
  new_dialog.entries   = dialog.entries; 

  // Generate translation table
  typedef std::map<std::string, std::vector<int> > String2Entries;
  String2Entries string2entries;

  for(unsigned int i = 0; i < dialog.entries.size(); ++i)
    {
      string2entries[&*dialog.texts.begin() + dialog.entries[i].offset].push_back(i);
    }

  // write text entries and recalculate offsets
  const char* texts = &*dialog.texts.begin();
  int         size  = dialog.texts.size();
  for(int i = 0; i < size; ++i)
    {
      std::string trans = dictionary.translate(texts+i);
      
      // generate new offsets
      String2Entries::iterator e = string2entries.find(texts+i);
      if (e != string2entries.end())
        {
          for(std::vector<int>::iterator  j = e->second.begin(); j != e->second.end(); ++j)
            new_dialog.entries[*j].offset = new_dialog.texts.size();
        }
      else
        {
          std::cerr << "Error entry not found!" << std::endl;
        }

      // copy text entry to buffer
      for(unsigned int j = 0; j < trans.length(); ++j)
        {
          new_dialog.texts.push_back(trans[j]);
        }
      new_dialog.texts.push_back('\0');
      
      // advance
      i += strlen(texts+i);
    }

  std::cout << "Writing to " << outfile << std::endl;
  new_dialog.write(outfile, 2);
}

int main(int argc, char** argv)
{
  if (argc == 1)
    {
      std::cout << "Dreamfall Dialog Extractor V0.1.1\n"
                << "=================================\n"
                << "\n"
                << "This program allows you to extract dialogs from Dreamfall.\n"
                << "To do so, first extract init.pak with the dreamfall-extractor.exe\n"
                << "and then extract the file init/dat/0212.dat with the dialog extractor\n"
                << "by drag&dropping it onto the dreamfall-dialog.exe. If you used a .pak\n"
                << "extractor that handles names, you find the dialog in the file:\n"
                << "data/generated/config/universe/localization.dat"
                << "\n"
                << "You should end up with a dreamfall-${LANG}.po in your current directory.\n"
                << "\n"
                << "For questions and comments mail Ingo Ruhnke <grumbel@gmx.de>\n"
                << "or join IRC, server irc.rizon.net, channel #Ragnar\n"
                << "\n"
                << "Full source code is available on request.\n"
                << std::endl;

      std::cout << "\nPress Enter to continue" << std::endl;
      getchar();
    }
  else if (strcmp(argv[1], "--downversion") == 0)
    {
      if (argc == 4)
        {
          std::vector<Dialog> dialogs;
          read_dialogs(argv[2], dialogs);

          for(int i = 0; i < int(dialogs.size()); ++i)
            {
              std::cout << "Language: " << dialogs[i].lang_code << " " << dialogs[i].language << std::endl;
            }

          dialogs[1].write(argv[3], 0);
         }
      else
        {
          std::cout << "Usage: ./dialog --downversion INFILE OUTFILE" << std::endl;
        }
    }
  else if (strcmp(argv[1], "--writepo") == 0)
    {
      
    }
  else if (strcmp(argv[1], "--generate") == 0)
    {
      if (argc == 4)
        {
          std::cout << "Generator mode" << std::endl;
          generate(argv[2], argv[3], "localization-new.dat");
        }
      else
        {
          std::cout << "Usage: ./dialog --generate localization.dat dreamfall-$LANG.po" << std::endl;
        }
    }
  else
    {
      for(int i = 1; i < argc; ++i)
        {
          extract(argv[i]);
        }
    }
}

/* EOF */
