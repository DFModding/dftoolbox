/*  $Id: dfmodactivator.cpp,v 1.4 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "dfmodactivator.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include <QApplication>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "icons.hpp"
#include "util.hpp"
#include "tljpak.hpp"
#include "startup_report.hpp"
#include "system.hpp"

#include "pak_insert_thread.hpp"
#include "progress_dialog.hpp"
#include "progress_logger.hpp"
#include "gui_progress_logger.hpp"

std::string dreamfall_path; // = "/media/windows/dreamfall/";

std::string cutprefix(const std::string& prefix, const std::string& path)
{
  return path.substr(prefix.length());
}

ModListEntry::ModListEntry(QWidget* parent,
                           const QIcon& icon,
                           const std::string& directory_,
                           const QString& title_string,
                           const QString& short_description,
                           const QString& long_description)
  : QFrame(parent),
    directory(directory_ + "dreamfall/")
{
  setFrameStyle(QFrame::Box | QFrame::Plain);
  setLineWidth(1);
  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setColor(QPalette::Window, Qt::white);
  pal.setColor(QPalette::Base, Qt::white);
  setPalette(pal);

  QHBoxLayout* hbox = new QHBoxLayout(this);

  QLabel* label = new QLabel;
  label->setFixedSize(48, 48);
  label->setPixmap(icon.pixmap(48, 48));
  hbox->addWidget(label, 0, Qt::AlignLeft | Qt::AlignTop);

  QVBoxLayout* description = new QVBoxLayout;
  hbox->addLayout(description, 1);

  QLabel* title = new QLabel(title_string);
  title->setTextFormat(Qt::PlainText);
  QFont title_font = title->font();
  title_font.setBold(true);
  if (title_font.pointSizeF() > 0)
    title_font.setPointSizeF(title_font.pointSizeF() * 1.2);
  title->setFont(title_font);
  description->addWidget(title);

  QLabel* text = new QLabel(short_description);
  text->setTextFormat(Qt::PlainText);
  text->setWordWrap(true);
  text->setTextInteractionFlags(Qt::TextSelectableByMouse);
  description->addWidget(text);

  arrowbutton = new QToolButton;
  arrowbutton->setArrowType(Qt::DownArrow);
  arrowbutton->setAutoRaise(true);
  arrowbutton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  connect(arrowbutton, &QToolButton::clicked, this, [this]{ on_toggle_long_desc(); });
  description->addWidget(arrowbutton);

  longtext = new QLabel(long_description);
  longtext->setTextFormat(Qt::PlainText);
  longtext->setWordWrap(true);
  longtext->setTextInteractionFlags(Qt::TextSelectableByMouse);
  longtext->hide();
  description->addWidget(longtext);

  QIcon select_icon;
  select_icon.addPixmap(Icon::cancel.pixmap(48, 48), QIcon::Normal, QIcon::Off);
  select_icon.addPixmap(Icon::ok.pixmap(48, 48), QIcon::Normal, QIcon::On);

  selectbutton = new QToolButton;
  selectbutton->setCheckable(true);
  selectbutton->setIcon(select_icon);
  selectbutton->setIconSize(QSize(48, 48));
  selectbutton->setFixedSize(48, 48);
  selectbutton->setStyleSheet("QToolButton { border: none; background: transparent; }");
  hbox->addWidget(selectbutton, 0, Qt::AlignRight | Qt::AlignTop);
}

std::string
ModListEntry::get_directory() const {
  return directory;
}

void
ModListEntry::on_toggle_long_desc()
{
  if (longtext->isVisible())
  {
    longtext->hide();
    arrowbutton->setArrowType(Qt::DownArrow);
  }
  else
  {
    longtext->show();
    arrowbutton->setArrowType(Qt::UpArrow);
  }
}

void
ModListEntry::setState(bool s)
{
  selectbutton->setChecked(s);
}

bool
ModListEntry::getState() const
{
  return selectbutton->isChecked();
}


DFModActivatorWindow::DFModActivatorWindow()
{
  setWindowTitle("DFModActivator");
  setWindowIcon(Icon::dftoolbox);
  resize(640, 480);

  // write own progress dialog here
  progress_dialog = new ProgressDialog(this, "Installing data, this may take a while");

  QMenu* filemenu = menuBar()->addMenu("File");
  filemenu->addAction("Dreamfall Path...", this, [this]{ on_dreamfall_path(); });
  filemenu->addAction("Quit", qApp, &QApplication::quit);

  QMenu* helpmenu = menuBar()->addMenu("Help");
  helpmenu->addAction("About", this, [this]{ on_about(); });

  QScrollArea* modlistscroll = new QScrollArea;
  modlistscroll->setWidgetResizable(true);
  modlistscroll->setFrameShape(QFrame::StyledPanel);
  modlistscroll->setFrameShadow(QFrame::Sunken);

  QWidget* modlist_widget = new QWidget;
  modlist_widget->setAutoFillBackground(true);
  QPalette pal = modlist_widget->palette();
  pal.setColor(QPalette::Window, Qt::white);
  modlist_widget->setPalette(pal);
  modlist = new QVBoxLayout(modlist_widget);
  modlistscroll->setWidget(modlist_widget);

  // Note that the labels of the first two buttons are swapped
  // compared to the names of the members, that's how they always were
  selectall_button  = new QPushButton("Select none");
  selectnone_button = new QPushButton("Select all");
  install_button    = new QPushButton("Install");
  uninstall_button  = new QPushButton("Uninstall");
  connect(selectall_button,  &QPushButton::clicked, this, [this]{ on_select_none(); });
  connect(selectnone_button, &QPushButton::clicked, this, [this]{ on_select_all(); });
  connect(install_button,    &QPushButton::clicked, this, [this]{ on_install(); });
  connect(uninstall_button,  &QPushButton::clicked, this, [this]{ on_uninstall(); });

  QHBoxLayout* buttonbar = new QHBoxLayout;
  buttonbar->addWidget(selectall_button);
  buttonbar->addWidget(selectnone_button);
  buttonbar->addStretch(1);
  buttonbar->addWidget(uninstall_button);
  buttonbar->addWidget(install_button);

  QWidget* central = new QWidget;
  QVBoxLayout* base = new QVBoxLayout(central);
  base->setContentsMargins(0, 0, 0, 0);
  base->addWidget(modlistscroll, 1);
  base->addLayout(buttonbar);
  setCentralWidget(central);

  statusBar()->showMessage("Ready.");

  // Search for mods
  scan_mod_directory();
  modlist->addStretch(1);
}

DFModActivatorWindow::~DFModActivatorWindow()
{
}

void
DFModActivatorWindow::on_about()
{
  QMessageBox::about(this,
                     "About DFModActivator V0.0",
                     "DFModActivator V0.0 is a little tool that allows you to install\nand remove "
                     "modifications for Dreamfall.\n\n"
                     "Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>");
}

void
DFModActivatorWindow::scan_mod_directory()
{
  Directory mod_directory = open_directory(get_data_path() + "mods/");
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

              // trim trailing newlines (the old loop used an unsigned
              // index that never terminated for an empty description)
              long_desc.erase(long_desc.find_last_not_of('\n') + 1);

              QIcon icon(QString::fromStdString(i->fullname + "/icon.gif"));
              if (icon.isNull())
                icon = Icon::kian;

              ModListEntry* entry = new ModListEntry(modlist->parentWidget(), icon,
                                                     i->fullname,
                                                     QString::fromStdString(title),
                                                     QString::fromStdString(short_desc),
                                                     QString::fromStdString(long_desc));
              modlist->addWidget(entry);
              entries.push_back(entry);

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
  try
    {
      file_write_atomic(get_config_path() + "dreamfall_path.txt", dreamfall_path + "\n");
    }
  catch(std::exception& err)
    {
      QMessageBox::warning(this, "Error", err.what());
    }
}

void
DFModActivatorWindow::on_uninstall()
{
  // FIXME: Insert real uninstall code here

  QMessageBox::information(this, "Uninstall complete",
                           "[UNIMPLEMENTED] All modifications have been uninstalled from Dreamfall.");
}

void
DFModActivatorWindow::on_install()
{
  if (dreamfall_path.empty())
    get_dreamfall_path();

  if (!dreamfall_path.empty())
    {
      std::vector<InsertRequest> inserts;

      collect_insert_requests(inserts);

      install_button->setEnabled(false);
      uninstall_button->setEnabled(false);

      GUIProgressLogger* logger = new GUIProgressLogger(progress_dialog); // FIXME: delete me
      std::shared_ptr<PakInsertThread> worker = std::make_shared<PakInsertThread>(inserts, logger);
      progress_dialog->set_thread(std::thread([worker]{ worker->run(); }));
      progress_dialog->show();
    }
}

void
DFModActivatorWindow::on_select_all()
{
  for(std::vector<ModListEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      (*i)->setState(true);
    }
}

void
DFModActivatorWindow::on_select_none()
{
  for(std::vector<ModListEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      (*i)->setState(false);
    }
}

bool
DFModActivatorWindow::get_dreamfall_path()
{
  const QString dir = QFileDialog::getExistingDirectory(this, "Select the Dreamfall directory");
  if (!dir.isEmpty())
    {
      if (!file_exists((dir + "/bin/res/").toStdString()))
        {
          QMessageBox::critical(this, "Dreamfall Path Error",
                                "The given directory isn't the Dreamfall directory:\n" + dir);
          return false;
        }
      else
        {
          set_dreamfall_path(dir.toStdString());
          return true;
        }
    }
  else
    {
      return false;
    }
}

void
DFModActivatorWindow::on_dreamfall_path()
{
  get_dreamfall_path();
}

int main(int argc,char** argv)
{
  try {
    attach_parent_console();
    parse_args(argc, argv, "dfmodtool");

    std::ifstream in(get_config_file("dreamfall_path.txt").c_str());
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

    QApplication application(argc, argv);
    application.setApplicationName("dfmodtool");
    application.setApplicationDisplayName("DFModTool");
    application.setApplicationVersion(QString::fromStdString(get_version()));
    application.setOrganizationName("Grumbel");

    print_startup_report("dfmodtool");

    Icon::init();

    DFModActivatorWindow window;
    window.show();

    return application.exec();
  } catch(std::exception& err) {
    std::cerr << "Fatal Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
}


/* EOF */
