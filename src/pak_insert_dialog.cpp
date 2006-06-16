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
#include "pak_insert_dialog.hpp"

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

ProgressDialog::ProgressDialog(FXApp* app, DFModActivatorWindow* window_, const FXString& title)
  : FXDialogBox(app, title, DECOR_RESIZE|DECOR_TITLE|DECOR_BORDER,//|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,
                0, 0, 800, 0),
    // FXuint opts=DECOR_TITLE|DECOR_BORDER, FXint x=0,
    // FXint y=0, FXint w=0, FXint h=0, FXint pl=10, FXint
    // pr=10, FXint pt=10, FXint pb=10, FXint hs=4, FXint
    // vs=4)
    window(window_),
    thread(0)
{
  sig = new FXGUISignal(getApp(), this, ProgressDialog::ID_THREAD_UPDATE);

  log              = new FXText(this, NULL, 0, LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
  log->setVisibleRows(25);
  log->setEditable(FALSE);

  FXMatrix* matrix = new FXMatrix(this, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X);

  new FXLabel(matrix, "Current:", NULL, LAYOUT_RIGHT);
  current_progress = new FXProgressBar(matrix, NULL, 0, PROGRESSBAR_NORMAL|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN, 0, 0, 300, 20);

  new FXLabel(matrix, "Total:",   NULL, LAYOUT_RIGHT);
  total_progress   = new FXProgressBar(matrix, NULL, 0, PROGRESSBAR_NORMAL|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN, 0, 0, 300, 20);

  ok_button        = new FXButton(this, "Close", NULL, this, ID_HIDE, ICON_ABOVE_TEXT|FRAME_RAISED|LAYOUT_RIGHT|LAYOUT_FIX_WIDTH, 0, 0, 80, 0);
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
ProgressDialog::onThreadUpdate(FXObject*, FXSelector, void*)
{
  current_progress->setTotal(thread->getCurrentSize());;
  current_progress->setProgress(thread->getCurrentProgress());
  total_progress->setTotal(thread->getTotalSize());
  total_progress->setProgress(thread->getTotalProgress());
  log->setText(thread->getLog().c_str());
  log->makePositionVisible(log->getLength());

  if (thread->is_done())
    {
      ok_button->enable();
      while(thread->running());
      delete thread;
      thread = NULL;
      window->install_button->enable();
      window->uninstall_button->enable();
    }

  return 1;
}

/* EOF */
