/*  $Id: shark3d.hpp,v 1.6 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SHARK3D_HPP
#define HEADER_SHARK3D_HPP

#include <istream>
#include <ostream>
#include <vector>
#include <string>

#include "shark3d_nodes.hpp"

class Shark3D
{
public:
  std::string magic;
  std::string version;

  SectionNode* section;

  Shark3D();
  virtual ~Shark3D();

  SectionNode* parse_section(std::istream& in);

  static Shark3D* parse_binary(std::istream& in);
  static Shark3D* parse_text  (std::istream& in);

  void write_text(std::ostream& out);
  void write_binary(std::ostream& out);

  SectionNode::Entries::iterator find(const std::string& ident) const;
  int           get_int(const std::string& ident, int def) const;
  std::string   get_string(const std::string& ident, const std::string& def) const;
  SectionNodes* get_sections(const std::string& ident) const;
};

#endif

/* EOF */
