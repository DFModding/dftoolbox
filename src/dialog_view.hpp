/*  $Id: dialog_view.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DIALOG_VIEW_HPP
#define HEADER_DIALOG_VIEW_HPP

#include <string>
#include <vector>

#include <QWidget>

#include "dialog.hpp"

class DFToolBoxWindow;
class QComboBox;
class QMainWindow;
class QPlainTextEdit;
class QToolBar;

/** */
class DialogView : public QWidget
{
private:
  DFToolBoxWindow* dftoolbox;
  QToolBar*        toolbar;
  QPlainTextEdit*  text;
  QComboBox*       langbox;
  typedef std::vector<Dialog> Dialogs;
  Dialogs dialogs;

  /** The text as last given to the text widget, so that saving it
      writes exactly the same bytes */
  std::string raw_text;

  void set_text(const std::string& str);
  void play(int dialog_id, const std::string& lang_code);

public:
  DialogView(QWidget* parent, QMainWindow* mainwindow, DFToolBoxWindow* dftoolbox);

  void set_dialog(const std::string& filename);
  void on_language_switch(int lang_id);
  bool on_hyperlink(const QPoint& pos);
  void on_save_text();
  void set_language(int id);
  void show_toolbar(bool t);

  bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif

/* EOF */
