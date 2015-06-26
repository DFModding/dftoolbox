/*  $Id: tljpak_manager.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <iostream>
#include <fstream>
#include "system.hpp"
#include "tljpak.hpp"
#include "location.hpp"
#include "progress_logger.hpp"
#include "data_manager.hpp"

DataManager data_manager;

DataManager::DataManager()
{
}

DataManager::~DataManager()
{
}

void
DataManager::init(ProgressLogger& logger)
{

}

void
DataManager::add_directory(const std::string& pakdirectory, ProgressLogger& logger)
{
  Directory directory = open_directory(pakdirectory, ".pak");

  logger.set_task_size(directory.size());

  char lang_code[3];
  int num;
  for(std::vector<DirectoryEntry>::iterator i = directory.begin();
      i != directory.end(); ++i)
    {
      if (i->name != "resource.pak")
        {
          logger.println("Reading: " + i->fullname);
           logger.sync();

           TLJPak* pak = new TLJPak(i->fullname);
           paks[i->name] = pak;

           if (sscanf(tolowercase(i->name).c_str(), "vox_%2s_%2d.pak", lang_code, &num) == 2)
             {
               std::cout << "Found Language Pak: " << i->name << " " << lang_code << " " << num << std::endl;
               paks_by_lang[lang_code].push_back(pak);
             }
	 }

      logger.increment_status();
      logger.sync();
    }

  logger.set_done();
}

TLJPak*
DataManager::get(const std::string& name)
{
  Paks::iterator i = paks.find(name);
  if (i != paks.end())
    return i->second;
  else
    return 0;
}

bool
DataManager::extract(const std::string& location, const std::string& outfile)
{
  for(Paks::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      int at = i->second->lookup(location);
      if (at != -1)
        {
          i->second->extract(at, outfile);
          return true;
        }
    }
  return false;
}

bool
DataManager::extract_by_language(const std::string& lang_code,
                                   const std::string& location, const std::string& outfile)
{
  //std::cout << "Langcode: " << lang_code << std::endl;
  PaksByLang::iterator i = paks_by_lang.find(lang_code);
  if (i != paks_by_lang.end())
    {
      for(std::vector<TLJPak*>::iterator j = i->second.begin(); j != i->second.end(); ++j)
        {
          int at = (*j)->lookup(location);
          if (at != -1)
            {
              (*j)->extract(at, outfile);
              return true;
            }
        }
      // Fallback;
      return extract(location, outfile);
    }
  else
    { // Fallback
      return extract(location, outfile);
    }
}

bool
DataManager::extract(const PakFileEntry& entry, const std::string& outfile)
{
  if (entry.pakfile.empty())
    {
      for(Paks::iterator i = paks.begin(); i != paks.end(); ++i)
        {
          int at = i->second->lookup(entry.fullname);
          if (at != -1)
            {
              i->second->extract(at, outfile);
              return true;
            }
        }
      return false;
    }
  else
    {
      Paks::iterator i = paks.find(entry.pakfile);
      if (i != paks.end())
        {
          int at = i->second->lookup(entry.fullname);
          if (at != -1)
            {
              i->second->extract(at, outfile);
              return true;
            }
          else
            {
              return false;
            }
        }
      else
        {
          return false;
        }
    }
}

FileType
DataManager::get_type(const std::string& pathname)
{
  for(Paks::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      FileType type = i->second->get_type(pathname);

      if (type != FILETYPE_INVALID)
        {
          return type;
        }
    }
  return FILETYPE_INVALID;
}

void
DataManager::add_filelist(const std::string& filename)
{
  if (!file_readlines(get_exe_path() + "df-directory.txt", filelist))
    {
      std::cout << "Ignoring: " << get_exe_path() << "df-directory.txt" << std::endl;
    }
}

void
DataManager::scan_paks(ProgressLogger& logger)
{
  create_dir(get_exe_path() + "/filetables");

  // Generate filetables for all paks
  logger.set_task_size(paks.size());
  logger.set_task_status(0);
  logger.sync();

  for(Paks::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      std::string pathpart, filepart;
      splitpath(i->second->filename, pathpart, filepart);
      std::string filetablecache = get_exe_path() + "filetables/" + filepart;

      std::ifstream in(filetablecache.c_str());

      if (in)
        {
          logger.println("reading cache " + i->second->filename);
          logger.sync();
          (*i).second->read_filetable(in);
        }
      else
        {
          logger.println("scanning " + i->second->filename);
          logger.sync();

          (*i).second->scan(filelist, logger.start_subtask());

          std::ofstream out(filetablecache.c_str());
          if (!out)
            {
              std::cout << "Couldn't write to filetable cache: " << filepart << std::endl;
            }
          else
            {
              (*i).second->dump_filetable(out);
              out.close();
            }
        }

      logger.increment_status();
      logger.sync();
    }

  logger.set_done();
  logger.sync();
}

DataManager::Files
DataManager::get_files()
{
  Files files;

  for(Paks::const_iterator i = paks.begin(); i != paks.end(); ++i)
    {
      for(unsigned int j = 0; j < i->second->files.size(); ++j)
        {
          PakFileEntry* entry = new PakFileEntry();

          splitpath(i->second->files[j].pathname, entry->fullname, entry->filename);
          //std::cout << i->second->files[j].pathname << std::endl;

          entry->fullname += "/" + entry->filename;

          entry->guesses  = i->second->files[j].guesses;
          entry->filesize = i->second->files[j].filesize;
          entry->type     = i->second->files[j].filetype;

          files[entry->fullname] = entry;
        }
    }

  return files;
}

DataManager::Files
DataManager::get_files(const std::string& pak)
{
  Paks::iterator i = paks.find(pak);

  if (i != paks.end())
    {
      Files files;

      for(unsigned int j = 0; j < i->second->files.size(); ++j)
        {
          PakFileEntry* entry = new PakFileEntry();

          splitpath(i->second->files[j].pathname, entry->fullname, entry->filename);

          entry->fullname += "/" + entry->filename;

          entry->guesses  = i->second->files[j].guesses;
          entry->filesize = i->second->files[j].filesize;
          entry->type     = i->second->files[j].filetype;
          entry->pakfile  = pak;

          files[entry->fullname] = entry;
        }
      return files;
    }
  else
    {
      return Files();
    }
}

std::vector<std::string>
DataManager::get_paks() const
{
  std::vector<std::string> lst;
  for(Paks::const_iterator i = paks.begin(); i != paks.end(); ++i)
    lst.push_back(i->first);
  return lst;
}

int
DataManager::get_num_dialogs(const std::string& lang) const
{
  return dialogs.size();
}

const Dialog&
DataManager::get_dialog(const std::string& lang) const
{
  assert(0);
}

const Dialog&
DataManager::get_dialog(int i) const
{
  return dialogs[i];
}

void
DataManager::scan_for_mp3s(const std::string& filename, ProgressLogger& logger)
{
  //std::cout << "Reading locations: " << std::endl;

  logger.set_task_size(2);
  logger.set_task_status(0);
  logger.sync();

  Location location(filename);
  //std::cout << "Speeches: " << filename << " " << location.get_speechs().size() << std::endl;

  logger.set_task_size(location.get_speechs().size()*2);
  logger.set_task_status(location.get_speechs().size());
  logger.sync();

  for(std::map<unsigned int, Speech>::const_iterator i = location.get_speechs().begin();
      i != location.get_speechs().end(); ++i)
    {
      mp3_to_speech[i->second.wavefile] = i->second;
      id_to_speech[i->second.text]      = i->second;

      logger.increment_status();
    }

  logger.set_done();
  logger.sync();
}

void
DataManager::scan_for_all_mp3s(ProgressLogger& logger)
{
  //std::cout << "Reading dialogs: " << std::endl;
  logger.println("Scanning for dialogs...");
  logger.sync();

  dialogs.clear();
  data_manager.extract("data/generated/config/universe/localization.dat", get_exe_path() + "/tmp.dialog");
  read_dialogs(get_exe_path() + "/tmp.dialog", dialogs);

  DreamfallFileEntry* dfentry = 0; // Little slow
  for(std::vector<DreamfallFileEntry*>::iterator i = filetable.begin(); i != filetable.end(); ++i)
    {
      if ((*i)->is_dir() && (*i)->get_dir().fullname == "data/generated/locations")
        {
          dfentry = (*i);
          break;
        }
    }

  if (!dfentry)
    {
      std::cout << "Error: Couldn't find locations" << std::endl;
    }
  else
    {
      logger.set_task_size(dfentry->get_dir().children.size());
      logger.set_task_status(0);
      logger.sync();

      for(std::vector<DreamfallFileEntry*>::iterator i = dfentry->get_dir().children.begin();
          i != dfentry->get_dir().children.end(); ++i)
        {
          std::cout << (*i)->get_url() << std::endl;
          if ((*i)->is_pak())
            {
              logger.println("    scanning " + (*i)->get_url());
              logger.sync();

              data_manager.extract((*i)->get_pak(), get_exe_path() + "/tmp.shark3d");
              scan_for_mp3s(get_exe_path() + "/tmp.shark3d", logger.start_subtask());
            }

          logger.increment_status();
          logger.sync();
        }


      logger.println("Scanning for dialogs... done");
      logger.println("");
      logger.println("### Done ###");
      logger.set_done();
      logger.sync();
    }
}

Speech
DataManager::get_speech_by_mp3(const std::string& str) const
{
  std::map<std::string, Speech>::const_iterator it = mp3_to_speech.find(str);

  if (it != mp3_to_speech.end())
    {
      return it->second;
    }
  else
    {
      throw std::runtime_error("DataManager::get_speech_by_mp3()");
    }
}

Speech
DataManager::get_speech_by_id(unsigned int id) const
{
  std::map<unsigned int, Speech>::const_iterator it = id_to_speech.find(id);

  if (it != id_to_speech.end())
    {
      return it->second;
    }
  else
    {
      throw std::runtime_error("DataManager::get_speech_by_id()");
    }
}

/* EOF */
