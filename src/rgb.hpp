/*  $Id: rgb.hpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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

#ifndef HEADER_RGB_HPP
#define HEADER_RGB_HPP

struct RGB
{
  unsigned int r;
  unsigned int g;
  unsigned int b;

  RGB() : r(0), g(0), b(0) {}

  RGB(unsigned short color)
    : r((color & 0x1F) << 3),
      g(((color & 0x7E0) >> 5) << 2),
      b(((color & 0xF800) >> 11) << 3)
  {
  }

  RGB& operator=(unsigned short color)
  {
    r = (color & 0x1F) << 3;
    g = ((color & 0x7E0) >> 5) << 2;
    b = ((color & 0xF800) >> 11) << 3;

    return *this;
  }
};

inline RGB operator*(unsigned int s, const RGB& a)
{
  RGB result;
  result.r = a.r * s;
  result.g = a.g * s;
  result.b = a.b * s;
  return result;
}

inline RGB operator*(const RGB& a, unsigned int s)
{
  RGB result;
  result.r = a.r * s;
  result.g = a.g * s;
  result.b = a.b * s;
  return result;
}

inline RGB operator/(const RGB& a, unsigned int s)
{
  RGB result;
  result.r = a.r / s;
  result.g = a.g / s;
  result.b = a.b / s;
  return result;
}

inline RGB operator+(const RGB& a, const RGB& b)
{
  RGB result;
  result.r = a.r + b.r;
  result.g = a.g + b.g;
  result.b = a.b + b.b;
  return result;
}


#endif

/* EOF */
