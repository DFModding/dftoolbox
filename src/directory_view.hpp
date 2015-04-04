/*  $Id: directory_view.hpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DIRECTORY_VIEW_HPP
#define HEADER_DIRECTORY_VIEW_HPP

#include "fx.h"

class DFToolBoxWindow;
class DreamfallFileEntry;

/** */
class DirectoryView : public FXVerticalFrame
{
  FXDECLARE(DirectoryView);
private:
  FXTreeItem* item;
  DFToolBoxWindow* dftoolbox;
  FXIconList*  iconlist;
  FXToolBar* toolbar;

public:
  DirectoryView() {}
  DirectoryView(FXComposite* parent, FXComposite* dock, DFToolBoxWindow* dftoolbox);

  void change_directory(FXTreeItem* item);

  long onCmdIconListBigIcons (FXObject*, FXSelector,void*);
  long onCmdIconListMiniIcons(FXObject*, FXSelector,void*);
  long onCmdIconListDetailed (FXObject*, FXSelector,void*);
  long onIconDblClick(FXObject* sender, FXSelector, void* data);

  void show_toolbar(bool t) { if (t) toolbar->show(); else toolbar->hide(); }
  void get_selection(std::vector<DreamfallFileEntry*>& lst);

  enum {
    ID_ICONLIST_BIG_ICONS = FXVerticalFrame::ID_LAST,
    ID_ICONLIST_MINI_ICONS,
    ID_ICONLIST_DETAILED,
    ID_ICON_DOUBLECLICKED,
  };
};

#endif

/* EOF */
