/*  $Id: icons.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "icons.hpp"

#include <iostream>
#include <QFile>
#include <QString>

#include "system.hpp"

namespace Icon {

QIcon folder_open;
QIcon folder_closed;
QIcon unknown_document;
QIcon search_forward;
QIcon search_backward;
QIcon new_file;
QIcon load_file;
QIcon save_file;
QIcon delete_file;
QIcon export_file;
QIcon import_file;
QIcon zoom_in;
QIcon zoom_out;
QIcon zoom_1_1;
QIcon grayscale;
QIcon normalmap;
QIcon editable;
QIcon indent;
QIcon play;
QIcon stop;
QIcon pause;
QIcon icons_list;
QIcon icons_detail;
QIcon icons_large;
QIcon collapse_tree;
QIcon expand_tree;
QIcon go;
QIcon up;
QIcon cancel;
QIcon ok;
QIcon kian;
QIcon large_folder;
QIcon large_file;
QIcon large_shark;
QIcon dftoolbox;
QIcon dftoolbox_mini;
QIcon dfmodtool;
QIcon dfmodtool_mini;
QIcon dreamfall;

namespace {

QIcon load_icon(const std::string& filename)
{
  const QString path = QString::fromStdString(get_data_path() + filename);
  if (!QFile::exists(path))
  {
    std::cout << "Error: Couldn't open " << filename << std::endl;
    return QIcon();
  }
  return QIcon(path);
}

} // namespace

void init()
{
  folder_open = load_icon("icons/folder_open.png");
  folder_closed = load_icon("icons/folder_closed.png");
  unknown_document = load_icon("icons/unknown_document.png");
  search_forward = load_icon("icons/search_forward.png");
  search_backward = load_icon("icons/search_backward.png");
  new_file = load_icon("icons/new_file.png");
  load_file = load_icon("icons/load_file.png");
  save_file = load_icon("icons/save_file.png");
  delete_file = load_icon("icons/delete_file.png");
  export_file = load_icon("icons/export_file.png");
  import_file = load_icon("icons/import_file.png");
  zoom_in = load_icon("icons/zoom_in.png");
  zoom_out = load_icon("icons/zoom_out.png");
  zoom_1_1 = load_icon("icons/zoom_1_1.png");
  grayscale = load_icon("icons/grayscale.png");
  normalmap = load_icon("icons/normalmap.png");
  editable = load_icon("icons/editable.png");
  indent = load_icon("icons/indent.png");
  play = load_icon("icons/play.png");
  stop = load_icon("icons/stop.png");
  pause = load_icon("icons/pause.png");
  icons_list = load_icon("icons/icons_list.png");
  icons_detail = load_icon("icons/icons_detail.png");
  icons_large = load_icon("icons/icons_large.png");
  collapse_tree = load_icon("icons/collapse_tree.png");
  expand_tree = load_icon("icons/expand_tree.png");
  go = load_icon("icons/go.png");
  up = load_icon("icons/up.png");
  cancel = load_icon("icons/cancel.png");
  ok = load_icon("icons/enabled.png");
  kian = load_icon("icons/kian.png");
  large_folder = load_icon("icons/large/folder.png");
  large_file = load_icon("icons/large/file.png");
  large_shark = load_icon("icons/large/shark.png");
  dftoolbox = load_icon("icons/dftoolbox.png");
  dftoolbox_mini = load_icon("icons/dftoolbox_mini.png");
  dfmodtool = load_icon("icons/dfmodtool.png");
  dfmodtool_mini = load_icon("icons/dfmodtool_mini.png");
  dreamfall = load_icon("icons/dreamfall.png");
}

} // namespace Icon

/* EOF */
