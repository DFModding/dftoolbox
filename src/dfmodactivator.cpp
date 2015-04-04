/*  $Id: dfmodactivator.cpp,v 1.4 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include "icons.hpp"
#include "util.hpp"
#include "tljpak.hpp"
#include "system.hpp"

#include "pak_insert_thread.hpp"
#include "progress_dialog.hpp"
#include "progress_logger.hpp"
#include "gui_progress_logger.hpp"
#include "dfmodactivator.hpp"

FXIcon* ok_icon;
FXIcon* cancel_icon;
FXIcon* kian_icon;
FXFont* title_font;
std::string dreamfall_path; // = "/media/windows/dreamfall/";

std::string cutprefix(const std::string& prefix, const std::string& path)
{
  return path.substr(prefix.length());
}

ModListEntry::ModListEntry() {}

ModListEntry::ModListEntry(FXComposite* parent,
                           FXIcon* icon,
                           std::string        directory_,
                           const FXString& title_string,
                           const FXString& short_description,
                           const FXString& long_description)
  : FXHorizontalFrame(parent, FRAME_NONE|LAYOUT_FILL_X|FRAME_LINE),
    directory(directory_ + "dreamfall/")
{
  this->setBackColor(FXRGB(255,255,255));
  FXLabel* label = new FXLabel(this, "", icon?icon:kian_icon, FRAME_NONE|LAYOUT_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,
                               0, 0, 48, 48);
  label->setBackColor(FXRGB(255,255,255));

  FXVerticalFrame* description = new FXVerticalFrame(this, LAYOUT_FILL_X);
  description->setBackColor(FXRGB(255,255,255));

  FXLabel* title = new FXLabel(description, title_string, NULL, 0, LAYOUT_FILL_X|LAYOUT_MIN_HEIGHT|FRAME_LINE);
  title->setFont(title_font);
  title->setBackColor(FXRGB(255,255,255));

  //title->setStyled(TRUE);
  //title->setTextStyle(title->getTextStyle() | TEXT_WORDWRAP);
  //title->appendStyledText(title_string, FXText::STYLE_BOLD);//|FXText::STYLE_UNDERLINE);

  FXText* text = new FXText(description, NULL, 0, LAYOUT_FILL_X);
  text->setStyled(TRUE);
  text->setTextStyle(text->getTextStyle() | TEXT_WORDWRAP);
  text->setVisibleRows(text->getNumRows());

  text->setEditable(FALSE);
  text->appendStyledText(short_description); //, FXText::STYLE_UNDERLINE);

  arrowbutton = new FXArrowButton(description, this, ID_TOGGLE_LONG_DESC, ARROW_DOWN|LAYOUT_FILL_X);
  arrowbutton->setBackColor(FXRGB(250,250,250));

  longtext = new FXText(description, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
  longtext->setStyled(FALSE);
  longtext->setTextStyle(longtext->getTextStyle() | TEXT_WORDWRAP);
  longtext->appendStyledText(long_description);
  longtext->setVisibleRows(longtext->getNumRows());
  longtext->hide();

  selectbutton =
    new FXToggleButton(this, "", "", cancel_icon, ok_icon, NULL, 0, FRAME_NONE|LAYOUT_RIGHT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,
                       0, 0, 48, 48);
  selectbutton->setBackColor(FXRGB(255,255,255));
}

std::string
ModListEntry::get_directory() const {
  return directory;
}

long
ModListEntry::onCmdToggleLongDesc(FXObject*,FXSelector,void*)
{
  if (longtext->shown())
    {
      longtext->hide();
      arrowbutton->setArrowStyle(ARROW_DOWN);
      this->recalc();
    }
  else
    {
      longtext->show();
      longtext->setVisibleRows(longtext->getNumRows());
      arrowbutton->setArrowStyle(ARROW_UP);
      this->recalc();
    }

  return 1;
}

void
ModListEntry::setState(FXbool s)
{
  return selectbutton->setState(s);
}

FXbool
ModListEntry::getState () const
{
  return selectbutton->getState();
}

FXDEFMAP(ModListEntry) ModListEntryMap[] = {
  FXMAPFUNC(SEL_COMMAND,       ModListEntry::ID_TOGGLE_LONG_DESC, ModListEntry::onCmdToggleLongDesc),
};
FXIMPLEMENT(ModListEntry, FXHorizontalFrame, ModListEntryMap, ARRAYNUMBER(ModListEntryMap));


DFModActivatorWindow::DFModActivatorWindow() {
}

DFModActivatorWindow::DFModActivatorWindow(FXApp* a)
  : FXMainWindow(a,"DFModActivator", NULL, NULL, DECOR_ALL, 0, 0, 640, 480)
{
  Icon::init(getApp());

  setMiniIcon(Icon::dftoolbox_mini);
  setIcon(Icon::dftoolbox);

  // write own progress dialog here
  progress_dialog = new ProgressDialog(getApp(), "Installing data, this may take a while");
  //progress_dialog->setBarStyle(PROGRESSBAR_HORIZONTAL);

  FXVerticalFrame* base = new FXVerticalFrame(this, FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y,
                                              0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0);

  menubar       = new FXMenuBar(base,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  modlistscroll = new FXScrollWindow(base, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  buttonbar = new FXHorizontalFrame(base, FRAME_NONE|LAYOUT_FILL_X|LAYOUT_SIDE_BOTTOM|PACK_UNIFORM_WIDTH);
  status  = new FXStatusBar(base,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X); //|STATUSBAR_WITH_DRAGCORNER);

  filemenu = new FXMenuPane(base);
  new FXMenuCommand(filemenu,"Dreamfall Path...", NULL, this, ID_DREAMFALL_PATH);
  new FXMenuCommand(filemenu,"Quit",NULL, getApp(), FXApp::ID_QUIT);
  new FXMenuTitle(menubar,"File",NULL,filemenu);

  helpmenu = new FXMenuPane(base);
  new FXMenuCommand(helpmenu,"About", NULL, this, ID_ABOUT);
  new FXMenuTitle(menubar,"Help",NULL,helpmenu);

  modlist = new FXVerticalFrame(modlistscroll, FRAME_SUNKEN| FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y,
                                0, 0, 0, 0,
                                DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 12);
  modlist->setBackColor(FXRGB(255,255,255));

  selectall_button  = new FXButton(buttonbar, "Select none",
                                   NULL, this, ID_SELECT_NONE, ICON_ABOVE_TEXT|FRAME_RAISED);
  selectnone_button = new FXButton(buttonbar, "Select all",
                                   NULL, this, ID_SELECT_ALL, ICON_ABOVE_TEXT|FRAME_RAISED);
  install_button    = new FXButton(buttonbar, "Install",
                                   NULL, this, ID_INSTALL, ICON_ABOVE_TEXT|FRAME_RAISED|LAYOUT_RIGHT|BUTTON_DEFAULT);
  uninstall_button    = new FXButton(buttonbar, "Uninstall",
                                   NULL, this, ID_UNINSTALL, ICON_ABOVE_TEXT|FRAME_RAISED|LAYOUT_RIGHT|BUTTON_DEFAULT);

  // Search for mods
  scan_mod_directory();
}

DFModActivatorWindow::~DFModActivatorWindow()
{
}

long
DFModActivatorWindow::onCmdAbout(FXObject*,FXSelector,void*)
{
  FXMessageBox about(this,
                     "About DFModActivator V0.0",
                     "DFModActivator V0.0 is a little tool that allows you to install\nand remove "
                     "modifications for Dreamfall.\n\n"
                     "Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>",
                     NULL,MBOX_OK|DECOR_TITLE|DECOR_BORDER);
  about.execute();
  return 1;
}

void
DFModActivatorWindow::scan_mod_directory()
{
  Directory mod_directory = open_directory(get_exe_path() + "mods/");
  for(Directory::iterator i = mod_directory.begin(); i != mod_directory.end(); ++i)
    {
      if (i->type == DirectoryEntry::T_DIRECTORY)
        {
          std::ifstream in((i->fullname + "/info.txt").c_str());
          if (!in)
            {
              std::cout << "Error: Ignoring directory " << i->fullname << " info.txt file missing" << std::endl;
            }
          else
            {
              std::string title, short_desc, long_desc, tmp;

              std::getline(in, title);
              std::getline(in, short_desc);
              while(std::getline(in, tmp))
                {
                  if (!(long_desc.empty() && tmp.empty())) // skip newline at the beginning
                    long_desc += tmp + "\n";
                }

              // trim
              for(std::string::size_type j = long_desc.size()-1; j >= 0; --j)
                if (long_desc[j] != '\n')
                  {
                    long_desc = long_desc.substr(0, j+1);
                    break;
                  }

              std::vector<char> icon_data;
              FXIcon* icon = 0;
              if (file_readbytes(i->fullname + "/icon.gif", icon_data))
                {
                  icon = new FXGIFIcon(getApp(), &*icon_data.begin());
                }

              entries.push_back(new ModListEntry(modlist, icon,
                                                 i->fullname,
                                                 title.c_str(), short_desc.c_str(), long_desc.c_str()));

              in.close();
            }
        }
    }
}

void
DFModActivatorWindow::collect_insert_requests(std::vector<InsertRequest>& inserts)
{
  // Collect insert requests
  for(std::vector<ModListEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      if ((*i)->getState())
        {
          //std::cout << "Install: " << (*i)->get_directory() << std::endl;

          std::vector<std::string> lst;
          find_files((*i)->get_directory(), lst);

          for(std::vector<std::string>::iterator j = lst.begin(); j != lst.end(); ++j)
            {
              //std::cout << *j << "\n  -> " << cutprefix((*i)->get_directory(), *j) << std::endl;
              inserts.push_back(InsertRequest(*j, cutprefix((*i)->get_directory(), *j)));
            }
        }
    }
}

void
DFModActivatorWindow::set_dreamfall_path(const std::string& path)
{
  dreamfall_path = path + "/";
  std::ofstream out((get_exe_path() + "dreamfall_path.txt").c_str());
  out << dreamfall_path << std::endl;
  out.close();
}

long
DFModActivatorWindow::onCmdUninstall(FXObject* obj, FXSelector sel, void* data)
{
  // FIXME: Insert real uninstall code here

  FXMessageBox::information(this, MBOX_OK, "Uninstall complete",
                            "[UNIMPLEMENTED] All modifications have been uninstalled from Dreamfall.");
  return 1;
}

long
DFModActivatorWindow::onCmdInstall(FXObject* obj, FXSelector sel, void* data)
{
  if (dreamfall_path.empty())
    get_dreamfall_path();

  if (!dreamfall_path.empty())
    {
      std::vector<InsertRequest> inserts;

      collect_insert_requests(inserts);

      install_button->disable();
      uninstall_button->disable();

      GUIProgressLogger* logger = new GUIProgressLogger(getApp(), progress_dialog,
                                                        ProgressDialog::ID_THREAD_UPDATE); // FIXME: delete me
      (new PakInsertThread(getApp(), progress_dialog, inserts, logger))->start(); // FIXME: Delete me
      progress_dialog->show(PLACEMENT_OWNER);
      //progress_dialog->execute();
    }
  return 1;
}

long
DFModActivatorWindow::onCmdSelectAll(FXObject*, FXSelector,void*) {
  for(std::vector<ModListEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      (*i)->setState(TRUE);
    }
  return 1;
}

long
DFModActivatorWindow::onCmdSelectNone(FXObject*, FXSelector,void*) {
  for(std::vector<ModListEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      (*i)->setState(FALSE);
    }
  return 1;
}

bool
DFModActivatorWindow::get_dreamfall_path()
{
  FXDirDialog open(this,"Select the Dreamfall directory");
  open.showFiles(FALSE);
  if(open.execute()) {
    if (!file_exists((open.getDirectory() + "/bin/res/").text()))
      {
        FXMessageBox::error(this, MBOX_OK, "Dreamfall Path Error",
                            "The given directory isn't the Dreamfall directory:\n%s",
                            open.getDirectory().text());
        return false;
      }
    else
      {
        set_dreamfall_path(open.getDirectory().text());
        return true;
      }
  } else {
    return false;
  }
}

long
DFModActivatorWindow::onCmdDreamfallPath(FXObject*, FXSelector,void*){
  get_dreamfall_path();
  return 1;
}

FXDEFMAP(DFModActivatorWindow) DFModActivatorWindowMap[] = {
  FXMAPFUNC(SEL_COMMAND,       DFModActivatorWindow::ID_ABOUT,          DFModActivatorWindow::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND,       DFModActivatorWindow::ID_DREAMFALL_PATH, DFModActivatorWindow::onCmdDreamfallPath),
  FXMAPFUNC(SEL_COMMAND,       DFModActivatorWindow::ID_INSTALL,        DFModActivatorWindow::onCmdInstall),
  FXMAPFUNC(SEL_COMMAND,       DFModActivatorWindow::ID_UNINSTALL,      DFModActivatorWindow::onCmdUninstall),
  FXMAPFUNC(SEL_COMMAND,       DFModActivatorWindow::ID_SELECT_NONE,    DFModActivatorWindow::onCmdSelectNone),
  FXMAPFUNC(SEL_COMMAND,       DFModActivatorWindow::ID_SELECT_ALL,     DFModActivatorWindow::onCmdSelectAll),
};
FXIMPLEMENT(DFModActivatorWindow, FXMainWindow, DFModActivatorWindowMap, ARRAYNUMBER(DFModActivatorWindowMap));

int main(int argc,char** argv)
{
  std::ifstream in((get_exe_path() + "dreamfall_path.txt").c_str());
  if (in)
    {
      std::getline(in, dreamfall_path);

      // ignore invalid an path
      if (!file_exists(dreamfall_path + "/bin/res/"))
        {
          std::cout << "Ignoring Dreamfall path: " << dreamfall_path
                    << "\ncouldn't find pak subdirectory" << std::endl;
          dreamfall_path = "";
        }

      in.close();
    }

  FXApp application("DFModActivator V0.0", "Grumbel");

  application.init(argc,argv);

  cancel_icon = new FXGIFIcon(&application, cancelicon_data);
  kian_icon   = new FXGIFIcon(&application, kianicon_data);

  std::vector<char> icon_data;
  if (file_readbytes(get_exe_path() + "icons/enabled.gif", icon_data))
    ok_icon = new FXGIFIcon(&application, &*icon_data.begin());

  ok_icon->create();
  cancel_icon->create();
  kian_icon->create();

  title_font = new FXFont(&application, "helvetica", 10, FXFont::Bold, FXFont::Straight,
                          FONTENCODING_DEFAULT, FXFont::NonExpanded,FXFont::Scalable);

  FXMainWindow* main = new DFModActivatorWindow(&application);

  application.create();

  main->show(PLACEMENT_SCREEN);

  return application.run();
}


/* EOF */
