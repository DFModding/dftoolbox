/*  $Id: shark_view.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SHARK_VIEW_HPP
#define HEADER_SHARK_VIEW_HPP

#include "fx.h"
#include <string>

class SharkView : FXVerticalFrame
{
  FXDECLARE(SharkView);
private:
  FXToolBar*   toolbar;
  FXText*      text;

public:
  SharkView();
  SharkView(FXComposite* parent, FXComposite* toolbar_dock);

  void set_shark(const std::string& filename);
  void set_text(const std::string& text);

  long onCmdToggleWrap(FXObject*,FXSelector,void*);
  long onCmdSaveText(FXObject*,FXSelector,void*);
  void show_toolbar(bool t) { if (t) toolbar->show(); else toolbar->hide(); }

  enum {
    ID_TOGGLE_WRAP=FXVerticalFrame::ID_LAST,
    ID_SAVE_TEXT,
  };
};

#endif

/* EOF */
