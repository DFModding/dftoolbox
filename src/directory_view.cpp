/*  $Id: directory_view.cpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <iostream>
#include "icons.hpp"
#include "dreamfall_file_entry.hpp"
#include "dftoolbox.hpp"
#include "directory_view.hpp"

FXDEFMAP(DirectoryView) DirectoryViewMap[] = {
  FXMAPFUNC(SEL_COMMAND,       DirectoryView::ID_ICONLIST_BIG_ICONS,  DirectoryView::onCmdIconListBigIcons),
  FXMAPFUNC(SEL_COMMAND,       DirectoryView::ID_ICONLIST_MINI_ICONS, DirectoryView::onCmdIconListMiniIcons),
  FXMAPFUNC(SEL_COMMAND,       DirectoryView::ID_ICONLIST_DETAILED,   DirectoryView::onCmdIconListDetailed),
  FXMAPFUNC(SEL_DOUBLECLICKED, DirectoryView::ID_ICON_DOUBLECLICKED,  DirectoryView::onIconDblClick),
};

// Object implementation
FXIMPLEMENT(DirectoryView, FXVerticalFrame, DirectoryViewMap, ARRAYNUMBER(DirectoryViewMap));

DirectoryView::DirectoryView(FXComposite* parent, FXComposite* dock, DFToolBoxWindow* dftoolbox_)
  : FXVerticalFrame(parent, FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_CENTER_Y,
                    0, 0, 0, 0,
                    0, 0, 0, 0),
    dftoolbox(dftoolbox_)
{
  toolbar = new FXToolBar(dock, LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  iconlist = new FXIconList(this, this, ID_ICON_DOUBLECLICKED,
                            LAYOUT_FILL_X|LAYOUT_FILL_Y|ICONLIST_DETAILED|ICONLIST_EXTENDEDSELECT|ICONLIST_COLUMNS|ICONLIST_AUTOSIZE);
  iconlist->appendHeader("Name",NULL, 300);
  iconlist->appendHeader("Type",NULL, 100);
  iconlist->appendHeader("Size",NULL, 60);
  iconlist->appendHeader("Pak", NULL, 150);

  new FXButton(toolbar, "\tShow Icons", Icon::icons_list,
               this, ID_ICONLIST_BIG_ICONS, BUTTON_TOOLBAR|FRAME_RAISED);
  new FXButton(toolbar, "\tShow Thumbnail View", Icon::icons_large,
               this, ID_ICONLIST_MINI_ICONS, BUTTON_TOOLBAR|FRAME_RAISED);
  new FXButton(toolbar, "\tShow Detail List", Icon::icons_detail,
               this, ID_ICONLIST_DETAILED, BUTTON_TOOLBAR|FRAME_RAISED);
}

FXint iconlist_sorter(const FXIconItem* a, const FXIconItem* b)
{
  const DreamfallFileEntry* lhs = static_cast<DreamfallFileEntry*>(a->getData());
  const DreamfallFileEntry* rhs = static_cast<DreamfallFileEntry*>(b->getData());

  if (lhs->get_type() == rhs->get_type())
    return (a->getText() > b->getText());
  else
    return (lhs->get_type() > rhs->get_type());
}

void
DirectoryView::change_directory(FXTreeItem* item)
{
  iconlist->clearItems();

  for(FXTreeItem* it = item->getFirst(); it != NULL; it = it->getNext())
    {
      DreamfallFileEntry* entry = static_cast<DreamfallFileEntry*>(it->getData());

      FXIcon* big;
      FXIcon* small;
      int     size;
      const char*   typestr = "DIRECTORY";
      const char* pak = "";

      switch(entry->get_type())
        {
        case DreamfallFileEntry::DIRECTORY_FILE_ENTRY:
          big   = Icon::large_folder;
          small = Icon::folder_closed;
          size  = it->getNumChildren();
          typestr = "Directory";
          break;

        case DreamfallFileEntry::PAK_FILE_ENTRY:
          big   = (entry->get_pak().type == FILETYPE_SHARK3D)?Icon::large_file:Icon::large_shark;
          small = Icon::unknown_document;
          size  = entry->get_pak().filesize;
          typestr = filetype2string(entry->get_pak().type);
          pak = entry->get_pak().pakfile.c_str();
          break;

          break;

        default:
          big   = Icon::large_file;
          small = Icon::unknown_document;
          size  = 0; //entry->filesize;
          break;
        }

      iconlist->appendItem(FXString().format("%s\t%s\t%dKB\t%s",
                                             entry->get_label().c_str(),
                                             typestr,
                                             size/1024,
                                             pak),
                           big, small,
                           it);
    }


  iconlist->setSortFunc(iconlist_sorter);
  iconlist->sortItems();
}

long
DirectoryView::onCmdIconListBigIcons (FXObject*, FXSelector,void*)
{
  iconlist->handle(this, FXSEL(SEL_COMMAND, FXIconList::ID_SHOW_BIG_ICONS), NULL);
  return 1;
}

long
DirectoryView::onCmdIconListMiniIcons(FXObject*, FXSelector,void*)
{
  iconlist->handle(this, FXSEL(SEL_COMMAND, FXIconList::ID_SHOW_MINI_ICONS), NULL);
  return 1;
}

long
DirectoryView::onCmdIconListDetailed (FXObject*, FXSelector,void*)
{
  iconlist->handle(this, FXSEL(SEL_COMMAND, FXIconList::ID_SHOW_DETAILS), NULL);
  return 1;
}

long
DirectoryView::onIconDblClick(FXObject* sender, FXSelector, void* data)
{
  FXint index      = reinterpret_cast<FXint>(data);
  FXIconList* lst  = static_cast<FXIconList*>(sender);
  FXIconItem* item = lst->getItem(index);

  dftoolbox->display(static_cast<FXTreeItem*>(item->getData()));

  return 1;
}

void
DirectoryView::get_selection(std::vector<DreamfallFileEntry*>& lst)
{
  for(int i = 0; i < iconlist->getNumItems(); ++i)
    {
      if (iconlist->getItem(i)->isSelected())
        lst.push_back(static_cast<DreamfallFileEntry*>
                      (static_cast<FXTreeItem*>
                       (iconlist->getItem(i)->getData())->getData()));
    }
}

/* EOF */
