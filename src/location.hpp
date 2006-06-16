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

#ifndef HEADER_LOCATION_HPP
#define HEADER_LOCATION_HPP

#include <vector>
#include <map>
#include <string>

#include "shark3d.hpp"

struct Speech
{
  unsigned int id;
  std::string  dispatcher;
  unsigned int actor_id;
  unsigned int actor_name;
  unsigned int text;
  std::string  wavefile;
  bool         subtiles;
  bool         lipsync;
  int          voice_pos;
  float        reverb_dry;
  float        reverb_wet;
};

struct Conversation
{
  struct Choice
  {
    struct Action
    {
      struct Cutscene
      {
        struct Action
        {
          int          action_type;
          unsigned int camera;
          std::string  metastate;
          unsigned int speech;
        };

        bool canskip;
        std::vector<Action> actions;
      };

      unsigned int action_enabled;
      std::vector<unsigned int> enable_keywords;
      unsigned int goto_choicelist;
      unsigned int firstline;
      Cutscene cutscene;
    };
    unsigned int id;
    unsigned int keyword_enabled;
    unsigned int keyword;
    unsigned int preview;

    std::vector<Action> actions;
  };

  unsigned int id;
  std::vector<Choice> choices;
};

/** 
 * Parses a data/generated/locations/{}.cdr file and gives out
 *  conversations and speech
 */
class Location
{
private:
  typedef std::map<unsigned int, Speech> Speechs;
  Speechs speechs;

public:
  Location(const std::string& filename);
  
  void parse(const std::string& filename);
  void parse_root(SectionNode* );
  void parse_actor_param(SectionNode* );
  void parse_children(SectionNode* );
  void parse_speech(SectionNode* );
  void parse_conversation(SectionNode* );

  const Speechs& get_speechs() const { return speechs; }
};

#endif

/* EOF */
