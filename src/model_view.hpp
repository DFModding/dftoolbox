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

#ifndef HEADER_MODEL_VIEW_HPP
#define HEADER_MODEL_VIEW_HPP

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QPoint>
#include <QVector3D>
#include <QWidget>

#include "model_scene.hpp"

class QAction;
class QComboBox;
class QLabel;
class QMainWindow;
class QOpenGLShaderProgram;
class QTimer;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;

/** Renders a ModelScene, textures get loaded while drawing so that a
    location shows up right away instead of after the last texture is
    decoded */
class ModelCanvas : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
private:
  struct GpuMesh
  {
    GLuint vao;
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLuint frame_buffer;
  };

  /** Nodes come before their children */
  struct DrawNode
  {
    int parent;
    QMatrix4x4 local;
    float scale;
    std::shared_ptr<const ModelPath> path;
    QMatrix4x4 world;
  };

  struct DrawItem
  {
    int node;
    std::shared_ptr<const ModelMesh> mesh;
    ModelMaterial material;

    /** The animation that came with the node */
    std::shared_ptr<const ModelAnimation> own_animation;

    QVector3D  bbox_min;
    QVector3D  bbox_max;
  };

  std::vector<DrawNode> nodes;
  std::vector<DrawItem> items;

  /** Replaces own_animation of the items it fits */
  std::shared_ptr<const ModelAnimation> animation;

  float   time;
  bool    playing;
  QTimer* animation_timer;
  QElapsedTimer animation_clock;
  std::vector<float> bone_rows;

  std::string pak;

  std::map<const ModelMesh*, GpuMesh> gpu_meshes;

  /** Keeps the meshes in gpu_meshes alive, as they are identified by address */
  std::vector<std::shared_ptr<const ModelMesh> > gpu_mesh_owners;

  /** A texture of 0 is one that can't be displayed */
  std::map<std::string, GLuint> textures;
  std::vector<std::string> pending_textures;
  std::set<std::string>    queued_textures;

  QOpenGLShaderProgram* program;

  bool show_textures;
  bool show_lighting;
  bool show_wireframe;
  bool cull;

  QVector3D target;
  float yaw;
  float pitch;
  float distance;
  float scene_size;

  QPoint last_mouse;
  std::set<int> keys;
  QTimer* fly_timer;
  QElapsedTimer fly_clock;

public:
  ModelCanvas(QWidget* parent);
  ~ModelCanvas();

  void set_scene(const ModelScene& scene);
  void clear();

  void set_show_textures(bool t);
  void set_show_lighting(bool t);
  void set_show_wireframe(bool t);
  void set_cull(bool t);

  /** Use \a animation on all the meshes it fits, null to go back to
      what the scene came with */
  void set_animation(std::shared_ptr<const ModelAnimation> animation_);

  void set_playing(bool t);
  void set_time(float seconds);

  /** True if anything in the scene can move */
  bool is_animated() const;

  /** True if a mesh came with a skeletal animation of its own */
  bool has_own_animation() const;

  /** Place the camera so that the whole scene is visible */
  void reset_camera();
  void set_camera(const QVector3D& target_, float yaw_, float pitch_, float distance_);

  /** True while there are textures left to load */
  bool is_loading() const { return !pending_textures.empty(); }

protected:
  void initializeGL() override;
  void paintGL() override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

private:
  void add_node(const ModelNode& node, int parent);
  void update_nodes();
  void update_timer();

  const GpuMesh& get_gpu_mesh(const DrawItem& item);
  GLuint get_texture(const std::string& name);
  GLuint load_texture(const std::string& name);
  void load_pending_textures();
  void free_gl();

  bool fits(const DrawItem& item, const ModelAnimation& animation_) const;
  void draw_item(const DrawItem& item, const QMatrix4x4& view_projection);
  QVector3D get_eye() const;
  void fly();
};

/** Tree of the content of a bundle or .sir file, next to its 3D view */
class ModelView : public QWidget
{
private:
  ModelLibrary library;

  QToolBar*    toolbar;
  QAction*     source_action;
  QAction*     play_action;
  QAction*     animation_widget;
  QComboBox*   animation_box;
  QTreeWidget* tree;
  ModelCanvas* canvas;
  QLabel*      info;

  std::string  bundle_name;
  std::string  pak;

  std::function<void ()> show_source;

public:
  ModelView(QWidget* parent, QMainWindow* mainwindow);

  /** Show the content of bundles/\a name.bun, throws on errors */
  void set_bundle(const std::string& name, const std::string& pak);

  /** Show a .sir file, returns false if there is nothing visible in it */
  bool set_sir(const std::string& filename, const std::string& pak);

  /** \a callback gets called when the user asks to see the .sir as text */
  void set_show_source(std::function<void ()> callback);

  void show_toolbar(bool t);

private:
  void on_item_selected(QTreeWidgetItem* item);
  void show_scene(const ModelScene& scene, const std::string& smr);
  void on_animation_selected(int index);
  void add_scene_nodes(QTreeWidgetItem* parent, const ModelNode& node);
};

#endif

/* EOF */
