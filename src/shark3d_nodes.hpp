/*  $Id: shark3d_nodes.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_SHARK3D_NODES_HPP
#define HEADER_SHARK3D_NODES_HPP

#include <istream>
#include <vector>
#include <string>
#include "util.hpp"

class SectionNode;

namespace shark3d_binary {
int         read_int(std::istream& in);
float       read_float(std::istream& in);
std::string read_string_basic(std::istream& in);
std::string read_string(std::istream& in);
} // namespace shark3d_binary

namespace shark3d_text {
std::string read_string(std::istream& in);
bool is_ident(const std::string& str);
bool is_string(const std::string& str);
bool is_int(const std::string& str);
bool is_float(const std::string& str);

struct Token {
  enum Type { T_IDENT, T_STRING, T_INT, T_FLOAT, T_SECTION_START, T_SECTION_END, T_NULL, T_EOF };

  Type type;
  std::string value;

  Token(Type type_, const std::string& v);

  int   get_int() const;
  float get_float() const;
  std::string get_string() const;
};

std::vector<Token> tokenize(std::istream& in);
SectionNode* parse(const std::vector<Token>& tokens, int& cur);
} // namespace shark3d_text


class Node   {
public:
  virtual ~Node() {}
  virtual void write_text(std::ostream& out, const std::string& prefix) = 0;
  virtual void write_binary(std::ostream& out) = 0;
};

class IntNode : public Node {
public:
  std::vector<int>  ints;

  IntNode();
  IntNode(int i);

  void write_text(std::ostream& out, const std::string& prefix);
  void write_binary(std::ostream& out);
};

class FloatNode : public Node {
public:
  std::vector<float> floats;

  FloatNode();
  FloatNode(float f);

  void write_text(std::ostream& out, const std::string& prefix);
  void write_binary(std::ostream& out);
};

class StringNode : public Node {
public:
  std::vector<std::string> strings;

  StringNode();
  StringNode(const std::string& str);

  void write_text(std::ostream& out, const std::string& prefix);
  void write_binary(std::ostream& out);
};

class NullNode : public Node {
public:
  void write_text(std::ostream& out, const std::string& prefix);
  void write_binary(std::ostream& out);
};

class SectionNode : public Node {
public:
  struct Entry {
    std::string name;
    Node* node;

    Entry(const std::string& name_, Node* node_)
      : name(name_), node(node_)
    {}
  };

  typedef std::vector<Entry> Entries;
  Entries entries;

  SectionNode();
  virtual ~SectionNode();

  void add(const std::string& name_, Node* node_);
  void write_text(std::ostream& out, const std::string& prefix);
  void write_binary(std::ostream& out);

  bool         has_subnode(const std::string& ident) const;

  int          get_int(const std::string& ident) const;
  float        get_float(const std::string& ident) const;
  std::string  get_string(const std::string& ident) const;
  SectionNode* get_section(const std::string& ident) const;

  std::vector<int>          get_ints(const std::string& ident) const;
  std::vector<float>        get_floats(const std::string& ident) const;
  std::vector<std::string>  get_strings(const std::string& ident) const;
  std::vector<SectionNode*> get_sections(const std::string& ident) const;
};

class SectionNodes : public Node {
public:
  std::vector<SectionNode*> sections;

  SectionNodes();
  SectionNodes(SectionNode* node);

  void write_text(std::ostream& out, const std::string& prefix);
  void write_binary(std::ostream& out);
};

#endif

/* EOF */
