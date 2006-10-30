/*  $Id$
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2005 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_GUI_PROGRESS_LOGGER_HPP
#define HEADER_GUI_PROGRESS_LOGGER_HPP

#include "fx.h"
#include <iostream>
#include "progress_logger.hpp"

class GUIChildProgressLogger : public ProgressLogger
{
private:
  ProgressLogger* parent;
  int task_size;
  int task_status;
  bool done;
  GUIChildProgressLogger* subtask;

public:
  mutable FXMutex mutex;

  GUIChildProgressLogger(ProgressLogger* parent_)
    : parent(parent_),
      task_size(1), 
      task_status(0),
      done(false),
      subtask(0)
  {}

  virtual ~GUIChildProgressLogger()
  {
    delete subtask;
  }

  void println(const std::string& str) 
  { FXMutexLock lock(mutex);

    parent->println("    " + str);
  }

  void set_task_size(int s) 
  { FXMutexLock lock(mutex);
    
    // can't allow task size smaller 1, else crash due to div by zero 
    task_size = std::max(1, s);
  }

  void set_task_status(int s) 
  { FXMutexLock lock(mutex);
    
    task_status = s;
  }
  
  ProgressLogger& start_subtask() 
  { FXMutexLock lock(mutex);

    delete subtask;
    subtask = new GUIChildProgressLogger(this);
    return *subtask;
  }

  int get_task_size()  const
  { FXMutexLock lock(mutex);

    return task_size;
  }

  int get_task_status()  const
  { FXMutexLock lock(mutex);  

    return task_status;
  }

  ProgressLogger* get_subtask() const
  { FXMutexLock lock(mutex);
    
    return subtask;
  }

  void set_done() 
  { FXMutexLock lock(mutex);
    done = true;
  }

  bool is_done() const 
  { FXMutexLock lock(mutex);
    return done;
  }

  void sync() {
    parent->sync();
  }
};

/** */
class GUIProgressLogger : public ProgressLogger
{
private:
  FXGUISignal signal;
  int task_size;
  int task_status;
  std::ostringstream log;
  bool done;

  GUIChildProgressLogger* subtask;
public:
  mutable FXMutex mutex;

  GUIProgressLogger(FXApp* app, FXObject* target, FXSelector sel)
    : signal(app, target, sel),
      task_size(1),
      task_status(0),
      done(false),
      subtask(0)
  {
  }

  virtual ~GUIProgressLogger() 
  {
    delete subtask;
  }

  void println(const std::string& str) 
  { FXMutexLock lock(mutex);
    
    log << str << std::endl;
  }

  void set_task_size(int s) 
  { FXMutexLock lock(mutex);

    task_size = s;
  }

  void set_task_status(int s) 
  { FXMutexLock lock(mutex);
    
    task_status = s;
  }

  ProgressLogger& start_subtask() 
  { FXMutexLock lock(mutex);

    delete subtask; // FIXME:
    subtask = new GUIChildProgressLogger(this);
    return *subtask;
  }

  int get_task_size()  const
  { FXMutexLock lock(mutex);  

    return task_size;
  }

  int get_task_status()  const
  { FXMutexLock lock(mutex);  

    return task_status;
  }

  ProgressLogger* get_subtask() const
  { FXMutexLock lock(mutex);
    
    return subtask;
  }

  std::string pop_log()
  {
    const std::string ret = log.str();
    log.str(""); // clear
    return ret;
  }

  void set_done() 
  { FXMutexLock lock(mutex);
    done = true;
  }

  bool is_done() const 
  { FXMutexLock lock(mutex);
    return done;
  }

  void sync()
  {
    signal.setData(this);
    signal.signal();
  }
};

#endif

/* EOF */
