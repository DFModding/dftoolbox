/*  $Id: export_dialog.cpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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

#include "export_dialog.hpp"

#include <algorithm>
#include <exception>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

#include "icons.hpp"
#include "dftoolbox.hpp"
#include "system.hpp"

ExportDialog::ExportDialog(DFToolBoxWindow* parent_)
  : QDialog(parent_),
    parent(parent_)
{
  setWindowTitle("Export Dialog");
  setAttribute(Qt::WA_DeleteOnClose);

  QVBoxLayout* vbox = new QVBoxLayout(this);

  QFormLayout* form = new QFormLayout;
  vbox->addLayout(form);

  filename = new QPlainTextEdit;
  filename->setReadOnly(true);
  filename->setLineWrapMode(QPlainTextEdit::NoWrap);
  form->addRow("Selection:", filename);

  {
    QHBoxLayout* hbox = new QHBoxLayout;
    pathname = new QLineEdit(QString::fromStdString(get_temp_path()));
    hbox->addWidget(pathname, 1);
    QPushButton* browse = new QPushButton(Icon::folder_open, "Browse");
    connect(browse, &QPushButton::clicked, this, [this]{ on_browse(); });
    hbox->addWidget(browse);
    form->addRow("Export to:", hbox);
  }
  {
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel("Convert Images to: "));
    QComboBox* typebox = new QComboBox;
    typebox->addItem("Native");
    typebox->addItem("Targa (.tga)");
    typebox->setEnabled(false);
    hbox->addWidget(typebox);
    hbox->addStretch(1);
    vbox->addLayout(hbox);
  }
  {
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel("Convert Shark3D to: "));
    QComboBox* typebox = new QComboBox;
    typebox->addItem("Text");
    typebox->addItem("Native");
    typebox->setEnabled(false);
    hbox->addWidget(typebox);
    hbox->addStretch(1);
    vbox->addLayout(hbox);
  }
  {
    QHBoxLayout* hbox = new QHBoxLayout;
    QLabel* label = new QLabel("Filter: ");
    label->setToolTip("Only export files matching the expression");
    hbox->addWidget(label);
    filter = new QLineEdit;
    filter->setEnabled(false);
    hbox->addWidget(filter);
    hbox->addStretch(1);
    vbox->addLayout(hbox);
  }

  preserve_path = new QCheckBox("preserve directory structure on export");
  vbox->addWidget(preserve_path);
  fix_extension = new QCheckBox("append proper file extension where necessary (.png -> .png.dds)");
  vbox->addWidget(fix_extension);

  QHBoxLayout* buttonbar = new QHBoxLayout;
  buttonbar->addStretch(1);
  QPushButton* export_button = new QPushButton(Icon::export_file, "Export");
  connect(export_button, &QPushButton::clicked, this, [this]{ on_export(); });
  buttonbar->addWidget(export_button);
  QPushButton* cancel_button = new QPushButton(Icon::cancel, "Cancel");
  connect(cancel_button, &QPushButton::clicked, this, [this]{ on_cancel(); });
  buttonbar->addWidget(cancel_button);
  vbox->addLayout(buttonbar);

  resize(640, sizeHint().height());
}

void
ExportDialog::set_filename(const std::string& filename_)
{
  filename->setPlainText(QString::fromStdString(filename_));
}

void
ExportDialog::set_file_entry(DreamfallFileEntry* entry)
{
  filename->setPlainText(QString::fromStdString(entry->get_url()));
  entries.push_back(entry);
}

void
ExportDialog::set_file_entries(const std::vector<DreamfallFileEntry*>& selection)
{
  QStringList lines;
  for(std::vector<DreamfallFileEntry*>::const_iterator i = selection.begin(); i != selection.end(); ++i)
    {
      lines << QString::fromStdString((*i)->get_url());
      entries.push_back(*i);
    }
  filename->setPlainText(lines.join("\n"));

  // show up to 20 lines
  const int rows = std::min(std::max(1, int(lines.size())), 20);
  filename->setFixedHeight(rows * filename->fontMetrics().lineSpacing() +
                           2 * filename->frameWidth() +
                           2 * static_cast<int>(filename->document()->documentMargin()) +
                           filename->horizontalScrollBar()->sizeHint().height() + 4);
}

void
ExportDialog::on_export()
{
  hide();

  try {
    parent->export_files(entries, pathname->text().toStdString(),
                         preserve_path->isChecked(), fix_extension->isChecked(),
                         true);
  } catch(std::exception& err) {
    QMessageBox::critical(parent, "Export failed", QString::fromStdString(err.what()));
  }

  close();
}

void
ExportDialog::on_cancel()
{
  close();
}

void
ExportDialog::on_browse()
{
  const QString dir = QFileDialog::getExistingDirectory(this, "Select the export directory",
                                                        pathname->text());
  if (!dir.isEmpty())
    pathname->setText(dir);
}

/* EOF */
