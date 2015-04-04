/*  $Id: dfmodactivator.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DFMODACTIVATOR_HPP
#define HEADER_DFMODACTIVATOR_HPP

#include "fx.h"
#include <string>
#include <vector>

#include "pak_insert_thread.hpp"

class ProgressDialog;

class ModListEntry : public FXHorizontalFrame
{
  FXDECLARE(ModListEntry);
protected:
  FXText* longtext;
  FXArrowButton* arrowbutton;
  FXToggleButton* selectbutton;
  std::string        directory;

public:
  ModListEntry();
  ModListEntry(FXComposite* parent,
               FXIcon* icon,
               std::string        directory_,
               const FXString& title_string, const FXString& short_description, const FXString& long_description);
  std::string get_directory() const;
  long onCmdToggleLongDesc(FXObject*,FXSelector,void*);
  void setState(FXbool s);
  FXbool getState () const;

  enum {
    ID_TOGGLE_LONG_DESC = FXWindow::ID_LAST
  };
};

class DFModActivatorWindow : public FXMainWindow
{
  FXDECLARE(DFModActivatorWindow);
protected:
  FXMenuBar*         menubar;
  FXMenuPane*        filemenu;
  FXMenuPane*          helpmenu;
  FXScrollWindow*      modlistscroll;
  FXVerticalFrame*      modlist;
  FXHorizontalFrame* buttonbar;
  FXButton*            selectnone_button;
  FXButton*            selectall_button;
public: FXButton*            install_button; private:
public: FXButton*            uninstall_button; private:
  FXStatusBar*       status;
  ProgressDialog* progress_dialog;
  std::vector<ModListEntry*> entries;
public:
  DFModActivatorWindow();
  DFModActivatorWindow(FXApp* a);

  virtual ~DFModActivatorWindow();

  long onCmdAbout(FXObject*,FXSelector,void*);
  void scan_mod_directory();
  void collect_insert_requests(std::vector<InsertRequest>& inserts);
  void set_dreamfall_path(const std::string& path);
  long onCmdInstall(FXObject* obj, FXSelector sel, void* data);
  long onCmdUninstall(FXObject* obj, FXSelector sel, void* data);
  long onCmdSelectAll(FXObject*, FXSelector,void*);
  long onCmdSelectNone(FXObject*, FXSelector,void*);
  bool get_dreamfall_path();
  long onCmdDreamfallPath(FXObject*, FXSelector,void*);

  enum{
    ID_ABOUT=FXMainWindow::ID_LAST,
    ID_DREAMFALL_PATH,
    ID_INSTALL,
    ID_UNINSTALL,
    ID_SELECT_ALL,
    ID_SELECT_NONE,
  };
};

#endif

/* EOF */

