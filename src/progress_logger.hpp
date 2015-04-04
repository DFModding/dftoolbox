/*  $Id: progress_logger.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_PROGRESS_LOGGER_HPP
#define HEADER_PROGRESS_LOGGER_HPP

#include <string>

/** */
class ProgressLogger
{
public:
  virtual ~ProgressLogger() {}

  /** Print a logfile entry */
  virtual void println(const std::string& str) =0;

  virtual void set_task_size(int s)   =0;
  virtual void set_task_status(int s) =0;

  virtual void increment_status(int n = 1) {
    set_task_status(get_task_status() + n);
  }

  virtual int get_task_size()   const =0;
  virtual int get_task_status()   const =0;
  virtual int get_collected_task_status() const;
  virtual int get_collected_task_size() const;

  virtual ProgressLogger& start_subtask() =0;
  virtual ProgressLogger* get_subtask() const =0;

  virtual int get_depth() const;

  virtual void set_done() =0;
  virtual bool is_done()  const =0;

  /** Signal the parent that the logger got updated */
  virtual void sync() =0;
};

#endif

/* EOF */
