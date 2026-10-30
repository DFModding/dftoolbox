// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "startup_report.hpp"

#include <iostream>
#include <thread>

#include <QGuiApplication>
#include <QScreen>
#include <QSysInfo>

#include "system.hpp"

void print_startup_report(const char* program)
{
  std::ostream& out = std::cerr;

  out << "program: " << program << " " << get_version()
      << ", " << DFTOOLBOX_BUILD_TYPE
      << ", " << DFTOOLBOX_TARGET
      << ", compiler " << __VERSION__ << std::endl;

  out << "system: " << QSysInfo::prettyProductName().toStdString()
      << ", kernel " << QSysInfo::kernelType().toStdString()
      << " " << QSysInfo::kernelVersion().toStdString()
      << ", " << QSysInfo::currentCpuArchitecture().toStdString()
      << ", " << std::thread::hardware_concurrency() << " CPUs" << std::endl;

  out << "library: Qt " << qVersion() << " (built against " << QT_VERSION_STR << ")" << std::endl;

  print_paths(out);

  out << "video: Qt platform " << QGuiApplication::platformName().toStdString();
  if (const QScreen* screen = QGuiApplication::primaryScreen())
    {
      const QSize size = screen->size();
      out << ", screen " << size.width() << "x" << size.height()
          << " at " << screen->refreshRate() << " Hz"
          << ", scale " << screen->devicePixelRatio();
    }
  out << std::endl;
}

/* EOF */
