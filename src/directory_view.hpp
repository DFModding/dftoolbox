/*  $Id: directory_view.hpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DIRECTORY_VIEW_HPP
#define HEADER_DIRECTORY_VIEW_HPP

#include <vector>

#include <QWidget>

class DFToolBoxWindow;
class DreamfallFileEntry;
class QItemSelectionModel;
class QListView;
class QMainWindow;
class QStackedWidget;
class QStandardItemModel;
class QToolBar;
class QTreeView;
class QTreeWidgetItem;

/** */
class DirectoryView : public QWidget
{
public:
  enum Mode { BIG_ICONS, MINI_ICONS, DETAILED };

private:
  DFToolBoxWindow*     dftoolbox;
  QToolBar*            toolbar;
  QStandardItemModel*  model;
  QItemSelectionModel* selection;
  QStackedWidget*      stack;
  QTreeView*           detailview;
  QListView*           listview;
  Mode                 mode;

  QTreeWidgetItem* tree_item_of(const QModelIndex& index) const;

public:
  DirectoryView(QWidget* parent, QMainWindow* mainwindow, DFToolBoxWindow* dftoolbox);

  void change_directory(QTreeWidgetItem* item);

  void set_mode(Mode mode_);
  void on_icon_double_click(const QModelIndex& index);

  void show_toolbar(bool t);
  void get_selection(std::vector<DreamfallFileEntry*>& lst);
};

#endif

/* EOF */
