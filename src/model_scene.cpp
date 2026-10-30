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

#include "model_scene.hpp"

#include <cmath>
#include <sstream>
#include <stdexcept>

#include "data_manager.hpp"
#include "shark3d.hpp"
#include "util.hpp"

namespace {

std::unique_ptr<Shark3D>
read_shark(DataManager& data_manager, const std::string& filename, const std::string& pak)
{
  std::vector<char> data;
  if (!data_manager.read(filename, pak, data))
    return std::unique_ptr<Shark3D>();

  std::istringstream in(std::string(data.data(), data.size()), std::ios::binary);
  return std::unique_ptr<Shark3D>(Shark3D::parse_binary(in));
}

std::string
get_string(const SectionNode* section, const std::string& ident)
{
  const std::vector<std::string> strings = section->get_strings(ident);
  return strings.empty() ? std::string() : strings.front();
}

int
get_int(const SectionNode* section, const std::string& ident, int def)
{
  const std::vector<int> ints = section->get_ints(ident);
  return ints.empty() ? def : ints.front();
}

float
get_float(const SectionNode* section, const std::string& ident, float def)
{
  const std::vector<float> floats = section->get_floats(ident);
  return floats.empty() ? def : floats.front();
}

/** Returns the first section named \a ident or null */
const SectionNode*
get_first(const SectionNode* section, const std::string& ident)
{
  const std::vector<SectionNode*> sections = section->get_sections(ident);
  return sections.empty() ? nullptr : sections.front();
}

ModelMaterial::Blend
parse_blend(const std::string& name, ModelMaterial::Blend def)
{
  if (name == "one")            return ModelMaterial::BLEND_ONE;
  if (name == "zero")           return ModelMaterial::BLEND_ZERO;
  if (name == "src_alpha")      return ModelMaterial::BLEND_SRC_ALPHA;
  if (name == "inv_src_alpha")  return ModelMaterial::BLEND_INV_SRC_ALPHA;
  if (name == "dest_alpha")     return ModelMaterial::BLEND_DEST_ALPHA;
  if (name == "inv_dest_alpha") return ModelMaterial::BLEND_INV_DEST_ALPHA;
  return def;
}

void
parse_pass(const SectionNode* pass, ModelMaterial& material)
{
  material.blend_src  = parse_blend(get_string(pass, "blend_mode_src"),  ModelMaterial::BLEND_ONE);
  material.blend_dest = parse_blend(get_string(pass, "blend_mode_dest"), ModelMaterial::BLEND_ZERO);

  const std::string alpha_test = get_string(pass, "alpha_test");
  if (alpha_test == "greater")
    material.alpha_test = ModelMaterial::ALPHA_TEST_GREATER;
  else if (alpha_test == "less")
    material.alpha_test = ModelMaterial::ALPHA_TEST_LESS;
  material.alpha_ref = get_float(pass, "alpha_ref", 0.0f);

  material.cull        = get_string(pass, "cull_mode") != "none";
  material.depth_write = get_int(pass, "depth_write", 1) != 0;
}

/** Shaders are trees of alternatives, this follows the path the
    game would take on a PC with a shader model 3 card down to the
    first pass, returns true if one was found */
bool
parse_shader(const SectionNode* shader, ModelMaterial& material)
{
  std::string type = get_string(shader, "type");
  if (type.empty())
    type = get_string(shader, "child_type");

  const SectionNode* param = get_first(shader, "param");
  if (!param)
    param = get_first(shader, "child_param");
  if (!param)
    return false;

  if (type == "eng_shader_std.shaderfactory|basic" ||
      type == "eng_shader_funcom.shaderfactory|basic")
    {
      const SectionNode* pass = get_first(param, "passes");
      if (!pass)
        return false;
      parse_pass(pass, material);
      return true;
    }
  else if (type == "eng_shader_std.shaderfactory|alternative" ||
           type == "eng_shader_std.shaderfactory|variants")
    {
      const std::vector<SectionNode*> children = param->get_sections("children");
      for(std::vector<SectionNode*>::const_iterator i = children.begin(); i != children.end(); ++i)
        {
          if (get_string(*i, "plat_pat") != "xbox")
            return parse_shader(*i, material);
        }
      return false;
    }
  else if (type == "eng_shader_special.shaderfactory|billboard" ||
           type == "eng_shader_shvol.shaderfactory|simple")
    {
      return parse_shader(param, material);
    }
  else
    {
      return false;
    }
}

std::string
replace_extension(const std::string& filename, const std::string& extension)
{
  return filename.substr(0, filename.rfind('.')) + extension;
}

/** Collects the trees and the bundle of a location */
void
find_trees(const SectionNode* group, std::vector<std::string>& trees, std::string& bundle)
{
  const std::vector<SectionNode*> children = group->get_sections("children");
  for(std::vector<SectionNode*>::const_iterator i = children.begin(); i != children.end(); ++i)
    {
      const std::string type = get_string(*i, "type");
      const SectionNode* param = get_first(*i, "param");

      if (param && type == "mod_engobj_funcom.locationinit")
        {
          bundle = get_string(param, "bundle");
        }
      else if (param && type == "mod_engobj_funcom.loadtree")
        {
          const std::string tree = get_string(param, "tree");
          if (!tree.empty())
            trees.push_back(tree);
        }
      else if (param && type == "mod_core.capsule")
        {
          const SectionNode* child_param = get_first(param, "child_param");
          if (child_param)
            find_trees(child_param, trees, bundle);
        }
    }
}

} // namespace

ModelMaterial::ModelMaterial()
  : blend_src(BLEND_ONE),
    blend_dest(BLEND_ZERO),
    alpha_test(ALPHA_TEST_NONE),
    alpha_ref(0.0f),
    cull(true),
    depth_write(true)
{
}

ModelNode::ModelNode()
  : scale(1.0f)
{
  static const float identity[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
  std::copy(identity, identity + 16, matrix);
}

ModelLibrary::ModelLibrary(DataManager& data_manager_)
  : data_manager(data_manager_)
{
}

std::vector<std::string>
ModelLibrary::get_bundle_names() const
{
  std::vector<std::string> names;
  const std::vector<std::string> paks = data_manager.get_paks();
  for(std::vector<std::string>::const_iterator i = paks.begin(); i != paks.end(); ++i)
    {
      if (has_suffix(*i, ".pak"))
        names.push_back(i->substr(0, i->size() - 4));
    }
  return names;
}

std::shared_ptr<ModelBundle>
ModelLibrary::get_bundle(const std::string& name)
{
  std::map<std::string, std::shared_ptr<ModelBundle> >::iterator i = bundles.find(name);
  if (i != bundles.end())
    return i->second;

  std::vector<char> data;
  if (!data_manager.read("bundles/" + name + ".bun", name + ".pak", data))
    return std::shared_ptr<ModelBundle>();

  std::shared_ptr<ModelBundle> bundle = std::make_shared<ModelBundle>(std::move(data));

  // They are big, so keep just enough to switch between objects of a location
  if (bundles.size() >= 4)
    bundles.clear();
  bundles[name] = bundle;
  return bundle;
}

std::shared_ptr<ModelBundle>
ModelLibrary::find_bundle(const std::string& smr, const std::string& preferred)
{
  if (!preferred.empty())
    {
      std::shared_ptr<ModelBundle> bundle = get_bundle(preferred);
      if (bundle && bundle->find_smr(smr))
        return bundle;
    }

  if (smr_index.empty())
    {
      const std::vector<std::string> names = get_bundle_names();
      for(std::vector<std::string>::const_iterator i = names.begin(); i != names.end(); ++i)
        {
          std::shared_ptr<ModelBundle> bundle = get_bundle(*i);
          if (bundle)
            {
              const std::vector<ModelBundle::SmrEntry>& smrs = bundle->get_smrs();
              for(std::vector<ModelBundle::SmrEntry>::const_iterator j = smrs.begin(); j != smrs.end(); ++j)
                smr_index.insert(std::make_pair(j->name, *i));
            }
        }
    }

  std::map<std::string, std::string>::iterator i = smr_index.find(smr);
  return i != smr_index.end() ? get_bundle(i->second) : std::shared_ptr<ModelBundle>();
}

const ModelMaterial&
ModelLibrary::get_material(const std::string& shader, const std::string& pak)
{
  std::map<std::string, ModelMaterial>::iterator i = materials.find(shader);
  if (i != materials.end())
    return i->second;

  ModelMaterial& material = materials[shader];

  const std::string::size_type hash = shader.find('#');
  if (hash != std::string::npos)
    {
      const std::string name = shader.substr(hash + 1);
      std::unique_ptr<Shark3D> file = read_shark(data_manager, shader.substr(0, hash), pak);
      if (file)
        {
          const SectionNodes* sections = file->get_sections("shaders");
          if (sections)
            {
              for(std::vector<SectionNode*>::const_iterator j = sections->sections.begin();
                  j != sections->sections.end(); ++j)
                {
                  if (get_string(*j, "name") == name)
                    {
                      parse_shader(*j, material);
                      break;
                    }
                }
            }
        }
    }

  return material;
}

std::shared_ptr<const ModelMesh>
ModelLibrary::load_mesh(const std::string& bundle_name, const std::string& smr, const std::string& model)
{
  const std::string key = bundle_name + "\n" + smr + "\n" + model;

  std::map<std::string, std::shared_ptr<const ModelMesh> >::iterator i = meshes.find(key);
  if (i != meshes.end())
    return i->second;

  std::shared_ptr<const ModelMesh> result;

  std::shared_ptr<ModelBundle> bundle = find_bundle(smr, bundle_name);
  if (bundle)
    {
      std::shared_ptr<ModelMesh> mesh = std::make_shared<ModelMesh>();
      if (bundle->load_mesh(smr, model, *mesh))
        {
          if (!mesh->bones.empty())
            load_skeleton(smr, "", *mesh);
          result = mesh;
        }
    }

  // What is on screen is kept alive by the canvas
  if (meshes.size() >= 4096)
    meshes.clear();

  meshes[key] = result;
  return result;
}

bool
ModelLibrary::load_node(const SectionNode* section, const std::string& smr, const std::string& bundle,
                        const std::string& pak, ModelScene& scene, ModelNode& node)
{
  node.name = get_string(section, "name");

  float scale = 1.0f;

  const std::string model  = get_string(section, "model");
  const std::string shader = get_string(section, "shader");

  // Things without shader are invisible, like collision volumes
  if (!model.empty() && !shader.empty())
    {
      try {
        node.mesh = load_mesh(bundle, smr, model);
        if (node.mesh)
          {
            node.material = get_material(shader, pak);
            if (node.mesh->rescale != 0.0f)
              scale = 1.0f / node.mesh->rescale;

            // Objects have their animation next to them, characters have many
            if (!node.mesh->bone_parents.empty())
              {
                std::shared_ptr<const ModelAnimation> animation =
                  load_animation(replace_extension(smr, ".bpr"), pak);
                if (animation && animation->tracks.size() == node.mesh->bones.size())
                  node.animation = animation;
              }
          }
      } catch(std::exception& err) {
        scene.warnings.push_back(smr + ": " + err.what());
      }
    }

  node.scale = scale;
  make_transform(section->get_floats("transl"), section->get_floats("quat"), scale, node.matrix);

  const std::string posgen = get_string(section, "posgen");
  if (!posgen.empty())
    node.path = get_path(replace_extension(smr, ".spr"), posgen, pak);

  const std::vector<SectionNode*> children = section->get_sections("child_array");
  for(std::vector<SectionNode*>::const_iterator i = children.begin(); i != children.end(); ++i)
    {
      ModelNode child;
      if (load_node(*i, smr, bundle, pak, scene, child))
        node.children.push_back(std::move(child));
    }

  return node.mesh || !node.children.empty();
}

void
ModelLibrary::load_skeleton(const std::string& smr, const std::string& pak, ModelMesh& mesh)
{
  std::unique_ptr<Shark3D> file = read_shark(data_manager, replace_extension(smr, ".skr"), pak);
  if (!file)
    return;

  const std::vector<SectionNode*> data = file->section->get_sections("data");
  if (data.empty())
    return;

  const std::vector<SectionNode*> skeletons = data.front()->get_sections("skel_array");
  for(std::vector<SectionNode*>::const_iterator i = skeletons.begin(); i != skeletons.end(); ++i)
    {
      if (get_string(*i, "name") != mesh.name)
        continue;

      // Each entry is the difference to the previous one, wrapping
      // around at 7 bits, so that the -1 of the root is 127
      const std::vector<int> deltas = (*i)->get_ints("bone_parent_idx_array");
      if (deltas.size() != mesh.bones.size())
        return;

      std::vector<int> parents;
      int parent = 0;
      for(size_t bone = 0; bone < deltas.size(); ++bone)
        {
          parent = (parent + deltas[bone]) & 0x7f;
          const int result = parent == 0x7f ? -1 : parent;
          if (result >= int(bone))
            return;
          parents.push_back(result);
        }

      mesh.bone_parents = parents;
      return;
    }
}

std::shared_ptr<const ModelPath>
ModelLibrary::get_path(const std::string& spr, const std::string& name, const std::string& pak)
{
  std::map<std::string, std::map<std::string, std::shared_ptr<const ModelPath> > >::iterator i = paths.find(spr);
  if (i == paths.end())
    {
      std::map<std::string, std::shared_ptr<const ModelPath> >& file_paths = paths[spr];

      std::unique_ptr<Shark3D> file = read_shark(data_manager, spr, pak);
      const std::vector<SectionNode*> data = file ? file->section->get_sections("data") : std::vector<SectionNode*>();
      if (!data.empty())
        {
          const std::vector<SectionNode*> sections = data.front()->get_sections("path_array");
          for(std::vector<SectionNode*>::const_iterator j = sections.begin(); j != sections.end(); ++j)
            {
              std::shared_ptr<ModelPath> path = std::make_shared<ModelPath>();
              path->duration = get_float(*j, "duration", 1.0f);

              const std::vector<SectionNode*> frames = (*j)->get_sections("frame_array");
              for(std::vector<SectionNode*>::const_iterator k = frames.begin(); k != frames.end(); ++k)
                {
                  const std::vector<float> position = (*k)->get_floats("transl");
                  const std::vector<float> rotation = (*k)->get_floats("quat");
                  if (position.size() != 3 || rotation.size() != 4)
                    continue;

                  ModelPath::Key key;
                  key.time = get_float(*k, "key", 0.0f);
                  std::copy(position.begin(), position.end(), key.position);
                  std::copy(rotation.begin(), rotation.end(), key.rotation);
                  path->keys.push_back(key);
                }

              // With a single key there is nothing to move
              if (path->keys.size() > 1)
                file_paths[get_string(*j, "name")] = path;
            }
        }

      i = paths.find(spr);
    }

  std::map<std::string, std::shared_ptr<const ModelPath> >::iterator j = i->second.find(name);
  return j != i->second.end() ? j->second : std::shared_ptr<const ModelPath>();
}

std::shared_ptr<const ModelAnimation>
ModelLibrary::load_animation(const std::string& filename, const std::string& pak)
{
  std::map<std::string, std::shared_ptr<const ModelAnimation> >::iterator i = animations.find(filename);
  if (i != animations.end())
    return i->second;

  std::shared_ptr<const ModelAnimation> result;

  std::vector<char> data;
  if (data_manager.read(filename, pak, data) && is_supported_animation(data))
    {
      std::shared_ptr<ModelAnimation> animation = std::make_shared<ModelAnimation>(parse_animation(data));
      animation->name = filename;
      result = animation;
    }

  animations[filename] = result;
  return result;
}

std::vector<std::string>
ModelLibrary::find_animations(const std::string& smr, int bone_count, const std::string& pak)
{
  const std::string directory = smr.substr(0, smr.rfind('/') + 1);

  std::vector<std::string> result;
  const std::vector<std::string> files = data_manager.find_files(directory, ".bpr");
  for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i)
    {
      std::vector<char> data;
      if (data_manager.read(*i, pak, data) && is_supported_animation(data) &&
          get_animation_bone_count(data) == bone_count)
        result.push_back(*i);
    }
  return result;
}

void
ModelLibrary::add_sir(ModelNode& parent, const std::string& sir, const std::string& pak,
                      const std::string& bundle, ModelScene& scene)
{
  std::unique_ptr<Shark3D> file = read_shark(data_manager, sir, pak);
  if (!file)
    throw std::runtime_error("Couldn't find " + sir);

  const std::vector<SectionNode*> data = file->section->get_sections("data");
  const SectionNode* root = data.empty() ? nullptr : get_first(data.front(), "root");
  if (!root)
    throw std::runtime_error(sir + " has no data/root");

  ModelNode node;
  if (load_node(root, replace_extension(sir, ".smr"), bundle, pak, scene, node))
    {
      node.name = sir;
      parent.children.push_back(std::move(node));
    }
}

ModelScene
ModelLibrary::load_sir(const std::string& sir, const std::string& pak)
{
  ModelScene scene;
  scene.pak = pak;
  scene.root.name = sir;
  add_sir(scene.root, sir, pak, has_suffix(pak, ".pak") ? pak.substr(0, pak.size() - 4) : pak, scene);
  return scene;
}

ModelScene
ModelLibrary::load_location(const std::string& name)
{
  ModelScene scene;
  scene.pak = name + ".pak";
  scene.root.name = name;

  const std::string filename = "data/generated/locations/" + name + ".cdr";
  std::unique_ptr<Shark3D> file = read_shark(data_manager, filename, scene.pak);
  if (!file)
    throw std::runtime_error("Couldn't find " + filename);

  const SectionNode* actor = get_first(file->section, "actor_param");
  const SectionNode* group = actor ? get_first(actor, "child_param") : nullptr;
  if (!group)
    throw std::runtime_error(filename + " isn't a location");

  std::vector<std::string> trees;
  std::string bundle;
  find_trees(group, trees, bundle);

  const auto add_trees = [&](bool animated)
    {
      for(std::vector<std::string>::const_iterator i = trees.begin(); i != trees.end(); ++i)
        {
          // Everything else is either invisible or gets moved to its
          // place by the game, which would stack it up at the origin
          if (i->compare(0, 14, "art/locations/") != 0 ||
              has_suffix(*i, "_col.sir") ||
              has_suffix(*i, "_fullscreenglow.sir") ||
              (i->find("/animation/") != std::string::npos ||
               i->find("/anim_") != std::string::npos) != animated)
            continue;

          try {
            add_sir(scene.root, *i, scene.pak, name, scene);
          } catch(std::exception& err) {
            scene.warnings.push_back(err.what());
          }
        }
    };

  // Animated trees only matter for cinematics, which have nothing else
  add_trees(false);
  if (scene.root.children.empty())
    add_trees(true);

  return scene;
}

/* EOF */
