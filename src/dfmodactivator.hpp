/*  $Id: dfmodactivator.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DFMODACTIVATOR_HPP
#define HEADER_DFMODACTIVATOR_HPP

#include <string>
#include <vector>

#include <QFrame>
#include <QIcon>
#include <QMainWindow>

#include "pak_insert_thread.hpp"

class ProgressDialog;
class QLabel;
class QPushButton;
class QToolButton;
class QVBoxLayout;

class ModListEntry : public QFrame
{
protected:
  QLabel* longtext;
  QToolButton* arrowbutton;
  QToolButton* selectbutton;
  std::string directory;

public:
  ModListEntry(QWidget* parent,
               const QIcon& icon,
               const std::string& directory_,
               const QString& title_string,
               const QString& short_description,
               const QString& long_description);

  std::string get_directory() const;
  void on_toggle_long_desc();
  void setState(bool s);
  bool getState() const;
};

class DFModActivatorWindow : public QMainWindow
{
protected:
  QVBoxLayout* modlist;
  QPushButton* selectnone_button;
  QPushButton* selectall_button;
public:
  QPushButton* install_button;
  QPushButton* uninstall_button;
protected:
  ProgressDialog* progress_dialog;
  std::vector<ModListEntry*> entries;

public:
  DFModActivatorWindow();

  virtual ~DFModActivatorWindow();

  void on_about();
  void scan_mod_directory();
  void collect_insert_requests(std::vector<InsertRequest>& inserts);
  void set_dreamfall_path(const std::string& path);
  void on_install();
  void on_uninstall();
  void on_select_all();
  void on_select_none();
  bool get_dreamfall_path();
  void on_dreamfall_path();
};

#endif

/* EOF */
