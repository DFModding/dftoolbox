/*  $Id: shark_view.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "shark_view.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <QAction>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QVBoxLayout>

#include "icons.hpp"
#include "shark3d.hpp"
#include "system.hpp"
#include "searchbar.hpp"
#include "ui_util.hpp"

SharkView::SharkView(QWidget* parent, QMainWindow* mainwindow)
  : QWidget(parent)
{
  toolbar = new QToolBar;
  mainwindow->addToolBar(toolbar);

  connect(add_tool_action(toolbar, Icon::indent, "Toggle Word Wrap", "Toggle Word Wrap"),
          &QAction::triggered, this, [this]{ on_toggle_wrap(); });
  connect(add_tool_action(toolbar, Icon::save_file, "Save Shark3D as text", "Save Shark3D as text"),
          &QAction::triggered, this, [this]{ on_save_text(); });

  model_action = toolbar->addAction("3D");
  model_action->setVisible(false);
  connect(model_action, &QAction::triggered, this, [this]{ if (show_model) show_model(); });

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);

  text = new QPlainTextEdit;
  text->setReadOnly(true);
  text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
  vbox->addWidget(text, 1);

  vbox->addWidget(new Searchbar(this, text));
}

void
SharkView::set_show_model(std::function<void ()> callback)
{
  show_model = callback;
}

void
SharkView::show_model_action(bool t)
{
  model_action->setVisible(t);
}

void
SharkView::set_text(const std::string& str)
{
  model_action->setVisible(false);
  raw_text = str;
  text->setPlainText(QString::fromStdString(str));
}

void
SharkView::set_shark(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
    {
      throw std::runtime_error("Error: Couldn't open " + filename);
    }
  else
    {
      Shark3D* shark = Shark3D::parse_binary(in);
      in.close();

      std::ostringstream str;
      shark->write_text(str);
      set_text(str.str());
      delete shark;
    }
}

void
SharkView::on_toggle_wrap()
{
  if (text->lineWrapMode() == QPlainTextEdit::NoWrap)
    text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
  else
    text->setLineWrapMode(QPlainTextEdit::NoWrap);
}

void
SharkView::on_save_text()
{
  save_text_file(this, raw_text);
}

void
SharkView::show_toolbar(bool t)
{
  toolbar->setVisible(t);
}

/* EOF */
