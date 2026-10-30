/*  $Id: dftoolbox.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DFTOOLBOX_HPP
#define HEADER_DFTOOLBOX_HPP

#include <map>
#include <string>
#include <vector>

#include <QMainWindow>

#include "icons.hpp"
#include "dialog.hpp"
#include "location.hpp"
#include "data_manager.hpp"

class DialogView;
class DirectoryView;
class DreamfallFileEntry;
class ImageView;
class ModelView;
class ProgressDialog;
class QAction;
class QComboBox;
class QLineEdit;
class QMenu;
class QPlainTextEdit;
class QSplitter;
class QStackedWidget;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class SharkView;
class SoundView;

struct Bookmark
{
  std::string label;
  std::string url;

  Bookmark(const std::string& label_,
           const std::string& url_)
    : label(label_), url(url_)
  {}
};

/** Get the DreamfallFileEntry that belongs to an item of the main tree */
DreamfallFileEntry* entry_of(QTreeWidgetItem* item);

class DFToolBoxWindow : public QMainWindow
{
protected:
  QMenu*       bookmarkmenu;

  QToolBar*    toolbar;
  QToolBar*    toolbar2;
  QToolBar*    modbar;

  QSplitter*   splitter;

  QComboBox*       paklist;
  QTreeWidget*     tree;
  QTreeWidgetItem* topmost;
  QTreeWidgetItem* current_tree_item;

  QLineEdit* locationbar;

  ImageView*     imageview;
  SharkView*     sharkview;
  ModelView*     modelview;
  SoundView*     soundview;
  DialogView*    dialogview;
  DirectoryView* directoryview;
  QPlainTextEdit* error_log;

  QStackedWidget*  switcher;

  QAction*       parent_action;

  ProgressDialog* progress_dialog;

  // Cache to speed up tree insertion
  typedef std::map<QTreeWidgetItem*, std::map<std::string, QTreeWidgetItem*> > Directories;
  Directories directories;

  std::vector<Bookmark*> bookmarks;

public:
  DFToolBoxWindow();

  virtual ~DFToolBoxWindow();

  /** the \a directories table is used for cashing */
  QTreeWidgetItem* add_directory(const std::string& pathname, QTreeWidgetItem* rootitem);

  void load_files();
  void build_trees();

  void display(QTreeWidgetItem* item);
  void init();
  void save_dreamfall_path();

  void export_files(const std::vector<DreamfallFileEntry*>& selection,
                    const std::string& outpath,
                    bool preserve_path, bool fix_extension, bool shark_as_text);

  void show_switcher(int i);
  void play_dialog(int i, const std::string& lang);

  void on_open();
  void on_save();
  void on_about();
  void on_bookmark(Bookmark* bookmark);
  void on_add_bookmark();
  void on_locationbar();
  void on_dir_change(QTreeWidgetItem* item);
  void on_dreamfall_path();
  void on_collapse_tree();
  void on_expand_tree();
  void on_goto_parent_dir();
  void on_scan_for_mp3();

  /** Find the tree item matching url */
  QTreeWidgetItem* find(QTreeWidgetItem* root, const std::string& url);

  void clear_bookmarks();
  void load_bookmarks(const std::string& filename);
  void save_bookmarks(const std::string& filename);

  enum {
    SWITCHER_ICONVIEW,
    SWITCHER_ERRORLOG,
    SWITCHER_IMAGEVIEW,
    SWITCHER_SHARKVIEW,
    SWITCHER_SOUNDVIEW,
    SWITCHER_DIALOGVIEW,
    SWITCHER_MODELVIEW
  };

private:
  void add_bookmark_action(Bookmark* bookmark);

  DFToolBoxWindow(const DFToolBoxWindow&) = delete;
  DFToolBoxWindow& operator=(const DFToolBoxWindow&) = delete;
};

#endif

/* EOF */
