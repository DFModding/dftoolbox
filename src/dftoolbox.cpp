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

#ifdef USE_SDL
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#endif

#include <map>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include "fx.h"

#include "FXExpression.h"
#include "dds.hpp"
#include "data_manager.hpp"
#include "system.hpp"
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
#include "image_view.hpp"
#include "progress_dialog.hpp"
#include "savegame.hpp"
#include "dialog_view.hpp"

std::string dreamfall_path;

FXDEFMAP(DFToolBoxWindow) DFToolBoxWindowMap[] = {
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_OPEN,           DFToolBoxWindow::onCmdOpen),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_SAVE,           DFToolBoxWindow::onCmdSave),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_ABOUT,          DFToolBoxWindow::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_EXPAND_TREE,    DFToolBoxWindow::onCmdExpandTree),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_COLLAPSE_TREE,  DFToolBoxWindow::onCmdCollapseTree),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_LOCATIONBAR,    DFToolBoxWindow::onCmdLocationbar),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_DREAMFALL_PATH, DFToolBoxWindow::onCmdDreamfallPath),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_BOOKMARK,       DFToolBoxWindow::onCmdBookmark),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_ADD_BOOKMARK,   DFToolBoxWindow::onCmdAddBookmark),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_DIRECTORYCHANG, DFToolBoxWindow::onDirChange),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_GOTO_PARENTDIR, DFToolBoxWindow::onCmdGotoParentDir),
  FXMAPFUNC(SEL_COMMAND,       DFToolBoxWindow::ID_SCAN_FOR_MP3,   DFToolBoxWindow::onCmdScanForMP3),
  FXMAPFUNC(SEL_IO_READ,       DFToolBoxWindow::ID_BUILD_TREE,     DFToolBoxWindow::onCmdBuildTree),
};

// Object implementation
FXIMPLEMENT(DFToolBoxWindow, FXMainWindow, DFToolBoxWindowMap, ARRAYNUMBER(DFToolBoxWindowMap));

DFToolBoxWindow::DFToolBoxWindow()
{
}

DFToolBoxWindow::DFToolBoxWindow(FXApp* a)
  : FXMainWindow(a,"DFToolBox", NULL, NULL, DECOR_ALL, 0, 0, 800, 600),
    topmost(0),
    image(0)
{
  Icon::init(getApp());

  setMiniIcon(Icon::dftoolbox_mini);
  setIcon(Icon::dftoolbox);

  // Icons
  big_folder_closed = new FXGIFIcon(getApp(), bigfolder);
  big_folder_closed->create();

  load_bookmarks(get_exe_path() + "/bookmarks.txt");

  menubar = new FXMenuBar(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|FRAME_RAISED);

  topdock = new FXDockSite(this,DOCKSITE_NO_WRAP|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);


  status  = new FXStatusBar(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);

  // Menus
  filemenu = new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&Dreamfall Path",  NULL, this, ID_DREAMFALL_PATH);
  (new FXMenuCommand(filemenu,"&Preferences",    NULL, this, ID_PREFERENCES))->disable();
  new FXMenuCommand(filemenu,"Scan for Dialogs", NULL, this, ID_SCAN_FOR_MP3);
  new FXMenuSeparator(filemenu);
  new FXMenuCommand(filemenu,"&Quit\tCtl-Q",NULL,getApp(),FXApp::ID_QUIT);
  new FXMenuTitle(menubar,"&File",NULL,filemenu);

  bookmarkmenu = new FXMenuPane(this);
  new FXMenuCommand(bookmarkmenu,"Add Bookmark...", NULL, this, ID_ADD_BOOKMARK);
  new FXMenuCommand(bookmarkmenu,"Edit Bookmarks",  NULL, NULL, 0);
  new FXMenuSeparator(bookmarkmenu);

  for(std::vector<Bookmark*>::iterator i = bookmarks.begin(); i != bookmarks.end(); ++i)
    {
      (new FXMenuCommand(bookmarkmenu, (*i)->label.c_str(), Icon::unknown_document,
                         this, ID_BOOKMARK))->setUserData(*i);
    }

  bookmark_title = new FXMenuTitle(menubar,"&Bookmarks",NULL, bookmarkmenu);

  helpmenu = new FXMenuPane(this);
  new FXMenuCommand(helpmenu, "About", NULL, this, ID_ABOUT);
  new FXMenuTitle(menubar,"&Help",NULL, helpmenu);

  toolbar = new FXToolBar(topdock,LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|FRAME_RAISED);

  // Toobar buttons: File manipulation
  new FXButton(toolbar, "\tSet Dreamfall Path\tSet Dreamfall Path", Icon::dreamfall,
               this, ID_DREAMFALL_PATH,
               BUTTON_TOOLBAR|FRAME_RAISED);

  (new FXButton(toolbar, "\tImport file\tImport file", Icon::import_file,
               NULL, 0, //this, ID_OPEN,
                BUTTON_TOOLBAR|FRAME_RAISED))->disable();

  new FXButton(toolbar, "\tExport Selected File\tExport Selected File", Icon::export_file,
               this, ID_SAVE, BUTTON_TOOLBAR|FRAME_RAISED);

  new FXHorizontalSeparator(toolbar, SEPARATOR_GROOVE);
  parent_button = new FXButton(toolbar, "\tGo to parent directory", Icon::up,
               this, ID_GOTO_PARENTDIR, BUTTON_TOOLBAR|FRAME_RAISED);

  splitter = new FXSplitter(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  group1 = new FXVerticalFrame(splitter,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
                               0, 0, 0, 0,
                               0, 0, 0, 0);
  //group2 = new FXVerticalFrame(splitter,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
  //                           0, 0, 0, 0,
  //                           0, 0, 0, 0);

  {
    FXHorizontalFrame* hbox = new FXHorizontalFrame(group1, FRAME_RAISED|LAYOUT_FILL_X,
                                                    0, 0, 0, 0,
                                                    0, 0, 0, 0);

    new FXButton(hbox, "\tCollapse Tree\tCollapse tree", Icon::collapse_tree,
                 this, ID_COLLAPSE_TREE,FRAME_RAISED|BUTTON_TOOLBAR);
    new FXButton(hbox, "\tExpand Tree\tExpand tree", Icon::expand_tree,
                 this, ID_EXPAND_TREE,FRAME_RAISED|BUTTON_TOOLBAR);

    paklist = new FXListBox(hbox, NULL, 0, LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
  }

  tree = new FXTreeList(group1, this, ID_DIRECTORYCHANG,
                        FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_RIGHT|TREELIST_SHOWS_BOXES|TREELIST_SHOWS_LINES|TREELIST_EXTENDEDSELECT|LAYOUT_MIN_WIDTH|TREELIST_BROWSESELECT ,
                        0, 0, 0, 0);
  splitter->setSplit(0, 250);

  switcher = new FXSwitcher(splitter,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT|FRAME_SUNKEN,
                            0, 0, 0, 0,
                            0, 0, 0, 0);

  directoryview = new DirectoryView(switcher, topdock, this);

  error_log = new FXText(switcher, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  imageview     = new ImageView(switcher, topdock);
  sharkview     = new SharkView(switcher, topdock);
  soundview     = new SoundView(switcher);
  dialogview    = new DialogView(switcher, topdock, this);

  show_switcher(SWITCHER_ICONVIEW);

  modbar  = new FXToolBar(topdock,LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|FRAME_RAISED);

  (new FXButton(modbar, "\tAllow editing\tAllow editing", Icon::editable, NULL, 0, BUTTON_TOOLBAR|FRAME_RAISED))->disable();
  new FXHorizontalSeparator(modbar, SEPARATOR_GROOVE);
  (new FXButton(modbar, "\tRevert current file back to original\tRevert current file back to original", Icon::delete_file, NULL, 0, BUTTON_TOOLBAR|FRAME_RAISED))->disable();
  (new FXButton(modbar, "\tNew Modification\tNew Modification", Icon::new_file,  NULL, 0, BUTTON_TOOLBAR|FRAME_RAISED))->disable();

  FXListBox* modlist = new FXListBox(modbar, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y);
  modlist->setTipText("Current mod directory");
  modlist->appendItem("All mods");
  modlist->appendItem("skribb_green_grubbers");
  modlist->appendItem("skribb_april_degothed");
  modlist->setNumVisible(std::min(modlist->getNumItems(), 20));
  modlist->disable();

  toolbar2 = new FXToolBar(topdock,LAYOUT_DOCK_NEXT|LAYOUT_SIDE_TOP|FRAME_RAISED|LAYOUT_FILL_X);
  new FXLabel(toolbar2, "Location:", NULL, LAYOUT_CENTER_Y);
  locationbar = new FXTextField(toolbar2,0, this, ID_LOCATIONBAR, TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_X|FRAME_SUNKEN|LAYOUT_CENTER_Y);
  new FXButton(toolbar2, "\tGo\tGo", Icon::go,
               this, ID_LOCATIONBAR, BUTTON_TOOLBAR|FRAME_RAISED);

  progress_dialog = new ProgressDialog(getApp(), "Reading pak data, this may take a while");

  // Look for dreamfall_path
  std::ifstream in((get_exe_path() + "dreamfall_path.txt").c_str());
  if (in)
    {
      std::getline(in, dreamfall_path);

      // ignore invalid an path
      if (!file_exists(dreamfall_path + "/bin/res/"))
        {
          std::cout << "Ignoring Dreamfall path: " << dreamfall_path
                    << "\ncouldn't find pak subdirectory" << std::endl;
          dreamfall_path = "";
        }
      else
        {
          init();
        }

      in.close();
    }
}

DFToolBoxWindow::~DFToolBoxWindow()
{
  delete filemenu;
  delete bookmarkmenu;
  delete helpmenu;

  delete big_folder_closed;
}

void
DFToolBoxWindow::init()
{
  load_files();

  // Init GUI stuff that needs the pak manager
  paklist->clearItems();
  paklist->appendItem("All .pak Files (*.pak)");
  paklist->appendItem("Savegames");
  paklist->appendItem("Modifications");
  //paklist->appendItem("----------------------");
  {
    const std::vector<std::string>& lst =  data_manager.get_paks();
    for(std::vector<std::string>::const_iterator i = lst.begin(); i != lst.end(); ++i)
      paklist->appendItem(i->c_str());
  }

  paklist->setNumVisible(std::min(paklist->getNumItems(), 20));
  paklist->disable();
}

void
DFToolBoxWindow::create()
{
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
}

FXTreeItem*
DFToolBoxWindow::add_directory(const std::string& pathname, FXTreeItem* rootitem)
{
  std::map<std::string, FXTreeItem*>& dir = directories[rootitem];

  std::map<std::string, FXTreeItem*>::iterator i = dir.find(pathname);

  if (i != dir.end())
    { // directory already there
      return i->second;
    }
  else
    {
      std::string pathpart, filepart;
      if (splitpath(pathname, pathpart, filepart))
        { //
          FXTreeItem* parent = add_directory(pathpart, rootitem);

          DreamfallFileEntry* parententry = static_cast<DreamfallFileEntry*>(parent->getData());

          FXTreeItem* child  = tree->appendItem(parent, filepart.c_str(), Icon::folder_open, Icon::folder_closed);

          DreamfallFileEntry* dfentry = new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY);
          data_manager.get_filetable().push_back(dfentry);

          dfentry->get_dir().name     = filepart;
          dfentry->get_dir().fullname = pathname;
          dfentry->get_dir().parent   = parententry;

          child->setData(dfentry);

          parententry->get_dir().children.push_back(dfentry);

          dir[pathname] = child;

          return child;
        }
      else
        { // new top level directory
          FXTreeItem* child = tree->appendItem(rootitem, pathname.c_str(), Icon::folder_open, Icon::folder_closed);
          dir[pathname] = child;

          DreamfallFileEntry* dfentry = new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY);
          data_manager.get_filetable().push_back(dfentry);

          dfentry->get_dir().name     = pathname;
          dfentry->get_dir().fullname = pathname;
          dfentry->get_dir().parent   = static_cast<DreamfallFileEntry*>(rootitem->getData());

          static_cast<DreamfallFileEntry*>(rootitem->getData())->get_dir().children.push_back(dfentry);
          child->setData(dfentry);

          return child;
        }
    }
}

FXint directory_tree_sorter(const FXTreeItem* a, const FXTreeItem* b)
{
  const DreamfallFileEntry* lhs = static_cast<DreamfallFileEntry*>(a->getData());
  const DreamfallFileEntry* rhs = static_cast<DreamfallFileEntry*>(b->getData());

  if (lhs->get_type() == rhs->get_type())
    return (a->getText() > b->getText());
  else
    return (lhs->get_type() > rhs->get_type());
}

class LoadFileThread : public FXThread
{
private:
  DataManager&  data_manager;
  ProgressLogger& logger;
  FXGUISignal&    sig;

public:
  LoadFileThread(DataManager& data_manager_, ProgressLogger& logger_, FXGUISignal& sig_)
    : data_manager(data_manager_),
      logger(logger_),
      sig(sig_)
  {
  }

  virtual FXint run()
  {
    logger.set_task_size(2);
    logger.set_task_status(0);
    logger.println("Reading Directory: " + dreamfall_path + "/bin/res/");
    logger.sync();

    data_manager.add_directory(dreamfall_path + "/bin/res/", logger.start_subtask());

    logger.increment_status();
    logger.println("Reading filelist");
    logger.sync();

    data_manager.add_filelist(get_exe_path() + "df-directory.txt");

    logger.println("Scanning paks... done");
    logger.sync();

    data_manager.scan_paks(logger.start_subtask());

    logger.increment_status();
    logger.println("");
    logger.println("### Done ###");
    logger.set_done();
    logger.sync();

    std::cout << "Firing signal" << std::endl;
    sig.signal();

    return 0;
  }
};

void
DFToolBoxWindow::load_files()
{
  progress_dialog->show(PLACEMENT_OWNER);
  progress_dialog->relayout(3);
  // Generate proper filetables for the pak
  GUIProgressLogger* logger = new GUIProgressLogger(getApp(), progress_dialog, ProgressDialog::ID_THREAD_UPDATE);
  // FIXME: Move buildtree into the worker thread


  FXThread* thread = new LoadFileThread(data_manager, *logger, *(new FXGUISignal(getApp(), this, ID_BUILD_TREE)));
  progress_dialog->set_thread(thread);
  thread->start();
}

long
DFToolBoxWindow::onCmdBuildTree(FXObject* sender, FXSelector, void* data)
{
  std::cout << "Build Tree" << std::endl;
  build_trees();
  return 1;
}

void
DFToolBoxWindow::build_trees()
{
  // In case we change the Dreamfall directory we clear what we have
  tree->clearItems();

  {
    DreamfallFileEntry* dfentry = new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY);
    dfentry->get_dir().name     = "Dreamfall";
    dfentry->get_dir().fullname = "Dreamfall";
    data_manager.get_filetable().push_back(dfentry);

    topmost = tree->appendItem(0, "Dreamfall", Icon::folder_open, Icon::folder_closed);
    topmost->setData(dfentry);
    current_tree_item = topmost;
  }

  tree->expandTree(topmost);

  std::cout << "Build 'all paks' tree" << std::endl;
  FXTreeItem* all_paks = tree->appendItem(topmost, "all paks", Icon::folder_open, Icon::folder_closed);
  FXTreeItem* by_pak   = tree->appendItem(topmost, "by pak",   Icon::folder_open, Icon::folder_closed);

  data_manager.get_filetable().push_back(new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY));
  data_manager.get_filetable().back()->get_dir().name     = "all paks";
  data_manager.get_filetable().back()->get_dir().fullname = "all paks";
  all_paks->setData(data_manager.get_filetable().back());

  data_manager.get_filetable().push_back(new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY));
  data_manager.get_filetable().back()->get_dir().name     = "by pak";
  data_manager.get_filetable().back()->get_dir().fullname = "by pak";
  by_pak->setData(data_manager.get_filetable().back());

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
                    FXTreeItem* item  = add_directory(pathpart, all_paks);
                    FXTreeItem* child = tree->appendItem(item, (filepart + " (" + *pak + ")").c_str(),
                                                         Icon::unknown_document, Icon::unknown_document);

                    DreamfallFileEntry* dfentry = new DreamfallFileEntry(*i->second);
                    data_manager.get_filetable().push_back(dfentry);
                    child->setData(dfentry);
                    static_cast<DreamfallFileEntry*>(item->getData())->get_dir().children.push_back(dfentry);
                  }
              }
          }
      }
  }
  {
    for(std::vector<std::string>::const_iterator i = pakfiles.begin(); i != pakfiles.end(); ++i)
      {
        FXTreeItem* this_pak   = tree->appendItem(by_pak, i->c_str(), Icon::folder_open, Icon::folder_closed);

        data_manager.get_filetable().push_back(new DreamfallFileEntry(DreamfallFileEntry::DIRECTORY_FILE_ENTRY));
        data_manager.get_filetable().back()->get_dir().name     = *i;
        data_manager.get_filetable().back()->get_dir().fullname = *i;
        this_pak->setData(data_manager.get_filetable().back());

        const DataManager::Files& files = data_manager.get_files(*i);
        for(DataManager::Files::const_iterator i = files.begin(); i != files.end(); ++i)
          {
            if (!i->second->filename.empty() || i->second->filename == "/") // FIXME: Empty files shouldn't happen, but they do
              {
                std::string pathpart, filepart;
                if (splitpath(i->first, pathpart, filepart))
                  {
                    FXTreeItem* item  = add_directory(pathpart, this_pak);
                    FXTreeItem* child = tree->appendItem(item, filepart.c_str(),
                                                         Icon::unknown_document, Icon::unknown_document);

                    DreamfallFileEntry* dfentry = new DreamfallFileEntry(*i->second);
                    data_manager.get_filetable().push_back(dfentry);
                    child->setData(dfentry);
                    static_cast<DreamfallFileEntry*>(item->getData())->get_dir().children.push_back(dfentry);
                  }
              }
          }
      }
  }

  std::cout << "Build 'by pak' tree" << std::endl;
  FXTreeItem* save_item  = add_directory("savegames", topmost);
  Directory directory = open_directory(dreamfall_path + "/saves/", ".sav");
  for(Directory::iterator i = directory.begin(); i != directory.end(); ++i)
    {
      FXTreeItem* this_save_item  = add_directory(i->name, save_item);

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

      FXTreeItem* child1 = tree->appendItem(this_save_item, dfentry1->get_label().c_str(),
                                            Icon::unknown_document, Icon::unknown_document);
      FXTreeItem* child2 = tree->appendItem(this_save_item, dfentry2->get_label().c_str(),
                                            Icon::unknown_document, Icon::unknown_document);
      FXTreeItem* child3 = tree->appendItem(this_save_item, dfentry3->get_label().c_str(),
                                            Icon::unknown_document, Icon::unknown_document);
      FXTreeItem* child4 = tree->appendItem(this_save_item, dfentry4->get_label().c_str(),
                                            Icon::unknown_document, Icon::unknown_document);
      FXTreeItem* child5 = tree->appendItem(this_save_item, dfentry5->get_label().c_str(),
                                            Icon::unknown_document, Icon::unknown_document);

      child1->setData(dfentry1);
      child2->setData(dfentry2);
      child3->setData(dfentry3);
      child4->setData(dfentry4);
      child5->setData(dfentry5);

      static_cast<DreamfallFileEntry*>(save_item->getData())->get_dir().children.push_back(dfentry1);
      static_cast<DreamfallFileEntry*>(save_item->getData())->get_dir().children.push_back(dfentry2);
      static_cast<DreamfallFileEntry*>(save_item->getData())->get_dir().children.push_back(dfentry3);
      static_cast<DreamfallFileEntry*>(save_item->getData())->get_dir().children.push_back(dfentry4);
      static_cast<DreamfallFileEntry*>(save_item->getData())->get_dir().children.push_back(dfentry5);
    }

  std::cout << "Sorting tree" << std::endl;
  // Tree is finished, so sort it
  tree->setSortFunc(directory_tree_sorter);
  tree->sortItems();
}

void set_expanded_whole_tree(FXTreeItem* item, bool expand)
{
  item->setExpanded(expand);
  for(FXTreeItem* i = item->getFirst(); i != NULL; i = i->getNext())
    set_expanded_whole_tree(i, expand);
}


void get_selection(FXTreeItem* item, std::vector<DreamfallFileEntry*>& selection)
{
  if (item->isSelected())
    selection.push_back(static_cast<DreamfallFileEntry*>(item->getData()));

  for(FXTreeItem* i = item->getFirst(); i != NULL; i = i->getNext())
    get_selection(i, selection);
}

long
DFToolBoxWindow::onCmdCollapseTree(FXObject*, FXSelector,void*)
{
  if (topmost)
    {
      for(FXTreeItem* i = topmost->getFirst(); i != NULL; i = i->getNext())
        set_expanded_whole_tree(i, false);
    }
  tree->recalc();
  return 1;
}


long
DFToolBoxWindow::onCmdExpandTree(FXObject*, FXSelector,void*)
{
  if (topmost)
    {
      for(FXTreeItem* i = topmost->getFirst(); i != NULL; i = i->getNext())
        set_expanded_whole_tree(i, true);
    }
  tree->recalc();
  return 1;
}

long
DFToolBoxWindow::onCmdDreamfallPath(FXObject*,FXSelector,void*)
{
  FXDirDialog open(this,"Select the Dreamfall directory");
  open.showFiles(FALSE);
  open.setDirectory(dreamfall_path.c_str());

  std::string new_dreamfall_path;
  while(new_dreamfall_path.empty() && open.execute())
    {
      if (!file_exists((open.getDirectory() + "/bin/res/").text()))
        {
          FXMessageBox::error(this, MBOX_OK, "Dreamfall Path Error",
                              "The given directory isn't the Dreamfall directory:\n%s",
                              open.getDirectory().text());
        }
      else
        {
          new_dreamfall_path = open.getDirectory().text() + std::string("/");
        }
    }

  if (!new_dreamfall_path.empty())
    {
      std::ofstream out((get_exe_path() + "dreamfall_path.txt").c_str());
      out << new_dreamfall_path << std::endl;
      out.close();

      dreamfall_path = new_dreamfall_path;
      init();
    }

  return 1;
}

// Open
long
DFToolBoxWindow::onCmdOpen(FXObject*,FXSelector,void*){

  FXFileDialog open(this,"Open Image");
  //open.setFilename(filename.c_str());
  //open.setPatternList(patterns);
  if(open.execute()){
    //filename=open.getFilename();
    //filelist->setCurrentFile(filename);
    //mrufiles.appendFile(filename);
    //loadimage(filename);
  }
  return 1;
}


// Save
long
DFToolBoxWindow::onCmdSave(FXObject*,FXSelector,void*)
{
  std::vector<DreamfallFileEntry*> selection;
  //get_selection(topmost, selection);
  if (switcher->getCurrent() == SWITCHER_ICONVIEW)
    directoryview->get_selection(selection);
  else
    selection.push_back(static_cast<DreamfallFileEntry*>(current_tree_item->getData()));

  if (selection.empty())
    {
      FXMessageBox::error(this, MBOX_OK,
                          "No files selected for export",
                          "No files selected for export");
    }
  else
    {
      ExportDialog* exportdialog = new ExportDialog(this);

      exportdialog->set_file_entries(selection);
      exportdialog->create();
      exportdialog->show(PLACEMENT_SCREEN);
      getApp()->refresh();
    }

  return 1;
}

long
DFToolBoxWindow::onDirChange(FXObject* sender, FXSelector, void* data)
{
  //FXTreeList* lst  = static_cast<FXTreeList*>(sender);
  FXTreeItem* item = static_cast<FXTreeItem*>(data);

  //lst->expandTree(item);
  display(item);

  return 1;
}

void
DFToolBoxWindow::show_switcher(int i)
{
  imageview->show_toolbar(false);
  sharkview->show_toolbar(false);
  soundview->show_toolbar(false);
  dialogview->show_toolbar(false);
  directoryview->show_toolbar(false);

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
    }

  topdock->layout();
  switcher->setCurrent(i);
}

void
DFToolBoxWindow::play_dialog(int id, const std::string& lang)
{
  Speech speech = data_manager.get_speech_by_id(id);

  // add a extract that takes a lang code
  data_manager.extract_by_language(lang, speech.wavefile, get_exe_path() + "tmp.mp3");
  soundview->set_music(get_exe_path() + "tmp.mp3", ""); //dialogs[lang].get_by_id(id));
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
DFToolBoxWindow::display(FXTreeItem* item)
{
  try {
    DreamfallFileEntry* entry = static_cast<DreamfallFileEntry*>(item->getData());

    if (entry->is_dir() && !entry->get_dir().parent)
      parent_button->disable();
    else
      parent_button->enable();

    current_tree_item = item;

    // FIXME: add info from where the file comes, ie. foo.pak:/foo/bar/baz
    //locationbar->setText(("pak://" + entry->fullname).c_str());
    locationbar->setText(entry->get_url().c_str());

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
              error_log->setText(FXString().format("%s: %s",
                                                   entry->get_save().fullname.c_str(),
                                                   err.what()));
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
              if (data_manager.extract(entry->get_pak(), get_exe_path() + "tmp.dat"))
                {
                  sharkview->set_shark(get_exe_path() + "tmp.dat");
                  show_switcher(SWITCHER_SHARKVIEW);
                }
            }
            break;

          case FILETYPE_WAV:
            if (data_manager.extract(entry->get_pak(), get_exe_path() + "tmp.wav"))
              {
                soundview->set_music(get_exe_path() + "tmp.wav", "Wave File");
                show_switcher(SWITCHER_SOUNDVIEW);
              }
            break;

          case FILETYPE_MP3:
            {
              if (data_manager.extract(entry->get_pak(), get_exe_path() + "tmp.mp3"))
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

                    soundview->set_music(get_exe_path() + "tmp.mp3", str.str());

                  } catch(...) {
                    soundview->set_music(get_exe_path() + "tmp.mp3",
                                         "<nothing found>\n\n"
                                         "If you want to see the dialog in text form displayed here, click \"File->Scan for Dialogs\".");
                  }

                  show_switcher(SWITCHER_SOUNDVIEW);
                }
            }
            break;

          case FILETYPE_LOCALISATION:
            if (data_manager.extract(entry->get_pak(), get_exe_path() + "tmp.dat"))
              {
                dialogview->set_dialog(get_exe_path() + "tmp.dat");
                show_switcher(SWITCHER_DIALOGVIEW);
              }
            break;

          case FILETYPE_DDS:
            {
              try {
                if (data_manager.extract(entry->get_pak(), get_exe_path() + "tmp.dat"))
                  {
                    imageview->set_image(get_exe_path() + "tmp.dat");
                    show_switcher(SWITCHER_IMAGEVIEW);
                  }
              } catch (std::exception& err) {
                error_log->setText(FXString().format("Error: %s", err.what()));
                show_switcher(SWITCHER_ERRORLOG);
              }
            }
            break;

          default:
            error_log->setText(FXString().format("Error: Unhandled file type: \"%s\" for file:\n  \"%s\"",
                                                 "UNKNOWN", // filetype2string(entry->type),
                                                 entry->get_url().c_str()));
            show_switcher(SWITCHER_ERRORLOG);
            break;
          }
      }
  } catch(std::exception& err) {
    error_log->setText(FXString().format("Error: %s", err.what()));
    show_switcher(SWITCHER_ERRORLOG);
  }
}

FXTreeItem*
DFToolBoxWindow::find(FXTreeItem* root, const std::string& url)
{
  for(FXTreeItem* item = root->getFirst(); item != NULL; item = item->getNext())
    { // FIXME: This is kind of super slow
      //std::cout << static_cast<DreamfallFileEntry*>(item->getData())->get_url()  << std::endl
      //          << url << std::endl  << std::endl;
      if (static_cast<DreamfallFileEntry*>(item->getData())->get_url() == url)
        {
          return item;
        }
      else if (item->getNumChildren() > 0)
        {
          FXTreeItem* ret = find(item, url);
          if (ret) return ret;
        }
    }
  return 0;
}

long
DFToolBoxWindow::onCmdBookmark(FXObject* obj, FXSelector, void* data)
{
  FXId* id = static_cast<FXId*>(obj);
  Bookmark* bookmark = static_cast<Bookmark*>(id->getUserData());

  FXTreeItem* item = find(topmost, bookmark->url);

  if (item) display(item);

  return 1;
}

long
DFToolBoxWindow::onCmdAddBookmark(FXObject*, FXSelector, void* data)
{
  if (current_tree_item)
    {
      std::string url = static_cast<DreamfallFileEntry*>(current_tree_item->getData())->get_url();
      FXString result = static_cast<DreamfallFileEntry*>(current_tree_item->getData())->get_label().c_str();

      if (FXInputDialog::getString(result, this,
                                   ("Bookmark for " + url).c_str(),
                                   "Please enter the name for the bookmark:", NULL))
        {
          std::cout << "Result: " << result.text() << std::endl;
          bookmarks.push_back(new Bookmark(result.text(), url));
          (new FXMenuCommand(bookmarkmenu, result, Icon::unknown_document,
                             this, ID_BOOKMARK))->setUserData(bookmarks.back());
          bookmarkmenu->create();
        }

      save_bookmarks(get_exe_path() + "/bookmarks.txt");
    }
  return 1;
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
  std::ofstream out(filename.c_str());
  if (!out)
    {
      std::cout << "DFToolBoxWindow::save_bookmarks: Couldn't write file: " << filename << std::endl;
    }
  else
    {
      out << "bookmarks" << std::endl;
      for(std::vector<Bookmark*>::iterator i = bookmarks.begin(); i != bookmarks.end(); ++i)
        {
          out << "{" << std::endl;
          out << "    label \"" << (*i)->label << "\"" << std::endl;
          out << "    url   \"" << (*i)->url   << "\"" << std::endl;
          out << "}" << std::endl;
        }
      out << "\n$" << std::endl;
      out.close();
    }
}

long
DFToolBoxWindow::onCmdLocationbar(FXObject*, FXSelector, void* data)
{
  /*
    DataManager::Files::iterator i = files.find(locationbar->getText().text());
    if (i != files.end())
    {
    // FIXME: add code here that opens the tree
    display(i->second);
    }
  */
  return 1;
}

long
DFToolBoxWindow::onCmdAbout(FXObject*,FXSelector,void*)
{
  FXMessageBox about(this,
                     "About DFToolBox V0.0",
                     "DFToolBox is a little tool that allows you to view and modify "
                     "Dreamfalls data files.\n\n"
                     "Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>",
                     NULL,MBOX_OK|DECOR_TITLE|DECOR_BORDER);
  about.execute();
  return 1;
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

long
DFToolBoxWindow::onCmdGotoParentDir(FXObject* sender, FXSelector, void* data)
{
  display(current_tree_item->getParent());
  return 1;
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
                  data_manager.extract((*i)->get_pak(), get_exe_path() + "/tmp.shark");
                  std::ifstream in((get_exe_path() + "/tmp.shark").c_str(), std::ios::binary);
                  if (!in)
                    {
                      throw std::runtime_error("Error: Couldn't open " + (get_exe_path() + "/tmp.shark"));
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

class ScanForMp3 : public FXThread
{
private:
  DFToolBoxWindow* dftoolbox;
  ProgressLogger& logger;

public:
  ScanForMp3(DFToolBoxWindow* dftoolbox_, ProgressLogger& logger_)
    : dftoolbox(dftoolbox_),
      logger(logger_)
  {
  }

  FXint run()
  {
    //dftoolbox->scan_for_all_mp3s(logger);
    return 0;
  }
};

long
DFToolBoxWindow::onCmdScanForMP3(FXObject* sender, FXSelector, void* data)
{
  progress_dialog->show(PLACEMENT_OWNER);
  progress_dialog->relayout(2);

  GUIProgressLogger* logger = new GUIProgressLogger(getApp(), progress_dialog, ProgressDialog::ID_THREAD_UPDATE);

  FXThread* thread = new ScanForMp3(this, *logger);
  progress_dialog->set_thread(thread);
  thread->start();

  return 1;
}

int main(int argc,char** argv)
{
  try {
#ifdef USE_SDL
    std::cout << "Initing SDL audio" << std::endl;
    // start SDL with audio support
    if(SDL_Init(SDL_INIT_AUDIO)==-1)
      {
        printf("SDL_Init: %s\n", SDL_GetError());
        exit(1);
      }

    std::cout << "Initing SDL mixer" << std::endl;
    // open 44.1KHz, signed 16bit, system byte order,
    //      stereo audio, using 1024 byte chunks
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1)
      {
        printf("Mix_OpenAudio: %s\n", Mix_GetError());
        exit(2);
      }

    atexit(Mix_CloseAudio);
    atexit(SDL_Quit);
#elif not __linux__
    freopen((get_exe_path() + "/stdout.txt").c_str(), "w", stdout);
    freopen((get_exe_path() + "/stderr.txt").c_str(), "w", stderr);
#endif

    FXApp application("DFToolBox", "Grumbel");

    application.init(argc,argv);

    new FXToolTip(&application);

    DFToolBoxWindow* main = new DFToolBoxWindow(&application);
    application.create();

    main->show(PLACEMENT_SCREEN);

    return application.run();
    // Icon::deinit();
  } catch(std::exception& err) {
    std::cout << "Fatal Error: " << err.what() << std::endl;
  }
  return 0;
}

/* EOF */
