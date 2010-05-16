/*  $Id: tljpak_manager.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_TLJPAK_MANAGER_HPP
#define HEADER_TLJPAK_MANAGER_HPP

#include <string>
#include <map>
#include <vector>
#include <set>
#include "dreamfall_file_entry.hpp"
#include "tljpak.hpp"
#include "location.hpp"
#include "dialog.hpp"

class ProgressLogger;
class DreamfallFileEntry;

/** */
class DataManager
{
public:
  typedef std::map<std::string, TLJPak*> Paks;
  typedef std::map<std::string, PakFileEntry*> Files;
  typedef std::map<std::string, std::vector<TLJPak*> > PaksByLang;
private:
  Paks paks;
  PaksByLang paks_by_lang;

  /** List of possible filenames, used to optimize guessing */
  std::vector<std::string> filelist;

  std::vector<Dialog> dialogs; 

  std::map<unsigned int, Speech> id_to_speech;
  std::map<std::string,  Speech> mp3_to_speech;

  // Simple cache to collect all pointers pointing to allocated memory
  std::vector<DreamfallFileEntry*> filetable;

public:
  DataManager();
  ~DataManager();
  
  void init(ProgressLogger& logger);

  void add_directory(const std::string& pakdirectory, ProgressLogger& logger);

  /** Add a filelist with filelist to resolve the hash values */
  void add_filelist(const std::string& filename);

  /** */
  TLJPak* get(const std::string& name);

  /** Extract the given file to \a outfile */
  bool extract(const std::string& location, const std::string& outfile);
  bool extract(const PakFileEntry& entry, const std::string& outfile);
  bool extract_by_language(const std::string& lang_code, const std::string& location, const std::string& outfile);

  /** Query the file for the type */
  FileType get_type(const std::string& pathname);
  
  /** Scans for all files, detects their name, filetype, etc. */
  void scan_paks(ProgressLogger& logger);

  Files get_files();
  Files get_files(const std::string& pak);

  std::vector<std::string> get_paks() const;

  // FIXME: Move dialog handling here
  int     get_num_dialogs(const std::string& lang) const;
  const Dialog& get_dialog(const std::string& lang) const;
  const Dialog& get_dialog(int i) const;
  const std::vector<Dialog>& get_dialogs() const { return dialogs; }

  void scan_for_mp3s(const std::string& filename, ProgressLogger& logger);
  void scan_for_all_mp3s(ProgressLogger& logger);

  Speech get_speech_by_mp3(const std::string& ) const;
  Speech get_speech_by_id(unsigned int id) const;

  std::vector<DreamfallFileEntry*>& get_filetable() { return filetable; }
};

extern DataManager data_manager;

#endif

/* EOF */
