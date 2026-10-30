/*  $Id: sound_view.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 3
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HEADER_SOUND_VIEW_HPP
#define HEADER_SOUND_VIEW_HPP

#include <string>

#include <QWidget>

class QAudioOutput;
class QBuffer;
class QMediaPlayer;
class QPlainTextEdit;
class QToolButton;

class SoundView : public QWidget
{
private:
  QMediaPlayer* player;
  QAudioOutput* audio_output;
  /** Holds the sound in memory: the file is a temporary that gets
      replaced by the next selection, which Windows refuses while the
      player has it open */
  QBuffer* buffer;
  QToolButton* play_button;
  QPlainTextEdit* text;

public:
  SoundView(QWidget* parent);

  void set_music(const std::string& filename, const std::string& desc);

  void on_play();
  void on_stop();

  void show_toolbar(bool t) { }
};

#endif

/* EOF */
