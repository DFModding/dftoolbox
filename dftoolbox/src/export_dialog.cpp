/*  $Id: export_dialog.cpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <algorithm>
#include "icons.hpp"
#include "dftoolbox.hpp"
#include "system.hpp"
#include "export_dialog.hpp"

FXDEFMAP(ExportDialog) ExportDialogMap[] = {
  FXMAPFUNC(SEL_COMMAND,       ExportDialog::ID_EXPORT,        ExportDialog::onCmdExport),
  FXMAPFUNC(SEL_COMMAND,       ExportDialog::ID_CANCEL,        ExportDialog::onCmdCancel),  
  FXMAPFUNC(SEL_COMMAND,       ExportDialog::ID_BROWSE,        ExportDialog::onCmdBrowse),  
};

// Object implementation
FXIMPLEMENT(ExportDialog, FXDialogBox, ExportDialogMap, ARRAYNUMBER(ExportDialogMap));

ExportDialog::ExportDialog(DFToolBoxWindow* parent_)
  : FXDialogBox(parent_->getApp(),
                "Export Dialog",
                DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE|DECOR_CLOSE,
                0, 0, 640, 0),
    parent(parent_)
{
  FXVerticalFrame* vbox = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y,
                                              0,0,0,0, 0,0,0,0);

  FXMatrix* matrix = new FXMatrix(vbox, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X);
  
  new FXLabel(matrix, "Selection:");
  filename = new FXText(matrix, NULL, 0, LAYOUT_FILL_COLUMN|LAYOUT_FILL_X|FRAME_SUNKEN|LAYOUT_CENTER_Y);
  filename->setEditable(FALSE);
  new FXLabel(matrix, "Export to:");
  
  {
    FXHorizontalFrame* hbox = new FXHorizontalFrame(matrix, FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 0,0,0,0);
    pathname = new FXTextField(hbox,0, NULL, 0, TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_X|FRAME_SUNKEN|LAYOUT_CENTER_Y);
    pathname->setText(get_exe_path().c_str());
    new FXButton(hbox, "Browse", Icon::folder_open, this, ID_BROWSE);
  }
  {
    FXHorizontalFrame* hbox = new FXHorizontalFrame(vbox, FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 0,0,0,0);
    new FXLabel(hbox, "Convert Images to: ");
    FXListBox* typebox = new FXListBox(hbox, NULL,0, LAYOUT_CENTER_Y|FRAME_SUNKEN);
  
    typebox->appendItem("Native");
    typebox->appendItem("Targa (.tga)");
    typebox->setNumVisible(std::min(typebox->getNumItems(), 20));
    typebox->disable();
  }
  {
    FXHorizontalFrame* hbox = new FXHorizontalFrame(vbox, FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 0,0,0,0);
    new FXLabel(hbox, "Convert Shark3D to: ");
    FXListBox* typebox = new FXListBox(hbox, NULL,0, LAYOUT_CENTER_Y|FRAME_SUNKEN);
  
    typebox->appendItem("Text");
    typebox->appendItem("Native");
    typebox->setNumVisible(std::min(typebox->getNumItems(), 20));
    typebox->disable();
  }
  {
    FXHorizontalFrame* hbox = new FXHorizontalFrame(vbox, FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 0,0,0,0);
    new FXLabel(hbox, "Filter: \tOnly export files matching the expression");
    filter = new FXTextField(hbox,0, NULL, 0, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|LAYOUT_CENTER_Y);
    filter->setNumColumns(16);
    filter->disable();
  }

  preserve_path = new FXCheckButton(vbox, "preserve directory structure on export");
  fix_extension = new FXCheckButton(vbox, "append proper file extension where necessary (.png -> .png.dds)");

  FXHorizontalFrame* buttonbar = new FXHorizontalFrame(vbox, FRAME_NONE|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH|LAYOUT_SIDE_BOTTOM);
  new FXButton(buttonbar, "Export", Icon::export_file, this, ID_EXPORT, LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK|TEXT_AFTER_ICON);
  new FXButton(buttonbar, "Cancel", Icon::cancel, this, ID_CANCEL, LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK|TEXT_AFTER_ICON);
}

void
ExportDialog::set_filename(const std::string& filename_)
{
  filename->setText(filename_.c_str());
  filename->setVisibleRows(filename->getNumRows());
}

void
ExportDialog::set_file_entry(DreamfallFileEntry* entry)
{
  filename->setText(entry->get_url().c_str());
  entries.push_back(entry);
}

void
ExportDialog::set_file_entries(const std::vector<DreamfallFileEntry*>& selection)
{
  filename->setText("");
  for(std::vector<DreamfallFileEntry*>::const_iterator i = selection.begin(); i != selection.end(); ++i)
    {
      filename->appendText((*i)->get_url().c_str());
      if (i+1 != selection.end())
        filename->appendText("\n");

      entries.push_back(*i);
    }
  filename->setVisibleRows(std::min(filename->getNumRows(), 20));
}

long 
ExportDialog::onCmdExport(FXObject*,FXSelector,void*)
{
  hide(); // FIXME: is there a 'destroy'?

  parent->export_files(entries, pathname->getText().text(), 
                       preserve_path->getCheck(), fix_extension->getCheck(),
                       true);

  return 1;  
}

long 
ExportDialog::onCmdCancel(FXObject*,FXSelector,void*)
{
  hide(); // FIXME: is there a 'destroy'?
  return 1;
}

long
ExportDialog::onCmdBrowse(FXObject*,FXSelector,void*)
{
  FXDirDialog open(this,"Select the Dreamfall directory");
  open.showFiles(FALSE);
  open.setDirectory(pathname->getText());

  if(open.execute()) {
    pathname->setText(open.getDirectory());
  }
  return 1;
}

/* EOF */
