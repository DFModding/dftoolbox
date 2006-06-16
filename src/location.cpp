/*  $Id: location.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <iostream>
#include <fstream>
#include "location.hpp"

Location::Location(const std::string& filename)
{
  parse(filename);
}

void
Location::parse(const std::string& filename)
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
Location::parse_root(SectionNode* root)
{
  parse_actor_param(root->get_section("actor_param"));
}

void
Location::parse_actor_param(SectionNode* node)
{ 
  node->get_string("scope");
  node->get_string("local_name");
  node->get_string("deleg_target");
  node->get_string("child_type");
  parse_children(node->get_section("child_param"));
}

void
Location::parse_children(SectionNode* node)
{
  std::vector<SectionNode*> sections = node->get_sections("children");

  for(std::vector<SectionNode*>::iterator i = sections.begin();
      i != sections.end(); ++i)
    {
      std::string type = (*i)->get_string("type");
      if (type == "mod_engobj_funcom.speech")
        {
          parse_speech((*i)->get_section("param"));
        }
      else
        {
          // ignore
        }
    }
}

void
Location::parse_speech(SectionNode* node)
{ // we get the first node of the param subsection
  Speech speech;
  
  speech.id         = node->get_int("id");
  speech.dispatcher = node->get_string("dispatcher");

  // goto subtiles subnode
  node = node->get_section("subtitles");
  
  speech.actor_id    = node->get_int("actor_id");
  speech.actor_name  = node->get_int("actor_name"); 
  speech.text        = node->get_int("text");
  speech.wavefile    = node->get_string("wavefile");
  speech.subtiles    = node->get_int("subtitles");
  speech.lipsync     = node->get_int("lipsync");
  speech.voice_pos   = node->get_int("voice_pos");
  speech.reverb_dry  = node->get_float("reverb_dry");
  speech.reverb_wet  = node->get_float("reverb_wet");

  speechs[speech.id]= (speech);
}

void
Location::parse_conversation(SectionNode* )
{
  
}

/* EOF */
