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

#ifndef HEADER_MODEL_ANIMATION_HPP
#define HEADER_MODEL_ANIMATION_HPP

#include <string>
#include <vector>

#include "model_bundle.hpp"

/** The keys of a bone, each key is a cubic polynomial that is valid
    up to the time of the next key */
struct ModelBoneTrack
{
  std::vector<float> rotation_times;
  std::vector<float> rotations;  // 16 per key, x y z w of the four coefficients

  std::vector<float> position_times;
  std::vector<float> positions;  // 12 per key, x y z of the four coefficients
};

/** A skeletal animation from a .bpr file, times are in the range 0 to 1 */
struct ModelAnimation
{
  std::string name;
  float duration;
  std::vector<ModelBoneTrack> tracks;
};

/** The movement of a node from a .spr file, see doc/bundle-format.txt */
struct ModelPath
{
  struct Key
  {
    float time;
    float position[3];
    float rotation[4];
  };

  float duration;
  std::vector<Key> keys;
};

/** Parses a tljbone .bpr file, throws if the data isn't one the preview understands */
ModelAnimation parse_animation(const std::vector<char>& data);

/** Returns true if \a data is in a .bpr format parse_animation() can read */
bool is_supported_animation(const std::vector<char>& data);

/** Number of bones of a tljbone .bpr file */
int get_animation_bone_count(const std::vector<char>& data);

/** Builds a column major matrix, the length of \a quaternion is a scale
    factor, see doc/bundle-format.txt */
void make_transform(const std::vector<float>& translation, const std::vector<float>& quaternion,
                    float scale, float* matrix);

/** Calculates the local matrix of a node for \a seconds */
void sample_path(const ModelPath& path, float seconds, float scale, float* matrix);

/** Calculates for every bone of \a mesh the matrix that moves a vertex
    from the bind pose to \a seconds into \a animation, as 12 floats
    (the first three rows of the matrix) each */
void sample_skeleton(const ModelMesh& mesh, const ModelAnimation& animation, float seconds,
                     std::vector<float>& rows);

/** Returns the frames to blend and the weight of the second one for a
    vertex animation, which is looped over FRAME_LOOP_SECONDS */
void sample_frames(const ModelMesh& mesh, float seconds, int& first, int& second, float& weight);

const float FRAME_LOOP_SECONDS = 4.0f;

#endif

/* EOF */
