/*  $Id: shark_view.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <fstream>
#include <stdexcept>
#include <iostream>
#include "icons.hpp"
#include "shark3d.hpp"
#include "system.hpp"
#include "searchbar.hpp"
#include "shark_view.hpp"

FXDEFMAP(SharkView) SharkViewMap[] = {
  FXMAPFUNC(SEL_COMMAND,       SharkView::ID_TOGGLE_WRAP,        SharkView::onCmdToggleWrap),
};

// Object implementation
FXIMPLEMENT(SharkView, FXVerticalFrame, SharkViewMap, ARRAYNUMBER(SharkViewMap));

SharkView::SharkView()
{
}

SharkView::SharkView(FXComposite* parent, FXComposite* toolbar_dock)
  : FXVerticalFrame(parent, FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_CENTER_Y,
                    0, 0, 0, 0,
                    0, 0, 0, 0)
{
  toolbar = new FXToolBar(toolbar_dock, LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|FRAME_RAISED|LAYOUT_FILL_X);

  new FXButton(toolbar, "\tToggle Word Wrap\tToggle Word Wrap", Icon::indent, this, ID_TOGGLE_WRAP, BUTTON_TOOLBAR|FRAME_RAISED);

  text = new FXText(this, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
  text->setEditable(FALSE);

  text->setTextStyle(text->getTextStyle() | TEXT_WORDWRAP);

  new Searchbar(this, text);
}

void
SharkView::set_shark(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in) 
    {
      throw std::runtime_error("Error: Couldn't open " + filename);
    }
  else
    {
      Shark3D* shark = Shark3D::parse_binary(in);
      in.close();
   
      std::ostringstream str;
      shark->write_text(str);
      text->setText(str.str().c_str(), str.str().size());
      delete shark;
    }
}

long
SharkView::onCmdToggleWrap(FXObject*,FXSelector,void*)
{
  if (text->getTextStyle() & TEXT_WORDWRAP)
    text->setTextStyle(text->getTextStyle() & ~TEXT_WORDWRAP);
  else
    text->setTextStyle(text->getTextStyle() | TEXT_WORDWRAP);

  return 1;
}

/* EOF */
