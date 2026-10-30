/*  $Id: directory_view.cpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "directory_view.hpp"

#include <algorithm>
#include <iostream>
#include <stdint.h>

#include <QAction>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QListView>
#include <QMainWindow>
#include <QMetaObject>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QToolBar>
#include <QTreeView>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "icons.hpp"
#include "dreamfall_file_entry.hpp"
#include "dftoolbox.hpp"
#include "ui_util.hpp"

namespace {

enum {
  LargeIconRole = Qt::UserRole + 1,
  SmallIconRole,
  TreeItemRole,   // the QTreeWidgetItem* of the main tree
  SizeRole
};

/** A row of the directory list, sorts directories before files, then
    by name (the same order the tree uses). */
class EntryItem : public QStandardItem
{
public:
  using QStandardItem::QStandardItem;

  bool operator<(const QStandardItem& other) const override
  {
    if (column() == 0)
    {
      const DreamfallFileEntry* lhs = entry_of(
        static_cast<QTreeWidgetItem*>(data(TreeItemRole).value<void*>()));
      const DreamfallFileEntry* rhs = entry_of(
        static_cast<QTreeWidgetItem*>(other.data(TreeItemRole).value<void*>()));

      if (lhs->get_type() == rhs->get_type())
        return text() < other.text();
      else
        return lhs->get_type() < rhs->get_type();
    }
    else if (column() == 2)
    {
      return data(SizeRole).toInt() < other.data(SizeRole).toInt();
    }
    else
    {
      return text() < other.text();
    }
  }
};

QStandardItem* make_item(const QString& text)
{
  QStandardItem* item = new EntryItem(text);
  item->setEditable(false);
  return item;
}

} // namespace

DirectoryView::DirectoryView(QWidget* parent, QMainWindow* mainwindow, DFToolBoxWindow* dftoolbox_)
  : QWidget(parent),
    dftoolbox(dftoolbox_),
    mode(DETAILED)
{
  toolbar = new QToolBar;
  mainwindow->addToolBar(toolbar);

  // Note that the icons and modes are paired like this in the old
  // version as well
  connect(add_tool_action(toolbar, Icon::icons_list, "Show Icons"),
          &QAction::triggered, this, [this]{ set_mode(BIG_ICONS); });
  connect(add_tool_action(toolbar, Icon::icons_large, "Show Thumbnail View"),
          &QAction::triggered, this, [this]{ set_mode(MINI_ICONS); });
  connect(add_tool_action(toolbar, Icon::icons_detail, "Show Detail List"),
          &QAction::triggered, this, [this]{ set_mode(DETAILED); });

  model = new QStandardItemModel(0, 4, this);
  model->setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "Size" << "Pak");
  selection = new QItemSelectionModel(model, this);

  detailview = new QTreeView;
  detailview->setModel(model);
  detailview->setSelectionModel(selection);
  detailview->setRootIsDecorated(false);
  detailview->setItemsExpandable(false);
  detailview->setUniformRowHeights(true);
  detailview->setAllColumnsShowFocus(true);
  detailview->setSelectionMode(QAbstractItemView::ExtendedSelection);
  detailview->setSelectionBehavior(QAbstractItemView::SelectRows);
  detailview->setEditTriggers(QAbstractItemView::NoEditTriggers);
  // the default indicator is descending
  detailview->header()->setSortIndicator(0, Qt::AscendingOrder);
  detailview->setSortingEnabled(true);
  detailview->header()->setStretchLastSection(true);
  detailview->setColumnWidth(0, 300);
  detailview->setColumnWidth(1, 100);
  detailview->setColumnWidth(2, 60);
  detailview->setColumnWidth(3, 150);

  listview = new QListView;
  listview->setModel(model);
  listview->setSelectionModel(selection);
  listview->setModelColumn(0);
  listview->setSelectionMode(QAbstractItemView::ExtendedSelection);
  listview->setEditTriggers(QAbstractItemView::NoEditTriggers);

  stack = new QStackedWidget;
  stack->addWidget(detailview);
  stack->addWidget(listview);

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->addWidget(stack);

  // Activating an entry changes the directory and with that clears
  // the model, so don't do it from within the view's own signal
  auto activate = [this](const QModelIndex& index){
    QMetaObject::invokeMethod(this, [this, index = QPersistentModelIndex(index)]{
      if (index.isValid())
        on_icon_double_click(index);
    }, Qt::QueuedConnection);
  };
  // activated is emitted on double-click and on Enter
  connect(detailview, &QAbstractItemView::activated, this, activate);
  connect(listview, &QAbstractItemView::activated, this, activate);

  set_mode(DETAILED);
}

QTreeWidgetItem*
DirectoryView::tree_item_of(const QModelIndex& index) const
{
  return static_cast<QTreeWidgetItem*>(
    model->item(index.row(), 0)->data(TreeItemRole).value<void*>());
}

void
DirectoryView::show_toolbar(bool t)
{
  toolbar->setVisible(t);
}

void
DirectoryView::change_directory(QTreeWidgetItem* item)
{
  model->removeRows(0, model->rowCount());

  for(int n = 0; n < item->childCount(); ++n)
    {
      QTreeWidgetItem* it = item->child(n);
      DreamfallFileEntry* entry = entry_of(it);

      QIcon big;
      QIcon small;
      int     size;
      const char*   typestr = "DIRECTORY";
      const char* pak = "";

      switch(entry->get_type())
        {
        case DreamfallFileEntry::DIRECTORY_FILE_ENTRY:
          big   = Icon::large_folder;
          small = Icon::folder_closed;
          size  = it->childCount(); // FIXME: this is a number of files, not a size
          typestr = "Directory";
          break;

        case DreamfallFileEntry::PAK_FILE_ENTRY:
          big   = (entry->get_pak().type == FILETYPE_SHARK3D)?Icon::large_shark:Icon::large_file;
          small = Icon::unknown_document;
          size  = entry->get_pak().filesize;
          typestr = filetype2string(entry->get_pak().type);
          pak = entry->get_pak().pakfile.c_str();
          break;

        default:
          big   = Icon::large_file;
          small = Icon::unknown_document;
          size  = 0; //entry->filesize;
          break;
        }

      QStandardItem* name_item = make_item(QString::fromStdString(entry->get_label()));
      name_item->setData(QVariant::fromValue(big), LargeIconRole);
      name_item->setData(QVariant::fromValue(small), SmallIconRole);
      name_item->setData(QVariant::fromValue(static_cast<void*>(it)), TreeItemRole);
      name_item->setData(QVariant::fromValue(mode == BIG_ICONS ? big : small), Qt::DecorationRole);

      QStandardItem* type_item = make_item(typestr);
      type_item->setData(QVariant::fromValue(static_cast<void*>(it)), TreeItemRole);

      QStandardItem* size_item = make_item(QString::asprintf("%dKB", size/1024));
      size_item->setData(size, SizeRole);

      QStandardItem* pak_item = make_item(QString::fromStdString(pak));

      model->appendRow(QList<QStandardItem*>() << name_item << type_item << size_item << pak_item);
    }

  model->sort(detailview->header()->sortIndicatorSection(),
              detailview->header()->sortIndicatorOrder());
}

void
DirectoryView::set_mode(Mode mode_)
{
  mode = mode_;

  // The icon list and the details share the model, only the icon differs
  for(int row = 0; row < model->rowCount(); ++row)
    {
      QStandardItem* item = model->item(row, 0);
      item->setData(item->data(mode == BIG_ICONS ? LargeIconRole : SmallIconRole),
                    Qt::DecorationRole);
    }

  switch(mode)
    {
    case BIG_ICONS:
      listview->setViewMode(QListView::IconMode);
      listview->setIconSize(QSize(48, 52));
      listview->setGridSize(QSize(120, 90));
      listview->setMovement(QListView::Static);
      listview->setFlow(QListView::LeftToRight);
      listview->setWrapping(true);
      listview->setResizeMode(QListView::Adjust);
      listview->setWordWrap(true);
      stack->setCurrentWidget(listview);
      break;

    case MINI_ICONS:
      listview->setViewMode(QListView::ListMode);
      listview->setIconSize(QSize(16, 16));
      listview->setGridSize(QSize());
      listview->setFlow(QListView::TopToBottom);
      listview->setWrapping(true);
      listview->setResizeMode(QListView::Adjust);
      listview->setWordWrap(false);
      stack->setCurrentWidget(listview);
      break;

    case DETAILED:
      stack->setCurrentWidget(detailview);
      break;
    }
}

void
DirectoryView::on_icon_double_click(const QModelIndex& index)
{
  dftoolbox->display(tree_item_of(index));
}

void
DirectoryView::get_selection(std::vector<DreamfallFileEntry*>& lst)
{
  // both views share the selection, but only the list view selects
  // just the first column
  std::vector<int> rows;
  const QModelIndexList indexes = selection->selectedIndexes();
  for(const QModelIndex& index : indexes)
    {
      if (index.column() == 0)
        rows.push_back(index.row());
    }
  std::sort(rows.begin(), rows.end());

  for(int row : rows)
    {
      lst.push_back(entry_of(tree_item_of(model->index(row, 0))));
    }
}

/* EOF */
