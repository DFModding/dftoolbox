/*  $Id: searchbar.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "searchbar.hpp"

#include <iostream>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolButton>

#include "icons.hpp"

Searchbar::Searchbar(QWidget* parent, QPlainTextEdit* text_)
  : QFrame(parent),
    text(text_)
{
  setFrameStyle(QFrame::Box | QFrame::Plain);
  setLineWidth(1);

  QHBoxLayout* hbox = new QHBoxLayout(this);
  hbox->setContentsMargins(0, 0, 0, 0);

  hbox->addWidget(new QLabel("Search:"));

  search = new QLineEdit;
  connect(search, &QLineEdit::returnPressed, this, [this]{ on_search_forward(); });
  hbox->addWidget(search, 1);

  QToolButton* backward = new QToolButton;
  backward->setIcon(Icon::search_backward);
  backward->setToolTip("Search backward");
  backward->setAutoRaise(true);
  connect(backward, &QToolButton::clicked, this, [this]{ on_search_backward(); });
  hbox->addWidget(backward);

  QToolButton* forward = new QToolButton;
  forward->setIcon(Icon::search_forward);
  forward->setToolTip("Search forward");
  forward->setAutoRaise(true);
  connect(forward, &QToolButton::clicked, this, [this]{ on_search_forward(); });
  hbox->addWidget(forward);
}

void
Searchbar::find(bool backward)
{
  const QString needle = search->text();
  if (needle.isEmpty())
    return;

  QTextDocument::FindFlags flags = QTextDocument::FindCaseSensitively;
  if (backward)
    flags |= QTextDocument::FindBackward;

  if (text->find(needle, flags))
  {
    text->centerCursor();
    return;
  }

  // nothing found between cursor and the end, wrap around
  const QTextCursor old_cursor = text->textCursor();
  QTextCursor cursor = old_cursor;
  cursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
  text->setTextCursor(cursor);

  if (text->find(needle, flags))
  {
    text->centerCursor();
  }
  else
  {
    text->setTextCursor(old_cursor);
    std::cout << "Nothing found" << std::endl;
  }
}

void
Searchbar::on_search_forward()
{
  find(false);
}

void
Searchbar::on_search_backward()
{
  find(true);
}

/* EOF */
