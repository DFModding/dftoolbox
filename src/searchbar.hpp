/*  $Id: searchbar.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SEARCHBAR_HPP
#define HEADER_SEARCHBAR_HPP

#include "fx.h"

/** */
class Searchbar : public FXHorizontalFrame
{
private:
  FXDECLARE(Searchbar);

  FXText*      text;
  FXTextField* search;

public:
  Searchbar() {}
  virtual ~Searchbar() {}
  Searchbar(FXComposite* parent, FXText* text);

  long onCmdSearchForward(FXObject*,FXSelector,void*);
  long onCmdSearchBackward(FXObject*,FXSelector,void*);

  enum {
    ID_SEARCH_FORWARD = FXHorizontalFrame::ID_LAST,
    ID_SEARCH_BACKWARD
  };
};

#endif

/* EOF */
