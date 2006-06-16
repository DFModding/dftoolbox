/*  $Id: pak_insert_thread.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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
#include <assert.h>

#include "util.hpp"
#include "system.hpp"
#include "tljpak.hpp"

#include "pak_insert_dialog.hpp"
#include "pak_insert_thread.hpp"

extern std::string dreamfall_path;
extern FXGUISignal* sig;


PakInsertThread::PakInsertThread(FXApp* app_, ProgressDialog* progress_dialog_,
                                 const std::vector<InsertRequest>& inserts_)
  : app(app_),
    progress_dialog(progress_dialog_),
    inserts(inserts_),
    total_progress(0),
    current_progress(0),
    total_size(1),
    current_size(1),
    done(false)
{
}

bool
PakInsertThread::is_done() const
{
  return done;
}

FXint
PakInsertThread::run()
{
  try {
    update();

    group_insert_requests(inserts, paks);
    check_errors(inserts);
    
    do_backup(paks);
    install_paks(paks);

    log << "\n### Installation of mods finshed, you can close this dialog now! ###" << std::endl;
  } catch(std::exception& err) {
    log << "\n" << "Error: " << err.what() << std::endl;
    std::cout << "Thread error: " << err.what() << std::endl;
  } catch (...) {
    std::cout << "Catched something?" << std::endl;
  }

  done = true;
  update();

  return 0;
}

std::string
PakInsertThread::getLog() const {
  return log.str();
}

int 
PakInsertThread::getTotalProgress() const {
  return total_progress;
}

int
PakInsertThread::getTotalSize() const {
  return total_size;
}

int
PakInsertThread::getCurrentProgress() const {
  return current_progress;
}

int
PakInsertThread::getCurrentSize() const {
  return current_size;
}

void
PakInsertThread::update() 
{
  sig->signal();
}

void
PakInsertThread::group_insert_requests(std::vector<InsertRequest>& inserts, PakList& paks) 
{
  // Group InsertRequests according to .pak
  log << "Pak Directory: " << dreamfall_path << "bin/res/" << std::endl;
  log << "Searching .pak files for filenames..." << std::endl;

  Directory pak_directory = open_directory(dreamfall_path + "bin/res/", ".pak");

  current_size     = pak_directory.size();
  current_progress = 1;
  update();

  for(Directory::iterator i = pak_directory.begin(); i != pak_directory.end(); ++i)
    {
      std::string orig_pak;
      // Use .orig instead of .pak if it exists
      if (file_exists(i->fullname + ".orig"))
        orig_pak = i->fullname + ".orig";
      else
        orig_pak = i->fullname;

      try {
        TLJPak pak(orig_pak);

        for(std::vector<InsertRequest>::iterator j = inserts.begin(); j != inserts.end(); ++j)
          {
            if (pak.lookup(j->location) != -1)
              {
                log << "  found " << j->location << " in " << i->name << std::endl;
                j->paks.push_back(i->fullname);
                paks[i->fullname].push_back(*j);
              }
          }
      } catch(std::exception& err) {
        log << "Ignoring: " << orig_pak << "\n  Reason: " << err.what() << std::endl;
      }

      current_progress += 1;
      update();
    }
  log << std::endl;
}

void
PakInsertThread::do_backup(const PakList& paks)
{
  bool backup_done = false;
  log << "Backing up pak files..." << std::endl;

  current_size = paks.size();
  current_progress = 1;

  // Do Backup of .paks
  for(PakList::const_iterator i = paks.begin(); i != paks.end(); ++i)
    {
      if (!file_exists(i->first + ".orig"))
        {
          log << "  copying " << i->first << " to " << i->first << ".orig" << std::endl;
          file_copy(i->first, i->first + ".orig");
          backup_done = true;
        }

      current_progress += 1;
      update();
    }

  if (!backup_done)
    log << "  backup already done" << std::endl;

  log << std::endl;
}

void
PakInsertThread::check_errors(const std::vector<InsertRequest>& inserts)
{
  bool found_errors = false;
  log << "Checking for errors..." << std::endl;
  for(std::vector<InsertRequest>::const_iterator i = inserts.begin(); i != inserts.end(); ++i)
    {
      if (i->paks.empty())
        {
          log << "  Warning: Couldn't find a pak containing " << i->location << "\n           ignoring "
              << i->filename << std::endl;
          found_errors = true;
        }
    }
  if (!found_errors)
    {
      log << "  no errors found" << std::endl;
    }
  log << std::endl;
}

void
PakInsertThread::install_paks(PakList& paks)
{
  // Modify the .paks
  total_size = paks.size()*2;

  for(PakList::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      TLJPak pak(i->first + ".orig");

      log << "Patching Pak: " << i->first << std::endl;
      current_size = 1;
      current_progress = 0;
      update();

      for (std::vector<InsertRequest>::iterator j = i->second.begin(); j != i->second.end(); ++j)
        {
          int loc = pak.lookup(j->location);
          const std::vector<char>& buffer = file_read(j->filename);
          if (loc != -1) 
            {
              pak.insert(loc, buffer);
              
              log << " + " << j->filename << std::endl; // + " - " + buffer.size()/1024.0f + "KB\n");
            }
          else
            {
              log << "Error: Couldn't find " << j->location << " in " << i->first << ".orig" << std::endl;
            }
        }

      current_progress = 1;
      update();
      
      current_progress = 0;
      total_progress += 1;

      log << "Writing Pak: " << i->first << "\n" << std::endl;
      update();

      pak.write(i->first);

      total_progress += 1;
      update();
    }
}

/* EOF */
