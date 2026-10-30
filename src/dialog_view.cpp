/*  $Id: dialog_view.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "dialog_view.hpp"

#include <iostream>
#include <sstream>

#include <QAction>
#include <QComboBox>
#include <QEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QToolBar>
#include <QVBoxLayout>

#include "dftoolbox.hpp"
#include "icons.hpp"
#include "searchbar.hpp"
#include "ui_util.hpp"

DialogView::DialogView(QWidget* parent, QMainWindow* mainwindow, DFToolBoxWindow* dftoolbox_)
  : QWidget(parent),
    dftoolbox(dftoolbox_)
{
  toolbar = new QToolBar;
  mainwindow->addToolBar(toolbar);

  // FIXME: add text wrap buttons
  connect(add_tool_action(toolbar, Icon::save_file, "Save text", "Save text"),
          &QAction::triggered, this, [this]{ on_save_text(); });

  langbox = new QComboBox;
  connect(langbox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int index){ on_language_switch(index); });
  toolbar->addWidget(langbox);

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);

  text = new QPlainTextEdit;
  text->setReadOnly(true);
  text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
  text->viewport()->installEventFilter(this);
  vbox->addWidget(text, 1);

  vbox->addWidget(new Searchbar(this, text));
}

void
DialogView::set_text(const std::string& str)
{
  raw_text = str;
  text->setPlainText(QString::fromStdString(str));
}

void
DialogView::set_dialog(const std::string& filename)
{
  dialogs.clear();
  read_dialogs(filename, dialogs);

  langbox->blockSignals(true);
  langbox->clear();
  langbox->addItem("All languages");
  for(Dialogs::iterator i = dialogs.begin(); i != dialogs.end(); ++i)
    {
      langbox->addItem(QString::fromStdString(i->lang_code + " - " + i->language));
    }
  langbox->setMaxVisibleItems(std::min(langbox->count(), 20));
  langbox->setCurrentIndex(0);
  langbox->blockSignals(false);
  set_language(0);
}

void
DialogView::set_language(int id)
{
  std::cout << "Filling text buffer" << std::endl;

  if (id >= 1 && id < int(dialogs.size()+1))
    {
      id -= 1;

      std::ostringstream str;
      for(unsigned int i = 0; i < dialogs[id].entries.size(); ++i)
        {
          str << i << ") [[dialog:" << dialogs[id].entries[i].id << "]]" << std::endl;
          str << &*dialogs[id].texts.begin() + dialogs[id].entries[i].offset << std::endl << std::endl;
        }
      set_text(str.str());
    }
  else if (dialogs.size() >= 1)
    {
      set_text(dialogs_to_text(dialogs));
    }
}

void
DialogView::on_language_switch(int lang_id)
{
  set_language(lang_id);
}

void
DialogView::play(int dialog_id, const std::string& lang_code)
{
  // play_dialog() throws if the speech data of the dialog isn't known,
  // which must not escape from the event handler
  try {
    dftoolbox->play_dialog(dialog_id, lang_code);
  } catch(std::exception& err) {
    std::cout << "Error: " << err.what() << std::endl;
    QMessageBox::warning(this, "Couldn't play dialog",
                         QString("Couldn't play dialog %1: %2").arg(dialog_id).arg(err.what()));
  }
}

bool
DialogView::on_hyperlink(const QPoint& pos)
{
  // Find the whitespace delimited word below the mouse cursor
  const QTextCursor cursor = text->cursorForPosition(pos);
  const QString block = cursor.block().text();
  int start = cursor.positionInBlock();
  int end   = start;

  auto is_delimiter = [](QChar c) { return c == ' ' || c == '\t' || c == '\n'; };
  while (start > 0 && !is_delimiter(block[start - 1]))
    start -= 1;
  while (end < block.size() && !is_delimiter(block[end]))
    end += 1;

  const QString link = block.mid(start, end - start);

  static const QRegularExpression link2("^\\[\\[dialog:(-?\\d+):(-?\\d+)\\]\\]$");
  static const QRegularExpression link1("^\\[\\[dialog:(-?\\d+)\\]\\]$");

  QRegularExpressionMatch match;
  if ((match = link2.match(link)).hasMatch())
    {
      const int lang_code = static_cast<int>(match.captured(1).toLongLong());
      const int dialog_id = static_cast<int>(match.captured(2).toLongLong());
      std::cout << "Dialog2: " << dialog_id << " " << lang_code << std::endl;
      if (lang_code >= 0 && lang_code < int(dialogs.size()))
        play(dialog_id, dialogs[lang_code].lang_code);
      return true;
    }
  else if ((match = link1.match(link)).hasMatch())
    {
      const int dialog_id = static_cast<int>(match.captured(1).toLongLong());
      std::cout << "Dialog1: " << dialog_id << std::endl;
      const int lang = langbox->currentIndex() - 1;
      if (lang >= 0 && lang < int(dialogs.size()))
        play(dialog_id, dialogs[lang].lang_code);
      return true;
    }
  else
    {
      return false;
    }
}

bool
DialogView::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == text->viewport() && event->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
      if (mouse_event->button() == Qt::LeftButton)
        return on_hyperlink(mouse_event->position().toPoint());
    }
  return QWidget::eventFilter(obj, event);
}

void
DialogView::on_save_text()
{
  save_text_file(this, raw_text);
}

void
DialogView::show_toolbar(bool t)
{
  toolbar->setVisible(t);
}

/* EOF */
