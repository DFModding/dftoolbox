/*  $Id: dreamfall_file_entry.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "dreamfall_file_entry.hpp"

std::string
SaveFileEntry::get_url() const
{
  switch (type)
    {
    case IMAGE_DATA:
      return "sav://" + name + "/image";

    case SHARK3D_DATA:
      return "sav://" + name + "/shark3d";

    case TEXT_DATA:
      return "sav://" + name + "/text";

    case MESSAGELOG_DATA:
      return "sav://" + name + "/messagelog";

    case JOURNAL_DATA:
      return "sav://" + name + "/journal";
    }
  return "<unknown>";
}


std::string
PakFileEntry::get_url() const
{
  if (!pakfile.empty())
    return "pak://" + pakfile + "/" + fullname;
  else
    return "paks://" + fullname;
}

std::string
DreamfallFileEntry::get_url() const
{
  switch(type)
    {
    case PAK_FILE_ENTRY:
      return pak.get_url();

    case SAVE_FILE_ENTRY:
      return save.get_url();

    case DIRECTORY_FILE_ENTRY:
      return "dir://" + dir.fullname;

    default:
      return "<unknown>";
    }
}

std::string
DreamfallFileEntry::get_label() const
{
  switch(type)
    {
    case PAK_FILE_ENTRY:
      return pak.filename;

    case SAVE_FILE_ENTRY:
      if (save.type == SaveFileEntry::IMAGE_DATA)
        return "image";
      else if (save.type == SaveFileEntry::SHARK3D_DATA)
        return "shark3d";
      else if (save.type == SaveFileEntry::MESSAGELOG_DATA)
        return "messagelog";
      else if (save.type == SaveFileEntry::JOURNAL_DATA)
        return "journal";
      else
        return "text";

    case DIRECTORY_FILE_ENTRY:
      return dir.name;

    default:
      return "<unknown>";
    }
}

std::string
DreamfallFileEntry::get_path() const
{
  switch(type)
    {
    case PAK_FILE_ENTRY:
      return pak.fullname;

    case SAVE_FILE_ENTRY:
      return "savegames/";

    case DIRECTORY_FILE_ENTRY:
      return dir.name;

    default:
      return "<unknown>";
    }
}

/* EOF */
