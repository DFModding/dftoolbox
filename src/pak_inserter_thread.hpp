/*  $Id: pak_inserter_thread.hpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_PAK_INSERTER_THREAD_HPP
#define HEADER_PAK_INSERTER_THREAD_HPP

class PakInsertThread : public FXThread
{
private:
  FXApp* app;
  ProgressDialog* progress_dialog;

  PakList paks;
  std::vector<InsertRequest> inserts;

  int total_progress;
  int current_progress;
  int total_size;
  int current_size;

  std::ostringstream log;

public:
  PakInsertThread(FXApp* app_, ProgressDialog* progress_dialog_,
                  const std::vector<InsertRequest>& inserts_)
    : app(app_),
      progress_dialog(progress_dialog_),
      inserts(inserts_),
      total_progress(0),
      current_progress(0),
      total_size(1),
      current_size(1)
  {
  }

  virtual FXint run()
  {
    try {
    group_insert_requests(inserts, paks);

    check_errors(inserts);

    do_backup(paks);
    install_paks(paks);

    log << "\n### Installation of mods finshed, you can close this dialog now! ###" << std::endl;
    } catch(std::exception& err) {
      log << "\n" << err.what() << std::endl;
    }
    return 0;
  }

  std::string getLog() const {
    return log.str();
  }

  int getTotalProgress() const {
    return total_progress;
  }

  int getTotalSize() const {
    return total_size;
  }

  int getCurrentProgress() const {
    return current_progress;
  }

  int getCurrentSize() const {
    return current_size;
  }

  void update() {
    FXGUISignal sig(app, progress_dialog, ProgressDialog::ID_THREAD_UPDATE);
    sig.signal();
  }

  void group_insert_requests(std::vector<InsertRequest>& inserts, PakList& paks)
  {
    // Group InsertRequests according to .pak
    log << "Pak Directory: " << dreamfall_path << "bin/res/" << std::endl;
    log << "Searching .pak files for filenames..." << std::endl;

    Directory pak_directory = open_directory(dreamfall_path + "bin/res/", ".pak");

    current_size     = pak_directory.size();
    current_progress = 1;

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
          log << "Ignoring: " << orig_pak << "\n  Reason: " << std::flush;
          log << err.what << std::endl;
        }

        current_progress += 1;
      }
    log << std::endl;
  }

  void do_backup(const PakList& paks)
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
            log << "  renaming " << i->first << " to " << i->first << ".orig" << std::endl;
            if (rename(i->first.c_str(), (i->first + ".orig").c_str()) != 0)
              {
                throw std::runtime_error(strerror(errno));
              }

            backup_done = true;
          }

        current_progress += 1;
      }

    if (!backup_done)
      log << "  backup already done" << std::endl;

    log << std::endl;
  }

  void check_errors(const std::vector<InsertRequest>& inserts)
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

  void install_paks(PakList& paks)
  {
    // Modify the .paks
    total_size = paks.size()*2;

    for(PakList::iterator i = paks.begin(); i != paks.end(); ++i)
      {
        TLJPak pak(i->first + ".orig");

        log << "Patching Pak: " << i->first << std::endl;

        current_size = 1;
        current_progress = 0;

        for (std::vector<InsertRequest>::iterator j = i->second.begin(); j != i->second.end(); ++j)
          {
            int loc = pak.lookup(j->location);
            const std::vector<char>& buffer = file_read(j->filename);
            assert(loc != -1);

            pak.insert(loc, buffer);

            log << " + " << j->filename << std::endl; // + " - " + buffer.size()/1024.0f + "KB\n");
          }

        current_progress = 1;
        current_progress = 0;
        total_progress += 1;

        log << "Writing Pak: " << i->first << "\n" << std::endl;

        pak.write(i->first);

        total_progress += 1;
      }
  }
};

#endif

/* EOF */
