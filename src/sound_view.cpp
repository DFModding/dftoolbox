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

#include "sound_view.hpp"

#include <iostream>

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QBuffer>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QPlainTextEdit>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

#include "icons.hpp"
#include "system.hpp"

static QString format_time(qint64 msec)
{
  const qint64 seconds = msec / 1000;
  return QString("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QChar('0'));
}

// http://www.gamedev.net/reference/articles/article2053.asp
SoundView::SoundView(QWidget* parent)
  : QWidget(parent),
    player(new QMediaPlayer(this)),
    audio_output(new QAudioOutput(this)),
    buffer(new QBuffer(this))
{
  player->setAudioOutput(audio_output);
  connect(player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state)
          {
            play_button->setChecked(state == QMediaPlayer::PlayingState);
          });
  connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 position)
          {
            update_time(position, player->duration());
          });
  connect(player, &QMediaPlayer::durationChanged, this, [this](qint64 duration)
          {
            update_time(player->position(), duration);
          });
  connect(player, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error, const QString& message)
          {
            std::cerr << "warning: can't play sound: " << message.toStdString() << std::endl;
          });

  const QAudioDevice device = QMediaDevices::defaultAudioOutput();
  if (device.isNull())
    {
      std::cerr << "warning: no audio output device, sounds can't be played" << std::endl;
    }
  else
    {
      const QAudioFormat format = device.preferredFormat();
      std::cerr << "audio: " << device.description().toStdString()
                << ", " << format.sampleRate() << " Hz"
                << ", " << format.channelCount() << " channels" << std::endl;
    }

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

  slider = new QSlider(Qt::Horizontal);
  slider->setEnabled(false);
  connect(slider, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);
  hbox->addWidget(slider, 1);

  time_label = new QLabel;
  hbox->addWidget(time_label);
  update_time(0, 0);

  text = new QPlainTextEdit;
  text->setReadOnly(true);
  text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
  vbox->addWidget(text, 1);
}

void
SoundView::set_music(const std::string& filename, const std::string& desc)
{
  text->setPlainText(QString::fromStdString(desc));

  player->setSource(QUrl());
  buffer->close();

  QFile file(QString::fromStdString(filename));
  if (!file.open(QIODevice::ReadOnly))
    {
      std::cerr << "warning: can't play sound: couldn't open " << filename << std::endl;
      return;
    }
  buffer->setData(file.readAll());
  buffer->open(QIODevice::ReadOnly);

  player->setSourceDevice(buffer, QUrl::fromLocalFile(file.fileName()));
  player->play();
}

void
SoundView::update_time(qint64 position, qint64 duration)
{
  slider->setEnabled(duration > 0);
  slider->setMaximum(static_cast<int>(duration));

  // don't fight with the user when the slider is getting dragged
  if (!slider->isSliderDown())
    slider->setValue(static_cast<int>(position));

  time_label->setText(format_time(position) + " / " + format_time(duration));
}

void
SoundView::on_play()
{
  if (player->playbackState() == QMediaPlayer::PlayingState)
    player->pause();
  else
    player->play();
}

void
SoundView::on_stop()
{
  player->stop();
}

/* EOF */
