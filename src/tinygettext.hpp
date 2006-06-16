//  $Id: tinygettext.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
// 
//  TinyGetText - A small flexible gettext() replacement
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#ifndef HEADER_TINYGETTEXT_H
#define HEADER_TINYGETTEXT_H

#include <map>
#include <vector>
#include <set>
#include <string>

namespace TinyGetText {

typedef int (*PluralFunc)(int n);

struct LanguageDef {
  const char* code;
  const char* name;
  int         nplural;
  PluralFunc  plural;

  LanguageDef(const char* code_, const char* name_,  int nplural_, PluralFunc plural_)
    : code(code_), name(name_), nplural(nplural_), plural(plural_)
  {}
};

/** A simple dictionary class that mimics gettext() behaviour. Each
    Dictionary only works for a single language, for managing multiple
    languages and .po files at once use the DictionaryManager. */
class Dictionary
{
private:
  typedef std::map<std::string, std::string> Entries;
  Entries entries;

  typedef std::map<std::string, std::map<int, std::string> > PluralEntries;
  PluralEntries plural_entries;

  LanguageDef language;
  std::string charset;
public:
  /** */
  Dictionary(const LanguageDef& language_, const std::string& charset = "");

  Dictionary();

  /** Return the charset used for this dictionary */
  std::string get_charset() const;

  /** Set a charset for this dictionary, this will NOT convert stuff,
      it is for information only, you have to convert stuff yourself
      when you add it with \a add_translation() */
  void set_charset(const std::string& charset);

  /** Set the language that is used for this dictionary, this is
      mainly needed to evaluate plural forms */
  void set_language(const LanguageDef& lang);

  /** Translate the string \a msgid to its correct plural form, based
      on the number of items given by \a num. \a msgid2 is \a msgid in
      plural form. */
  std::string translate(const std::string& msgid, const std::string& msgid2, int num);

  /** Translate the string \a msgid. */
  std::string translate(const std::string& msgid);
  /** Translate the string \a msgid. */
  const char* translate(const char* msgid);
    
  /** Add a translation from \a msgid to \a msgstr to the dictionary,
      where \a msgid is the singular form of the message, msgid2 the
      plural form and msgstrs a table of translations. The right
      translation will be calculated based on the \a num argument to
      translate(). */
  void add_translation(const std::string& msgid, const std::string& msgid2,
                       const std::map<int, std::string>& msgstrs);

  /** Add a translation from \a msgid to \a msgstr to the
      dictionary */
  void add_translation(const std::string& msgid, const std::string& msgstr);
};

/** Read the content of the .po file given as \a in into the
    dictionary given as \a dict */
void read_po_file(Dictionary& dict, std::istream& in);
LanguageDef& get_language_def(const std::string& name);

} // namespace TinyGetText

#endif

/* EOF */
