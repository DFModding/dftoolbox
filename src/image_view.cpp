/*  $Id: image_view.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "image_view.hpp"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cmath>

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QToolBar>
#include <QVBoxLayout>

#include "icons.hpp"
#include "system.hpp"
#include "ui_util.hpp"

#define SCALE_LIMIT 4

ImageView::ImageView(QWidget* parent, QMainWindow* mainwindow)
  : QWidget(parent),
    scale(0)
{
  toolbar = new QToolBar;
  mainwindow->addToolBar(toolbar);

  connect(add_tool_action(toolbar, Icon::zoom_1_1, "Zoom 1:1"),
          &QAction::triggered, this, [this]{ set_scale(0); });
  connect(add_tool_action(toolbar, Icon::zoom_in, "Zoom in"),
          &QAction::triggered, this, [this]{ set_scale(scale + 1); });
  connect(add_tool_action(toolbar, Icon::zoom_out, "Zoom out"),
          &QAction::triggered, this, [this]{ set_scale(scale - 1); });
  toolbar->addSeparator();
  add_tool_action(toolbar, Icon::grayscale, "Grayscale")->setEnabled(false);
  add_tool_action(toolbar, Icon::normalmap, "Normalmap")->setEnabled(false);

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);

  // Text view for info on file format and stuff
  text = new QPlainTextEdit("Hello World");
  text->hide();
  vbox->addWidget(text);

  imagelabel = new QLabel;
  scrollarea = new QScrollArea;
  scrollarea->setWidget(imagelabel);
  scrollarea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  vbox->addWidget(scrollarea, 1);
}

void
ImageView::set_image(const std::string& filename)
{
  cleanup();

  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    throw std::runtime_error("Error: Couldn't open " + filename);

  image = read_dds_image(in);
  std::cout << "DDS: " << image.width() << "x" << image.height() << std::endl;
  in.close();

  imagelabel->setPixmap(QPixmap::fromImage(image));
  imagelabel->adjustSize();

  set_scale(scale);
}

void
ImageView::cleanup()
{
  imagelabel->clear();
  image = QImage();
}

void
ImageView::set_save_image(const std::string& filename)
{
  cleanup();

  image = read_save_image(filename);
  imagelabel->setPixmap(QPixmap::fromImage(image));
  imagelabel->adjustSize();

  set_scale(scale);
}

void
ImageView::set_scale(int scale_)
{
  if (image.isNull())
    return;

  if (scale_ >= -SCALE_LIMIT && scale_ <= SCALE_LIMIT)
    {
      float factor = powf(2, scale_);
      int width     = image.width();
      int height    = image.height();

      if (width*height * factor <= 1048*1048*2 && width*height*factor > 8*8)
        {
          scale = scale_;

          if (scale == 0)
            {
              imagelabel->setPixmap(QPixmap::fromImage(image));
            }
          else
            {
              imagelabel->setPixmap(QPixmap::fromImage(
                image.scaled(static_cast<int>(width*factor), static_cast<int>(height*factor),
                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            }
          imagelabel->adjustSize();
        }
    }
  else
    {
      std::cout << "Ignoring scale request" << std::endl;
    }
}

void
ImageView::show_toolbar(bool t)
{
  toolbar->setVisible(t);
}

/* EOF */
