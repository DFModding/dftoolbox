/*  $Id: image_view.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_IMAGE_VIEW_HPP
#define HEADER_IMAGE_VIEW_HPP

#include <string>

#include <QImage>
#include <QWidget>

class QLabel;
class QMainWindow;
class QPlainTextEdit;
class QScrollArea;
class QToolBar;

/** */
class ImageView : public QWidget
{
private:
  QToolBar*       toolbar;
  QScrollArea*    scrollarea;
  QLabel*         imagelabel;
  QPlainTextEdit* text;

  QImage image;
  int scale;

public:
  ImageView(QWidget* parent, QMainWindow* mainwindow);

  void set_image(const std::string& filename);
  void set_save_image(const std::string& filename);

  void cleanup();
  void set_scale(int scale_);

  void show_toolbar(bool t);
};

#endif

/* EOF */
