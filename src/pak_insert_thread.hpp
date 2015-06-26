/*  $Id: pak_insert_thread.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_PAK_INSERTER_THREAD_HPP
#define HEADER_PAK_INSERTER_THREAD_HPP

#include "fx.h"

#include <string>
#include <map>
#include <vector>
#include <sstream>

class ProgressLogger;
class ProgressDialog;

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

class PakInsertThread : public FXThread
{
private:
  FXApp* app;
  ProgressDialog* progress_dialog;

  typedef std::map<std::string, std::vector<InsertRequest> > PakList;

  PakList paks;
  std::vector<InsertRequest> inserts;

  ProgressLogger* mlogger;
public:
  PakInsertThread(FXApp* app_, ProgressDialog* progress_dialog_,
                  const std::vector<InsertRequest>& inserts_,
                  ProgressLogger* logger_);

  virtual ~PakInsertThread();

  FXint run();
  std::string getLog() const;

  void update();
  void group_insert_requests(std::vector<InsertRequest>& inserts, PakList& paks,
                             ProgressLogger* logger);
  void do_backup(const PakList& paks,
                 ProgressLogger* logger);
  void check_errors(const std::vector<InsertRequest>& inserts,
                    ProgressLogger* logger);
  void install_paks(PakList& paks,
                    ProgressLogger* logger);
};

#endif

/* EOF */
