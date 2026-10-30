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

#ifndef HEADER_MODEL_SCENE_HPP
#define HEADER_MODEL_SCENE_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "model_animation.hpp"
#include "model_bundle.hpp"

class DataManager;
class SectionNode;

/** The parts of a Shark3D shader that matter for the preview, the
    shader programs themselves are not used */
struct ModelMaterial
{
  enum Blend { BLEND_ONE, BLEND_ZERO, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA,
               BLEND_DEST_ALPHA, BLEND_INV_DEST_ALPHA };
  enum AlphaTest { ALPHA_TEST_NONE, ALPHA_TEST_GREATER, ALPHA_TEST_LESS };

  Blend blend_src;
  Blend blend_dest;

  AlphaTest alpha_test;
  float     alpha_ref;

  bool cull;
  bool depth_write;

  ModelMaterial();

  bool is_blended() const { return blend_src != BLEND_ONE || blend_dest != BLEND_ZERO; }
};

/** A node in the hierarchy of a .sir file */
struct ModelNode
{
  std::string name;

  /** Transformation relative to the parent, column major */
  float matrix[16];

  /** Scale of the mesh, needed to rebuild matrix for \a path */
  float scale;

  /** Movement of the node, null if it doesn't move */
  std::shared_ptr<const ModelPath> path;

  std::shared_ptr<const ModelMesh> mesh;
  ModelMaterial material;

  /** Skeletal animation that belongs to the mesh, null if there is none */
  std::shared_ptr<const ModelAnimation> animation;

  std::vector<ModelNode> children;

  ModelNode();
};

/** A .sir file (or a whole location) with everything it refers to loaded */
struct ModelScene
{
  ModelNode root;

  /** Files or objects that couldn't be loaded */
  std::vector<std::string> warnings;

  /** Pak to prefer for looking up the files of this scene */
  std::string pak;
};

/** Finds and caches the data that scenes are made of */
class ModelLibrary
{
private:
  DataManager& data_manager;

  /** The last bundles used, they hold their vertex data, so they don't get kept forever */
  std::map<std::string, std::shared_ptr<ModelBundle> > bundles;

  /** Bundle name of every .smr, filled on demand as this requires reading all the bundles */
  std::map<std::string, std::string> smr_index;

  std::map<std::string, std::shared_ptr<const ModelMesh> > meshes;
  std::map<std::string, ModelMaterial> materials;

  /** Paths by the .spr they are from and their name */
  std::map<std::string, std::map<std::string, std::shared_ptr<const ModelPath> > > paths;

  std::map<std::string, std::shared_ptr<const ModelAnimation> > animations;

public:
  ModelLibrary(DataManager& data_manager_);

  /** Returns the bundle from bundles/\a name.bun, or null if there is none */
  std::shared_ptr<ModelBundle> get_bundle(const std::string& name);

  /** Finds the bundle that holds \a smr, \a preferred (if not empty) is tried first */
  std::shared_ptr<ModelBundle> find_bundle(const std::string& smr, const std::string& preferred);

  /** Load a single .sir file from \a pak, throws if the file can't be
      read or isn't a .sir */
  ModelScene load_sir(const std::string& sir, const std::string& pak);

  /** Load all the trees of a location that are part of its environment */
  ModelScene load_location(const std::string& name);

  /** Load a mesh that doesn't come from a .sir, such as a single entry in a bundle */
  std::shared_ptr<const ModelMesh> load_mesh(const std::string& bundle,
                                             const std::string& smr, const std::string& model);

  /** \a shader is of the form "file.sdr#name", on errors defaults are returned */
  const ModelMaterial& get_material(const std::string& shader, const std::string& pak);

  /** Loads a .bpr, returns null if it is in a format that isn't supported */
  std::shared_ptr<const ModelAnimation> load_animation(const std::string& filename, const std::string& pak);

  /** Returns the .bpr files next to or below \a smr that fit a mesh with \a bone_count bones */
  std::vector<std::string> find_animations(const std::string& smr, int bone_count, const std::string& pak);

  /** Names of all the bundles */
  std::vector<std::string> get_bundle_names() const;

private:
  std::shared_ptr<const ModelPath> get_path(const std::string& spr, const std::string& name,
                                            const std::string& pak);
  void load_skeleton(const std::string& smr, const std::string& pak, ModelMesh& mesh);

  void add_sir(ModelNode& parent, const std::string& sir, const std::string& pak,
               const std::string& bundle, ModelScene& scene);
  bool load_node(const SectionNode* section, const std::string& smr, const std::string& bundle,
                 const std::string& pak, ModelScene& scene, ModelNode& node);

  ModelLibrary(const ModelLibrary&) = delete;
  ModelLibrary& operator=(const ModelLibrary&) = delete;
};

#endif

/* EOF */
