/*  ui_util.hpp
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

#ifndef HEADER_UI_UTIL_HPP
#define HEADER_UI_UTIL_HPP

#include <string>

#include <QString>

class QAction;
class QIcon;
class QToolBar;
class QWidget;

/** Add an icon-only button to \a toolbar, \a tip is shown as tooltip
    and \a status (if not empty) in the status bar. */
QAction* add_tool_action(QToolBar* toolbar, const QIcon& icon,
                         const QString& tip, const QString& status = QString());

/** Ask for a filename and write \a content to it, returns false if
    the dialog got canceled. */
bool save_text_file(QWidget* parent, const std::string& content);

#endif

/* EOF */
