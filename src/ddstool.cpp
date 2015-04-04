/*  $Id: ddstool.cpp,v 1.1 2006/06/16 02:17:53 ingo Exp ingo $
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
#include "tga.hpp"
#include "dds.hpp"

int main(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i)
    {
      std::ifstream in(argv[i], std::ios::binary);
      if(!in)
        {
          std::cout << "Error: Couldn't open " << argv[i] << std::endl;
        }
      else
        {
          DDS dds(in);

          std::cout << "Filename:      " << argv[i] << std::endl;
          std::cout << "Size:          " << dds.width << "x" << dds.height << std::endl;
          std::cout << "Flags:         " << dds.flags << std::endl;
          std::cout << "Pitch:         " << dds.dwPitchOrLinearSize << std::endl;
          std::cout << "Depth:         " << dds.dwDepth << std::endl;
          std::cout << "MipmapCount:   " << dds.dwMipMapCount << std::endl;
          std::cout << "PixFmt.FourCC: " << std::string((char*)(&dds.pixel_format.dwFourCC), 4) << " " << dds.pixel_format.dwFourCC << std::endl;
          std::cout << "PixFmt.Bits:   " << dds.pixel_format.dwRGBBitCount << std::endl;
          std::cout << "PixFmt.RMask   " << dds.pixel_format.dwRBitMask << std::endl;
          std::cout << "PixFmt.BMask   " << dds.pixel_format.dwBBitMask << std::endl;
          std::cout << "PixFmt.GMask   " << dds.pixel_format.dwGBitMask << std::endl;
          std::cout << "PixFmt.AMask   " << dds.pixel_format.dwRGBAlphaBitMask << std::endl;
          std::cout << std::endl;

          std::ofstream out("data.tga", std::ios::binary);
          TGA tga(dds.width, dds.height, dds.get_data());
          tga.write(out);
          out.close();
        }
    }
}

/* EOF */
