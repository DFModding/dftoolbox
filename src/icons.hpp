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

#ifndef HEADER_ICONS_HPP
#define HEADER_ICONS_HPP

#include <QIcon>

namespace Icon {

extern QIcon folder_open;
extern QIcon folder_closed;
extern QIcon unknown_document;
extern QIcon search_forward;
extern QIcon search_backward;
extern QIcon new_file;
extern QIcon load_file;
extern QIcon save_file;
extern QIcon delete_file;
extern QIcon export_file;
extern QIcon import_file;
extern QIcon zoom_in;
extern QIcon zoom_out;
extern QIcon zoom_1_1;
extern QIcon grayscale;
extern QIcon normalmap;
extern QIcon editable;
extern QIcon indent;
extern QIcon play;
extern QIcon stop;
extern QIcon pause;
extern QIcon icons_list;
extern QIcon icons_detail;
extern QIcon icons_large;
extern QIcon collapse_tree;
extern QIcon expand_tree;
extern QIcon go;
extern QIcon up;
extern QIcon cancel;
extern QIcon ok;
extern QIcon kian;
extern QIcon large_folder;
extern QIcon large_file;
extern QIcon large_shark;
extern QIcon dftoolbox;
extern QIcon dftoolbox_mini;
extern QIcon dfmodtool;
extern QIcon dfmodtool_mini;
extern QIcon dreamfall;

/** Load all icons from the data directory, needs to be called after
    the QApplication has been created. */
void init();

} // namespace Icon

#endif

/* EOF */
