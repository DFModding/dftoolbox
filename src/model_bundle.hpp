/*
**  ___  ___ _____         _ ___
** |   \| __|_   _|__  ___| | _ ) _____ __
** | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
** |___/|_|   |_|\___/\___/_|___/\___/_\_\
**
**  DFToolBox - Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 3
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HEADER_MODEL_BUNDLE_HPP
#define HEADER_MODEL_BUNDLE_HPP

#include <array>
#include <cstdint>
#include <string>
#include <vector>

/** A vertex reduced to what the preview needs, see doc/bundle-format.txt */
struct ModelVertex
{
  float position[3];
  float normal[3];
  float uv[2];
  float uv2[2];

  /** RGBA, white if the mesh has no vertex colors */
  uint8_t color[4];

  /** Indices into ModelMesh::bones, the weights sum up to one */
  uint8_t bones[4];
  float   weights[4];
};

/** A bone in its bind pose, in the space of the model */
struct ModelBone
{
  std::string name;
  float position[3];
  float rotation[4];
};

/** Position and normal of a vertex in a frame of a vertex animation */
struct ModelFrameVertex
{
  float position[3];
  float normal[3];
};

/** A range of indices that share a set of textures */
struct ModelStage
{
  int first_index;
  int index_count;

  /** Texture pathname of each texture layer, empty if a layer is unused */
  std::vector<std::string> textures;
};

struct ModelMesh
{
  std::string name;

  /** The vertices have to be scaled by 1/rescale */
  float rescale;

  float center[3];
  float bound[3];

  std::vector<ModelVertex> vertices;
  std::vector<uint16_t>    indices;
  std::vector<ModelStage>  stages;

  std::vector<ModelBone> bones;

  /** Index of the parent of each bone, -1 for the root. Empty if the
      skeleton is not known, the bones are then of no use */
  std::vector<int> bone_parents;

  /** Number of vertex animation frames, vertices hold the first one */
  int  frame_count;

  /** Time of each frame in the range 0 to 1, empty if there is only one */
  std::vector<float> frame_times;

  /** The vertices of each frame, one frame after the other */
  std::vector<ModelFrameVertex> frames;

  bool has_normals;
  bool has_colors;
};

/** A .bun file, it holds the meshes and the vertex data of a location,
    meshes are grouped by the .smr file that they were compiled
    from. The format is documented in doc/bundle-format.txt */
class ModelBundle
{
public:
  struct MeshEntry
  {
    std::string name;
    uint32_t    offset;
    int         data_index;
  };

  struct SmrEntry
  {
    std::string name;
    std::vector<MeshEntry> meshes;
  };

  struct VertexData
  {
    uint32_t vertex_size;
    uint32_t length;
    size_t   offset;
  };

  struct StreamFormat
  {
    uint32_t vertex_size;
    std::array<int32_t, 16> channels;
    int32_t  extra_streams;
  };

private:
  std::vector<char>         data;
  size_t                    zero_pos;
  size_t                    format_table;
  std::vector<std::string>  textures;
  std::vector<VertexData>   vertex_data;
  std::vector<StreamFormat> stream_formats;
  std::vector<SmrEntry>     smrs;

public:
  /** Throws std::runtime_error if \a data isn't a valid bundle */
  ModelBundle(std::vector<char> data);

  const std::vector<SmrEntry>&     get_smrs() const     { return smrs; }
  const std::vector<std::string>&  get_textures() const { return textures; }

  const SmrEntry* find_smr(const std::string& smr) const;

  /** Throws std::runtime_error if the mesh can't be read, returns
      false if there is nothing that could be drawn in it, those are
      found in the data: no geometry or only collision data */
  bool load_mesh(const std::string& smr, const std::string& model, ModelMesh& mesh) const;
  bool load_mesh(const MeshEntry& entry, ModelMesh& mesh) const;

private:
  void read_header();
  void read_vertices(const StreamFormat& format, const VertexData& vertices, int count,
                     ModelMesh& mesh) const;
  void read_frame(const StreamFormat& format, const VertexData& vertices, int count,
                  ModelMesh& mesh) const;
  void read_bones(size_t mesh_pos, size_t part_pos, ModelMesh& mesh) const;
  bool is_drawable(const StreamFormat* format, int32_t bitcode) const;

  const StreamFormat* get_format(int32_t format_offset) const;

  ModelBundle(const ModelBundle&) = delete;
  ModelBundle& operator=(const ModelBundle&) = delete;
};

#endif

/* EOF */
