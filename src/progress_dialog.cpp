/*  $Id: pak_insert_dialog.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>
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

#include <iostream>
#include "dfmodactivator.hpp"
#include "pak_insert_thread.hpp"
#include "progress_logger.hpp"
#include "gui_progress_logger.hpp"
#include "progress_dialog.hpp"

FXDEFMAP(ProgressDialog) ProgressDialogMap[] = {
  FXMAPFUNC(SEL_COMMAND,  ProgressDialog::ID_HIDE,          ProgressDialog::onCmdHide),
  FXMAPFUNC(SEL_IO_READ,  ProgressDialog::ID_THREAD_UPDATE, ProgressDialog::onThreadUpdate),
};
FXIMPLEMENT(ProgressDialog, FXDialogBox, ProgressDialogMap, ARRAYNUMBER(ProgressDialogMap));

// FIXME: move this somewhere less global
FXGUISignal* sig;

ProgressDialog::ProgressDialog() 
{
}

ProgressDialog::ProgressDialog(FXApp* app, const FXString& title)
  : FXDialogBox(app, title, DECOR_RESIZE|DECOR_TITLE|DECOR_BORDER,//|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,
                0, 0, 800, 0),
    // FXuint opts=DECOR_TITLE|DECOR_BORDER, FXint x=0,
    // FXint y=0, FXint w=0, FXint h=0, FXint pl=10, FXint
    // pr=10, FXint pt=10, FXint pb=10, FXint hs=4, FXint
    // vs=4)
    thread(0)
{
  sig = new FXGUISignal(getApp(), this, ProgressDialog::ID_THREAD_UPDATE);

  vbox = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y, 
                             0, 0, 0, 0,
                             0, 0, 0, 0);
  
  FXPacker* frame  = new FXPacker(vbox, LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_SUNKEN, 0, 0, 0, 0,   0, 0, 0, 0);
  log              = new FXText(frame, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
  log->setVisibleRows(25);
  log->setEditable(FALSE);

  FXMatrix* matrix = new FXMatrix(vbox, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X);

  level3_label    = new FXLabel(matrix, "Current:", NULL, LAYOUT_RIGHT);
  level3_progress = new FXProgressBar(matrix, NULL, 0, PROGRESSBAR_NORMAL|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN, 0, 0, 300, 20);

  current_label    = new FXLabel(matrix, "Current:", NULL, LAYOUT_RIGHT);
  current_progress = new FXProgressBar(matrix, NULL, 0, PROGRESSBAR_NORMAL|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN, 0, 0, 300, 20);

  new FXLabel(matrix, "Total:",   NULL, LAYOUT_RIGHT);
  total_progress   = new FXProgressBar(matrix, NULL, 0, PROGRESSBAR_NORMAL|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN, 0, 0, 300, 20);

  ok_button        = new FXButton(vbox, "Close", NULL, this, ID_HIDE, ICON_ABOVE_TEXT|FRAME_RAISED|LAYOUT_RIGHT|LAYOUT_FIX_WIDTH, 0, 0, 80, 0);
  ok_button->disable();
}

ProgressDialog::~ProgressDialog() {

}

void
ProgressDialog::set_thread(PakInsertThread* thread_)
{
  thread = thread_;
  thread->start();
}

void
ProgressDialog::appendMessage(const std::string& msg) {
  log->appendText(msg.c_str());
  log->makePositionVisible(log->getLength());
}

long
ProgressDialog::onCmdToggleLongDesc(FXObject*,FXSelector,void*)
{
  hide();
  return 1;
}

long
ProgressDialog::onThreadUpdate(FXObject*, FXSelector, void* data)
{
  GUIProgressLogger* logger = static_cast<GUIProgressLogger*>(data);

  ProgressLogger* sublogger = logger->get_subtask();
  if (sublogger)
    {
      if (!current_progress->shown())
        {
          current_progress->show();
          current_label->show();
          vbox->recalc();
        }

      if (sublogger->get_subtask())
        {
          level3_progress->setProgress(sublogger->get_subtask()->get_collected_task_status());
          level3_progress->setTotal(sublogger->get_subtask()->get_collected_task_size());          
        }

      current_progress->setProgress(sublogger->get_collected_task_status());
      current_progress->setTotal(sublogger->get_collected_task_size());
    }
  else
    {
      if (current_progress->shown())
        {
          current_progress->hide();
          current_label->hide();
          vbox->recalc();
        }
    }

      total_progress->setTotal(logger->get_collected_task_size());
      total_progress->setProgress(logger->get_collected_task_status());

  log->appendText(logger->pop_log().c_str());
  log->makePositionVisible(log->getLength());
  
  if (logger->is_done())
    {
      ok_button->enable();
      //while(!logger->is_done()); // Child for thread to get done
      //delete thread;
      //thread = NULL;

      // FIXME:
      // window->install_button->enable();
      // window->uninstall_button->enable();
    }

  return 1;
}

/* EOF */
