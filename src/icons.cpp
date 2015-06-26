/*  $Id: icons.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <stdexcept>
#include <iostream>
#include "system.hpp"
#include "icons.hpp"

namespace Icon {

FXIcon* folder_open;
FXIcon* folder_closed;
FXIcon* unknown_document;

FXIcon* search_forward;
FXIcon* search_backward;

FXIcon* new_file;
FXIcon* load_file;
FXIcon* save_file;
FXIcon* delete_file;

FXIcon* export_file;
FXIcon* import_file;

FXIcon* zoom_in;
FXIcon* zoom_out;
FXIcon* zoom_1_1;

FXIcon* grayscale;
FXIcon* normalmap;

FXIcon* editable;
FXIcon* indent;

FXIcon* play;
FXIcon* stop;
FXIcon* pause;

FXIcon* icons_list;
FXIcon* icons_detail;
FXIcon* icons_large;

FXIcon* collapse_tree;
FXIcon* expand_tree;

FXIcon* go;
FXIcon* up;

FXIcon* cancel;
FXIcon* unknown;

FXIcon* large_folder;
FXIcon* large_file;
FXIcon* large_shark;

FXIcon* dftoolbox;
FXIcon* dftoolbox_mini;

FXIcon* dfmodtool;
FXIcon* dfmodtool_mini;

FXIcon* dreamfall;

FXIcon* load_icon(FXApp* app, const std::string& filename)
{
  std::vector<char> data;
  if (file_readbytes(get_exe_path() + "/" + filename, data))
    {
      FXIcon* icon = new FXGIFIcon(app, &*data.begin());
      icon->create();
      return icon;
    }
  else
    {
      std::cout << "Error: Couldn't open " << filename << std::endl;
      FXIcon* icon = new FXGIFIcon(app, unknown_icon_data);
      icon->create();
      return icon;
    }
}

void init(FXApp* app)
{
  folder_open      = load_icon(app, "icons/folder_open.gif");
  folder_closed    = load_icon(app, "icons/folder_closed.gif");
  unknown_document = load_icon(app, "icons/unknown_document.gif");

  search_forward  = load_icon(app, "icons/search_forward.gif");
  search_backward = load_icon(app, "icons/search_backward.gif");

  new_file    = load_icon(app, "icons/new_file.gif");
  load_file   = load_icon(app, "icons/load_file.gif");
  save_file   = load_icon(app, "icons/save_file.gif");
  delete_file = load_icon(app, "icons/delete_file.gif");

  export_file = load_icon(app, "icons/export_file.gif");
  import_file = load_icon(app, "icons/import_file.gif");

  zoom_in  = load_icon(app, "icons/zoom_in.gif");
  zoom_out = load_icon(app, "icons/zoom_out.gif");
  zoom_1_1 = load_icon(app, "icons/zoom_1_1.gif");

  grayscale = load_icon(app, "icons/grayscale.gif");
  normalmap = load_icon(app, "icons/normalmap.gif");

  editable = load_icon(app, "icons/editable.gif");
  indent   = load_icon(app, "icons/indent.gif");

  play  = load_icon(app, "icons/play.gif");
  stop  = load_icon(app, "icons/stop.gif");
  pause = load_icon(app, "icons/pause.gif");

  icons_list   = load_icon(app, "icons/icons_list.gif");
  icons_detail = load_icon(app, "icons/icons_detail.gif");
  icons_large  = load_icon(app, "icons/icons_large.gif");

  collapse_tree = load_icon(app, "icons/collapse_tree.gif");
  expand_tree   = load_icon(app, "icons/expand_tree.gif");

  go = load_icon(app, "icons/go.gif");
  up = load_icon(app, "icons/up.gif");
  cancel = load_icon(app, "icons/cancel.gif");

  large_folder = load_icon(app, "icons/large/folder.gif");
  large_file  =  load_icon(app, "icons/large/file.gif");
  large_shark  = load_icon(app, "icons/large/shark.gif");

  dftoolbox      = load_icon(app, "icons/dftoolbox.gif");
  dftoolbox_mini = load_icon(app, "icons/dftoolbox_mini.gif");

  dfmodtool      = load_icon(app, "icons/dfmodtool.gif");
  dfmodtool_mini = load_icon(app, "icons/dfmodtool_mini.gif");

  dreamfall = load_icon(app, "icons/dreamfall.gif");
}

void deinit()
{
  delete dftoolbox;
  delete dftoolbox_mini;

  delete dfmodtool;
  delete dfmodtool_mini;

  delete unknown;

  delete search_forward;
  delete search_backward;

  delete folder_open;
  delete folder_closed;
  delete unknown_document;

  delete new_file;
  delete load_file;
  delete save_file;
  delete delete_file;

  delete export_file;
  delete import_file;

  delete zoom_in;
  delete zoom_out;
  delete zoom_1_1;

  delete grayscale;
  delete normalmap;

  delete editable;
  delete indent;

  delete play;
  delete stop;
  delete pause;

  delete icons_list;
  delete icons_detail;
  delete icons_large;

  delete collapse_tree;
  delete expand_tree;

  delete go;
  delete up;
  delete cancel;

  delete large_folder;
  delete large_file;
  delete large_shark;

  delete dreamfall;
}

} // namespace Icon

/* EOF */
