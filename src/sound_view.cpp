/*  $Id: sound_view.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef USE_SDL
#include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include "icons.hpp"
#include "system.hpp"
#include "sound_view.hpp"

FXDEFMAP(SoundView) SoundViewMap[] = {
  FXMAPFUNC(SEL_COMMAND,       SoundView::ID_PLAY,        SoundView::onCmdPlay),
  FXMAPFUNC(SEL_COMMAND,       SoundView::ID_STOP,        SoundView::onCmdStop),
  FXMAPFUNC(SEL_COMMAND,       SoundView::ID_MCI_COMMAND, SoundView::onCmdMciCommand),
};

// Object implementation
FXIMPLEMENT(SoundView, FXVerticalFrame, SoundViewMap, ARRAYNUMBER(SoundViewMap));

SoundView::SoundView()
{
}

// http://www.gamedev.net/reference/articles/article2053.asp
SoundView::SoundView(FXComposite* parent)
  : FXVerticalFrame(parent, FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_CENTER_Y,
                    0, 0, 0, 0,
                    0, 0, 0, 0)
#ifdef USE_SDL
  ,music(0)
#endif
{
  FXHorizontalFrame* hbox = new FXHorizontalFrame(this, FRAME_RAISED|LAYOUT_FILL_X);

  play_button = new FXToggleButton(hbox, "\tPlay", "\tPause", Icon::play, Icon::pause, this, ID_PLAY);
  new FXButton(hbox, "\tStop", Icon::stop, this, ID_STOP);
  (new FXSlider(hbox, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y))->disable();

  text  = new FXText(this, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_WORDWRAP);
#ifndef __linux__
  input = new FXTextField(this, 0, this, ID_MCI_COMMAND,
                          TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_X|FRAME_SUNKEN);
#endif
}

SoundView::~SoundView()
{

}

void
SoundView::set_music(const std::string& filename, const std::string& desc)
{
  text->setText(desc.c_str());
  play_button->setState(1);

#ifdef USE_SDL
  if (music)
    Mix_FreeMusic(music);

  music = Mix_LoadMUS(filename.c_str());
  if(!music) {
    std::cout << "Mix_LoadMUS(\"" << filename << "\"): " <<  Mix_GetError() << std::endl;
  }

  if(Mix_PlayMusic(music, 1)==-1) {
    printf("Mix_PlayMusic: %s\n", Mix_GetError());
  }
#else
  if (1)
    {
      issue_msi_command("stop sound1");
      issue_msi_command("close sound1");

      std::string copy_file = get_exe_path() + "/tmp_sound.dat";
      file_copy(filename, copy_file);

      if (has_suffix(filename, ".wav"))
        issue_msi_command("open \"" + copy_file + "\" type waveaudio alias sound1");
      else
        issue_msi_command("open \"" + copy_file + "\" type mpegvideo alias sound1");
      issue_msi_command("play sound1 from 0");
    }
  else
    {
      if (!PlaySound(filename.c_str(), NULL,SND_FILENAME|SND_ASYNC))
        {
          std::cout << "Something went wrong file playing: " << filename << std::endl;
        }
    }
#endif
}

long
SoundView::onCmdPlay(FXObject*,FXSelector,void*)
{
#ifdef USE_SDL
  if (Mix_PausedMusic())
    Mix_ResumeMusic();
  else if (!Mix_PlayingMusic())
    Mix_PlayMusic(music, 1);
  else
    Mix_PauseMusic();

  return 1;
#else
   if (play_button->getState() == 1)
     {
	issue_msi_command("play sound1");
	play_button->setState(1);
     }
   else
     {
	issue_msi_command("pause sound1");
	play_button->setState(0);
     }

  return 1;
#endif
}

long
SoundView::onCmdStop(FXObject*,FXSelector,void*)
{
  play_button->setState(0);

#ifdef USE_SDL
  Mix_HaltMusic();
  //Mix_PauseMusic();
  return 1;
#else
   issue_msi_command("stop sound1");
   return 1;
#endif
}

long
SoundView::onCmdMciCommand(FXObject*,FXSelector,void*)
{
  issue_msi_command(input->getText().text());
  return 1;
}

void
SoundView::issue_msi_command(const std::string& cmd)
{ // http://www.gamedev.net/reference/articles/article2053.asp
#ifndef __linux__
  char buf[1024] = "";
  unsigned int err;

  std::cout << "msiCommand: " << cmd << std::endl;
  text->appendText("msiCommand: ");
  text->appendText(cmd.c_str());
  text->appendText("\n");

  err = mciSendString(cmd.c_str(), buf, 1024, 0);

  std::cout << "msiReturn: '" << buf << "'" << std::endl;
  text->appendText("msiReturn: ");
  text->appendText(buf);
  text->appendText("\n");

  if (err)
    {
      char err_msg[128];
      mciGetErrorString(err, err_msg, 128);

      std::cout << "SoundView:mciSendString: Error: " << err_msg << std::endl;
      text->appendText("msiError: ");
      text->appendText(err_msg);
      text->appendText("\n");
    }
  text->appendText("\n");
#endif
}

/* EOF */
