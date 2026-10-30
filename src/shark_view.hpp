/*  $Id: shark_view.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SHARK_VIEW_HPP
#define HEADER_SHARK_VIEW_HPP

#include <functional>
#include <string>

#include <QWidget>

class QAction;
class QMainWindow;
class QPlainTextEdit;
class QToolBar;

class SharkView : public QWidget
{
private:
  QToolBar*       toolbar;
  QAction*        model_action;
  QPlainTextEdit* text;

  std::function<void ()> show_model;

  /** The text as given to set_text(), so that saving it writes
      exactly the same bytes */
  std::string raw_text;

public:
  SharkView(QWidget* parent, QMainWindow* mainwindow);

  void set_shark(const std::string& filename);
  void set_text(const std::string& text);

  /** \a callback gets called when the user asks for the 3D view */
  void set_show_model(std::function<void ()> callback);

  /** The 3D view is available for the current file */
  void show_model_action(bool t);

  void on_toggle_wrap();
  void on_save_text();
  void show_toolbar(bool t);
};

#endif

/* EOF */
