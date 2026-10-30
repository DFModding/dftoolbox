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

#include "model_bundle.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace {

// Vertex channel types, the value is an index into channel_size
enum { CHANNEL_UNUSED = -1, CHANNEL_FLOAT2 = 1, CHANNEL_FLOAT3 = 2, CHANNEL_FLOAT4 = 3, CHANNEL_COLOR = 4 };

const int channel_size[] = { 0, 8, 12, 16, 4 };

// Offsets into the mesh header
const size_t MESH_RESCALE     = 0x04;
const size_t MESH_CENTER      = 0x08;
const size_t MESH_BOUND       = 0x14;
const size_t MESH_BONE_COUNT  = 0x38;
const size_t MESH_BONE_NAMES  = 0x3c;
const size_t MESH_BONE_DATA   = 0x40;
const size_t MESH_TEX_COUNT   = 0x44;
const size_t MESH_TEX_TABLE   = 0x48;
const size_t MESH_PART_COUNT  = 0x54;
const size_t MESH_PART_TABLE  = 0x58;

// Offsets into the header of a mesh part
const size_t BONE_NAME_SIZE   = 0x28;
const size_t PART_FORMAT      = 0x50;
const size_t PART_BITCODE     = 0x54;
const size_t PART_INDEX_COUNT = 0x64;
const size_t PART_INDEX_TABLE = 0x68;
const size_t PART_BONE_COUNT  = 0x6c;
const size_t PART_BONE_TABLE  = 0x70;
const size_t PART_BONE_STAGES = 0x74;
const size_t PART_STAGE_VERTEX= 0x78;
const size_t PART_STAGE_INDEX = 0x7c;
const size_t PART_STAGE_BONES = 0x80;
const size_t PART_TEX_STAGES  = 0xb4;
const size_t PART_STAGE_USE   = 0xc4;
const size_t PART_FRAME_COUNT = 0xc8;
const size_t PART_FRAME_TIMES = 0xcc;
const size_t PART_VERTEX_COUNT= 0xd0;
const size_t PART_TEX_COUNT   = 0x104;
const size_t PART_TEX_TABLE   = 0x108;

const size_t TEX_LAYER_TABLE  = 0x08;

const int FORMAT_SIZE = 72;

/** Bounds checked access to the bundle */
class Reader
{
private:
  const std::vector<char>& data;

public:
  Reader(const std::vector<char>& data_) : data(data_) {}

  void check(size_t pos, size_t size) const
  {
    if (pos > data.size() || size > data.size() - pos)
      {
        std::ostringstream str;
        str << "Bundle: read of " << size << " bytes at 0x" << std::hex << pos << " is out of range";
        throw std::runtime_error(str.str());
      }
  }

  template<class T>
  T get(size_t pos) const
  {
    check(pos, sizeof(T));
    T value;
    memcpy(&value, data.data() + pos, sizeof(T));
    return value;
  }

  std::string get_string(size_t pos, size_t max_length) const
  {
    check(pos, 0);
    size_t length = std::min(max_length, data.size() - pos);
    const char* begin = data.data() + pos;
    return std::string(begin, std::find(begin, begin + length, '\0'));
  }

  /** Counts come straight from the file, so they get checked to not
      request a huge allocation on a broken file */
  template<class T>
  std::vector<T> get_table(size_t pos, int count) const
  {
    if (count <= 0)
      return std::vector<T>();

    check(pos, sizeof(T) * size_t(count));
    std::vector<T> table(count);
    memcpy(table.data(), data.data() + pos, sizeof(T) * size_t(count));
    return table;
  }
};

/** Where the attributes the preview cares about are located in a
    vertex, offsets are -1 if an attribute doesn't exist */
struct VertexLayout
{
  int position;
  int normal;
  int uv;
  int uv2;
  int color;
  int weights;
  int bones;

  VertexLayout() : position(-1), normal(-1), uv(-1), uv2(-1), color(-1), weights(-1), bones(-1) {}
};

VertexLayout
get_layout(const ModelBundle::StreamFormat& format)
{
  VertexLayout layout;

  int offset = 0;
  for(int i = 0; i < 16; ++i)
    {
      const int type = format.channels[i];
      if (type == CHANNEL_UNUSED)
        continue;

      if (type < 0 || type >= int(sizeof(channel_size) / sizeof(channel_size[0])))
        throw std::runtime_error("Bundle: unknown vertex channel type");

      if (format.extra_streams != 0)
        {
          // Formats of vertex animated meshes have their channels
          // packed, so the channel number means nothing
          if (type == CHANNEL_FLOAT3 && layout.position == -1)
            layout.position = offset;
          else if (type == CHANNEL_FLOAT3 && layout.normal == -1)
            layout.normal = offset;
          else if (type == CHANNEL_COLOR && layout.color == -1)
            layout.color = offset;
          else if (type == CHANNEL_FLOAT2 && layout.uv == -1)
            layout.uv = offset;
          else if (type == CHANNEL_FLOAT2 && layout.uv2 == -1)
            layout.uv2 = offset;
        }
      else
        {
          if (i == 0 && type == CHANNEL_FLOAT3)
            layout.position = offset;
          else if (i == 1 && type == CHANNEL_FLOAT4)
            layout.weights = offset;
          else if (i == 2 && type == CHANNEL_COLOR)
            layout.bones = offset;
          else if (i == 3 && type == CHANNEL_FLOAT3)
            layout.normal = offset;
          else if (i == 5 && type == CHANNEL_COLOR)
            layout.color = offset;
          else if (i == 7 && type == CHANNEL_FLOAT2)
            layout.uv = offset;
          else if (i == 8 && type == CHANNEL_FLOAT2)
            layout.uv2 = offset;
        }

      offset += channel_size[type];
    }

  return layout;
}

} // namespace

ModelBundle::ModelBundle(std::vector<char> data_)
  : data(std::move(data_)),
    zero_pos(0),
    format_table(0)
{
  read_header();
}

void
ModelBundle::read_header()
{
  Reader reader(data);

  const size_t origin = size_t(reader.get<uint32_t>(0)) + 4;

  const int texture_count = reader.get<int32_t>(4);
  size_t pos = 8;
  for(int i = 0; i < texture_count; ++i)
    {
      // The length byte doesn't tell more than the terminating zero
      textures.push_back(reader.get_string(pos + 1, 0xff));
      pos += textures.back().size() + 2;
    }

  if (pos != origin)
    throw std::runtime_error("Bundle: texture table doesn't end at the data origin");

  const int vertex_data_count = reader.get<int32_t>(pos);
  pos += 4;
  for(int i = 0; i < vertex_data_count; ++i)
    {
      VertexData entry;
      entry.vertex_size = reader.get<uint32_t>(pos);
      entry.length      = reader.get<uint32_t>(pos + 4);
      entry.offset      = pos + 8;
      reader.check(entry.offset, entry.length);
      vertex_data.push_back(entry);
      pos = entry.offset + entry.length;
    }

  // All offsets in the rest of the file are relative to this point
  zero_pos = pos;

  const int smr_count    = reader.get<int32_t>(pos);
  const int format_count = reader.get<int32_t>(pos + 4);

  const std::vector<uint32_t> smr_table = reader.get_table<uint32_t>(pos + 12, smr_count);
  pos += 12 + 4 * size_t(smr_count);
  format_table = pos - zero_pos;

  for(int i = 0; i < format_count; ++i)
    {
      StreamFormat format;
      format.vertex_size = reader.get<uint32_t>(pos);
      for(int j = 0; j < 16; ++j)
        format.channels[j] = reader.get<int32_t>(pos + 4 + 4 * j);
      format.extra_streams = reader.get<int32_t>(pos + 68);
      stream_formats.push_back(format);
      pos += FORMAT_SIZE;
    }

  // The vertex data isn't referenced by the meshes, instead they use
  // it up in order, so all meshes have to be walked to find it
  int data_index = 0;
  for(int i = 0; i < smr_count; ++i)
    {
      const size_t smr_pos = zero_pos + smr_table[i];

      SmrEntry smr;
      smr.name = reader.get_string(smr_pos, 0x80);

      const int mesh_count = reader.get<int32_t>(smr_pos + 0x80);
      const std::vector<uint32_t> mesh_table = reader.get_table<uint32_t>(smr_pos + 0x84, mesh_count);

      for(int j = 0; j < mesh_count; ++j)
        {
          const size_t mesh_pos = zero_pos + mesh_table[j];

          MeshEntry mesh;
          mesh.name       = reader.get_string(zero_pos + reader.get<uint32_t>(mesh_pos), 0x100);
          mesh.offset     = mesh_table[j];
          mesh.data_index = data_index;
          smr.meshes.push_back(mesh);

          const int part_count = reader.get<int32_t>(mesh_pos + MESH_PART_COUNT);
          const std::vector<uint32_t> parts = reader.get_table<uint32_t>(mesh_pos + MESH_PART_TABLE, part_count);
          for(int k = 0; k < part_count; ++k)
            {
              const size_t part_pos = zero_pos + parts[k];
              if (is_drawable(get_format(reader.get<int32_t>(part_pos + PART_FORMAT)),
                              reader.get<int32_t>(part_pos + PART_BITCODE)))
                {
                  data_index += reader.get<int32_t>(part_pos + PART_FRAME_COUNT);
                }
            }
        }

      smrs.push_back(smr);
    }

  if (data_index != vertex_data_count)
    throw std::runtime_error("Bundle: meshes don't match the vertex data");
}

const ModelBundle::StreamFormat*
ModelBundle::get_format(int32_t format_offset) const
{
  // Meshes refer to their format by its offset in the file
  const int64_t relative = int64_t(format_offset) - int64_t(format_table);
  if (format_offset == 0 || relative < 0 || relative % FORMAT_SIZE != 0 ||
      relative / FORMAT_SIZE >= int64_t(stream_formats.size()))
    return nullptr;

  return &stream_formats[relative / FORMAT_SIZE];
}

bool
ModelBundle::is_drawable(const StreamFormat* format, int32_t bitcode) const
{
  return format && bitcode != 0 && format->vertex_size != 0;
}

const ModelBundle::SmrEntry*
ModelBundle::find_smr(const std::string& smr) const
{
  for(std::vector<SmrEntry>::const_iterator i = smrs.begin(); i != smrs.end(); ++i)
    if (i->name == smr)
      return &*i;
  return nullptr;
}

bool
ModelBundle::load_mesh(const std::string& smr, const std::string& model, ModelMesh& mesh) const
{
  const SmrEntry* entry = find_smr(smr);
  if (entry)
    for(std::vector<MeshEntry>::const_iterator i = entry->meshes.begin(); i != entry->meshes.end(); ++i)
      if (i->name == model)
        return load_mesh(*i, mesh);
  return false;
}

bool
ModelBundle::load_mesh(const MeshEntry& entry, ModelMesh& mesh) const
{
  Reader reader(data);

  const size_t mesh_pos = zero_pos + entry.offset;

  // Only the first part is geometry, the second part that skinned
  // meshes have is their shadow volume
  const int part_count = reader.get<int32_t>(mesh_pos + MESH_PART_COUNT);
  if (part_count == 0)
    return false;

  const size_t part_pos = zero_pos + reader.get<uint32_t>(mesh_pos + MESH_PART_TABLE);

  const StreamFormat* format = get_format(reader.get<int32_t>(part_pos + PART_FORMAT));
  const int part_textures = reader.get<int32_t>(part_pos + PART_TEX_COUNT);
  if (!is_drawable(format, reader.get<int32_t>(part_pos + PART_BITCODE)) || part_textures == 0)
    return false;

  mesh = ModelMesh();
  mesh.name        = entry.name;
  mesh.rescale     = reader.get<float>(mesh_pos + MESH_RESCALE);
  mesh.frame_count = reader.get<int32_t>(part_pos + PART_FRAME_COUNT);
  for(int i = 0; i < 3; ++i)
    {
      mesh.center[i] = reader.get<float>(mesh_pos + MESH_CENTER + 4 * i);
      mesh.bound[i]  = reader.get<float>(mesh_pos + MESH_BOUND  + 4 * i);
    }

  if (mesh.frame_count <= 0 || entry.data_index >= int(vertex_data.size()))
    throw std::runtime_error("Bundle: " + entry.name + ": invalid vertex data reference");

  if (entry.data_index + mesh.frame_count > int(vertex_data.size()))
    throw std::runtime_error("Bundle: " + entry.name + ": invalid vertex data reference");

  const int vertex_count = reader.get<int32_t>(part_pos + PART_VERTEX_COUNT) / mesh.frame_count;
  read_vertices(*format, vertex_data[entry.data_index], vertex_count, mesh);

  if (mesh.frame_count > 1)
    {
      mesh.frame_times = reader.get_table<float>(zero_pos + reader.get<uint32_t>(part_pos + PART_FRAME_TIMES),
                                                 mesh.frame_count);
      for(int i = 0; i < mesh.frame_count; ++i)
        read_frame(*format, vertex_data[entry.data_index + i], vertex_count, mesh);
    }

  mesh.indices = reader.get_table<uint16_t>(zero_pos + reader.get<uint32_t>(part_pos + PART_INDEX_TABLE),
                                            reader.get<int32_t>(part_pos + PART_INDEX_COUNT));

  read_bones(mesh_pos, part_pos, mesh);

  // Geometry is cut into stages, each small enough for the bones it
  // uses to fit into the shader constants, those are then grouped
  // into texture stages
  const int  stage_count  = reader.get<int32_t>(part_pos + PART_BONE_STAGES);
  const int  texture_stage_count = reader.get<int32_t>(part_pos + PART_TEX_STAGES);
  const auto stage_indices = reader.get_table<uint16_t>(zero_pos + reader.get<uint32_t>(part_pos + PART_STAGE_INDEX),
                                                        stage_count);
  const auto stage_use     = reader.get_table<int32_t>(zero_pos + reader.get<uint32_t>(part_pos + PART_STAGE_USE),
                                                       texture_stage_count);
  const auto mesh_textures = reader.get_table<int32_t>(zero_pos + reader.get<uint32_t>(mesh_pos + MESH_TEX_TABLE),
                                                       reader.get<int32_t>(mesh_pos + MESH_TEX_COUNT));
  const auto layers        = reader.get_table<uint32_t>(part_pos + PART_TEX_TABLE, part_textures);

  int stage = 0;
  int first_index = 0;
  for(int i = 0; i < texture_stage_count; ++i)
    {
      ModelStage result;
      result.first_index = first_index;
      result.index_count = 0;

      for(int j = 0; j < stage_use[i]; ++j, ++stage)
        {
          if (stage >= stage_count)
            throw std::runtime_error("Bundle: " + entry.name + ": texture stages don't match the bone stages");
          result.index_count += stage_indices[stage];
        }
      first_index += result.index_count;

      for(int j = 0; j < part_textures; ++j)
        {
          const size_t layer_pos = zero_pos + layers[j];
          const size_t table_pos = zero_pos + reader.get<uint32_t>(layer_pos + TEX_LAYER_TABLE);
          const int texture = reader.get<int32_t>(table_pos + 4 * size_t(i));

          if (texture == -1)
            {
              result.textures.push_back(std::string());
            }
          else if (texture < 0 || texture >= int(mesh_textures.size()) ||
                   mesh_textures[texture] < 0 || mesh_textures[texture] >= int(textures.size()))
            {
              throw std::runtime_error("Bundle: " + entry.name + ": invalid texture reference");
            }
          else
            {
              result.textures.push_back(textures[mesh_textures[texture]]);
            }
        }

      mesh.stages.push_back(result);
    }

  if (first_index != int(mesh.indices.size()))
    throw std::runtime_error("Bundle: " + entry.name + ": stages don't match the index count");

  return true;
}

void
ModelBundle::read_vertices(const StreamFormat& format, const VertexData& vertices, int count,
                           ModelMesh& mesh) const
{
  if (vertices.vertex_size != format.vertex_size || count < 0 ||
      vertices.length != format.vertex_size * uint32_t(count))
    throw std::runtime_error("Bundle: " + mesh.name + ": vertex data doesn't match the stream format");

  const VertexLayout layout = get_layout(format);
  if (layout.position == -1)
    throw std::runtime_error("Bundle: " + mesh.name + ": stream format has no position");

  mesh.has_normals = layout.normal != -1;
  mesh.has_colors  = layout.color  != -1;

  mesh.vertices.resize(count);
  for(int i = 0; i < count; ++i)
    {
      const char* src = data.data() + vertices.offset + size_t(i) * format.vertex_size;
      ModelVertex& vertex = mesh.vertices[i];

      memset(&vertex, 0, sizeof(vertex));
      memcpy(vertex.position, src + layout.position, sizeof(vertex.position));

      if (layout.normal != -1)
        memcpy(vertex.normal, src + layout.normal, sizeof(vertex.normal));
      if (layout.uv != -1)
        memcpy(vertex.uv, src + layout.uv, sizeof(vertex.uv));
      if (layout.uv2 != -1)
        memcpy(vertex.uv2, src + layout.uv2, sizeof(vertex.uv2));

      if (layout.weights != -1 && layout.bones != -1)
        {
          // The shader only looks at three weights, the fourth is what's left
          memcpy(vertex.weights, src + layout.weights, sizeof(vertex.weights));
          vertex.weights[3] = std::max(0.0f, 1.0f - vertex.weights[0] - vertex.weights[1] - vertex.weights[2]);
          memcpy(vertex.bones, src + layout.bones, sizeof(vertex.bones));
        }

      if (layout.color != -1)
        { // stored in D3DCOLOR order
          const uint8_t* color = reinterpret_cast<const uint8_t*>(src + layout.color);
          vertex.color[0] = color[2];
          vertex.color[1] = color[1];
          vertex.color[2] = color[0];
          vertex.color[3] = color[3];
        }
      else
        {
          memset(vertex.color, 0xff, sizeof(vertex.color));
        }
    }
}

void
ModelBundle::read_frame(const StreamFormat& format, const VertexData& vertices, int count, ModelMesh& mesh) const
{
  if (vertices.vertex_size != format.vertex_size ||
      vertices.length != format.vertex_size * uint32_t(count))
    throw std::runtime_error("Bundle: " + mesh.name + ": frame doesn't match the stream format");

  const VertexLayout layout = get_layout(format);

  for(int i = 0; i < count; ++i)
    {
      const char* src = data.data() + vertices.offset + size_t(i) * format.vertex_size;

      ModelFrameVertex vertex;
      memset(&vertex, 0, sizeof(vertex));
      memcpy(vertex.position, src + layout.position, sizeof(vertex.position));
      if (layout.normal != -1)
        memcpy(vertex.normal, src + layout.normal, sizeof(vertex.normal));
      mesh.frames.push_back(vertex);
    }
}

void
ModelBundle::read_bones(size_t mesh_pos, size_t part_pos, ModelMesh& mesh) const
{
  Reader reader(data);

  const int bone_count = reader.get<int32_t>(mesh_pos + MESH_BONE_COUNT);
  if (bone_count <= 0)
    return;

  const size_t names = zero_pos + reader.get<uint32_t>(mesh_pos + MESH_BONE_NAMES);
  const auto   poses = reader.get_table<float>(zero_pos + reader.get<uint32_t>(mesh_pos + MESH_BONE_DATA),
                                               7 * bone_count);
  mesh.bones.resize(bone_count);
  for(int i = 0; i < bone_count; ++i)
    {
      ModelBone& bone = mesh.bones[i];
      bone.name = reader.get_string(names + size_t(i) * BONE_NAME_SIZE, BONE_NAME_SIZE);
      std::copy(&poses[i*7], &poses[i*7] + 3, bone.position);
      std::copy(&poses[i*7 + 3], &poses[i*7] + 7, bone.rotation);
    }

  // Each stage has its own palette of the bones it uses, with all
  // of them in one palette the stages don't matter for drawing
  const int stage_count = reader.get<int32_t>(part_pos + PART_BONE_STAGES);
  const uint32_t assignment = reader.get<uint32_t>(part_pos + PART_STAGE_BONES);
  if (assignment == 0)
    return;

  const auto stage_vertices = reader.get_table<uint16_t>(zero_pos + reader.get<uint32_t>(part_pos + PART_STAGE_VERTEX),
                                                         stage_count);
  const auto stage_bones    = reader.get_table<uint16_t>(zero_pos + assignment, stage_count);
  const auto palette        = reader.get_table<uint16_t>(zero_pos + reader.get<uint32_t>(part_pos + PART_BONE_TABLE),
                                                         reader.get<int32_t>(part_pos + PART_BONE_COUNT));

  size_t vertex = 0;
  size_t base = 0;
  for(int stage = 0; stage < stage_count; ++stage)
    {
      if (base + stage_bones[stage] > palette.size() || vertex + stage_vertices[stage] > mesh.vertices.size())
        throw std::runtime_error("Bundle: " + mesh.name + ": bone stages don't match the mesh");

      for(int i = 0; i < stage_vertices[stage]; ++i, ++vertex)
        for(int k = 0; k < 4; ++k)
          {
            uint8_t& bone = mesh.vertices[vertex].bones[k];
            if (mesh.vertices[vertex].weights[k] == 0.0f)
              {
                bone = 0;
              }
            else if (bone >= stage_bones[stage] || palette[base + bone] >= bone_count || palette[base + bone] > 255)
              {
                throw std::runtime_error("Bundle: " + mesh.name + ": invalid bone reference");
              }
            else
              {
                bone = uint8_t(palette[base + bone]);
              }
          }

      base += stage_bones[stage];
    }
}

/* EOF */
