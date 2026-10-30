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

#include "model_animation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace {

const size_t BPR_HEADER_SIZE = 16;
const size_t BPR_BONE_SIZE   = 24;
const size_t BPR_ORIGIN      = 8;  // file offsets are relative to this

template<class T>
T get(const std::vector<char>& data, size_t pos)
{
  if (pos + sizeof(T) > data.size())
    throw std::runtime_error("Animation: file is truncated");
  T value;
  memcpy(&value, data.data() + pos, sizeof(T));
  return value;
}

/** Like an IEEE half, but with 5 more added to the exponent */
float read_float16(uint16_t value)
{
  if (value == 0)
    return 0.0f;

  const int exponent = ((value >> 10) & 0x1f) - 5;
  const int fraction =  value & 0x3ff;

  const float result = exponent == 0
    ? std::ldexp(float(fraction), -24)
    : std::ldexp(float(fraction + 1024), exponent - 25);

  return (value & 0x8000) ? -result : result;
}

float read_time(const std::vector<char>& data, size_t& pos, char version)
{
  if (version == '0')
    {
      const float time = get<float>(data, pos);
      pos += 4;
      return time;
    }
  else
    {
      const float time = float(get<uint16_t>(data, pos)) / 65536.0f;
      pos += 2;
      return time;
    }
}

float read_value(const std::vector<char>& data, size_t& pos, char version)
{
  if (version == '0')
    {
      const float value = get<float>(data, pos);
      pos += 4;
      return value;
    }
  else
    {
      const float value = read_float16(get<uint16_t>(data, pos));
      pos += 2;
      return value;
    }
}

/** Finds the key that \a time is in and the position within it */
int find_key(const std::vector<float>& times, float time, float& t)
{
  // times are sorted, so this is the last key not after time
  const std::vector<float>::const_iterator next = std::upper_bound(times.begin(), times.end(), time);
  if (next == times.begin())
    {
      t = 0.0f;
      return 0;
    }

  const int key = int(next - times.begin()) - 1;
  const float end   = next == times.end() ? 1.0f : *next;
  const float begin = times[key];
  t = end > begin ? (time - begin) / (end - begin) : 0.0f;
  return key;
}

float wrap(float seconds, float duration)
{
  if (duration <= 0.0f)
    return 0.0f;
  const float time = std::fmod(seconds, duration) / duration;
  return time < 0.0f ? time + 1.0f : time;
}

/** 4x4 column major multiplication of out = a * b */
void multiply(const float* a, const float* b, float* out)
{
  for(int column = 0; column < 4; ++column)
    for(int row = 0; row < 4; ++row)
      {
        out[column*4 + row] = 0.0f;
        for(int k = 0; k < 4; ++k)
          out[column*4 + row] += a[k*4 + row] * b[column*4 + k];
      }
}

/** Inverse of a matrix of rotation and translation */
void invert_rigid(const float* m, float* out)
{
  for(int column = 0; column < 3; ++column)
    {
      for(int row = 0; row < 3; ++row)
        out[column*4 + row] = m[row*4 + column];
      out[column*4 + 3] = 0.0f;
      out[12 + column] = -(m[column*4 + 0] * m[12] + m[column*4 + 1] * m[13] + m[column*4 + 2] * m[14]);
    }
  out[15] = 1.0f;
}

void local_bone_matrix(const ModelBoneTrack& track, float time, float* matrix)
{
  std::vector<float> position(3, 0.0f);
  std::vector<float> rotation;

  if (!track.position_times.empty())
    {
      float t;
      const int key = find_key(track.position_times, time, t);
      const float* coefficients = &track.positions[key * 12];
      for(int k = 0; k < 4; ++k)
        for(int axis = 0; axis < 3; ++axis)
          position[axis] = position[axis] * t + coefficients[k*3 + axis];
    }

  if (!track.rotation_times.empty())
    {
      float t;
      const int key = find_key(track.rotation_times, time, t);
      const float* coefficients = &track.rotations[key * 16];
      float q[4] = { 0, 0, 0, 0 };
      for(int k = 0; k < 4; ++k)
        for(int axis = 0; axis < 4; ++axis)
          q[axis] = q[axis] * t + coefficients[k*4 + axis];

      // Version 0 leaves out w
      if (q[3] == 0.0f)
        q[3] = 1.0f - q[0]*q[0] - q[1]*q[1] - q[2]*q[2];

      const float length = std::sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
      if (length > 0.0f)
        rotation.assign({ q[0] / length, q[1] / length, q[2] / length, q[3] / length });
    }

  make_transform(position, rotation, 1.0f, matrix);
}

} // namespace

bool
is_supported_animation(const std::vector<char>& data)
{
  return data.size() >= BPR_HEADER_SIZE && memcmp(data.data(), "tljbone", 7) == 0 &&
    (data[7] == '0' || data[7] == '2');
}

int
get_animation_bone_count(const std::vector<char>& data)
{
  return get<int32_t>(data, 8);
}

ModelAnimation
parse_animation(const std::vector<char>& data)
{
  if (!is_supported_animation(data))
    throw std::runtime_error("Animation: not a supported tljbone file");

  const char version = data[7];
  const int count = get<int32_t>(data, 8);
  if (count < 0 || BPR_HEADER_SIZE + size_t(count) * BPR_BONE_SIZE > data.size())
    throw std::runtime_error("Animation: invalid bone count");

  ModelAnimation animation;
  animation.duration = get<float>(data, 12);
  animation.tracks.resize(count);

  const auto check_count = [&data](int keys)
    {
      if (keys < 0 || size_t(keys) > data.size())
        throw std::runtime_error("Animation: invalid key count");
    };

  for(int i = 0; i < count; ++i)
    {
      size_t entry = BPR_HEADER_SIZE + size_t(i) * BPR_BONE_SIZE;
      const int rotation_count = get<int32_t>(data, entry);
      size_t rotation_times    = size_t(get<uint32_t>(data, entry + 4)) + BPR_ORIGIN;
      size_t rotation_data     = size_t(get<uint32_t>(data, entry + 8)) + BPR_ORIGIN;
      const int position_count = get<int32_t>(data, entry + 12);
      size_t position_times    = size_t(get<uint32_t>(data, entry + 16)) + BPR_ORIGIN;
      size_t position_data     = size_t(get<uint32_t>(data, entry + 20)) + BPR_ORIGIN;

      check_count(rotation_count);
      check_count(position_count);

      ModelBoneTrack& track = animation.tracks[i];

      track.rotation_times.resize(rotation_count);
      for(int key = 0; key < rotation_count; ++key)
        track.rotation_times[key] = read_time(data, rotation_times, version);

      track.rotations.resize(size_t(rotation_count) * 16);
      for(size_t k = 0; k < track.rotations.size(); ++k)
        {
          // Version 0 stores only x, y and z of the coefficients
          if (version == '0' && k % 4 == 3)
            track.rotations[k] = 0.0f;
          else
            track.rotations[k] = read_value(data, rotation_data, version);
        }

      track.position_times.resize(position_count);
      for(int key = 0; key < position_count; ++key)
        track.position_times[key] = read_time(data, position_times, version);

      track.positions.resize(size_t(position_count) * 12);
      for(size_t k = 0; k < track.positions.size(); ++k)
        track.positions[k] = read_value(data, position_data, version);
    }

  return animation;
}

void
make_transform(const std::vector<float>& translation, const std::vector<float>& quaternion,
               float scale, float* m)
{
  float x = 0, y = 0, z = 0, w = 1;
  if (quaternion.size() == 4)
    {
      x = quaternion[0];
      y = quaternion[1];
      z = quaternion[2];
      w = quaternion[3];

      const float length2 = x*x + y*y + z*z + w*w;
      if (length2 > 0)
        {
          const float length = std::sqrt(length2);
          x /= length;
          y /= length;
          z /= length;
          w /= length;
          scale *= length2;
        }
      else
        {
          x = y = z = 0;
          w = 1;
        }
    }

  m[0]  = scale * (1 - 2*y*y - 2*z*z);
  m[1]  = scale * (2*x*y + 2*z*w);
  m[2]  = scale * (2*x*z - 2*y*w);
  m[3]  = 0;

  m[4]  = scale * (2*x*y - 2*z*w);
  m[5]  = scale * (1 - 2*x*x - 2*z*z);
  m[6]  = scale * (2*y*z + 2*x*w);
  m[7]  = 0;

  m[8]  = scale * (2*x*z + 2*y*w);
  m[9]  = scale * (2*y*z - 2*x*w);
  m[10] = scale * (1 - 2*x*x - 2*y*y);
  m[11] = 0;

  m[12] = translation.size() == 3 ? translation[0] : 0;
  m[13] = translation.size() == 3 ? translation[1] : 0;
  m[14] = translation.size() == 3 ? translation[2] : 0;
  m[15] = 1;
}

void
sample_path(const ModelPath& path, float seconds, float scale, float* matrix)
{
  const float time = wrap(seconds, path.duration);

  std::vector<float> times(path.keys.size());
  for(size_t i = 0; i < path.keys.size(); ++i)
    times[i] = path.keys[i].time;

  float t;
  const int first = find_key(times, time, t);
  const int second = (first + 1) % int(path.keys.size());
  const ModelPath::Key& a = path.keys[first];
  const ModelPath::Key& b = path.keys[second];

  std::vector<float> position(3);
  for(int i = 0; i < 3; ++i)
    position[i] = a.position[i] + (b.position[i] - a.position[i]) * t;

  // Take the short way, keeping the length as that is the scale
  float dot = 0.0f, length_a = 0.0f, length_b = 0.0f;
  for(int i = 0; i < 4; ++i)
    {
      dot += a.rotation[i] * b.rotation[i];
      length_a += a.rotation[i] * a.rotation[i];
      length_b += b.rotation[i] * b.rotation[i];
    }

  std::vector<float> rotation(4);
  float length2 = 0.0f;
  for(int i = 0; i < 4; ++i)
    {
      rotation[i] = a.rotation[i] + ((dot < 0.0f ? -b.rotation[i] : b.rotation[i]) - a.rotation[i]) * t;
      length2 += rotation[i] * rotation[i];
    }

  if (length2 > 0.0f)
    {
      const float factor = std::sqrt((length_a + (length_b - length_a) * t) / length2);
      for(int i = 0; i < 4; ++i)
        rotation[i] *= factor;
    }

  make_transform(position, rotation, scale, matrix);
}

void
sample_skeleton(const ModelMesh& mesh, const ModelAnimation& animation, float seconds,
                std::vector<float>& rows)
{
  const int count = int(mesh.bones.size());
  const float time = wrap(seconds, animation.duration);

  std::vector<float> current(size_t(count) * 16);
  rows.resize(size_t(count) * 12);

  for(int i = 0; i < count; ++i)
    {
      float bind[16], inverse[16], local[16], temp[16], result[16];

      const ModelBone& bone = mesh.bones[i];
      make_transform(std::vector<float>(bone.position, bone.position + 3),
                     std::vector<float>(bone.rotation, bone.rotation + 4), 1.0f, bind);
      invert_rigid(bind, inverse);
      local_bone_matrix(animation.tracks[i], time, local);

      // current = parent * bind * local * bind^-1
      multiply(bind, local, temp);
      multiply(temp, inverse, result);

      const int parent = mesh.bone_parents[i];
      if (parent >= 0)
        {
          multiply(&current[size_t(parent) * 16], result, temp);
          memcpy(&current[size_t(i) * 16], temp, sizeof(temp));
        }
      else
        {
          memcpy(&current[size_t(i) * 16], result, sizeof(result));
        }

      for(int row = 0; row < 3; ++row)
        for(int column = 0; column < 4; ++column)
          rows[size_t(i) * 12 + row*4 + column] = current[size_t(i) * 16 + column*4 + row];
    }
}

void
sample_frames(const ModelMesh& mesh, float seconds, int& first, int& second, float& weight)
{
  const float time = wrap(seconds, FRAME_LOOP_SECONDS);
  first = find_key(mesh.frame_times, time, weight);
  second = (first + 1) % int(mesh.frame_times.size());
}

/* EOF */
