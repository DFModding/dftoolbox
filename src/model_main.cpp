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

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

#include "data_manager.hpp"
#include "model_bundle.hpp"
#include "model_scene.hpp"
#include "progress_logger.hpp"
#include "system.hpp"
#include "util.hpp"

namespace {

/** The paks are opened quick enough to not need any progress display */
class NullProgressLogger : public ProgressLogger
{
public:
  void println(const std::string&) override {}
  void set_task_size(int) override {}
  void set_task_status(int) override {}
  int  get_task_size() const override { return 1; }
  int  get_task_status() const override { return 0; }
  ProgressLogger& start_subtask() override { return *this; }
  ProgressLogger* get_subtask() const override { return nullptr; }
  void set_done() override {}
  bool is_done() const override { return true; }
  void sync() override {}
};

void open_paks(const std::string& directory)
{
  NullProgressLogger logger;
  data_manager.add_directory(directory + "/", logger);
  data_manager.add_filelist("df-directory.txt");
}

void print_mesh(const ModelMesh& mesh)
{
  std::cout << "    " << mesh.name
            << " vertices=" << mesh.vertices.size()
            << " triangles=" << mesh.indices.size() / 3
            << " frames=" << mesh.frame_count
            << " bones=" << mesh.bones.size()
            << " rescale=" << mesh.rescale
            << std::endl;

  for(std::vector<ModelStage>::const_iterator i = mesh.stages.begin(); i != mesh.stages.end(); ++i)
    {
      std::cout << "      stage first=" << i->first_index << " count=" << i->index_count << std::endl;
      for(int j = 0; j < int(i->textures.size()); ++j)
        if (!i->textures[j].empty())
          std::cout << "        layer " << j << ": " << i->textures[j] << std::endl;
    }
}

/** Reads every mesh, returns the number of failures */
int check_bundle(const ModelBundle& bundle, const std::string& name)
{
  int meshes = 0, skipped = 0, failed = 0, triangles = 0;

  const std::vector<ModelBundle::SmrEntry>& smrs = bundle.get_smrs();
  for(std::vector<ModelBundle::SmrEntry>::const_iterator i = smrs.begin(); i != smrs.end(); ++i)
    {
      for(std::vector<ModelBundle::MeshEntry>::const_iterator j = i->meshes.begin(); j != i->meshes.end(); ++j)
        {
          try {
            ModelMesh mesh;
            if (bundle.load_mesh(*j, mesh))
              {
                meshes += 1;
                triangles += mesh.indices.size() / 3;
                for(std::vector<uint16_t>::const_iterator k = mesh.indices.begin(); k != mesh.indices.end(); ++k)
                  if (*k >= mesh.vertices.size())
                    throw std::runtime_error("Bundle: " + mesh.name + ": index out of range");
              }
            else
              {
                skipped += 1;
              }
          } catch(std::exception& err) {
            std::cout << name << ": " << i->name << ": " << err.what() << std::endl;
            failed += 1;
          }
        }
    }

  std::cout << name << ": smrs=" << smrs.size() << " meshes=" << meshes
            << " skipped=" << skipped << " failed=" << failed
            << " triangles=" << triangles << std::endl;
  return failed;
}

/** Reads every animation and checks that it results in usable
    numbers for the meshes of \a library, returns the number of failures */
int check_animations(ModelLibrary& library, const std::vector<std::string>& names)
{
  int parsed = 0, unsupported = 0, failed = 0;

  const std::vector<std::string> files = data_manager.find_files("", ".bpr");
  for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i)
    {
      try {
        if (library.load_animation(*i, ""))
          parsed += 1;
        else
          unsupported += 1;
      } catch(std::exception& err) {
        std::cout << *i << ": " << err.what() << std::endl;
        failed += 1;
      }
    }

  int skinned = 0, without_skeleton = 0, sampled = 0, broken = 0;
  for(std::vector<std::string>::const_iterator i = names.begin(); i != names.end(); ++i)
    {
      std::shared_ptr<ModelBundle> bundle = library.get_bundle(*i);
      if (!bundle)
        continue;

      const std::vector<ModelBundle::SmrEntry>& smrs = bundle->get_smrs();
      for(std::vector<ModelBundle::SmrEntry>::const_iterator j = smrs.begin(); j != smrs.end(); ++j)
        for(std::vector<ModelBundle::MeshEntry>::const_iterator k = j->meshes.begin(); k != j->meshes.end(); ++k)
          {
            std::shared_ptr<const ModelMesh> mesh = library.load_mesh(*i, j->name, k->name);
            if (!mesh || mesh->bones.empty())
              continue;

            skinned += 1;
            if (mesh->bone_parents.empty())
              {
                without_skeleton += 1;
                continue;
              }

            const std::vector<std::string> found = library.find_animations(j->name, int(mesh->bones.size()), "");
            for(size_t a = 0; a < std::min(found.size(), size_t(3)); ++a)
              {
                std::shared_ptr<const ModelAnimation> animation = library.load_animation(found[a], "");
                if (!animation)
                  continue;

                std::vector<float> rows;
                sample_skeleton(*mesh, *animation, animation->duration * 0.37f, rows);
                sampled += 1;
                for(size_t r = 0; r < rows.size(); ++r)
                  if (!std::isfinite(rows[r]) || std::fabs(rows[r]) > 1000.0f)
                    {
                      broken += 1;
                      break;
                    }
              }
          }
    }

  std::cout << "animations: parsed=" << parsed << " unsupported=" << unsupported << " failed=" << failed << "\n"
            << "skinned meshes: " << skinned << " without_skeleton=" << without_skeleton
            << " sampled=" << sampled << " broken=" << broken << std::endl;
  return failed + broken;
}

int count_nodes(const ModelNode& node, int& meshes)
{
  int count = 1;
  if (node.mesh)
    meshes += 1;
  for(std::vector<ModelNode>::const_iterator i = node.children.begin(); i != node.children.end(); ++i)
    count += count_nodes(*i, meshes);
  return count;
}

void print_scene(const ModelScene& scene)
{
  int meshes = 0;
  const int nodes = count_nodes(scene.root, meshes);
  std::cout << scene.root.name << ": trees=" << scene.root.children.size()
            << " nodes=" << nodes << " meshes=" << meshes
            << " warnings=" << scene.warnings.size() << std::endl;
  for(std::vector<ModelNode>::const_iterator i = scene.root.children.begin(); i != scene.root.children.end(); ++i)
    std::cout << "  " << i->name << std::endl;
  for(std::vector<std::string>::const_iterator i = scene.warnings.begin(); i != scene.warnings.end(); ++i)
    std::cout << "  warning: " << *i << std::endl;
}

void usage()
{
  std::cout << "Usage: dftool-model list FILE.bun\n"
            << "       dftool-model check PAKDIR\n"
            << "       dftool-model location PAKDIR [NAME]...\n"
            << "       dftool-model sir PAKDIR FILE.sir [PAK]\n"
            << "\n"
            << "  list      Show the meshes of a bundle\n"
            << "  check     Read every mesh of every bundle\n"
            << "  location  Load a whole location, all of them if no NAME is given\n"
            << "  sir       Load a single .sir file, PAK is the pak to look in first"
            << std::endl;
}

} // namespace

int main(int argc, char** argv)
{
  try {
    if (argc == 3 && strcmp(argv[1], "list") == 0)
      {
        std::vector<char> data;
        if (!file_readbytes(argv[2], data))
          throw std::runtime_error(std::string("Couldn't read ") + argv[2]);

        ModelBundle bundle(std::move(data));
        const std::vector<ModelBundle::SmrEntry>& smrs = bundle.get_smrs();
        for(std::vector<ModelBundle::SmrEntry>::const_iterator i = smrs.begin(); i != smrs.end(); ++i)
          {
            std::cout << i->name << std::endl;
            for(std::vector<ModelBundle::MeshEntry>::const_iterator j = i->meshes.begin(); j != i->meshes.end(); ++j)
              {
                ModelMesh mesh;
                if (bundle.load_mesh(*j, mesh))
                  print_mesh(mesh);
                else
                  std::cout << "    " << j->name << " (not drawable)" << std::endl;
              }
          }
      }
    else if (argc == 3 && strcmp(argv[1], "check") == 0)
      {
        open_paks(argv[2]);

        ModelLibrary library(data_manager);
        const std::vector<std::string> names = library.get_bundle_names();
        int failed = 0;
        for(std::vector<std::string>::const_iterator i = names.begin(); i != names.end(); ++i)
          {
            std::shared_ptr<ModelBundle> bundle = library.get_bundle(*i);
            if (bundle)
              failed += check_bundle(*bundle, *i);
          }
        failed += check_animations(library, names);
        std::cout << "failed: " << failed << std::endl;
        return failed != 0;
      }
    else if (argc >= 3 && strcmp(argv[1], "location") == 0)
      {
        open_paks(argv[2]);

        ModelLibrary library(data_manager);
        std::vector<std::string> names(argv + 3, argv + argc);
        if (names.empty())
          names = library.get_bundle_names();

        for(std::vector<std::string>::const_iterator i = names.begin(); i != names.end(); ++i)
          {
            try {
              print_scene(library.load_location(*i));
            } catch(std::exception& err) {
              std::cout << *i << ": " << err.what() << std::endl;
            }
          }
      }
    else if ((argc == 4 || argc == 5) && strcmp(argv[1], "sir") == 0)
      {
        open_paks(argv[2]);

        ModelLibrary library(data_manager);
        print_scene(library.load_sir(argv[3], argc == 5 ? argv[4] : ""));
      }
    else
      {
        usage();
        return 1;
      }
  } catch(std::exception& err) {
    std::cout << "Error: " << err.what() << std::endl;
    return 1;
  }
  return 0;
}

/* EOF */
