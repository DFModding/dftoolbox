/*  $Id: export_dialog.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_EXPORT_DIALOG_HPP
#define HEADER_EXPORT_DIALOG_HPP

#include "fx.h"
#include <string>
#include "dreamfall_file_entry.hpp"

class DFToolBoxWindow;

/** */
class ExportDialog : public FXDialogBox
{
  FXDECLARE(ExportDialog);
private:
  DFToolBoxWindow* parent;
  FXText* filename;
  FXTextField* pathname;
  FXTextField* filter;
  FXCheckButton* preserve_path;
  FXCheckButton* fix_extension;

  std::vector<DreamfallFileEntry*> entries;

public:
  ExportDialog() {}
  ExportDialog(DFToolBoxWindow* parent);
  virtual ~ExportDialog() {}

  void set_filename(const std::string& filename);
  void set_file_entry(DreamfallFileEntry* entry);
  void set_file_entries(const std::vector<DreamfallFileEntry*>& selection);

  long onCmdExport(FXObject*,FXSelector,void*);
  long onCmdCancel(FXObject*,FXSelector,void*);

  long onCmdBrowse(FXObject*,FXSelector,void*);

  enum {
    ID_EXPORT = FXDialogBox::ID_LAST,
    ID_BROWSE,
    ID_CANCEL
  };
};

#endif

/* EOF */
