/*  $Id: location.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SAVEGAME_HPP
#define HEADER_SAVEGAME_HPP

#include <vector>
#include <string>
#include "shark3d.hpp"

struct MessageLog
{
  struct Entry
  {
    int subj;
    int body;
    int read;
  };

  std::vector<Entry> entries;
};

struct Journal
{
  struct Entry
  {
    int id;
    int read;
    std::vector<int> array;
  };
  std::vector<Entry> entries;
};

struct ConversationLog
{
  struct Entry
  {
    std::vector<int> list;
  };

  std::vector<Entry> entries;
};

/** */
class Savegame
{
private:
  ConversationLog conversation_log;
  MessageLog      message_log;
  Journal         journal;
public:
  Savegame(const std::string& filename);

  void parse(const std::string& filename);
  void parse_root(SectionNode* node);

  void parse_conversationlog(SectionNode* node);
  void parse_messagelog(SectionNode* node);
  void parse_journal(SectionNode* node);

  const ConversationLog& get_conversation_log() const { return conversation_log; }
  const MessageLog&      get_message_log()      const { return message_log; }
  const Journal&         get_journal()          const { return journal; }
};

#endif

/* EOF */
