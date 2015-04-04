/*  $Id: sound_view.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SOUND_VIEW_HPP
#define HEADER_SOUND_VIEW_HPP

#include "fx.h"
#ifdef USE_SDL
#include <SDL/SDL_mixer.h>
#endif
#include <string>

class SoundView : public FXVerticalFrame
{
private:
  FXDECLARE(SoundView);
#ifdef USE_SDL
  Mix_Music* music;
#endif 
  FXTextField* input;
  FXToggleButton* play_button;
  FXText* text;
public:
  SoundView();
  SoundView(FXComposite* parent);
  ~SoundView();

  void set_music(const std::string& filename, const std::string& desc);

  long onCmdPlay(FXObject*,FXSelector,void*);
  long onCmdStop(FXObject*,FXSelector,void*);
  long onCmdMciCommand(FXObject*,FXSelector,void*);

  void show_toolbar(bool t) {  }
  void issue_msi_command(const std::string& cmd);

  enum {
    ID_PLAY = FXHorizontalFrame::ID_LAST,
    ID_STOP,
    ID_MCI_COMMAND
  };
};

#endif

/* EOF */
