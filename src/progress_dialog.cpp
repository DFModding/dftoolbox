/*  $Id: pak_insert_dialog.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "progress_dialog.hpp"

#include <algorithm>
#include <iostream>

#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QTextCursor>
#include <QVBoxLayout>

#include "gui_progress_logger.hpp"
#include "progress_logger.hpp"

ProgressDialog::ProgressDialog(QWidget* parent, const QString& title)
  : QDialog(parent),
    ok_button(nullptr)
{
  setWindowTitle(title);

  QVBoxLayout* vbox = new QVBoxLayout(this);

  log = new QPlainTextEdit;
  log->setReadOnly(true);
  log->setMinimumHeight(25 * log->fontMetrics().lineSpacing() +
                        2 * log->frameWidth() + 2 * static_cast<int>(log->document()->documentMargin()));
  vbox->addWidget(log, 1);

  QGridLayout* grid = new QGridLayout;
  grid->setColumnStretch(1, 1);
  vbox->addLayout(grid);

  label.resize(3);
  progress.resize(3);

  // top to bottom: Subtask, Task, Total; index 0 is the Total
  const char* names[3] = { "Total:", "Task:", "Subtask:" };
  for(int i = 2; i >= 0; --i)
  {
    const int row = 2 - i;
    label[i] = new QLabel(names[i]);
    progress[i] = new QProgressBar;
    progress[i]->setTextVisible(false);
    progress[i]->setRange(0, 1);
    progress[i]->setValue(0);
    progress[i]->setFixedHeight(20);
    grid->addWidget(label[i], row, 0, Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(progress[i], row, 1);
  }

  ok_button = new QPushButton("Close");
  ok_button->setFixedWidth(80);
  ok_button->setEnabled(false);
  connect(ok_button, &QPushButton::clicked, this, [this]{ hide(); });
  vbox->addWidget(ok_button, 0, Qt::AlignRight);

  resize(800, sizeHint().height());
}

ProgressDialog::~ProgressDialog()
{
  // Don't tear down the dialog while the worker is still using the logger
  if (thread.joinable())
    thread.join();
}

void
ProgressDialog::relayout(int bars)
{
  std::cout << "ProgressDialog::relayout()" << std::endl;
  log->clear();
  for(int i = 0; i < int(progress.size()); ++i)
  {
    progress[i]->setVisible(i < bars);
    label[i]->setVisible(i < bars);
  }
  layout()->activate();
}

void
ProgressDialog::set_thread(std::thread t)
{
  if (thread.joinable())
    thread.join();
  thread = std::move(t);
}

void
ProgressDialog::on_thread_update(GUIProgressLogger* logger)
{
  int i = 0;
  for(ProgressLogger* l = logger; l != nullptr && i < int(progress.size()); l = l->get_subtask())
  {
    // QProgressBar is int-ranged and treats a maximum of 0 as "busy"
    const int total  = std::max(1, l->get_collected_task_size());
    const int status = std::min(std::max(0, l->get_collected_task_status()), total);
    progress[i]->setMaximum(total);
    progress[i]->setValue(status);
    i += 1;
  }

  const std::string text = logger->pop_log();
  if (!text.empty())
  {
    log->moveCursor(QTextCursor::End);
    log->insertPlainText(QString::fromStdString(text));
    log->moveCursor(QTextCursor::End);
    log->ensureCursorVisible();
  }

  if (logger->is_done())
  {
    ok_button->setEnabled(true);

    // The worker is finished, so this returns immediately
    if (thread.joinable())
      thread.join();

    // FIXME:
    // window->install_button->enable();
    // window->uninstall_button->enable();
  }
}

/* EOF */
