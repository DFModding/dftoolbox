/*  $Id: bundle.cpp,v 1.5 2006/06/16 02:17:53 ingo Exp ingo $
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

#include <assert.h>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include "system.hpp"

/*
   File Layout:

   offset
   text_count
   data: width 20 bytes each
   followed by data with 32 or 64 width
   followed by unknown
   followed by something with width 76
   followed by something with width 28

   some data seems to be the alphabet, some other data seems to be skeleton structure

   0002.dat might contain the scene 3d model!
*/
//tower_square.bun

struct Mesh
{
  float rescale;

  Vector pos_center;
  Vector pos_bound;

  std::string name;

  std::vector<int> textures;
};

bool is_uv(float f)
{
  return f <= 1.0f and f >= -1.0f;
}

bool is_one_or_zero(float f)
{
  if (f == -1.0f || f == 1.0f || f == 0.0f)
    return true;
  else if (f < 0.000001f and f > -0.000001f)
    return true;
  else
    return false;
}
/*
  block sizes used:
  8 block: 12
  5315 block: 20
  20 block: 24
  20 block: 28
  4 block: 32
  557 block: 44
  848 block: 56
  1527 block: 60
  1905 block: 64
  31 block: 72
  544 block: 76
  14 block: 80

*/

std::string directory_name_from_bun(const std::string& name)
{
  std::string::size_type pos1 = name.find_last_of('\\');
  std::string::size_type pos2 = name.find_last_of('/');
  std::string::size_type dotpos = name.find_last_of('.');
  std::string::size_type namestart;

  if (pos1 == std::string::npos && pos2 == std::string::npos)
    namestart = 0;
  else if (pos1 == std::string::npos)
    namestart = pos2 + 1;
  else
    namestart = pos1 + 1;

  if (dotpos == std::string::npos || dotpos <= namestart || namestart > name.size())
    throw std::runtime_error("Error: Not a valid .bun pathname: " + name);

  return name.substr(namestart, dotpos - namestart);
}

class Bundle
{
public:
  /** Chunk of vertex data */
  struct Chunk {
    int size;

    // length of the stored data (64, 20, 80, ...)
    int length;
    int offset;
  };

  /** ??? */
  struct StreamFormat
  {
    int size;
    int channel[16];
    int extra;
  };

  // Offset where the data starts after the strings
  int data_offset;
  int files_offset;

  int numFiles;
  int numStreamFormats;
  int numSomething;

  std::vector<std::string> textures;

  /** Table that points to blocks with float data */
  std::vector<Chunk> chunks;
  std::vector<StreamFormat> stream_formats;
  std::vector<unsigned int> mesh_pos;
  std::vector<int> file_offsets;

  void read_textures(std::istream& in)
  {
    int entry_count = read_int32(in);

    for(int i = 0; i < entry_count; ++i)
      {
        char length = in.get(); // length of the string that follows
        char buffer[length+1];
        in.read(buffer, length+1);
        textures.push_back(buffer);

        //std::cout << "Texture: " << buffer << std::endl;
      }
  }

  void read_chunks(std::istream& in)
  {
    int chunk_count = read_int32(in);
    //std::cout << "ChunkCount: " << chunk_count << std::endl;

    // Read Chunks
    for(int i = 0; i < chunk_count; ++i)
      {
        int length = read_int32(in);
        int size   = read_int32(in);

        Chunk chunk;

        chunk.size   = size;
        chunk.length = length;
        chunk.offset = in.tellg();

        chunks.push_back(chunk);

        // skip chunk data
        in.seekg(size, std::ios::cur);

        //std::cout << "Chunk: " << chunk.length << " " << chunk.size << " " << chunk.offset << std::endl;
      }
  }

  void read_mesh(std::istream& in, int j)
  {
    in.seekg(mesh_pos[j], std::ios::beg);

    int mesh_posName = read_int32(in) + files_offset;

    Mesh mesh;

    mesh.rescale      = read_float(in); // 1.0f/read_float(in);

    mesh.pos_center = read_vector(in);
    mesh.pos_bound  = read_vector(in);

    // 0 Junk
    for(int i = 0; i < 6; ++i)
      assert(read_int32(in) == 0);

    int num_bones  = read_int32(in);
    int bone_names_pos = read_int32(in) + files_offset;
    int bone_data_pos  = read_int32(in) + files_offset;

    int num_textures = read_int32(in);
    int textures_pos = read_int32(in) + files_offset;

    // 0 Junk
    for(int i = 0; i < 2; ++i)
      assert(read_int32(in) == 0);

    int num_cf = read_int32(in);
    int posCF[num_cf] __attribute__((unused));
    for (int k = 0; k < num_cf; ++k)
      {
        posCF[k] = read_int32(in) + files_offset;
        // pointer to useless '0xcf' bytes
        //std::cout << "CF: " << posCF[k] << std::endl;
      }

    int current_pos = in.tellg();
    mesh.name = read_string0(in);

    if (num_bones > 0)
      {
        std::vector<std::string> bonenames;

        for (int i = 0; i < num_bones; ++i)
          {
            bonenames.push_back(&*read_bytes(in, 0x28).begin());
            std::cout << "BoneName: " << bonenames.back() << std::endl;
          }

        std::cout << "Bone Float: ";
        for (int k = 0; k < num_bones; k++)
          {
            for (int l = 0; l < 7; l++)
              std::cout << read_float(in) << " ";
            std::cout << std::endl;
          }
        std::cout << std::endl;
      }

    std::cout << "\n------------------------------\n"
              << "Meshname:     " << mesh.name << "\n"
              << "VectorCenter: "
              << mesh.pos_center.x << " " << mesh.pos_center.y << " " << mesh.pos_center.z << std::endl
              << "VectorBound: "
              << mesh.pos_bound.x << " " << mesh.pos_bound.y << " " << mesh.pos_bound.z << std::endl
              << "Rescale:      " << mesh.rescale << "\n\n"
              << "numbones:     " << num_bones << "\n"
              << "bone_names_pos:   " << bone_names_pos << "\n"
              << "bone_data_pos:    " << bone_data_pos << "\n\n"
              << "num_textures: " << num_textures << "\n"
              << "textures:     " << textures_pos << "\n"
              << "POS:          " << current_pos << " " << mesh_posName
              << std::endl;


    if (num_textures > 0)
      assert(static_cast<int>(in.tellg()) == textures_pos);

    for(int i = 0; i < num_textures; ++i)
      {
        mesh.textures.push_back(read_int32(in));
        std::cout << "Textures: " << mesh.textures.back() << std::endl;
      }


    // skip over cf junk
    for(int i = 0; i < 18*4; ++i)
      in.get();

    int numMagic = read_int32(in);
    int posMagic = read_int32(in) + files_offset;

    int val5_2 = read_int32(in);
    if (val5_2 == 0)
      return;
    int formatIndex = (val5_2/4 - file_offsets.size()-3) /18;
    int val5_1 = read_int32(in);
    int vxPos  = read_int32(in);
    int val5_3 = read_int32(in);
    int val5_4 = read_int32(in);

    int  numIdx = read_int32(in);
    int  posIdx = read_int32(in) + files_offset;

    int  numTa1 = read_int32(in);
    int  posTa1 = read_int32(in) + files_offset;
    int  numTa2 = read_int32(in);
    int  posTa2 = read_int32(in) + files_offset;

    int posAfter = read_int32(in) + files_offset;
    int posSth   = read_int32(in) + files_offset;


    std::cout << "numMagic: " << numMagic << std::endl;
    std::cout << "posMagic: " << posMagic << std::endl;

    std::cout << "val5_2: " << val5_2 << std::endl;
    std::cout << "formatIndex: " << formatIndex << std::endl;
    std::cout << "val5_1: " << val5_1 << std::endl;
    std::cout << "vxPos:  " << vxPos << std::endl;
    std::cout << "val5_3: " << val5_3 << std::endl;
    std::cout << "val5_4: " << val5_4 << std::endl;

    std::cout << "numIdx: " << numIdx << std::endl;
    std::cout << "posIdx: " << posIdx << std::endl;

    std::cout << "numTa1: " << numTa1 << std::endl;
    std::cout << "posTa1: " << posTa1 << std::endl;
    std::cout << "numTa2: " << numTa2 << std::endl;
    std::cout << "posTa2: " << posTa2 << std::endl;

    std::cout << "posAfter: " << posAfter << std::endl;
    std::cout << "posSth:   " << posSth << std::endl;

    std::cout << "Something: " << read_int32(in) << std::endl;
    std::cout << "Something (==0?): " << read_int32(in) << std::endl; // should be 0


    for (int l = 0; l < 4; ++l)
      std::cout << "Strange: " << read_int32(in) << std::endl;

    int  numTaX1 = read_int32(in);
    long posTaX1 = read_int32(in) + files_offset;
    int  numTaX2 = read_int32(in);
    long posTaX2 = read_int32(in) + files_offset;
    int  numTaX3 = read_int32(in);
    long posTaX3 = read_int32(in) + files_offset;

    int  numTa3  = read_int32(in);
    long posTa3a = read_int32(in) + files_offset;
    long posTa3b = read_int32(in) + files_offset;
    long posTa3c = read_int32(in) + files_offset;

    long posAfter2 = read_int32(in) + files_offset;
    int  numTa4    = read_int32(in);
    long posTa4    = read_int32(in) + files_offset;

    int numVertices = read_int32(in);

    std::cout << "numTaX1: " << numTaX1 << std::endl;
    std::cout << "posTaX1: " << posTaX1 << std::endl;
    std::cout << "numTaX2: " << numTaX2 << std::endl;
    std::cout << "posTaX2: " << posTaX2 << std::endl;
    std::cout << "numTaX3: " << numTaX3 << std::endl;
    std::cout << "posTaX3: " << posTaX3 << std::endl;

    std::cout << "numTa3: " << numTa3 << std::endl;
    std::cout << "posTa3a: " << posTa3a << std::endl;
    std::cout << "posTa3b: " << posTa3b << std::endl;
    std::cout << "posTa3c: " << posTa3c << std::endl;

    std::cout << "posAfter2: " << posAfter2 << std::endl;
    std::cout << "numTa4: " << numTa4 << std::endl;
    std::cout << "posTa4: " << posTa4 << std::endl;
    std::cout << "numVertices: " << numVertices << std::endl;

    // FIXME: THIS IS BUGGY
    std::cout << "posBonus: @" << in.tellg() << " ";
    long posBonus[22];
    for (int l = 0; l < 19; l++)
      {
        posBonus[l] = read_int32(in) + files_offset;
        std::cout << " " << posBonus[l];
      }
    std::cout << std::endl;


    int  numIdxBonus = read_int32(in);
    long posIdxBonus = read_int32(in) + files_offset;

    std::cout << "numIdxBonus: " << numIdxBonus << std::endl;
    std::cout << "posIdxBonus: " << posIdxBonus << std::endl;


    int numLinx = read_int32(in);
    if (numLinx == 0)
      return ;

    std::cout << "numLinx: " << numLinx << std::endl;
    return; // after this: buggy

    long posLinx[numLinx] __attribute__((unused));
    for (int l = 0; l < numLinx; l++)
      {
        posLinx[l] = read_int32(in);
      }

    in.seekg(posIdx, std::ios::beg);
    std::cout << "idxPos: " << in.tellg() << " vs " << posIdx << std::endl;


    unsigned short indices[numIdx];
    std::cout << "indices: ";
    for (int i = 0; i < numIdx; i++)
      {
        indices[i] = read_uint16(in);
        std::cout << " " << indices[i];
      }
    std::cout << std::endl;

    read_bytes(in, numTa1 * 2);
    read_bytes(in, numTa2 * 2);
    read_bytes(in, numTa2 * 2);
    if (posSth != 0)
      read_bytes(in, numTa2 * 2);

    read_bytes(in, numTaX1 * 4);
    read_bytes(in, numTaX2 * 4);
    read_bytes(in, numTaX3 * 4);

    int stageVx[numTa3] __attribute__((unused));
    for (int i = 0; i < numTa3; i++)
      stageVx[i] = read_int32(in);

    int stageIdx[numTa3] __attribute__((unused));
    for (int i = 0; i < numTa3; i++)
      stageIdx[i] = read_int32(in);

    if (posTa3c != 0)
      read_bytes(in, numTa3 * 4);

    read_bytes(in, numTa3 * 4);

    read_bytes(in, numTa4 * 4);

    // ...


    if ((vxPos & 0xff) != 0)
      return;

    if (vxPos == 0 || stream_formats[formatIndex].size == 0)
      return;


    int curIdx = 0; // FIXME: WRONG!
    if (chunks[curIdx].size / chunks[curIdx].length != numVertices)
      std::cout << "len mismatch: " << numVertices << " " << chunks[curIdx].size/chunks[curIdx].length << std::endl;

    in.seekg(chunks[curIdx].offset, std::ios::beg);

    std::vector<int> vertices;
    //= new uint[chunks[curIdx].size() * numVertices/4];
    for (int y = 0; y < chunks[curIdx].size * numVertices/4; y++)
      {
        vertices.push_back(read_int32(in));
      }
    curIdx++;
  }

  void read_file(std::istream& in, int e)
  {
    in.seekg(file_offsets[e], std::ios::beg);
    std::cout << "\n########### File (" << e << "): " << file_offsets[e] << std::endl;

    char filename[0x80];
    in.read(filename, 0x80);

    std::cout << "Modelfile: " << filename << std::endl;

    int numMeshes = read_int32(in);

    std::cout << "numMeshes: " << numMeshes << std::endl;
    for (int j = 0; j < numMeshes; ++j)
      {
        mesh_pos.push_back(read_int32(in) + files_offset);
        //std::cout << "mesh_pos: " << mesh_pos.back() << "  @ " << in.tellg() << std::endl;
      }

    for (int j = 0; j < numMeshes; ++j)
      {
        read_mesh(in, j);
      }
  }

  void read_files(std::istream& in)
  {
    std::cout << "Files: " << numFiles << " " << numStreamFormats << " " << numSomething << std::endl;

    // Read Files
    for (int i = 0; i < numFiles; ++i)
      {
        int offset = read_int32(in);
        file_offsets.push_back(offset + files_offset);

        // std::cout << "File Offsets: " << offset << std::endl;
      }

    // Read StreamFormat
    for (int k = 0; k < numStreamFormats; k++)
      {
        StreamFormat format;

        format.size = read_int32(in) / 4;
        for (int i = 0; i < 16; i++)
          format.channel[i] = read_int32(in);
        format.extra = read_int32(in);

        stream_formats.push_back(format);

        std::cout << "StreamFormat: " << format.size << " " << format.extra << std::endl;
      }

    std::cout << "Begin of FileData: " << in.tellg() << std::endl;

    for (int e = 0; e < numFiles; e++)
      {
        read_file(in, e);
      }
  }

  Bundle(std::istream& in)
  {
    in.seekg(0, std::ios::end);
    std::cout << "Filesize: " << in.tellg() << std::endl;
    in.seekg(0, std::ios::beg);

    data_offset = read_int32(in);

    read_textures(in);
    read_chunks(in);

    files_offset = in.tellg();

    // Read Files
    numFiles         = read_int32(in);
    numStreamFormats = read_int32(in);
    numSomething     = read_int32(in);  // FIXME: What is this!?

    read_files(in);
  }
};

int main(int argc, char** argv)
{
  for(int arg_i = 1; arg_i < argc; ++arg_i)
    {
      std::ifstream in(argv[arg_i], std::ios::binary);

      Bundle bundle(in);
      in.close();
    }
}

/* EOF */
