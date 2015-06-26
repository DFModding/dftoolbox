/*  $Id: shark3d.cpp,v 1.6 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <sstream>
#include <iostream>
#include <stdexcept>

#include "shark3d_nodes.hpp"
#include "shark3d.hpp"

Shark3D::Shark3D()
  : section(0)
{
}

Shark3D::~Shark3D()
{
  delete section;
}

Shark3D*
Shark3D::parse_binary(std::istream& in)
{
  Shark3D* shark3d = new Shark3D();

  shark3d->magic   = shark3d_binary::read_string_basic(in);
  shark3d->version = shark3d_binary::read_string_basic(in);

  if (shark3d->magic != "shark3d_snake_binary" || shark3d->version != "2x4")
    {
      throw std::runtime_error("Shark3D: Error file is not a shark3d_snake_binary");
    }
  else
    {
      shark3d->section = shark3d->parse_section(in);

      return shark3d;
    }
}

SectionNode*
Shark3D::parse_section(std::istream& in)
{
  using namespace shark3d_binary;
  SectionNode* section = new SectionNode();

  int num = read_int(in);
  for(int i = 0; i < num; ++i)
    {
      std::string name = read_string(in);
      char code = in.get();
      switch(code)
        {
        case 0x0: // nil
          section->add(name, new NullNode());
          break;

        case 0x1: // int
          section->add(name, new IntNode(read_int(in)));
          break;

        case 0x2: // int array
          {
            IntNode* int_node = new IntNode();
            int n = read_int(in);
            for(int j = 0; j < n; ++j)
              int_node->ints.push_back(read_int(in));
            section->add(name, int_node);
          }
          break;

        case 0x4: // float
          section->add(name, new FloatNode(read_float(in)));
          break;

        case 0x8: // float array
          {
            FloatNode* float_node = new FloatNode();
            int n = read_int(in);
            for (int j = 0; j < n; ++j)
              float_node->floats.push_back(read_float(in));
            section->add(name, float_node);
          }
          break;

        case 0x10: // string
          section->add(name, new StringNode(read_string(in)));
          break;

        case 0x20: // string array
          {
            StringNode* string_node = new StringNode();
            int n = read_int(in);
            for(int j = 0; j < n; ++j)
              string_node->strings.push_back(read_string(in));
            section->add(name, string_node);
          }
          break;

        case 0x40: // section
          section->add(name, new SectionNodes(parse_section(in)));
          break;

        case 0x80: // multi section
          {
            SectionNodes* section_nodes = new SectionNodes();
            int n = read_int(in);
            for(int j = 0; j < n; ++j)
              section_nodes->sections.push_back(parse_section(in));
            section->add(name, section_nodes);
          }
          break;

        default:
          {
            std::ostringstream str;
            str << "Error: Unrecognized code: 0x" << std::hex << int(code) << " at 0x" << in.tellg();
            throw std::runtime_error(str.str());
          }
          break;
        }
    }
  return section;
}

Shark3D*
Shark3D::parse_text(std::istream& in)
{
  const std::vector<shark3d_text::Token>& tokens = shark3d_text::tokenize(in);

  if (0)
    for(int i = 0; i < int(tokens.size()); ++i)
      {
        std::cout << tokens[i].type << " '" << tokens[i].value << "'" << std::endl;
      }

  int cur = 0;
  Shark3D* doc = new Shark3D();
  doc->magic   = "shark3d_snake_binary";
  doc->version = "2x4";
  doc->section = shark3d_text::parse(tokens, cur);
  return doc;
}

void
Shark3D::write_text(std::ostream& out)
{
  out << "# " << magic << " " << version << std::endl;
  section->write_text(out, "");
  out << "$" << std::endl;
}

void
Shark3D::write_binary(std::ostream& out)
{
  out.write(magic.c_str(), magic.size()+1);
  out.write(version.c_str(), version.size()+1);
  section->write_binary(out);
}

SectionNode::Entries::iterator
Shark3D::find(const std::string& ident) const
{
  for(SectionNode::Entries::iterator i = section->entries.begin(); i != section->entries.end(); ++i)
    {
      if (i->name == ident)
        {
          return i;
        }
    }
  return section->entries.end();
}

int
Shark3D::get_int(const std::string& ident, int def) const
{
  SectionNode::Entries::iterator i = find(ident);
  if (i != section->entries.end())
    {
      IntNode* intnode = dynamic_cast<IntNode*>(i->node);
      if (intnode && intnode->ints.size() != 0)
        return intnode->ints.front();
    }

  return def;
}

std::string
Shark3D::get_string(const std::string& ident, const std::string& def) const
{
  SectionNode::Entries::iterator i = find(ident);
  if (i != section->entries.end())
    {
      StringNode* stringnode = dynamic_cast<StringNode*>(i->node);
      if (stringnode && stringnode->strings.size() != 0)
        return stringnode->strings.front();
    }

  return def;
}

SectionNodes*
Shark3D::get_sections(const std::string& ident) const
{
  SectionNode::Entries::iterator i = find(ident);
  if (i != section->entries.end())
    {
      SectionNodes* sectionnode = dynamic_cast<SectionNodes*>(i->node);
      if (sectionnode && sectionnode->sections.size() != 0)
        return sectionnode;
    }

  return 0;
}

/* EOF */
