/*  $Id: tljpak_manager.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <iostream>
#include <fstream>
#include "system.hpp"
#include "tljpak.hpp"
#include "progress_logger.hpp"
#include "tljpak_manager.hpp"

TLJPakManager::TLJPakManager()
{
}

TLJPakManager::~TLJPakManager()
{
}

void
TLJPakManager::add_directory(const std::string& pakdirectory, ProgressLogger& logger)
{
  Directory directory = open_directory(pakdirectory, ".pak");

  logger.set_task_size(directory.size());

  for(std::vector<DirectoryEntry>::iterator i = directory.begin();
      i != directory.end(); ++i)
    {
      if (i->name != "resource.pak") 
	 {
           logger.println("Reading: " + i->fullname);
           logger.sync();

           paks[i->name] = new TLJPak(i->fullname);
	 }       

      logger.increment_status();
      logger.sync();
    }

  logger.set_done();
}

TLJPak*
TLJPakManager::get(const std::string& name)
{
  Paks::iterator i = paks.find(name);
  if (i != paks.end()) 
    return i->second;
  else
    return 0;
}

bool
TLJPakManager::extract(const std::string& location, const std::string& outfile)
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
TLJPakManager::extract_by_language(const std::string& lang_code, 
                                   const std::string& location, const std::string& outfile)
{
  return extract(location, outfile);
}

bool
TLJPakManager::extract(const PakFileEntry& entry, const std::string& outfile)
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
TLJPakManager::get_type(const std::string& pathname)
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
TLJPakManager::add_filelist(const std::string& filename)
{
  if (!file_readlines(get_exe_path() + "df-directory.txt", filelist))
    {
      std::cout << "Ignoring: " << get_exe_path() << "df-directory.txt" << std::endl;
    }
}

void
TLJPakManager::scan_paks(ProgressLogger& logger)
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
          logger.println("Scanning " + i->second->filename);
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

TLJPakManager::Files
TLJPakManager::get_files() 
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

TLJPakManager::Files
TLJPakManager::get_files(const std::string& pak)
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
TLJPakManager::get_paks() const
{
  std::vector<std::string> lst;
  for(Paks::const_iterator i = paks.begin(); i != paks.end(); ++i)
    lst.push_back(i->first);
  return lst;
}

/* EOF */
