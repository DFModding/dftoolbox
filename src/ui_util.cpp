/*  ui_util.cpp
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

#include "ui_util.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>

#include <QAction>
#include <QFileDialog>
#include <QIcon>
#include <QToolBar>

#include "dds.hpp"

QAction*
add_tool_action(QToolBar* toolbar, const QIcon& icon,
                const QString& tip, const QString& status)
{
  QAction* action = toolbar->addAction(icon, tip);
  action->setToolTip(tip);
  if (!status.isEmpty())
    action->setStatusTip(status);
  return action;
}

bool
save_text_file(QWidget* parent, const std::string& content)
{
  // the dialog asks about overwriting existing files itself
  const QString file = QFileDialog::getSaveFileName(parent, "Save Document");
  if (file.isEmpty())
    return false;

  std::ofstream out(file.toStdString().c_str());
  out << content;
  out.close();
  return true;
}

QImage
read_dds_image(std::istream& in)
{
  DDS dds(in);

  // DDS::get_data() is RGBA, one byte per channel; copy() so that the
  // image doesn't depend on the lifetime of the DDS data
  const std::vector<char>& data = dds.get_data();
  if (data.size() < size_t(dds.width) * dds.height * 4)
    throw std::runtime_error("DDS: not enough image data");

  return QImage(reinterpret_cast<const uchar*>(data.data()), dds.width, dds.height,
                dds.width * 4, QImage::Format_RGBA8888).copy();
}

QImage
read_save_image(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    throw std::runtime_error("Couldn't open: " + filename);

  // Extract screenshot, the last 256*256 RGBA pixels of the file
  std::vector<uchar> buffer(256*256*4);
  in.seekg(-(256*256*4), std::ios::end);
  in.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
  if (!in)
    throw std::runtime_error("Couldn't read screenshot from: " + filename);

  return QImage(buffer.data(), 256, 256, 256*4, QImage::Format_RGBA8888).copy();
}

/* EOF */
