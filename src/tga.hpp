/*  $Id: tga.hpp,v 1.2 2006/06/16 02:17:53 ingo Exp ingo $
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
#include <vector>

struct TGA
{
  char  identsize;          // size of ID field that follows 18 char header (0 usually)
  char  colourmaptype;      // type of colour map 0=none, 1=has palette
  char  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

  short colourmapstart;     // first colour map entry in palette
  short colourmaplength;    // number of colours in palette
  char  colourmapbits;      // number of bits per palette entry 15,16,24,32

  short xstart;             // image x origin
  short ystart;             // image y origin
  short width;              // image width in pixels
  short height;             // image height in pixels
  char  bits;               // image bits per pixel 8,16,24,32
  char  descriptor;         // image descriptor bits (vh flip bits)
    
  std::vector<char> buffer;

  // pixel data follows header
  TGA(short width_, short height_, const std::vector<char>& buffer_)
    : identsize(0), 
      colourmaptype(0),
      imagetype(2),
      
      colourmapstart(0),
      colourmaplength(0),
      colourmapbits(0),

      xstart(0),
      ystart(0),

      width(width_), 
      height(height_),
      bits(32),
      descriptor(0 | (1 << 5))   
  {
    buffer.resize(buffer_.size());
    for(int i = 0; i < int(buffer.size())/4; ++i)
      {
        buffer[4*i+0] = buffer_[4*i+2];
        buffer[4*i+1] = buffer_[4*i+1];
        buffer[4*i+2] = buffer_[4*i+0];
        buffer[4*i+3] = buffer_[4*i+3];
      }
  }

  TGA(std::istream& in)
  {
    in.read(reinterpret_cast<char*>(&identsize),       sizeof(identsize));
    in.read(reinterpret_cast<char*>(&colourmaptype),   sizeof(colourmaptype));
    in.read(reinterpret_cast<char*>(&imagetype),       sizeof(imagetype));

    in.read(reinterpret_cast<char*>(&colourmapstart),   sizeof(colourmapstart));
    in.read(reinterpret_cast<char*>(&colourmaplength), sizeof(colourmaplength));
    in.read(reinterpret_cast<char*>(&colourmapbits),   sizeof(colourmapbits));

    in.read(reinterpret_cast<char*>(&xstart),          sizeof(xstart));
    in.read(reinterpret_cast<char*>(&ystart),          sizeof(ystart));

    in.read(reinterpret_cast<char*>(&width),           sizeof(width));
    in.read(reinterpret_cast<char*>(&height),          sizeof(height));
    in.read(reinterpret_cast<char*>(&bits),            sizeof(bits));
    in.read(reinterpret_cast<char*>(&descriptor),      sizeof(descriptor));    

    buffer.resize(width * height * bits/8);
    in.read(&*buffer.begin(), buffer.size());
  }
   
  char* get_data() 
  {
    return &*buffer.begin();
  }

  int get_size()
  {
    return buffer.size();
  }

  void write(std::ostream& out)
  {
    out.write(reinterpret_cast<char*>(&identsize),       sizeof(identsize));
    out.write(reinterpret_cast<char*>(&colourmaptype),   sizeof(colourmaptype));
    out.write(reinterpret_cast<char*>(&imagetype),       sizeof(imagetype));

    out.write(reinterpret_cast<char*>(&colourmapstart),   sizeof(colourmapstart));
    out.write(reinterpret_cast<char*>(&colourmaplength), sizeof(colourmaplength));
    out.write(reinterpret_cast<char*>(&colourmapbits),   sizeof(colourmapbits));

    out.write(reinterpret_cast<char*>(&xstart),          sizeof(xstart));
    out.write(reinterpret_cast<char*>(&ystart),          sizeof(ystart));

    out.write(reinterpret_cast<char*>(&width),           sizeof(width));
    out.write(reinterpret_cast<char*>(&height),          sizeof(height));
    out.write(reinterpret_cast<char*>(&bits),            sizeof(bits));
    out.write(reinterpret_cast<char*>(&descriptor),      sizeof(descriptor));

    out.write(&*buffer.begin(), buffer.size());
  }
};

/* EOF */
