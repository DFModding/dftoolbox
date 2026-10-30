/*  $Id: progress_logger.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "progress_logger.hpp"

int
ProgressLogger::get_collected_task_status() const
{
  if (get_subtask())
    {
      return std::min(1000*get_task_size(),
                      1000*get_task_status()
                      + 1000*get_subtask()->get_task_status()/get_subtask()->get_task_size());
    }
  else
    {
      return 1000*get_task_status();
    }
}

int
ProgressLogger::get_collected_task_size() const
{
  return 1000*get_task_size();
}

int
ProgressLogger::get_depth() const
{
  int depth = 1;
  for(ProgressLogger* i = get_subtask(); i != NULL; i = i->get_subtask())
    depth += 1;
  return depth;
}

/* EOF */
