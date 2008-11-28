/*  $Id: dialog_view.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_DIALOG_VIEW_HPP
#define HEADER_DIALOG_VIEW_HPP

#include "fx.h"
#include <string>
#include "dialog.hpp"

class DFToolBoxWindow;

/** */
class DialogView : public FXVerticalFrame
{
private:
  FXDECLARE(DialogView);

  DFToolBoxWindow* dftoolbox;
  FXToolBar*   toolbar;
  FXText*      text;
  FXListBox*   langbox;
  typedef std::vector<Dialog> Dialogs; 
  Dialogs dialogs;

public:
  DialogView() {}
  DialogView(FXComposite* parent, FXComposite* dock, DFToolBoxWindow* dftoolbox);

  void set_dialog(const std::string& filename);
  long onCmdLanguageSwitch(FXObject*,FXSelector,void*);
  long onCmdHyperlink(FXObject*,FXSelector,void*);
  long onCmdSaveText(FXObject*,FXSelector,void*);
  void set_language(int id);
  void show_toolbar(bool t) { if (t) toolbar->show(); else toolbar->hide(); }

  enum {
    ID_LANGUAGE_SWITCH = FXHorizontalFrame::ID_LAST,
    ID_HYPERLINK,
    ID_SAVE_TEXT
  };
};

#endif

/* EOF */
