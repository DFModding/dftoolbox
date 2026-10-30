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

#include "sound_view.hpp"

#include <iostream>

#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

#include "icons.hpp"
#include "system.hpp"

// http://www.gamedev.net/reference/articles/article2053.asp
SoundView::SoundView(QWidget* parent)
  : QWidget(parent),
    music(nullptr)
{
  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0, 0, 0, 0);

  QHBoxLayout* hbox = new QHBoxLayout;
  vbox->addLayout(hbox);

  // unchecked shows "play", checked (playing) shows "pause"
  QIcon play_icon;
  play_icon.addPixmap(Icon::play.pixmap(24, 24), QIcon::Normal, QIcon::Off);
  play_icon.addPixmap(Icon::pause.pixmap(24, 24), QIcon::Normal, QIcon::On);

  play_button = new QToolButton;
  play_button->setCheckable(true);
  play_button->setIcon(play_icon);
  play_button->setToolTip("Play");
  connect(play_button, &QToolButton::clicked, this, [this]{ on_play(); });
  hbox->addWidget(play_button);

  QToolButton* stop_button = new QToolButton;
  stop_button->setIcon(Icon::stop);
  stop_button->setToolTip("Stop");
  connect(stop_button, &QToolButton::clicked, this, [this]{ on_stop(); });
  hbox->addWidget(stop_button);

  QSlider* slider = new QSlider(Qt::Horizontal);
  slider->setEnabled(false);
  hbox->addWidget(slider, 1);

  text = new QPlainTextEdit;
  text->setReadOnly(true);
  text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
  vbox->addWidget(text, 1);
}

SoundView::~SoundView()
{
  if (music)
    Mix_FreeMusic(music);
}

void
SoundView::set_music(const std::string& filename, const std::string& desc)
{
  text->setPlainText(QString::fromStdString(desc));
  play_button->setChecked(true);

  if (music)
    Mix_FreeMusic(music);

  music = Mix_LoadMUS(filename.c_str());
  if(!music) {
    std::cout << "Mix_LoadMUS(\"" << filename << "\"): " <<  Mix_GetError() << std::endl;
    return;
  }

  if(Mix_PlayMusic(music, 1)==-1) {
    printf("Mix_PlayMusic: %s\n", Mix_GetError());
  }
}

void
SoundView::on_play()
{
  if (Mix_PausedMusic())
    Mix_ResumeMusic();
  else if (!Mix_PlayingMusic())
    Mix_PlayMusic(music, 1);
  else
    Mix_PauseMusic();
}

void
SoundView::on_stop()
{
  play_button->setChecked(false);

  Mix_HaltMusic();
}

/* EOF */
