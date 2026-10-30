// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADER_STARTUP_REPORT_HPP
#define HEADER_STARTUP_REPORT_HPP

/** Prints what the program runs on to stderr, one "topic: details"
    line per item, needs a QGuiApplication */
void print_startup_report(const char* program);

#endif

/* EOF */
