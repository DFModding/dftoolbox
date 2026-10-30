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

#include "dreamfall_file_entry.hpp"

#include "util.hpp"

namespace {

/** \a filename without its extension */
std::string strip_extension(const std::string& filename)
{
  const std::string::size_type dot = filename.rfind('.');
  return dot == std::string::npos ? filename : filename.substr(0, dot);
}

} // namespace

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

std::vector<ExportFormat>
DreamfallFileEntry::get_export_formats() const
{
  std::vector<ExportFormat> formats;

  switch(type)
    {
    case PAK_FILE_ENTRY:
      formats.push_back(EXPORT_ORIGINAL);
      if (pak.type == FILETYPE_DDS)
        formats.push_back(EXPORT_PNG);
      else if (pak.type == FILETYPE_SHARK3D || pak.type == FILETYPE_LOCALISATION)
        formats.push_back(EXPORT_TEXT);
      break;

    case SAVE_FILE_ENTRY:
      formats.push_back(EXPORT_ORIGINAL);
      formats.push_back(save.type == SaveFileEntry::IMAGE_DATA ? EXPORT_PNG : EXPORT_TEXT);
      break;

    default:
      break;
    }

  return formats;
}

std::string
DreamfallFileEntry::get_export_name(ExportFormat format) const
{
  if (type == SAVE_FILE_ENTRY)
    {
      if (format == EXPORT_ORIGINAL)
        return save.name;

      return strip_extension(save.name) + "-" + get_label() +
        (format == EXPORT_PNG ? ".png" : ".txt");
    }

  switch(format)
    {
    case EXPORT_PNG:
      return strip_extension(pak.filename) + ".png";

    case EXPORT_TEXT:
      return pak.filename + ".txt";

    case EXPORT_ORIGINAL:
      break;
    }

  if (pak.type == FILETYPE_DDS && !has_suffix(tolowercase(pak.filename), ".dds"))
    return pak.filename + ".dds";
  else if (pak.type == FILETYPE_STFU4)
    return pak.filename + ".stfu4";
  else
    return pak.filename;
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
