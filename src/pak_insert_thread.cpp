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

#include "progress_dialog.hpp"
#include "progress_logger.hpp"
#include "pak_insert_thread.hpp"

#define STR(x) static_cast<std::ostringstream&>(std::ostringstream() << x).str()

extern std::string dreamfall_path;
extern FXGUISignal* sig;

PakInsertThread::PakInsertThread(FXApp* app_, ProgressDialog* progress_dialog_,
                                 const std::vector<InsertRequest>& inserts_,
                                 ProgressLogger* logger_)
  : app(app_),
    progress_dialog(progress_dialog_),
    inserts(inserts_),
    mlogger(logger_)
{
}

PakInsertThread::~PakInsertThread()
{
}

FXint
PakInsertThread::run()
{
  try {
    update();

    mlogger->set_task_size(4);
    mlogger->set_task_status(0);

    group_insert_requests(inserts, paks, &mlogger->start_subtask());
    mlogger->increment_status();

    check_errors(inserts, &mlogger->start_subtask());
    mlogger->increment_status();

    do_backup(paks, &mlogger->start_subtask());
    mlogger->increment_status();

    install_paks(paks, &mlogger->start_subtask());
    mlogger->increment_status();

    mlogger->println("");
    mlogger->println("### Installation of mods finshed, you can close this dialog now! ###");
  } catch(std::exception& err) {
    mlogger->println("");
    mlogger->println(STR("Error: " << err.what()));
    std::cout << "Thread error: " << err.what() << std::endl;
  } catch (...) {
    std::cout << "Catched something?" << std::endl;
  }

  mlogger->set_done();
  update();

  return 0;
}

void
PakInsertThread::update()
{
  sig->setData(mlogger);
  sig->signal();
}

void
PakInsertThread::group_insert_requests(std::vector<InsertRequest>& inserts, PakList& paks,
                                       ProgressLogger* logger)
{
  // Group InsertRequests according to .pak
  logger->println("Pak Directory: " + dreamfall_path + "bin/res/");
  logger->println("Searching .pak files for filenames...");

  Directory pak_directory = open_directory(dreamfall_path + "bin/res/", ".pak");

  // FIXME: Use sub logger
  logger->set_task_size(pak_directory.size());
  logger->set_task_status(1);
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
                std::ostringstream str;
                str << "  found " << j->location << " in " << i->name;
                logger->println(str.str());

                j->paks.push_back(i->fullname);
                paks[i->fullname].push_back(*j);
              }
          }
      } catch(std::exception& err) {
        std::ostringstream str;
        str << "Ignoring: " << orig_pak << "\n  Reason: " << err.what();
        logger->println(str.str());
      }

      logger->increment_status();
      update();
    }
  logger->println("");
}

void
PakInsertThread::do_backup(const PakList& paks, ProgressLogger* logger)
{
  bool backup_done = false;
  logger->println("Backing up pak files...");

  // FIXME: Use sublogger
  logger->set_task_size(paks.size());
  logger->set_task_status(1);

  // Do Backup of .paks
  for(PakList::const_iterator i = paks.begin(); i != paks.end(); ++i)
    {
      if (!file_exists(i->first + ".orig"))
        {
          logger->println(STR("  copying " << i->first << " to " << i->first << ".orig"));
          file_copy(i->first, i->first + ".orig");
          backup_done = true;
        }

      logger->increment_status(1);
      update();
    }

  if (!backup_done)
    logger->println("  backup already done");

  logger->println("");
}

void
PakInsertThread::check_errors(const std::vector<InsertRequest>& inserts,
                              ProgressLogger* logger)
{
  bool found_errors = false;
  logger->println("Checking for errors...");
  for(std::vector<InsertRequest>::const_iterator i = inserts.begin(); i != inserts.end(); ++i)
    {
      if (i->paks.empty())
        {
          logger->println(STR("  Warning: Couldn't find a pak containing " << i->location
                              << "\n           ignoring " << i->filename));
          found_errors = true;
        }
    }
  if (!found_errors)
    {
      logger->println("  no errors found");
    }
  logger->println("");
}

void
PakInsertThread::install_paks(PakList& paks, ProgressLogger* logger)
{
  // Modify the .paks
  logger->set_task_size(paks.size()*2);

  for(PakList::iterator i = paks.begin(); i != paks.end(); ++i)
    {
      TLJPak pak(i->first + ".orig");

      logger->println(STR("Patching Pak: " << i->first));
      update();

      for (std::vector<InsertRequest>::iterator j = i->second.begin(); j != i->second.end(); ++j)
        {
          int loc = pak.lookup(j->location);
          const std::vector<char>& buffer = file_read(j->filename);
          if (loc != -1)
            {
              pak.insert(loc, buffer);

              logger->println(STR(" + " << j->filename)); // + " - " + buffer.size()/1024.0f + "KB\n");
            }
          else
            {
              logger->println(STR("Error: Couldn't find " << j->location << " in " << i->first << ".orig"));
            }
        }

      logger->increment_status();
      update();

      logger->println(STR("Writing Pak: " << i->first << "\n"));
      update();

      pak.write(i->first);
      logger->increment_status();
      update();
    }
}

/* EOF */
