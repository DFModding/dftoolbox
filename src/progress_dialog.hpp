/*  $Id: pak_insert_dialog.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_PROGRESS_DIALOG_HPP
#define HEADER_PROGRESS_DIALOG_HPP

#include <string>
#include <thread>
#include <vector>

#include <QDialog>

class GUIProgressLogger;
class QLabel;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;

class ProgressDialog : public QDialog
{
private:
  QPlainTextEdit* log;

  std::vector<QLabel*>       label;
  std::vector<QProgressBar*> progress;

  QPushButton* ok_button;
  std::thread thread;

public:
  ProgressDialog(QWidget* parent, const QString& title);
  ~ProgressDialog();

  /** Update the dialog from the state of \a logger, must be called
      from the GUI thread (GUIProgressLogger::sync() takes care of
      that). Once the logger is done the worker thread gets joined. */
  void on_thread_update(GUIProgressLogger* logger);

  /** Relayout the widget to show \a bars number of progressbars */
  void relayout(int bars);

  /** Hand over the worker thread, the dialog joins it once the
      logger reports that it is done. */
  void set_thread(std::thread t);
};

#endif

/* EOF */
