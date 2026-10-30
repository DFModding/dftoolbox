// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <string>

#include "dreamfall_file_entry.hpp"

namespace {

int g_failures = 0;

void check(const std::string& actual, const std::string& expected)
{
  if (actual != expected)
    {
      std::cerr << "FAIL: expected " << expected << ", got " << actual << std::endl;
      ++g_failures;
    }
}

DreamfallFileEntry make_pak(FileType type, const std::string& filename)
{
  PakFileEntry pak;
  pak.type = type;
  pak.filename = filename;
  pak.fullname = "art/" + filename;
  return DreamfallFileEntry(pak);
}

} // namespace

int main()
{
  const DreamfallFileEntry texture = make_pak(FILETYPE_DDS, "foo.bar.png");
  check(texture.get_export_name(EXPORT_ORIGINAL), "foo.bar.png.dds");
  check(texture.get_export_name(EXPORT_PNG), "foo.bar.png");
  check(make_pak(FILETYPE_DDS, "foo.tga").get_export_name(EXPORT_PNG), "foo.png");
  check(make_pak(FILETYPE_DDS, "foo.DDS").get_export_name(EXPORT_ORIGINAL), "foo.DDS");

  check(make_pak(FILETYPE_STFU4, "foo_nml.png").get_export_name(EXPORT_ORIGINAL), "foo_nml.png.stfu4");
  check(make_pak(FILETYPE_STFU4, "foo_nml.png").get_export_name(EXPORT_PNG), "foo_nml.png");

  const DreamfallFileEntry shark = make_pak(FILETYPE_SHARK3D, "foo.sdr");
  check(shark.get_export_name(EXPORT_ORIGINAL), "foo.sdr");
  check(shark.get_export_name(EXPORT_TEXT), "foo.sdr.txt");

  const DreamfallFileEntry sound = make_pak(FILETYPE_MP3, "foo.mp3");
  check(std::to_string(sound.get_export_formats().size()), "1");

  const DreamfallFileEntry image(SaveFileEntry(SaveFileEntry::IMAGE_DATA, "save1.sav", "/saves/save1.sav"));
  check(image.get_export_name(EXPORT_ORIGINAL), "save1.sav");
  check(image.get_export_name(EXPORT_PNG), "save1-image.png");

  const DreamfallFileEntry journal(SaveFileEntry(SaveFileEntry::JOURNAL_DATA, "save1.sav", "/saves/save1.sav"));
  check(journal.get_export_name(EXPORT_TEXT), "save1-journal.txt");

  const DreamfallFileEntry directory{DirectoryFileEntry()};
  check(std::to_string(directory.get_export_formats().size()), "0");

  return g_failures == 0 ? 0 : 1;
}
