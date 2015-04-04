/*  $Id: dialog_view.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <stdint.h>
#include <sstream>
#include "dialog_view.hpp"
#include "dftoolbox.hpp"
#include "searchbar.hpp"

FXDEFMAP(DialogView) DialogViewMap[] = {
  FXMAPFUNC(SEL_COMMAND,         DialogView::ID_LANGUAGE_SWITCH,   DialogView::onCmdLanguageSwitch),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS, DialogView::ID_HYPERLINK,         DialogView::onCmdHyperlink),
  FXMAPFUNC(SEL_COMMAND,         DialogView::ID_SAVE_TEXT,         DialogView::onCmdSaveText),
};

// Object implementation
FXIMPLEMENT(DialogView, FXVerticalFrame, DialogViewMap, ARRAYNUMBER(DialogViewMap));

DialogView::DialogView(FXComposite* parent, FXComposite* dock,  DFToolBoxWindow* dftoolbox_)
  : FXVerticalFrame(parent, FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y,
                    0, 0, 0, 0,
                    0, 0, 0, 0),
    dftoolbox(dftoolbox_)
{
  toolbar = new FXToolBar(dock, LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  // FIXME: add text wrap buttons
  new FXButton(toolbar, "\tSave text\tSave text", Icon::save_file,
               this, ID_SAVE_TEXT, BUTTON_TOOLBAR|FRAME_RAISED);

  langbox = new FXListBox(toolbar, this, ID_LANGUAGE_SWITCH, LAYOUT_CENTER_Y);

  text = new FXText(this, this, ID_HYPERLINK, LAYOUT_FILL_X|LAYOUT_FILL_Y);
  //text = new FXText(this, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
  text->setEditable(FALSE);
  text->setTextStyle(text->getTextStyle() | TEXT_WORDWRAP);
  //text->setStyled(TRUE);

  new Searchbar(this, text);
}

void
DialogView::set_dialog(const std::string& filename)
{
  dialogs.clear();
  read_dialogs(filename, dialogs);

  langbox->clearItems();
  int c = 0;
  langbox->appendItem("All languages");
  for(Dialogs::iterator i = dialogs.begin(); i != dialogs.end(); ++i)
    {
      langbox->appendItem((i->lang_code + " - " + i->language).c_str(), NULL,
                          reinterpret_cast<void*>(c));
      c += 1;
    }
  langbox->setNumVisible(std::min(langbox->getNumItems(), 20));
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

          // FIXME: Can't use this since appendText is *HORRIBLY* slow
          //text->appendText("Hello World");
          //text->appendText(FXString().format("%d) ", i));
          //text->appendText(FXString().format("dialog:%d\n", dialogs[id].entries[i].id));//, FXText::STYLE_BOLD);

          //text->appendText(&*dialogs[id].texts.begin() + dialogs[id].entries[i].offset);
          //text->appendText("\n\n");
          //if (i % 10 == 0)
          //  std::cout << i << "/" << dialogs[id].entries.size() << "                     \r" << std::endl;
        }
      text->setText(str.str().c_str());
    }
  else if (dialogs.size() >= 1)
    {
      std::ostringstream str;
      for(unsigned int i = 0; i < dialogs[0].entries.size(); ++i)
        {
          str << i << ")" << std::endl;

          for(unsigned int j = 0; j < dialogs.size(); ++j)
            {
              str << std::endl;
              str << dialogs[j].language << ": ";
              str << "[[dialog:" << j << ":" << dialogs[j].entries[i].id << "]]";
              //str << "                ";
              //str << (dialogs[j].entries[i].id & 0xffff);
              str << std::endl;
              str << &*dialogs[j].texts.begin() + dialogs[j].entries[i].offset << std::endl;
            }
          str << "___________________________________________________________________" << std::endl;
        }
      text->setText(str.str().c_str());
    }
}

long
DialogView::onCmdLanguageSwitch(FXObject*,FXSelector,void* data)
{
  int lang_id = reinterpret_cast<intptr_t>(data);
  set_language(lang_id);
  return 1;
}

bool is_hyperlink(const FXString& link)
{
  if (link.length() <= 4)
    {
      return false;
    }
  else if (link[0] == '[' && link[1] == '[' &&
           link[link.length()-1] == ']' && link[link.length()-2] == ']')
    {
      return true;
    }
  else
    {
      return false;
    }
}

long
DialogView::onCmdHyperlink(FXObject*,FXSelector,void* data)
{
  FXEvent* event = static_cast<FXEvent*>(data);

  FXint pos   = text->getPosAt(event->win_x,event->win_y);

  const char* delim = text->getDelimiters();

  text->setDelimiters(" \t\n");

  FXint start = text->wordStart(pos);
  FXint end   = text->wordEnd(pos);

  text->setDelimiters(delim);

  FXString link;
  text->extractText(link, start, end - start);

  if (is_hyperlink(link))
    {
      FXint dialog_id;
      FXint lang_code;

      if (link.scan("[[dialog:%d:%d]]", &lang_code, &dialog_id) == 2)
        {
          std::cout << "Dialog2: " << dialog_id << " " << lang_code << std::endl;
          dftoolbox->play_dialog(dialog_id, dialogs[lang_code].lang_code);
        }
      else if (link.scan("[[dialog:%d]]", &dialog_id) == 1)
        {
          std::cout << "Dialog1: " << dialog_id << std::endl;
          dftoolbox->play_dialog(dialog_id, dialogs[langbox->getCurrentItem()-1].lang_code);
        }
      return 1;
    }
  else
    {
      return 0;
    }
}

long
DialogView::onCmdSaveText(FXObject*,FXSelector,void*)
{
  FXFileDialog savedialog(this,tr("Save Document"));
  FXString file;
  savedialog.setSelectMode(SELECTFILE_ANY);
  // savedialog.setPatternList(getPatterns());
  // savedialog.setCurrentPattern(getCurrentPattern());
  // savedialog.setFilename(file);
  if(savedialog.execute())
    {
      //setCurrentPattern(savedialog.getCurrentPattern());
      file=savedialog.getFilename();
      if(FXStat::exists(file))
        {
          if(MBOX_CLICKED_NO==FXMessageBox::question(this,MBOX_YES_NO, "Overwrite Document",
                                                     "Overwrite existing document: %s?",file.text()))
            return 1;
        }

      std::ofstream out(file.text());
      out << text->getText().text();
      out.close();
    }

  return 1;
}

/* EOF */
