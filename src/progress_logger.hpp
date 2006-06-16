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

#include <iomanip>
#include <iostream>

/** */
class ProgressLogger
{
private:
  int total_size;
  int total_progress;
  int current_size;
  int current_progress;
public:
  ProgressLogger() 
    : total_size(0),
      total_progress(0),
      current_size(0),
      current_progress(0)
  {}

  virtual ~ProgressLogger() {}

  virtual void print(int i) {
    std::cout << i << std::flush;
  }

  virtual void print(const std::string& str) {
    std::cout << str << std::flush;
  }


  virtual void println(const std::string& str) {
    print(str);
    print("\n");
  }

  virtual void set_total_size(int s) { 
    total_size     = s;
    total_progress = 0;
  }
  virtual void set_total_progress(int s) {
    total_progress = s;
    std::cout << std::setw(5) << total_progress << "/" << total_size 
              << std::setw(5) << current_progress << "/" << current_size 
              << "            \r" << std::flush;
  }

  virtual void set_current_size(int s) {
    current_size     = s;
    current_progress = 0;
  }
  virtual void set_current_progress(int s) {
    current_progress = s;
    std::cout << std::setw(5) << total_progress << "/" << total_size 
              << std::setw(5) << current_progress << "/" << current_size 
              << "            \r" << std::flush;
  }

private:
  ProgressLogger (const ProgressLogger&);
  ProgressLogger& operator= (const ProgressLogger&);
};

#endif

/* EOF */
