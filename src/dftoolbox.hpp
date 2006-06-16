/*  $Id: dftoolbox.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>
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
#include "icons.hpp"
#include "dialog.hpp"
#include "location.hpp"
#include "tljpak_manager.hpp"

class SharkView;
class SoundView;
class ImageView;
class DialogView;
class DirectoryView;
class ProgressDialog;

struct Bookmark
{
  std::string label;
  std::string url;

  Bookmark(const std::string& label_,
           const std::string& url_)
    : label(label_), url(url_)
  {}
};

class DFToolBoxWindow : public FXMainWindow
{
  FXDECLARE(DFToolBoxWindow);
protected:
  FXMenuBar*   menubar;
  FXDockSite*  topdock;
  FXMenuPane*  filemenu;
  FXMenuPane*  bookmarkmenu;
  FXMenuPane*  helpmenu;

  FXToolBar*   toolbar;
  FXToolBar*   toolbar2;
  FXToolBar*   modbar;

  FXSplitter*  splitter;
  FXStatusBar* status;

  FXListBox*  paklist;
  FXTreeList* tree;
  FXTreeItem* topmost;
  FXTreeItem* current_tree_item;

  FXTextField* locationbar;

  ImageView*     imageview;
  SharkView*     sharkview;
  SoundView*     soundview;
  DialogView*    dialogview;  
  DirectoryView* directoryview;  
  FXText*        error_log;

  FXVerticalFrame* group1;
  FXVerticalFrame* group2;
  FXSwitcher*      switcher;

  FXGIFIcon         *big_folder_closed;

  FXImage*       image;
  FXButton*      parent_button; 

  FXMenuTitle* bookmark_title;
  ProgressDialog* progress_dialog;

  std::map<unsigned int, Speech> id_to_speech;
  std::map<std::string,  Speech> mp3_to_speech;
  std::vector<Dialog> dialogs; 

  // Cache to speed up tree insertion
  typedef std::map<FXTreeItem*, std::map<std::string, FXTreeItem*> > Directories;
  Directories directories;

  TLJPakManager pak_manager;

  // Simple cache to collect all pointers pointing to allocated memory
  std::vector<DreamfallFileEntry*> filetable;

  std::vector<Bookmark*> bookmarks;

public:
  DFToolBoxWindow();
  DFToolBoxWindow(FXApp* a);

  virtual ~DFToolBoxWindow();

  void create();
  FXTreeItem* add_directory(const std::string& pathname, FXTreeItem* rootitem);
  void load_files();

  void display(DirectoryFileEntry* dfentry);
  void display(FXTreeItem* item);
  void init();

  void export_files(const std::vector<DreamfallFileEntry*>& selection,
                    const std::string& outpath, 
                    bool preserve_path, bool fix_extension, bool shark_as_text);

  void show_switcher(int i);
  void play_dialog(int i, const std::string& lang);

  long onCmdOpen(FXObject*,FXSelector,void*);
  long onCmdSave(FXObject*,FXSelector,void*);
  long onCmdAbout(FXObject*,FXSelector,void*);
  long onCmdBookmark(FXObject*, FXSelector, void* data);
  long onCmdAddBookmark(FXObject*, FXSelector, void* data);
  long onCmdLocationbar(FXObject*, FXSelector, void* data);
  long onDirChange(FXObject* sender, FXSelector, void* data);
  long onCmdDreamfallPath(FXObject*,FXSelector,void*);
  long onCmdCollapseTree(FXObject*, FXSelector,void*);
  long onCmdExpandTree(FXObject*, FXSelector,void*);
  long onCmdGotoParentDir(FXObject* sender, FXSelector, void* data);
  long onCmdScanForMP3(FXObject* sender, FXSelector, void* data);
  void scan_for_mp3s(const std::string& filename);
  
  /** Find the tree item matching url */
  FXTreeItem* find(FXTreeItem* root, const std::string& url);

  enum {
    SWITCHER_ICONVIEW,
    SWITCHER_ERRORLOG,
    SWITCHER_IMAGEVIEW,
    SWITCHER_SHARKVIEW,
    SWITCHER_SOUNDVIEW,
    SWITCHER_DIALOGVIEW
  };

  enum { 
    ID_SAVE = FXMainWindow::ID_LAST,
    ID_OPEN,
    ID_ABOUT,
    ID_DIRECTORYCHANG,
    ID_BOOKMARK,
    ID_ADD_BOOKMARK,
    ID_PREFERENCES,
    ID_LOCATIONBAR,
    ID_COLLAPSE_TREE,
    ID_EXPAND_TREE,
    ID_DREAMFALL_PATH,
    ID_GOTO_PARENTDIR,
    ID_SCAN_FOR_MP3
  };
};

#endif

/* EOF */
