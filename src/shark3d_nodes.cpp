/*  $Id: shark3d_nodes.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

#include "util.hpp"
#include "shark3d_nodes.hpp"

std::map<int, std::string> directory;
std::map<std::string, int> directory2;

namespace shark3d_binary {

int read_int(std::istream& in)
{
  int num = 0, c, shift = 0;
  do
    {
      c = in.get();
      num |= (c & 0x7f) << shift;
      shift += 7;
    } while (c >= 0x80 && c != -1);

  return num;
}

float read_float(std::istream& in)
{
  unsigned char byte[4];
  
  byte[3] = in.get();
  byte[2] = in.get();
  byte[1] = in.get();
  byte[0] = in.get();

  return *reinterpret_cast<float*>(byte);
}

std::string read_string_basic(std::istream& in)
{
  std::string str;
  int c;
  while((c = in.get()) != 0 && c != -1)
    str += c;
  return str;
}

std::string read_string(std::istream& in)
{
  int num = read_int(in);
  {
    int idx = directory.size() - num;
   
    if (directory.find(idx) != directory.end())
      {
        return directory[idx];
      }
    else
      {
        std::string ret = read_string_basic(in);
        directory[idx] = ret;
        return ret;
      }
  }
}
} // namespace binary_parser

namespace shark3d_text {

std::string read_string(std::istream& in)
{
  std::string ret;
  int c;

  while(!in.eof() && ret.empty())
    {
      // skip loading space
      while(isspace((c = in.get())));

      if (c == '#')
        { // skip comment
          while((c = in.get()) != '\n');
        }
      else
        {
          ret += c;
        }

      if (ret[0] == '"')
        {
          while((c = in.get()) != '"')
            ret += c;
          ret += '"';
          //ret = ret.substr(1);
        }
      else
        {
          while(!isspace((c = in.get())) && c != -1)
            ret += c;
        }
    }

  return ret;
}

bool is_ident(const std::string& str)
{
  if (!isalpha(str[0]))
    return false;

  for(int i = 0; i < int(str.size()); ++i)
    if (!isalnum(str[i]) && str[i] != '_')
      return false;

  return true;
}

bool is_string(const std::string& str)
{
  return str[0] == '"' && str[str.size()-1] == '"';
}

bool is_int(const std::string& str)
{
  for(int i = 0; i < int(str.size()); ++i)
    if (!isdigit(str[i]) && str[i] != '-')
      return false;
  return true;
}

bool is_float(const std::string& str)
{
  for(int i = 0; i < int(str.size()); ++i)
    if (!isdigit(str[i]) && str[i] != '-' && str[i] != '+' && str[i] != 'e' && str[i] != '.')
      return false;
  return true; 
}

Token::Token(Type type_, const std::string& v) 
  : type(type_), value(v)
{}

int
Token::get_int() const
{
  assert(type == T_INT);
  int v = 0; 
  assert(from_string(value, v));
  return v;
}

float
Token::get_float() const
{
  assert(type == T_FLOAT);
  float v = 0; 
  assert(from_string(value, v));
  return v;
}

std::string
Token::get_string() const
{
  if (type == T_STRING)
    return value.substr(1, value.size()-2);
  else if (type == T_IDENT)
    return value;
  else
    assert(false);
}

std::vector<Token> tokenize(std::istream& in)
{
  std::vector<Token> tokens; 

  while(true)
    {
      std::string str = read_string(in);

      if (!in)
        {
          tokens.push_back(Token(Token::T_EOF, str));
          break;          
        }

      if (is_ident(str))
        {
          tokens.push_back(Token(Token::T_IDENT, str));
        }
      else if (is_int(str))
        {
          tokens.push_back(Token(Token::T_INT, str));
        }
      else if (is_float(str))
        {
          tokens.push_back(Token(Token::T_FLOAT, str));
        }
      else if (is_string(str))
        {
          tokens.push_back(Token(Token::T_STRING, str));
        }
      else if (str == "$")
        {
          tokens.push_back(Token(Token::T_EOF, str));
          break;
        }
      else if (str == "{")
        {
          tokens.push_back(Token(Token::T_SECTION_START, str));
        }
      else if (str == "}")
        {
          tokens.push_back(Token(Token::T_SECTION_END, str));
        }
      else if (str == "{}")
        {
          tokens.push_back(Token(Token::T_NULL, str));
        }
      else 
        {
          std::ostringstream out;
          out << "Error: Unknown token '" << str << "'";
          throw std::runtime_error(out.str());          
        }
    }
  return tokens;
}

SectionNode* parse(const std::vector<Token>& tokens, int& cur)
{
  SectionNode* section = new SectionNode();

  while(true)
    {
      // Get the ident
      std::string ident;
      if (tokens[cur].type == Token::T_IDENT)
        {
          ident = tokens[cur].get_string();
          cur += 1;
        }
      else if (tokens[cur].type == Token::T_SECTION_END ||
               tokens[cur].type == Token::T_EOF)
        {
          cur += 1;
          return section;
        }
      else
        {
          std::ostringstream str;
          str << "Error: Unexpected token, wanted ident, but got " << tokens[cur].type << " '" << tokens[cur].value << "'";
          throw std::runtime_error(str.str());
        }

      // Get the value
      switch(tokens[cur].type)
        {
        case Token::T_STRING:
          {
            StringNode* node = new StringNode();
            while(tokens[cur].type == Token::T_STRING)
              {
                node->strings.push_back(tokens[cur].get_string());
                cur += 1;
              }
            section->add(ident, node);
          }
          break;
      
        case Token::T_INT:
          {
            IntNode* node = new IntNode();
            while(tokens[cur].type == Token::T_INT)
              {
                node->ints.push_back(tokens[cur].get_int());
                cur += 1;
              }
            section->add(ident, node);
          }
          break;

        case Token::T_FLOAT:
          {
            FloatNode* node = new FloatNode();
            while(tokens[cur].type == Token::T_FLOAT)
              {
                node->floats.push_back(tokens[cur].get_float());
                cur += 1;
              }
            section->add(ident, node);
          }
          break;

        case Token::T_NULL:
          section->add(ident, new NullNode());
          cur += 1;
          break;

          // Section Marker
        case Token::T_SECTION_START:
          {
            SectionNodes* node = new SectionNodes();
            while(tokens[cur].type == Token::T_SECTION_START)
              {
                node->sections.push_back(parse(tokens, ++cur));
              }
            section->add(ident, node);
          }
          break;

        default:
          std::ostringstream str;
          str << "Error: Unexpected token, wanted value got " << tokens[cur].type << "'" << tokens[cur].value << "'";
          throw std::runtime_error(str.str());
        }
    }  

  return section;
}

} // namespace shark3d_text

void write_int(std::ostream& out, int iv)
{
  unsigned int v = *reinterpret_cast<unsigned int*>(&iv);
  do { 
    if ((v & 0x7f) >= v)
      out.put((v & 0x7f));
    else
      out.put((v & 0x7f) | 0x80);
    v = v >> 7;
    //std::cerr << v << std::endl;
  }  while(v != 0);
}

void write_float(std::ostream& out, float v)
{
  char* buf = reinterpret_cast<char*>(&v);
  out.put(buf[3]);
  out.put(buf[2]);
  out.put(buf[1]);
  out.put(buf[0]);
}

void write_string(std::ostream& out, const std::string& str, bool use_dir = true)
{
  if (use_dir)
    {
      std::map<std::string, int>::iterator it = directory2.find(str);
      if (it == directory2.end())
        {
          directory2[str] = directory2.size();
          out.put(0);
          out.write(str.c_str(), str.size()+1);
        }
      else
        {
          write_int(out, directory2.size() - it->second + 1);
        }
    }
  else
    {
      out.put(0);
      out.write(str.c_str(), str.size()+1);
    }
}


IntNode::IntNode() 
{
}

IntNode::IntNode(int i)
{
  ints.push_back(i); 
}

void
IntNode::write_text(std::ostream& out, const std::string& prefix) 
{
  for(std::vector<int>::iterator i = ints.begin(); i != ints.end(); ++i)
    out << *i << " ";
}

void
IntNode::write_binary(std::ostream& out)
{
  assert(ints.size() > 0);
  if (ints.size() == 1) 
    {
      out.put(0x1);
      write_int(out, ints[0]);
    }
  else
    {
      out.put(0x2);
      write_int(out, ints.size());
      for(int i = 0; i < int(ints.size()); ++i)
        write_int(out, ints[i]);
    }
}


FloatNode::FloatNode() 
{
}

FloatNode::FloatNode(float f)
{
  floats.push_back(f); 
}

void
FloatNode::write_text(std::ostream& out, const std::string& prefix) 
{
  for(std::vector<float>::iterator i = floats.begin(); i != floats.end(); ++i)
    {  
      out.precision(26); // FIXME: not sure how much we need
      out << std::showpoint << *i << " ";
    }
}

void
FloatNode::write_binary(std::ostream& out) 
{
  assert(floats.size() > 0);
  if (floats.size() == 1) 
    {
      out.put(0x4);
      write_float(out, floats[0]);
    }
  else
    {
      out.put(0x8);
      write_int(out, floats.size());
      for(int i = 0; i < int(floats.size()); ++i)
        write_float(out, floats[i]);
    }
}

StringNode::StringNode() 
{
}

StringNode::StringNode(const std::string& str) 
{
  strings.push_back(str); 
}

void
StringNode::write_text(std::ostream& out, const std::string& prefix) 
{
  for(std::vector<std::string>::iterator i = strings.begin(); i != strings.end(); ++i)
    out << "\"" << *i << "\" ";
}

void
StringNode::write_binary(std::ostream& out)
{
  assert(strings.size() > 0);
  if (strings.size() == 1) 
    {
      out.put(0x10);
      write_string(out, strings[0]);
    }
  else
    {
      out.put(0x20);
      write_int(out, strings.size());
      for(int i = 0; i < int(strings.size()); ++i)
        {
          write_string(out, strings[i]);
        }
    }
}

void
NullNode::write_text(std::ostream& out, const std::string& prefix) {
  out << "{}";
}

void
NullNode::write_binary(std::ostream& out)
{
  out.put(0);
}

SectionNode::SectionNode() 
{
}

SectionNode::~SectionNode() 
{
  for(Entries::iterator i = entries.begin(); i != entries.end(); ++i)
    delete i->node;
}

void
SectionNode::add(const std::string& name_, Node* node_) 
{
  entries.push_back(Entry(name_, node_));
}

void
SectionNode::write_text(std::ostream& out, const std::string& prefix) 
{
  for(Entries::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      out << prefix << i->name << ' ';
      i->node->write_text(out, prefix);
      out << std::endl;;
    }
}

void
SectionNode::write_binary(std::ostream& out)
{
  //assert(entries.size() > 0);
  write_int(out, entries.size());
  for(Entries::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      write_string(out, i->name);
      i->node->write_binary(out);
    }
}

SectionNodes::SectionNodes() 
{
}

SectionNodes::SectionNodes(SectionNode* node) 
{
  sections.push_back(node); 
}

void
SectionNodes::write_text(std::ostream& out, const std::string& prefix) 
{
  for(std::vector<SectionNode*>::size_type i = 0; i < sections.size(); ++i)
    {
      out << std::endl;
      out << prefix << "{" << std::endl;
      sections[i]->write_text(out, prefix + "   ");
      out << prefix << "}";
    }
}

void
SectionNodes::write_binary(std::ostream& out)
{
  assert(sections.size() > 0);
  if (sections.size() > 1)
    {
      out.put(0x80);
      write_int(out, sections.size());
    }
  else
    {
      out.put(0x40);
    }
  for(std::vector<SectionNode*>::size_type i = 0; i < sections.size(); ++i)
    {
      sections[i]->write_binary(out);
    }
}

std::vector<float>
SectionNode::get_floats(const std::string& ident) const
{
  for(Entries::const_iterator i = entries.begin(); i != entries.end(); ++i)
    {
      if (i->name == ident)
        {
          FloatNode* node = dynamic_cast<FloatNode*>(i->node);
          if (node)
            return node->floats;
        }
    }  

  return std::vector<float>();
}

SectionNode*
SectionNode::get_section(const std::string& ident) const
{
  const std::vector<SectionNode*>& sections = get_sections(ident);
  if (sections.size() == 1)
    {
      return sections.front();
    }
  else if (sections.size() > 1)
    {
      std::cout << "Warning: Found more then one section for: " << ident << std::endl;
      return sections.front();
    }
  else
    {
      std::cout << "Warning: Couldn't find section for: " << ident << std::endl;
      return 0;
    }
}

int
SectionNode::get_int(const std::string& ident) const
{
  const std::vector<int>& ints = get_ints(ident);
  if (ints.size() == 1)
    {
      return ints.front();
    }
  else if (ints.size() > 1)
    {
      std::cout << "Warning: Found more then one int for: " << ident << std::endl;
      return ints.front();
    }
  else
    {
      std::cout << "Warning: Couldn't find int for: " << ident << std::endl;
      return 0;
    }
}

float
SectionNode::get_float(const std::string& ident) const
{
  // FIXME: add error checking
  return get_floats(ident).front();
}

std::string
SectionNode::get_string(const std::string& ident) const
{
  // FIXME: add error checking
  return get_strings(ident).front();
}

std::vector<int>
SectionNode::get_ints(const std::string& ident) const
{
  for(Entries::const_iterator i = entries.begin(); i != entries.end(); ++i)
    {
      if (i->name == ident)
        {
          IntNode* node = dynamic_cast<IntNode*>(i->node);
          if (node)
            return node->ints;
        }
    }  

  return std::vector<int>();
}

std::vector<SectionNode*>
SectionNode::get_sections(const std::string& ident) const
{
  for(Entries::const_iterator i = entries.begin(); i != entries.end(); ++i)
    {
      if (i->name == ident)
        {
          SectionNodes* node = dynamic_cast<SectionNodes*>(i->node);
          if (node)
            return node->sections;
        }
    }  

  return std::vector<SectionNode*>();  
}

std::vector<std::string>
SectionNode::get_strings(const std::string& ident) const
{
  for(Entries::const_iterator i = entries.begin(); i != entries.end(); ++i)
    {
      if (i->name == ident)
        {
          StringNode* node = dynamic_cast<StringNode*>(i->node);
          if (node)
            return node->strings;
        }
    }  
  return std::vector<std::string>();
}

/* EOF */
