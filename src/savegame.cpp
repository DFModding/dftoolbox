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

#include <fstream>
#include <iostream>
#include "savegame.hpp"

Savegame::Savegame(const std::string& filename)
{
  parse(filename);
}

void
Savegame::parse(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::binary);

  if (!in)
    {
      std::cout << "Couldn't open: " << filename << std::endl;
    }
  else
    {
      Shark3D* shark = Shark3D::parse_binary(in);
     
      parse_root(shark->section);
 
      in.close();
    }
}

void
Savegame::parse_root(SectionNode* node)
{
  const std::vector<SectionNode*>& sections = node->get_sections("actors");

  for(std::vector<SectionNode*>::const_iterator i = sections.begin();
      i != sections.end(); ++i)
    {
      std::string type = (*i)->get_string("name"); 
      if (type == "conversationlog")
        {
          parse_conversationlog((*i)->get_section("data"));
        }
      else if (type == "messagelog")
        {
          parse_messagelog((*i)->get_section("data"));
        }
      else if (type == "journal")
        {
          parse_journal((*i)->get_section("data"));
        }
    }
}

void
Savegame::parse_conversationlog(SectionNode* node)
{
  const std::vector<SectionNode*>& sections = node->get_sections("entries");
  
  for(std::vector<SectionNode*>::const_iterator i = sections.begin();
      i != sections.end(); ++i)
    {
      ConversationLog::Entry entry;
      entry.list = (*i)->get_ints("list");

      //// convert weirdo ids into ids that can be locked up in Dialog
      //for(int i = 1; i < int(entry.list.size()); ++i)
      //entry.list[i] = entry.list[i-1] + entry.list[i];
      
      conversation_log.entries.push_back(entry);
    }
}

void
Savegame::parse_messagelog(SectionNode* node)
{
  const std::vector<SectionNode*>& sections = node->get_sections("entries");
  
  for(std::vector<SectionNode*>::const_iterator i = sections.begin();
      i != sections.end(); ++i)
    {
      MessageLog::Entry messagelog_enty;

      messagelog_enty.subj = (*i)->get_int("subj");
      messagelog_enty.body = (*i)->get_int("body");
      messagelog_enty.read = (*i)->get_int("read");

      message_log.entries.push_back(messagelog_enty);
    }
}

void
Savegame::parse_journal(SectionNode* node)
{
  const std::vector<SectionNode*>& entries = node->get_sections("entries");
  
  for(std::vector<SectionNode*>::const_iterator i = entries.begin();
      i != entries.end(); ++i)
    {
      Journal::Entry entry; 

      entry.id    = (*i)->get_int("id");
      entry.read  = (*i)->get_int("read");
      entry.array = (*i)->get_ints("array");

      journal.entries.push_back(entry);
    }  
}

/* EOF */
