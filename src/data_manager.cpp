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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <stdexcept>
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

bool
DataManager::read(const std::string& location, const std::string& pak, std::vector<char>& data)
{
  Paks::iterator preferred = paks.find(pak);
  if (preferred != paks.end())
    {
      int at = preferred->second->lookup(location);
      if (at != -1)
        {
          preferred->second->read(at, data);
          return true;
        }
    }

  for(Paks::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      int at = i->second->lookup(location);
      if (at != -1)
        {
          i->second->read(at, data);
          return true;
        }
    }
  return false;
}

std::vector<std::string>
DataManager::find_files(const std::string& prefix, const std::string& suffix) const
{
  std::set<std::string> result;

  // Filenames in the paks only exist after scan_paks()
  for(std::vector<std::string>::const_iterator i = filelist.begin(); i != filelist.end(); ++i)
    {
      const std::string name = tolowercase(*i);
      if (name.compare(0, prefix.size(), prefix) == 0 && has_suffix(name, suffix))
        result.insert(name);
    }

  for(Paks::const_iterator i = paks.begin(); i != paks.end(); ++i)
    {
      const std::vector<FileEntry>& files = i->second->files;
      for(std::vector<FileEntry>::const_iterator j = files.begin(); j != files.end(); ++j)
        {
          if (j->pathname.compare(0, prefix.size(), prefix) == 0 && has_suffix(j->pathname, suffix))
            result.insert(j->pathname);
        }
    }
  return std::vector<std::string>(result.begin(), result.end());
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
  std::vector<std::string> lines;
  if (!file_readlines(filename, lines))
    {
      std::cout << "Ignoring: " << filename << std::endl;
    }
  else
    {
      std::set<std::string> seen(filelist.begin(), filelist.end());
      for(std::vector<std::string>::iterator i = lines.begin(); i != lines.end(); ++i)
        {
          // strip trailing '\r' from files with DOS line endings
          if (!i->empty() && (*i)[i->size()-1] == '\r')
            i->erase(i->size()-1);

          if (!i->empty() && seen.insert(*i).second)
            {
              filelist.push_back(*i);

              std::string::size_type slash = i->rfind('/');
              if (slash != std::string::npos)
                known_directories.insert(tolowercase(i->substr(0, slash)));
            }
        }
    }
}

void
DataManager::scan_paks(ProgressLogger& logger)
{
  std::string cachedir = get_cache_path() + "filetables/";
  create_dir(cachedir);

  // Generate filetables for all paks
  logger.set_task_size(paks.size());
  logger.set_task_status(0);
  logger.sync();

  for(Paks::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      std::string pathpart, filepart;
      splitpath(i->second->filename, pathpart, filepart);
      std::string filetablecache = cachedir + filepart + ".txt";

      // invalidate the cache when the pak or the filelist changes, the
      // leading number is raised when the type detection changes
      std::ostringstream cache_key;
      int64_t size = 0, mtime = 0;
      file_stat(i->second->filename, size, mtime);
      cache_key << "2:" << size << ":" << mtime << ":" << filelist.size();

      std::ifstream in(filetablecache.c_str());

      if (in && i->second->read_filetable(in, cache_key.str()))
        {
          logger.println("reading cache " + i->second->filename);
          logger.sync();
        }
      else
        {
          in.close();

          logger.println("scanning " + i->second->filename);
          logger.sync();

          (*i).second->scan(filelist, logger.start_subtask(), &known_directories);

          std::ofstream out(filetablecache.c_str());
          if (!out)
            {
              std::cout << "Couldn't write to filetable cache: " << filetablecache << std::endl;
            }
          else
            {
              (*i).second->dump_filetable(out, cache_key.str());
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
      mp3_to_speech[tolowercase(i->second.wavefile)] = i->second;
      id_to_speech[i->second.text]      = i->second;

      logger.increment_status();
    }

  logger.set_done();
  logger.sync();
}

void
DataManager::scan_for_all_mp3s(ProgressLogger& logger)
{
  logger.println("Scanning for dialogs...");
  logger.sync();

  dialogs.clear();
  id_to_speech.clear();
  mp3_to_speech.clear();

  const std::string dialogfile = get_temp_path() + "tmp.dialog";
  try {
    if (extract("data/generated/config/universe/localization.dat", dialogfile))
      read_dialogs(dialogfile, dialogs);
    else
      logger.println("Couldn't find localization.dat");
  } catch(const std::exception& err) {
    logger.println(std::string("Couldn't read localization.dat: ") + err.what());
  }

  // Speech is stored in the location itself, e.g. locations/airship.cdr,
  // not in the files of the subdirectories
  const std::string prefix = "data/generated/locations/";
  std::vector<std::string> locations = find_files(prefix, ".cdr");
  locations.erase(std::remove_if(locations.begin(), locations.end(),
                                 [&prefix](const std::string& location) {
                                   return location.find('/', prefix.size()) != std::string::npos;
                                 }),
                  locations.end());

  logger.set_task_size(locations.size());
  logger.set_task_status(0);
  logger.sync();

  const std::string locationfile = get_temp_path() + "tmp.shark3d";
  for(std::vector<std::string>::iterator i = locations.begin(); i != locations.end(); ++i)
    {
      logger.println("    scanning " + *i);
      logger.sync();

      try {
        if (extract(*i, locationfile))
          scan_for_mp3s(locationfile, logger.start_subtask());
      } catch(const std::exception& err) {
        logger.println("Couldn't read " + *i + ": " + err.what());
      }

      logger.increment_status();
      logger.sync();
    }

  logger.println("Scanning for dialogs... done");
  logger.set_done();
  logger.sync();
}

Speech
DataManager::get_speech_by_mp3(const std::string& str) const
{
  std::map<std::string, Speech>::const_iterator it = mp3_to_speech.find(tolowercase(str));

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
