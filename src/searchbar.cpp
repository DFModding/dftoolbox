/*  $Id: searchbar.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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
#include "searchbar.hpp"

FXDEFMAP(Searchbar) SearchbarMap[] = {
  FXMAPFUNC(SEL_COMMAND,       Searchbar::ID_SEARCH_FORWARD,       Searchbar::onCmdSearchForward),
  FXMAPFUNC(SEL_COMMAND,       Searchbar::ID_SEARCH_BACKWARD,      Searchbar::onCmdSearchBackward),
};

// Object implementation
FXIMPLEMENT(Searchbar, FXHorizontalFrame, SearchbarMap, ARRAYNUMBER(SearchbarMap));

Searchbar::Searchbar(FXComposite* parent, FXText* text_)
  : FXHorizontalFrame(parent, FRAME_NONE|LAYOUT_FILL_X|FRAME_LINE,
                      0, 0, 0, 0,
                      0, 0, 0, 0),
    text(text_)
{
  new FXLabel(this, "Search:", NULL, FRAME_NONE|LAYOUT_CENTER_Y);
  search = new FXTextField(this, 0, this, ID_SEARCH_FORWARD, TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_X|FRAME_SUNKEN|LAYOUT_CENTER_Y);
  new FXButton(this, "\tSearch backward", Icon::search_backward, this, ID_SEARCH_BACKWARD, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_CENTER_Y);
  new FXButton(this, "\tSearch forward", Icon::search_forward, this, ID_SEARCH_FORWARD, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_CENTER_Y);
}


long
Searchbar::onCmdSearchForward(FXObject*,FXSelector,void*)
{
  FXint beg = 0;
  FXint end = 0;

  if (text->findText(search->getText(),
                     &beg, &end, text->getCursorPos(),
                     SEARCH_FORWARD|SEARCH_WRAP|SEARCH_EXACT))
    {
      //text->makePositionVisible(beg);
      text->setCenterLine(beg);
      text->setCursorPos(end);
      text->setSelection(beg,end-beg);
    }
  else
    {
      std::cout << "Nothing found" << std::endl;
    }
  return 1;
}

long
Searchbar::onCmdSearchBackward(FXObject*,FXSelector,void*)
{
  FXint beg = 0;
  FXint end = 0;

  if (text->findText(search->getText(),
                     &beg, &end, (text->getCursorPos() - search->getText().length()-1) % text->getLength(),
                     SEARCH_BACKWARD|SEARCH_WRAP|SEARCH_EXACT))
    {
      text->setCenterLine(beg);
      text->setCursorPos(end);
      text->setSelection(beg,end-beg);
    }
  else
    {
      std::cout << "Nothing found" << std::endl;
    }
  return 1;
}

/* EOF */
