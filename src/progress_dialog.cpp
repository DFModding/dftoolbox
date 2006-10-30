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

  label.resize(3);
  progress.resize(3);

  label[2]    = new FXLabel(matrix, "Subtask:", NULL, LAYOUT_RIGHT);
  progress[2] = new FXProgressBar(matrix, NULL, 0, PROGRESSBAR_NORMAL|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN, 
                                  0, 0, 300, 20);

  label[1]    = new FXLabel(matrix, "Task:", NULL, LAYOUT_RIGHT);
  progress[1] = new FXProgressBar(matrix, NULL, 0, PROGRESSBAR_NORMAL|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN, 
                                  0, 0, 300, 20);

  label[0]    = new FXLabel(matrix, "Total:", NULL, LAYOUT_RIGHT);
  progress[0] = new FXProgressBar(matrix, NULL, 0, PROGRESSBAR_NORMAL|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN, 
                                  0, 0, 300, 20);
  
  ok_button        = new FXButton(vbox, "Close", NULL, this, ID_HIDE, ICON_ABOVE_TEXT|FRAME_RAISED|LAYOUT_RIGHT|LAYOUT_FIX_WIDTH, 0, 0, 80, 0);
  ok_button->disable();
}

ProgressDialog::~ProgressDialog() {

}

long
ProgressDialog::onCmdToggleLongDesc(FXObject*,FXSelector,void*)
{
  hide();
  return 1;
}

void
ProgressDialog::relayout(int bars)
{
  std::cout << "ProgressDialog::relayout()" << std::endl;
  log->setText("");
  for(int i = 0; i < int(progress.size()); ++i)
    {
      if (i < bars)
        {
          progress[i]->show();
          label[i]->show();
        }
      else
        {
          progress[i]->hide();
          label[i]->hide();
        }
    }
  vbox->recalc(); 
}

void
ProgressDialog::set_thread(FXThread* t)
{
  thread = t;
}

long
ProgressDialog::onThreadUpdate(FXObject*, FXSelector, void* data)
{
  GUIProgressLogger* logger = static_cast<GUIProgressLogger*>(data);

  int i = 0;
  for(ProgressLogger* l = logger; l != NULL && i < int(progress.size()); l = l->get_subtask())
    {
      progress[i]->setTotal(l->get_collected_task_size());
      progress[i]->setProgress(l->get_collected_task_status());
      i += 1;
    }
  
  log->appendText(logger->pop_log().c_str());

  //std::cout << log->getYPosition() << " "  << log->getContentHeight()  << std::endl;
  log->makePositionVisible(log->getLength());
  
  if (logger->is_done())
    {
      ok_button->enable();
      
      // Kill thread 
      if (thread){
        thread->join();
        delete thread;
        thread = 0;
      }

      // FIXME:
      // window->install_button->enable();
      // window->uninstall_button->enable();
    }

  return 1;
}

/* EOF */
