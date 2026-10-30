/*  $Id: dftoolbox.cpp,v 1.3 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "dds.hpp"
#include "data_manager.hpp"
#include "system.hpp"
#include "config.hpp"
#include "util.hpp"
#include "shark3d.hpp"
#include "dftoolbox.hpp"
#include "progress_logger.hpp"
#include "export_dialog.hpp"

#include "gui_progress_logger.hpp"
#include "location.hpp"
#include "directory_view.hpp"
#include "shark_view.hpp"
#include "sound_view.hpp"
#include "startup_report.hpp"
#include "image_view.hpp"
#include "model_view.hpp"
#include "progress_dialog.hpp"
#include "savegame.hpp"
#include "dialog_view.hpp"
#include "ui_util.hpp"

std::string dreamfall_path;

namespace {

/** Item of the main tree, it knows its DreamfallFileEntry and sorts
    directories before files and then by name. */
class TreeItem : public QTreeWidgetItem
{
public:
  DreamfallFileEntry* entry;

  TreeItem(const QString& text, const QIcon& icon, DreamfallFileEntry* entry_)
    : QTreeWidgetItem(),
      entry(entry_)
  {
    setText(0, text);
    setIcon(0, icon);
  }

  bool operator<(const QTreeWidgetItem& other) const override
  {
    const DreamfallFileEntry* lhs = entry;
    const DreamfallFileEntry* rhs = static_cast<const TreeItem&>(other).entry;

    if (lhs->get_type() == rhs->get_type())
      return text(0) < other.text(0);
    else
      return lhs->get_type() < rhs->get_type();
  }
};

/** Create a new item below \a parent (or a toplevel item that the
    caller has to add if it is null) */
TreeItem* append_item(QTreeWidgetItem* parent, const std::string& text,
                      const QIcon& icon, DreamfallFileEntry* entry)
{
  TreeItem* item = new TreeItem(QString::fromStdString(text), icon, entry);
  if (parent)
    parent->addChild(item);
  return item;
}

/** Remove the leading directory \a prefix from \a path, if it has it */
std::string strip_directory(const std::string& path, const std::string& prefix)
{
  if (path == prefix)
    return "";
  else if (path.compare(0, prefix.size() + 1, prefix + "/") == 0)
    return path.substr(prefix.size() + 1);
  else
    return path;
}

std::string join_directory(const std::string& lhs, const std::string& rhs)
{
  return rhs.empty() ? lhs : lhs + "/" + rhs;
}

/** Get the directory of \a entry in the 'by type' tree, \a directory
    is where the file is in the pak. Empty if it doesn't belong there,
    which is the case for files that are only of use to other files,
    like .lip and .ikr. */
std::string get_category(const PakFileEntry& entry, const std::string& directory,
                         const std::string& pak)
{
  const std::string& name = entry.fullname;
  const std::string art   = strip_directory(directory, "art");

  std::string category;
  if (entry.type == FILETYPE_DDS || entry.type == FILETYPE_STFU4)
    {
      category = join_directory("textures", art);
    }
  else if (entry.type == FILETYPE_WAV || entry.type == FILETYPE_MP3)
    {
      const std::string voice = strip_directory(directory, "sound/voice-over");
      const std::string music = strip_directory(directory, "sound/music");

      if (voice != directory)
        {
          // All languages use the same filenames, vox_en_00.pak -> en
          const std::string lang = tolowercase(pak).compare(0, 4, "vox_") == 0 ? pak.substr(4, 2) : "unknown";
          category = join_directory("voice/" + lang, voice);
        }
      else if (music != directory)
        {
          category = join_directory("music", music);
        }
      else
        {
          category = join_directory("sound effects", strip_directory(directory, "sound"));
        }
    }
  else if (entry.type == FILETYPE_BUNDLE || has_suffix(name, ".sir"))
    {
      category = join_directory("3d models", art);
    }
  else if (entry.type == FILETYPE_TLJBONE || has_suffix(name, ".bpr"))
    {
      category = join_directory("animations", art);
    }
  else if (entry.type == FILETYPE_LOCALISATION)
    {
      category = "localization";
    }
  else if (has_suffix(name, ".sdr") || has_suffix(name, ".sgr") || has_suffix(name, ".scr"))
    {
      category = join_directory("shaders", strip_directory(art, "shaders"));
    }
  else if (has_suffix(name, ".cdr"))
    {
      category = join_directory("scripts", strip_directory(strip_directory(directory, "data"), "scripts"));
    }

  return category.empty() ? "" : "by type/" + category;
}

} // namespace

DreamfallFileEntry* entry_of(QTreeWidgetItem* item)
{
  return static_cast<TreeItem*>(item)->entry;
}

DFToolBoxWindow::DFToolBoxWindow()
  : topmost(nullptr),
    current_tree_item(nullptr)
{
  setWindowTitle("DFToolBox");
  setWindowIcon(Icon::dftoolbox);
  resize(800, 600);

  load_bookmarks(get_config_file("bookmarks.txt"));

  // Shows the status tips of the toolbar buttons, "Ready." is what it
  // shows while there is no tip
  statusBar()->addWidget(new QLabel("Ready."));

  // Menus
  QMenu* filemenu = menuBar()->addMenu("&File");
  filemenu->addAction("&Dreamfall Path", this, [this]{ on_dreamfall_path(); });
  filemenu->addAction("&Preferences")->setEnabled(false);
  filemenu->addSeparator();
  QAction* quit = filemenu->addAction("&Quit", qApp, &QApplication::quit);
  quit->setShortcut(QKeySequence("Ctrl+Q"));

  bookmarkmenu = menuBar()->addMenu("&Bookmarks");
  bookmarkmenu->addAction("Add Bookmark...", this, [this]{ on_add_bookmark(); });
  bookmarkmenu->addAction("Edit Bookmarks");
  bookmarkmenu->addSeparator();

  for(std::vector<Bookmark*>::iterator i = bookmarks.begin(); i != bookmarks.end(); ++i)
    {
      add_bookmark_action(*i);
    }

  QMenu* helpmenu = menuBar()->addMenu("&Help");
  helpmenu->addAction("About", this, [this]{ on_about(); });

  toolbar = addToolBar("Main");

  // Toobar buttons: File manipulation
  connect(add_tool_action(toolbar, Icon::dreamfall, "Set Dreamfall Path", "Set Dreamfall Path"),
          &QAction::triggered, this, [this]{ on_dreamfall_path(); });

  add_tool_action(toolbar, Icon::import_file, "Import file", "Import file")->setEnabled(false);

  connect(add_tool_action(toolbar, Icon::export_file, "Export Selected File", "Export Selected File"),
          &QAction::triggered, this, [this]{ on_save(); });

  toolbar->addSeparator();
  parent_action = add_tool_action(toolbar, Icon::up, "Go to parent directory");
  connect(parent_action, &QAction::triggered, this, [this]{ on_goto_parent_dir(); });

  splitter = new QSplitter;
  setCentralWidget(splitter);

  {
    QWidget* group1 = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout(group1);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    QWidget* hbox_widget = new QWidget;
    QHBoxLayout* hbox = new QHBoxLayout(hbox_widget);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);

    QToolButton* collapse = new QToolButton;
    collapse->setIcon(Icon::collapse_tree);
    collapse->setToolTip("Collapse Tree");
    collapse->setStatusTip("Collapse tree");
    collapse->setAutoRaise(true);
    connect(collapse, &QToolButton::clicked, this, [this]{ on_collapse_tree(); });
    hbox->addWidget(collapse);

    QToolButton* expand = new QToolButton;
    expand->setIcon(Icon::expand_tree);
    expand->setToolTip("Expand Tree");
    expand->setStatusTip("Expand tree");
    expand->setAutoRaise(true);
    connect(expand, &QToolButton::clicked, this, [this]{ on_expand_tree(); });
    hbox->addWidget(expand);

    paklist = new QComboBox;
    hbox->addWidget(paklist, 1);

    vbox->addWidget(hbox_widget);

    tree = new QTreeWidget;
    tree->setHeaderHidden(true);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    vbox->addWidget(tree, 1);

    splitter->addWidget(group1);
  }

  // Selecting an item (by mouse or keyboard) shows it, clicking the
  // item that is already current has to do the same
  connect(tree, &QTreeWidget::currentItemChanged, this,
          [this](QTreeWidgetItem* current, QTreeWidgetItem*){
            if (current)
              on_dir_change(current);
          });
  connect(tree, &QTreeWidget::itemClicked, this,
          [this](QTreeWidgetItem* item, int){
            if (item != current_tree_item)
              on_dir_change(item);
          });
  // there is no open/closed icon pair in QTreeWidget
  connect(tree, &QTreeWidget::itemExpanded, this, [](QTreeWidgetItem* item){
    if (entry_of(item)->is_dir())
      item->setIcon(0, Icon::folder_open);
  });
  connect(tree, &QTreeWidget::itemCollapsed, this, [](QTreeWidgetItem* item){
    if (entry_of(item)->is_dir())
      item->setIcon(0, Icon::folder_closed);
  });

  switcher = new QStackedWidget;
  splitter->addWidget(switcher);
  splitter->setStretchFactor(1, 1);
  splitter->setSizes(QList<int>() << 250 << 550);

  // the order of creation has to match SWITCHER_*
  directoryview = new DirectoryView(switcher, this, this);
  switcher->addWidget(directoryview);

  error_log = new QPlainTextEdit;
  error_log->setReadOnly(true);
  switcher->addWidget(error_log);

  imageview     = new ImageView(switcher, this);
  switcher->addWidget(imageview);
  sharkview     = new SharkView(switcher, this);
  switcher->addWidget(sharkview);
  soundview     = new SoundView(switcher);
  switcher->addWidget(soundview);
  dialogview    = new DialogView(switcher, this, this);
  switcher->addWidget(dialogview);
  modelview     = new ModelView(switcher, this);
  switcher->addWidget(modelview);

  modelview->set_show_source([this]{ show_switcher(SWITCHER_SHARKVIEW); });
  sharkview->set_show_model([this]{ show_switcher(SWITCHER_MODELVIEW); });

  show_switcher(SWITCHER_ICONVIEW);

  modbar = addToolBar("Modifications");

  add_tool_action(modbar, Icon::editable, "Allow editing", "Allow editing")->setEnabled(false);
  modbar->addSeparator();
  add_tool_action(modbar, Icon::delete_file, "Revert current file back to original",
                  "Revert current file back to original")->setEnabled(false);
  add_tool_action(modbar, Icon::new_file, "New Modification", "New Modification")->setEnabled(false);

  QComboBox* modlist = new QComboBox;
  modlist->setToolTip("Current mod directory");
  modlist->addItem("All mods");
  modlist->addItem("skribb_green_grubbers");
  modlist->addItem("skribb_april_degothed");
  modlist->setEnabled(false);
  modbar->addWidget(modlist);

  addToolBarBreak(Qt::TopToolBarArea);

  toolbar2 = addToolBar("Location");
  toolbar2->addWidget(new QLabel("Location:"));
  locationbar = new QLineEdit;
  connect(locationbar, &QLineEdit::returnPressed, this, [this]{ on_locationbar(); });
  toolbar2->addWidget(locationbar);
  connect(add_tool_action(toolbar2, Icon::go, "Go", "Go"),
          &QAction::triggered, this, [this]{ on_locationbar(); });

  progress_dialog = new ProgressDialog(this, "Reading pak data, this may take a while");

  // Look for dreamfall_path
  std::ifstream in(get_config_file("dreamfall_path.txt").c_str());
  if (in)
    {
      std::getline(in, dreamfall_path);
      in.close();

      // ignore invalid an path
      if (!file_exists(dreamfall_path + "/bin/res/"))
        {
          std::cout << "Ignoring Dreamfall path: " << dreamfall_path
                    << "\ncouldn't find pak subdirectory" << std::endl;
          dreamfall_path = "";
        }
    }

  if (dreamfall_path.empty())
    {
      dreamfall_path = find_dreamfall_path();
      if (!dreamfall_path.empty())
        {
          std::cout << "Found Dreamfall in: " << dreamfall_path << std::endl;
          save_dreamfall_path();
        }
    }

  if (!dreamfall_path.empty())
    init();
}

DFToolBoxWindow::~DFToolBoxWindow()
{
  // the dialog joins the loading thread, which has to happen before
  // the things it uses go away
  delete progress_dialog;
  clear_bookmarks();
}

void
DFToolBoxWindow::add_bookmark_action(Bookmark* bookmark)
{
  QAction* action = bookmarkmenu->addAction(Icon::unknown_document,
                                            QString::fromStdString(bookmark->label));
  connect(action, &QAction::triggered, this, [this, bookmark]{ on_bookmark(bookmark); });
}

void
DFToolBoxWindow::init()
{
  load_files();

  // Init GUI stuff that needs the pak manager
  paklist->clear();
  paklist->addItem("All .pak Files (*.pak)");
  paklist->addItem("Savegames");
  paklist->addItem("Modifications");
  {
    const std::vector<std::string>& lst =  data_manager.get_paks();
    for(std::vector<std::string>::const_iterator i = lst.begin(); i != lst.end(); ++i)
      paklist->addItem(QString::fromStdString(*i));
  }

  paklist->setMaxVisibleItems(std::min(paklist->count(), 20));
  paklist->setEnabled(false);
}

QTreeWidgetItem*
DFToolBoxWindow::add_directory(const std::string& pathname, QTreeWidgetItem* rootitem)
{
  std::map<std::string, QTreeWidgetItem*>& dir = directories[rootitem];

  std::map<std::string, QTreeWidgetItem*>::iterator i = dir.find(pathname);

  if (i != dir.end())
    { // directory already there
      return i->second;
    }
  else
    {
      std::string pathpart, filepart;
      if (splitpath(pathname, pathpart, filepart))
        { //
          QTreeWidgetItem* parent = add_directory(pathpart, rootitem);

          DreamfallFileEntry* parententry = entry_of(parent);

          DreamfallFileEntry* dfentry = new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY);
          data_manager.get_filetable().push_back(dfentry);

          dfentry->get_dir().name     = filepart;
          dfentry->get_dir().fullname = pathname;
          dfentry->get_dir().parent   = parententry;

          QTreeWidgetItem* child  = append_item(parent, filepart, Icon::folder_closed, dfentry);

          parententry->get_dir().children.push_back(dfentry);

          dir[pathname] = child;

          return child;
        }
      else
        { // new top level directory
          DreamfallFileEntry* dfentry = new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY);
          data_manager.get_filetable().push_back(dfentry);

          QTreeWidgetItem* child = append_item(rootitem, pathname, Icon::folder_closed, dfentry);
          dir[pathname] = child;

          dfentry->get_dir().name     = pathname;
          dfentry->get_dir().fullname = pathname;
          dfentry->get_dir().parent   = entry_of(rootitem);

          entry_of(rootitem)->get_dir().children.push_back(dfentry);

          return child;
        }
    }
}

/** Worker that reads the pak files, runs on its own thread */
static void load_files_worker(DataManager& data_manager, ProgressLogger& logger, DFToolBoxWindow* window)
{
  logger.set_task_size(3);
  logger.set_task_status(0);
  logger.println("Reading Directory: " + dreamfall_path + "/bin/res/");
  logger.sync();

  data_manager.add_directory(dreamfall_path + "/bin/res/", logger.start_subtask());

  logger.increment_status();
  logger.println("Reading filelist");
  logger.sync();

  data_manager.add_filelist(get_data_path() + "df-directory.txt");

  logger.println("Scanning paks... done");
  logger.sync();

  data_manager.scan_paks(logger.start_subtask());

  logger.increment_status();
  logger.sync();

  data_manager.scan_for_all_mp3s(logger.start_subtask());

  logger.increment_status();
  logger.println("");
  logger.println("### Done ###");
  logger.set_done();
  logger.sync();

  std::cout << "Firing signal" << std::endl;
  // the tree has to be built in the GUI thread
  QMetaObject::invokeMethod(window, [window]{ window->build_trees(); }, Qt::QueuedConnection);
}

void
DFToolBoxWindow::load_files()
{
  progress_dialog->show();
  progress_dialog->relayout(3);
  // Generate proper filetables for the pak
  GUIProgressLogger* logger = new GUIProgressLogger(progress_dialog); // FIXME: delete me
  // FIXME: Move buildtree into the worker thread

  progress_dialog->set_thread(std::thread(load_files_worker, std::ref(data_manager),
                                          std::ref(*logger), this));
}

void
DFToolBoxWindow::build_trees()
{
  std::cout << "Build Tree" << std::endl;

  // In case we change the Dreamfall directory we clear what we have
  tree->clear();
  directories.clear();
  topmost = nullptr;
  current_tree_item = nullptr;

  // The tree gets filled while it isn't attached to the widget and is
  // sorted once at the end, that's a lot faster than doing it live
  tree->setUpdatesEnabled(false);
  tree->setSortingEnabled(false);

  {
    DreamfallFileEntry* dfentry = new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY);
    dfentry->get_dir().name     = "Dreamfall";
    dfentry->get_dir().fullname = "Dreamfall";
    data_manager.get_filetable().push_back(dfentry);

    topmost = append_item(nullptr, "Dreamfall", Icon::folder_closed, dfentry);
    current_tree_item = topmost;
  }

  std::cout << "Build 'all paks' tree" << std::endl;
  data_manager.get_filetable().push_back(new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY));
  data_manager.get_filetable().back()->get_dir().name     = "all paks";
  data_manager.get_filetable().back()->get_dir().fullname = "all paks";
  QTreeWidgetItem* all_paks = append_item(topmost, "all paks", Icon::folder_closed,
                                          data_manager.get_filetable().back());

  data_manager.get_filetable().push_back(new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY));
  data_manager.get_filetable().back()->get_dir().name     = "by pak";
  data_manager.get_filetable().back()->get_dir().fullname = "by pak";
  QTreeWidgetItem* by_pak = append_item(topmost, "by pak", Icon::folder_closed,
                                        data_manager.get_filetable().back());

  const std::vector<std::string>& pakfiles = data_manager.get_paks();

  // Get filelist and pack it into Tree
  {
    for(std::vector<std::string>::const_iterator pak = pakfiles.begin(); pak != pakfiles.end(); ++pak)
      {
        const DataManager::Files& files = data_manager.get_files(*pak);

        for(DataManager::Files::const_iterator i = files.begin(); i != files.end(); ++i)
          {
            if (!i->second->filename.empty() || i->second->filename == "/") // FIXME: Empty files shouldn't happen, but they do
              {
                std::string pathpart, filepart;
                if (splitpath(i->first, pathpart, filepart))
                  {
                    QTreeWidgetItem* item  = add_directory(pathpart, all_paks);

                    DreamfallFileEntry* dfentry = new DreamfallFileEntry(*i->second);
                    data_manager.get_filetable().push_back(dfentry);
                    append_item(item, filepart + " (" + *pak + ")",
                                Icon::unknown_document, dfentry);
                    entry_of(item)->get_dir().children.push_back(dfentry);

                    const std::string category = get_category(*i->second, pathpart, *pak);
                    if (!category.empty())
                      {
                        QTreeWidgetItem* category_item = add_directory(category, topmost);
                        append_item(category_item, filepart + " (" + *pak + ")",
                                    Icon::unknown_document, dfentry);
                        entry_of(category_item)->get_dir().children.push_back(dfentry);
                      }
                  }
              }
          }
      }
  }
  {
    for(std::vector<std::string>::const_iterator i = pakfiles.begin(); i != pakfiles.end(); ++i)
      {
        data_manager.get_filetable().push_back(new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY));
        data_manager.get_filetable().back()->get_dir().name     = *i;
        data_manager.get_filetable().back()->get_dir().fullname = *i;
        QTreeWidgetItem* this_pak = append_item(by_pak, *i, Icon::folder_closed,
                                                data_manager.get_filetable().back());

        const DataManager::Files& files = data_manager.get_files(*i);
        for(DataManager::Files::const_iterator i = files.begin(); i != files.end(); ++i)
          {
            if (!i->second->filename.empty() || i->second->filename == "/") // FIXME: Empty files shouldn't happen, but they do
              {
                std::string pathpart, filepart;
                if (splitpath(i->first, pathpart, filepart))
                  {
                    QTreeWidgetItem* item  = add_directory(pathpart, this_pak);

                    DreamfallFileEntry* dfentry = new DreamfallFileEntry(*i->second);
                    data_manager.get_filetable().push_back(dfentry);
                    append_item(item, filepart, Icon::unknown_document, dfentry);
                    entry_of(item)->get_dir().children.push_back(dfentry);
                  }
              }
          }
      }
  }

  std::cout << "Build 'by pak' tree" << std::endl;
  QTreeWidgetItem* save_item  = add_directory("savegames", topmost);
  Directory directory;
  try {
    directory = open_directory(dreamfall_path + "/saves/", ".sav");
  } catch(std::exception& err) {
    std::cout << "Warning: " << err.what() << std::endl;
  }
  for(Directory::iterator i = directory.begin(); i != directory.end(); ++i)
    {
      QTreeWidgetItem* this_save_item  = add_directory(i->name, save_item);

      SaveFileEntry save_image;
      save_image.type = SaveFileEntry::IMAGE_DATA;
      save_image.name = i->name;
      save_image.fullname = i->fullname;

      SaveFileEntry save_shark;
      save_shark.type = SaveFileEntry::SHARK3D_DATA;
      save_shark.name = i->name;
      save_shark.fullname = i->fullname;

      SaveFileEntry save_text;
      save_text.type = SaveFileEntry::TEXT_DATA;
      save_text.name = i->name;
      save_text.fullname = i->fullname;

      SaveFileEntry save_messagelog;
      save_messagelog.type = SaveFileEntry::MESSAGELOG_DATA;
      save_messagelog.name = i->name;
      save_messagelog.fullname = i->fullname;

      SaveFileEntry save_journal;
      save_journal.type = SaveFileEntry::JOURNAL_DATA;
      save_journal.name = i->name;
      save_journal.fullname = i->fullname;

      DreamfallFileEntry* dfentry1 = new DreamfallFileEntry(save_image);
      DreamfallFileEntry* dfentry2 = new DreamfallFileEntry(save_shark);
      DreamfallFileEntry* dfentry3 = new DreamfallFileEntry(save_text);
      DreamfallFileEntry* dfentry4 = new DreamfallFileEntry(save_messagelog);
      DreamfallFileEntry* dfentry5 = new DreamfallFileEntry(save_journal);

      data_manager.get_filetable().push_back(dfentry1);
      data_manager.get_filetable().push_back(dfentry2);
      data_manager.get_filetable().push_back(dfentry3);
      data_manager.get_filetable().push_back(dfentry4);
      data_manager.get_filetable().push_back(dfentry5);

      append_item(this_save_item, dfentry1->get_label(), Icon::unknown_document, dfentry1);
      append_item(this_save_item, dfentry2->get_label(), Icon::unknown_document, dfentry2);
      append_item(this_save_item, dfentry3->get_label(), Icon::unknown_document, dfentry3);
      append_item(this_save_item, dfentry4->get_label(), Icon::unknown_document, dfentry4);
      append_item(this_save_item, dfentry5->get_label(), Icon::unknown_document, dfentry5);

      entry_of(save_item)->get_dir().children.push_back(dfentry1);
      entry_of(save_item)->get_dir().children.push_back(dfentry2);
      entry_of(save_item)->get_dir().children.push_back(dfentry3);
      entry_of(save_item)->get_dir().children.push_back(dfentry4);
      entry_of(save_item)->get_dir().children.push_back(dfentry5);
    }


  std::cout << "Sorting tree" << std::endl;
  // Tree is finished, so attach it and sort it
  tree->addTopLevelItem(topmost);
  tree->sortItems(0, Qt::AscendingOrder);
  tree->expandItem(topmost);
  tree->setUpdatesEnabled(true);
}

void
DFToolBoxWindow::on_collapse_tree()
{
  if (topmost)
    {
      tree->setUpdatesEnabled(false);
      tree->collapseAll();
      tree->expandItem(topmost);
      tree->setUpdatesEnabled(true);
    }
}

void
DFToolBoxWindow::on_expand_tree()
{
  if (topmost)
    {
      tree->setUpdatesEnabled(false);
      tree->expandAll();
      tree->setUpdatesEnabled(true);
    }
}

void
DFToolBoxWindow::on_dreamfall_path()
{
  std::string new_dreamfall_path;
  QString start_dir = QString::fromStdString(dreamfall_path);
  while(new_dreamfall_path.empty())
    {
      const QString dir = QFileDialog::getExistingDirectory(this, "Select the Dreamfall directory", start_dir);
      if (dir.isEmpty())
        break;

      if (!file_exists((dir + "/bin/res/").toStdString()))
        {
          QMessageBox::critical(this, "Dreamfall Path Error",
                                "The given directory isn't the Dreamfall directory:\n" + dir);
          start_dir = dir;
        }
      else
        {
          new_dreamfall_path = dir.toStdString() + "/";
        }
    }

  if (!new_dreamfall_path.empty())
    {
      dreamfall_path = new_dreamfall_path;
      save_dreamfall_path();
      init();
    }
}

void
DFToolBoxWindow::save_dreamfall_path()
{
  try
    {
      file_write_atomic(get_config_path() + "dreamfall_path.txt", dreamfall_path + "\n");
    }
  catch(std::exception& err)
    {
      QMessageBox::warning(this, "Error", err.what());
    }
}

// Open
void
DFToolBoxWindow::on_open()
{
  const QString file = QFileDialog::getOpenFileName(this, "Open Image");
  if (!file.isEmpty()) {
    //filename=file;
    //filelist->setCurrentFile(filename);
    //mrufiles.appendFile(filename);
    //loadimage(filename);
  }
}

// Save
void
DFToolBoxWindow::on_save()
{
  std::vector<DreamfallFileEntry*> selection;
  if (switcher->currentIndex() == SWITCHER_ICONVIEW)
    directoryview->get_selection(selection);
  else if (current_tree_item)
    selection.push_back(entry_of(current_tree_item));

  if (selection.empty())
    {
      QMessageBox::critical(this, "No files selected for export",
                            "No files selected for export");
    }
  else
    {
      ExportDialog* exportdialog = new ExportDialog(this);

      exportdialog->set_file_entries(selection);
      exportdialog->show();
    }
}

void
DFToolBoxWindow::on_dir_change(QTreeWidgetItem* item)
{
  display(item);
}

void
DFToolBoxWindow::show_switcher(int i)
{
  imageview->show_toolbar(false);
  sharkview->show_toolbar(false);
  soundview->show_toolbar(false);
  dialogview->show_toolbar(false);
  directoryview->show_toolbar(false);
  modelview->show_toolbar(false);

  switch(i)
    {
    case SWITCHER_ICONVIEW:
      directoryview->show_toolbar(true);
      break;

    case SWITCHER_IMAGEVIEW:
      imageview->show_toolbar(true);
      break;

    case SWITCHER_SHARKVIEW:
      sharkview->show_toolbar(true);
      break;

    case SWITCHER_SOUNDVIEW:
      soundview->show_toolbar(true);
      break;
    case SWITCHER_DIALOGVIEW:
      dialogview->show_toolbar(true);
      break;

    case SWITCHER_MODELVIEW:
      modelview->show_toolbar(true);
      break;
    }

  switcher->setCurrentIndex(i);
}

void
DFToolBoxWindow::play_dialog(int id, const std::string& lang)
{
  Speech speech = data_manager.get_speech_by_id(id);

  // add a extract that takes a lang code
  data_manager.extract_by_language(lang, speech.wavefile, get_temp_path() + "tmp.mp3");
  soundview->set_music(get_temp_path() + "tmp.mp3", ""); //dialogs[lang].get_by_id(id));
}

unsigned int
pad_with_ones(unsigned int num)
{
  for(int i = 0; i < 32; ++i)
    {
      if ((num & (1 << (31 - i))) == 0)
        num |= (1 << (31 - i));
      else
        return num;
    }
  return num;
}

void
DFToolBoxWindow::display(QTreeWidgetItem* item)
{
  try {
    DreamfallFileEntry* entry = entry_of(item);

    if (entry->is_dir() && !entry->get_dir().parent)
      parent_action->setEnabled(false);
    else
      parent_action->setEnabled(true);

    current_tree_item = item;

    // FIXME: add info from where the file comes, ie. foo.pak:/foo/bar/baz
    //locationbar->setText(("pak://" + entry->fullname).c_str());
    locationbar->setText(QString::fromStdString(entry->get_url()));

    if (entry->get_type() == DreamfallFileEntry::DIRECTORY_FILE_ENTRY)
      {
        directoryview->change_directory(item);
        show_switcher(SWITCHER_ICONVIEW);
      }
    else if (entry->get_type() == DreamfallFileEntry::SAVE_FILE_ENTRY)
      {
        if (entry->get_save().type == SaveFileEntry::IMAGE_DATA)
          {
            imageview->set_save_image(entry->get_save().fullname);
            show_switcher(SWITCHER_IMAGEVIEW);
          }
        else if (entry->get_save().type == SaveFileEntry::JOURNAL_DATA)
          {
            std::ostringstream str;

            Savegame savegame(entry->get_save().fullname);
            const std::vector<Journal::Entry>& entries = savegame.get_journal().entries;

            for(std::vector<Journal::Entry>::const_iterator i = entries.begin();
                i != entries.end(); ++i)
              {
                int last = 0;
                for(int j = 0; j < int((*i).array.size()); ++j)
                  {
                    str << "ThisCombined: " << last+(*i).array[j] << "    "
                        << "Last: " << last << "    "
                        << "This: " << (*i).array[j] << std::endl;

                    last += (*i).array[j];
                    str << data_manager.get_dialog(0).get_by_cut_id(last)
                        << std::endl
                        << std::endl;
                  }
              }

            sharkview->set_text(str.str());
            show_switcher(SWITCHER_SHARKVIEW);
          }
        else if (entry->get_save().type == SaveFileEntry::MESSAGELOG_DATA)
          {
            Savegame savegame(entry->get_save().fullname);
            const std::vector<MessageLog::Entry>& entries = savegame.get_message_log().entries;
            std::ostringstream str;
            for(std::vector<MessageLog::Entry>::const_iterator i = entries.begin();
                i != entries.end(); ++i)
              {
                str << data_manager.get_dialog(0).get_by_id((*i).subj) << std::endl;
                str << data_manager.get_dialog(0).get_by_id((*i).body) << std::endl << std::endl;
                str << "read: " << (*i).read << std::endl;
                str << "________________________________________________________\n\n";
              }

            sharkview->set_text(str.str());
            show_switcher(SWITCHER_SHARKVIEW);
          }
        else if (entry->get_save().type == SaveFileEntry::TEXT_DATA)
          {
            Savegame savegame(entry->get_save().fullname);
            const std::vector<ConversationLog::Entry>& ints = savegame.get_conversation_log().entries;
            std::ostringstream str;
            for(int i = 0; i < int(ints.size()); ++i)
              {
                unsigned int last = 0;
                for(int j = 0; j < int(ints[i].list.size()); ++j)
                  {
                    std::cout << j << "/" << ints[i].list.size() << std::endl;

                    unsigned int this_id = last + ints[i].list[j];

                    std::string text = data_manager.get_dialog(0).get_by_id(this_id);
                    if (text.empty())
                      {
                        this_id = last + pad_with_ones(ints[i].list[j]);
                        text = data_manager.get_dialog(0).get_by_id(this_id);
                      }

                    str << ints[i].list[j] << " " << this_id << " " << last << std::endl;
                    str << "  " << text << std::endl  << std::endl;

                    last = this_id;
                  }

                str << "\n________________________________________________________\n\n";
              }
            sharkview->set_text(str.str());
            show_switcher(SWITCHER_SHARKVIEW);
          }
        else // SaveFileEntry::SHARK3D_DATA
          {
            try {
              sharkview->set_shark(entry->get_save().fullname);
              show_switcher(SWITCHER_SHARKVIEW);
            } catch (std::exception& err) {
              error_log->setPlainText(QString::fromStdString(entry->get_save().fullname) + ": " +
                                      err.what());
              show_switcher(SWITCHER_ERRORLOG);
            }
          }
      }
    else if (entry->get_type() == DreamfallFileEntry::PAK_FILE_ENTRY)
      {
        switch(entry->get_pak().type)
          {
          case FILETYPE_SHARK3D:
            {
              if (data_manager.extract(entry->get_pak(), get_temp_path() + "tmp.dat"))
                {
                  sharkview->set_shark(get_temp_path() + "tmp.dat");

                  bool has_model = false;
                  if (has_suffix(entry->get_pak().fullname, ".sir"))
                    {
                      try {
                        has_model = modelview->set_sir(entry->get_pak().fullname, entry->get_pak().pakfile);
                      } catch(std::exception& err) {
                        // .sir files aren't required to come with geometry
                        std::cout << err.what() << std::endl;
                      }
                    }
                  sharkview->show_model_action(has_model);
                  show_switcher(has_model ? SWITCHER_MODELVIEW : SWITCHER_SHARKVIEW);
                }
            }
            break;

          case FILETYPE_BUNDLE:
            {
              std::string name = entry->get_pak().filename;
              name = name.substr(0, name.rfind('.'));
              modelview->set_bundle(name, entry->get_pak().pakfile);
              show_switcher(SWITCHER_MODELVIEW);
            }
            break;

          case FILETYPE_WAV:
            if (data_manager.extract(entry->get_pak(), get_temp_path() + "tmp.wav"))
              {
                soundview->set_music(get_temp_path() + "tmp.wav", "Wave File");
                show_switcher(SWITCHER_SOUNDVIEW);
              }
            break;

          case FILETYPE_MP3:
            {
              if (data_manager.extract(entry->get_pak(), get_temp_path() + "tmp.mp3"))
                {
                  try {
                    Speech speech = data_manager.get_speech_by_mp3(entry->get_pak().fullname);

                    std::ostringstream str;
                    for(std::vector<Dialog>::const_iterator i = data_manager.get_dialogs().begin();
                        i != data_manager.get_dialogs().end(); ++i)
                      {
                        const std::string& text = i->get_by_id(speech.text);
                        if (text.empty())
                          {
                            str << "<empty>\nError: Shouldn't happen!" << std::endl;
                          }
                        else
                          {
                            // FIXME: We could also add the actor's
                            // name, but its a bit hard to get from
                            // down here
                            str << i->get_by_id(speech.actor_name)
                                << " (" << i->lang_code << " - " << i->language << "):\n"
                                << text << std::endl << std::endl;
                          }
                      }

                    soundview->set_music(get_temp_path() + "tmp.mp3", str.str());

                  } catch(...) {
                    soundview->set_music(get_temp_path() + "tmp.mp3",
                                         "<no dialog found for this sound>");
                  }

                  show_switcher(SWITCHER_SOUNDVIEW);
                }
            }
            break;

          case FILETYPE_LOCALISATION:
            if (data_manager.extract(entry->get_pak(), get_temp_path() + "tmp.dat"))
              {
                dialogview->set_dialog(get_temp_path() + "tmp.dat");
                show_switcher(SWITCHER_DIALOGVIEW);
              }
            break;

          case FILETYPE_DDS:
            {
              try {
                if (data_manager.extract(entry->get_pak(), get_temp_path() + "tmp.dat"))
                  {
                    imageview->set_image(get_temp_path() + "tmp.dat");
                    show_switcher(SWITCHER_IMAGEVIEW);
                  }
              } catch (std::exception& err) {
                error_log->setPlainText(QString("Error: ") + err.what());
                show_switcher(SWITCHER_ERRORLOG);
              }
            }
            break;

          default:
            error_log->setPlainText(QString("Error: Unhandled file type: \"%1\" for file:\n  \"%2\"")
                                    .arg("UNKNOWN") // filetype2string(entry->type)
                                    .arg(QString::fromStdString(entry->get_url())));
            show_switcher(SWITCHER_ERRORLOG);
            break;
          }
      }
  } catch(std::exception& err) {
    error_log->setPlainText(QString("Error: ") + err.what());
    show_switcher(SWITCHER_ERRORLOG);
  }
}

QTreeWidgetItem*
DFToolBoxWindow::find(QTreeWidgetItem* root, const std::string& url)
{
  for(int n = 0; n < root->childCount(); ++n)
    { // FIXME: This is kind of super slow
      QTreeWidgetItem* item = root->child(n);
      //std::cout << entry_of(item)->get_url()  << std::endl
      //          << url << std::endl  << std::endl;
      if (entry_of(item)->get_url() == url)
        {
          return item;
        }
      else if (item->childCount() > 0)
        {
          QTreeWidgetItem* ret = find(item, url);
          if (ret) return ret;
        }
    }
  return 0;
}

void
DFToolBoxWindow::on_bookmark(Bookmark* bookmark)
{
  if (!topmost)
    return;

  QTreeWidgetItem* item = find(topmost, bookmark->url);

  if (item) display(item);
}

void
DFToolBoxWindow::on_add_bookmark()
{
  if (current_tree_item)
    {
      std::string url = entry_of(current_tree_item)->get_url();
      QString label = QString::fromStdString(entry_of(current_tree_item)->get_label());

      bool ok = false;
      const QString result = QInputDialog::getText(this,
                                                   QString::fromStdString("Bookmark for " + url),
                                                   "Please enter the name for the bookmark:",
                                                   QLineEdit::Normal, label, &ok);
      if (ok)
        {
          std::cout << "Result: " << result.toStdString() << std::endl;
          bookmarks.push_back(new Bookmark(result.toStdString(), url));
          add_bookmark_action(bookmarks.back());
        }

      save_bookmarks(get_config_path() + "bookmarks.txt");
    }
}

void
DFToolBoxWindow::clear_bookmarks()
{
  for(std::vector<Bookmark*>::iterator i = bookmarks.begin(); i != bookmarks.end(); ++i)
    {
      delete *i;
    }
  bookmarks.clear();
}

void
DFToolBoxWindow::load_bookmarks(const std::string& filename)
{
  std::ifstream in(filename.c_str());
  if (!in)
    {
      std::cout << "DFToolBoxWindow: Couldn't open file " << filename << std::endl;
    }
  else
    {
      clear_bookmarks();
      Shark3D* shark3d = Shark3D::parse_text(in);
      SectionNodes* nodes = shark3d->get_sections("bookmarks");
      if (nodes)
        {
          for(std::vector<SectionNode*>::iterator i = nodes->sections.begin();
              i != nodes->sections.end(); ++i)
            {
              bookmarks.push_back(new Bookmark((*i)->get_string("label"),
                                               (*i)->get_string("url")));
            }
        }

      delete shark3d;
    }
}

void
DFToolBoxWindow::save_bookmarks(const std::string& filename)
{
  std::ostringstream out;
  out << "bookmarks" << std::endl;
  for(std::vector<Bookmark*>::iterator i = bookmarks.begin(); i != bookmarks.end(); ++i)
    {
      out << "{" << std::endl;
      out << "    label \"" << (*i)->label << "\"" << std::endl;
      out << "    url   \"" << (*i)->url   << "\"" << std::endl;
      out << "}" << std::endl;
    }
  out << "\n$" << std::endl;

  try
    {
      file_write_atomic(filename, out.str());
    }
  catch(std::exception& err)
    {
      QMessageBox::warning(this, "Error", err.what());
    }
}

void
DFToolBoxWindow::on_locationbar()
{
  /*
    DataManager::Files::iterator i = files.find(locationbar->getText().text());
    if (i != files.end())
    {
    // FIXME: add code here that opens the tree
    display(i->second);
    }
  */
}

void
DFToolBoxWindow::on_about()
{
  QMessageBox::about(this,
                     "About DFToolBox V0.0",
                     "DFToolBox is a little tool that allows you to view and modify "
                     "Dreamfalls data files.\n\n"
                     "Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>");
}

void group_entries(const std::vector<DreamfallFileEntry*>& selection,
                   std::set<DreamfallFileEntry*>& out)
{
  for(std::vector<DreamfallFileEntry*>::const_iterator i = selection.begin();
      i != selection.end();
      ++i)
    {
      if ((*i)->is_pak())
        {
          out.insert(*i);
        }
      else if ((*i)->is_dir())
        {
          group_entries((*i)->get_dir().children, out);
        }
      else
        {
          // ignoring saves for now
        }
    }
}

void
DFToolBoxWindow::on_goto_parent_dir()
{
  if (current_tree_item && current_tree_item->parent())
    display(current_tree_item->parent());
}

void
DFToolBoxWindow::export_files(const std::vector<DreamfallFileEntry*>& selection,
                              const std::string& outpath,
                              bool preserve_path,
                              bool fix_extension,
                              bool shark_as_text)
{
  std::set<DreamfallFileEntry*> entries;
  group_entries(selection, entries);

  for(std::set<DreamfallFileEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      if ((*i)->is_pak())
        {
          std::string pathpart, filepart;
          if (splitpath((*i)->get_pak().fullname, pathpart, filepart))
            {
              std::string outfile;
              if (preserve_path)
                {
                  outfile = outpath + "/" + (*i)->get_pak().fullname;
                  create_hierachy(outfile);
                }
              else
                {
                  outfile = outpath + "/" + (*i)->get_pak().filename;
                }

              if (fix_extension)
                {
                  if ((*i)->get_pak().type == FILETYPE_DDS)
                    outfile += ".dds";
                  else if ((*i)->get_pak().type == FILETYPE_SHARK3D && shark_as_text)
                    outfile += ".txt";
                }

              if ((*i)->get_pak().type == FILETYPE_SHARK3D && shark_as_text)
                {
                  data_manager.extract((*i)->get_pak(), get_temp_path() + "/tmp.shark");
                  std::ifstream in((get_temp_path() + "/tmp.shark").c_str(), std::ios::binary);
                  if (!in)
                    {
                      throw std::runtime_error("Error: Couldn't open " + (get_temp_path() + "/tmp.shark"));
                    }
                  else
                    {
                      Shark3D* shark = Shark3D::parse_binary(in);
                      in.close();

                      std::ofstream out(outfile.c_str());
                      if (!out)
                        {
                          std::cout << "export_files: Couldn't open " << outfile << std::endl;
                        }
                      else
                        {
                          shark->write_text(out);
                          out.close();
                        }
                    }

                }
              else
                {
                  data_manager.extract((*i)->get_pak(), outfile);
                }
            }
        }
      else
        {
          std::cout << "Error this shouldn't happen" << std::endl;
        }
    }
}

int main(int argc,char** argv)
{
  try {
    attach_parent_console();
    parse_args(argc, argv, "dftoolbox");

    QApplication application(argc, argv);
    application.setApplicationName("dftoolbox");
    application.setApplicationDisplayName("DFToolBox");
    application.setApplicationVersion(QString::fromStdString(get_version()));
    application.setDesktopFileName("dftoolbox");
    application.setOrganizationName("Grumbel");

    print_startup_report("dftoolbox");

    Icon::init();

    DFToolBoxWindow window;
    window.show();

    return application.exec();
  } catch(std::exception& err) {
    std::cerr << "Fatal Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
}

/* EOF */
