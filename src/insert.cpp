/*  $Id: insert.cpp,v 1.3 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <assert.h>
#include <iostream>
#include <string>
#include <set>
#include "tljpak.hpp"
#include "config.hpp"
#include "system.hpp"

std::string cutprefix(const std::string& prefix, const std::string& path)
{
  return path.substr(prefix.length());
}

std::string cutdir(const std::string& pathname)
{
  std::string::size_type i = pathname.find_first_of('/');
  if (i != std::string::npos)
    {
      return pathname.substr(i+1);
    }
  else
    {
      return pathname;
    }
}

struct InsertRequest
{
  /** Filename of the file to insert */
  std::string filename;

  /** Where to insert the given file */
  std::string location;

  /** The pak files to modify (can be multiple since some pak contain
      duplicate data */
  std::vector<std::string> paks;

  InsertRequest(const std::string& filename_,
                const std::string& location_)
    : filename(filename_), location(location_)
  {}
};

int main(int argc, char** argv)
{
  try {
  Config config;

  std::string modpath = get_exe_path() + "dreamfall-mods/";

  std::vector<std::string> lst;
  find_files(modpath, lst);

  std::vector<InsertRequest> inserts;
  for(std::vector<std::string>::iterator i = lst.begin(); i != lst.end(); ++i)
    {
      std::cout << *i << "\n  -> " << cutdir(cutprefix(modpath, *i)) << std::endl;
      inserts.push_back(InsertRequest(*i, cutdir(cutprefix(modpath, *i))));
    }

  std::cout << "Pak Directory: " << config.dreamfall_path + "bin/res/" << std::endl;
  Directory pak_directory = open_directory(config.dreamfall_path + "bin/res/", ".pak");
  typedef std::map<std::string, std::vector<InsertRequest> > PakList;
  PakList paks;
  for(Directory::iterator i = pak_directory.begin(); i != pak_directory.end(); ++i)
    {
      std::string orig_pak;
      if (file_exists(i->fullname + ".orig"))
        orig_pak = i->fullname + ".orig";
      else
        orig_pak = i->fullname;

      TLJPak pak(orig_pak);

      for(std::vector<InsertRequest>::iterator j = inserts.begin(); j != inserts.end(); ++j)
        {
          if (pak.lookup(j->location) != -1)
            {
              std::cout << "Found: " << j->location << " in " << i->name << std::endl;
              j->paks.push_back(i->fullname);
              paks[i->fullname].push_back(*j);
            }
        }
    }
  std::cout << std::endl;

  for(std::vector<InsertRequest>::iterator i = inserts.begin(); i != inserts.end(); ++i)
    {
      if (i->paks.empty())
        std::cout << "Warning: Couldn't find a pak containing " << i->location << "\n         ignoring "
                  << i->filename << std::endl;
    }
  std::cout << std::endl;

  // Do Backup of .paks
  for(PakList::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      if (!file_exists(i->first + ".orig"))
        {
          std::cout << "Backing up " << i->first << std::endl;
          file_copy(i->first, i->first + ".orig");
        }
    }
  std::cout << std::endl;

  // Modify the .paks
  for(PakList::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      TLJPak pak(i->first + ".orig");
      std::cout << "Reading Pak: " << i->first << std::endl;
      for (std::vector<InsertRequest>::iterator j = i->second.begin(); j != i->second.end(); ++j)
        {
          int loc = pak.lookup(j->location);
          const std::vector<char>& buffer = file_read(j->filename);
          assert(loc != -1);

          pak.insert(loc, buffer);
          std::cout << " + " << j->filename << " - " << buffer.size()/1024.0f << "KB" << std::endl;
        }
      std::cout << "Writing Pak: " << i->first << std::endl;

      std::cout << std::endl;

      pak.write(i->first);
    }


  // Workflow:
  // - insert files into .pak
  // - write out .pak
  } catch(std::exception& err) {
    std::cout << "Error: " << err.what() << std::endl;
  }
  wait();
  return 0;
}

/* EOF */
