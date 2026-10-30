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

#include "model_view.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>

#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QSplitter>
#include <QSurfaceFormat>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "data_manager.hpp"
#include "dds.hpp"
#include "util.hpp"

namespace {

// More than any mesh of the game uses
const int MAX_BONES = 128;

const char* vertex_shader = R"(#version 330 core
#define MAX_BONES 128
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec2 uv2;
layout(location = 4) in vec4 color;
layout(location = 5) in vec4 weights;
layout(location = 6) in vec4 bones;
layout(location = 7) in vec3 next_position;
layout(location = 8) in vec3 next_normal;

uniform mat4 view_projection;
uniform mat4 model;

// Three rows for each bone
uniform vec4  bone_rows[MAX_BONES * 3];
uniform bool  use_bones;
uniform float next_weight;

out vec3 world_position;
out vec3 world_normal;
out vec2 frag_uv;
out vec2 frag_uv2;
out vec4 frag_color;

void main()
{
  vec4 point = vec4(mix(position, next_position, next_weight), 1.0);
  vec3 direction = mix(normal, next_normal, next_weight);

  if (use_bones)
    {
      vec4 rows[3] = vec4[3](vec4(0.0), vec4(0.0), vec4(0.0));
      for(int i = 0; i < 4; ++i)
        for(int row = 0; row < 3; ++row)
          rows[row] += weights[i] * bone_rows[int(bones[i]) * 3 + row];

      point = vec4(dot(rows[0], point), dot(rows[1], point), dot(rows[2], point), 1.0);
      direction = vec3(dot(rows[0].xyz, direction), dot(rows[1].xyz, direction), dot(rows[2].xyz, direction));
    }

  vec4 world = model * point;
  world_position = world.xyz;
  world_normal   = mat3(model) * direction;
  frag_uv    = uv;
  frag_uv2   = uv2;
  frag_color = color;
  gl_Position = view_projection * world;
}
)";

const char* fragment_shader = R"(#version 330 core
in vec3 world_position;
in vec3 world_normal;
in vec2 frag_uv;
in vec2 frag_uv2;
in vec4 frag_color;

uniform sampler2D base_texture;
uniform sampler2D lightmap_texture;
uniform bool use_base;
uniform bool use_lightmap;
uniform bool use_colors;
uniform bool use_headlight;
uniform vec3 eye;
uniform int   alpha_test;
uniform float alpha_ref;

out vec4 result;

void main()
{
  vec4 color = use_base ? texture(base_texture, frag_uv) : vec4(0.7, 0.7, 0.7, 1.0);

  if ((alpha_test == 1 && color.a <= alpha_ref) ||
      (alpha_test == 2 && color.a >= alpha_ref))
    discard;

  // The game's pixel shaders multiply baked light with mul_x2
  if (use_lightmap)
    color.rgb *= 2.0 * texture(lightmap_texture, frag_uv2).rgb;
  else if (use_colors)
    color.rgb *= 2.0 * frag_color.rgb;

  if (use_headlight)
    {
      float len = length(world_normal);
      float light = 1.0;
      if (len > 0.0)
        light = abs(dot(world_normal / len, normalize(eye - world_position)));
      color.rgb *= 0.35 + 0.65 * light;
    }

  result = color;
}
)";

GLenum to_gl(ModelMaterial::Blend blend)
{
  switch(blend)
    {
    case ModelMaterial::BLEND_ONE:            return GL_ONE;
    case ModelMaterial::BLEND_ZERO:           return GL_ZERO;
    case ModelMaterial::BLEND_SRC_ALPHA:      return GL_SRC_ALPHA;
    case ModelMaterial::BLEND_INV_SRC_ALPHA:  return GL_ONE_MINUS_SRC_ALPHA;
    case ModelMaterial::BLEND_DEST_ALPHA:     return GL_DST_ALPHA;
    case ModelMaterial::BLEND_INV_DEST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
    }
  return GL_ONE;
}

bool contains(const std::string& str, const char* part)
{
  return str.find(part) != std::string::npos;
}

bool is_lightmap(const std::string& texture)
{
  return contains(texture, "lightmap");
}

/** Everything that doesn't hold colors for the first set of texture coordinates */
bool is_base_texture(const std::string& texture)
{
  return !texture.empty() && !is_lightmap(texture) &&
    !contains(texture, "_nml.") && !contains(texture, "_hmap.");
}

std::string find_base_texture(const std::vector<std::string>& stage_textures)
{
  std::vector<std::string>::const_iterator i =
    std::find_if(stage_textures.begin(), stage_textures.end(), is_base_texture);
  return i != stage_textures.end() ? *i : std::string();
}

std::string find_lightmap(const std::vector<std::string>& stage_textures)
{
  std::vector<std::string>::const_iterator i =
    std::find_if(stage_textures.begin(), stage_textures.end(), is_lightmap);
  return i != stage_textures.end() ? *i : std::string();
}

} // namespace

ModelCanvas::ModelCanvas(QWidget* parent)
  : QOpenGLWidget(parent),
    program(nullptr),
    show_textures(true),
    show_lighting(true),
    show_wireframe(false),
    cull(true),
    time(0.0f),
    playing(true),
    yaw(0.6f),
    pitch(0.3f),
    distance(5.0f),
    scene_size(1.0f)
{
  QSurfaceFormat format;
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setSamples(4);
  setFormat(format);

  setFocusPolicy(Qt::ClickFocus);

  fly_timer = new QTimer(this);
  fly_timer->setInterval(16);
  connect(fly_timer, &QTimer::timeout, this, [this]{ fly(); });

  animation_timer = new QTimer(this);
  animation_timer->setInterval(16);
  connect(animation_timer, &QTimer::timeout, this, [this]
          {
            time += animation_clock.restart() / 1000.0f;
            update();
          });
}

ModelCanvas::~ModelCanvas()
{
  if (program)
    {
      makeCurrent();
      free_gl();
      delete program;
      doneCurrent();
    }
}

void
ModelCanvas::free_gl()
{
  for(std::map<const ModelMesh*, GpuMesh>::iterator i = gpu_meshes.begin(); i != gpu_meshes.end(); ++i)
    {
      glDeleteVertexArrays(1, &i->second.vao);
      glDeleteBuffers(1, &i->second.vertex_buffer);
      glDeleteBuffers(1, &i->second.index_buffer);
      glDeleteBuffers(1, &i->second.frame_buffer);
    }
  gpu_meshes.clear();
  gpu_mesh_owners.clear();

  for(std::map<std::string, GLuint>::iterator i = textures.begin(); i != textures.end(); ++i)
    {
      if (i->second)
        glDeleteTextures(1, &i->second);
    }
  textures.clear();
  pending_textures.clear();
  queued_textures.clear();
}

void
ModelCanvas::clear()
{
  nodes.clear();
  items.clear();
  animation.reset();
  update_timer();
  if (program)
    {
      makeCurrent();
      free_gl();
      doneCurrent();
    }
  update();
}

void
ModelCanvas::add_node(const ModelNode& node, int parent)
{
  DrawNode draw_node;
  draw_node.parent = parent;
  draw_node.local  = QMatrix4x4(node.matrix).transposed();
  draw_node.scale  = node.scale;
  draw_node.path   = node.path;
  draw_node.world  = parent == -1 ? draw_node.local : nodes[parent].world * draw_node.local;
  nodes.push_back(draw_node);

  const int index = int(nodes.size()) - 1;

  if (node.mesh && !node.mesh->vertices.empty())
    {
      DrawItem item;
      item.node     = index;
      item.mesh     = node.mesh;
      item.material = node.material;
      item.own_animation = node.animation;

      const QMatrix4x4& matrix = nodes[index].world;

      bool first = true;
      for(std::vector<ModelVertex>::const_iterator i = node.mesh->vertices.begin();
          i != node.mesh->vertices.end(); ++i)
        {
          const QVector3D p = matrix.map(QVector3D(i->position[0], i->position[1], i->position[2]));
          if (first)
            {
              item.bbox_min = item.bbox_max = p;
              first = false;
            }
          else
            {
              item.bbox_min = QVector3D(std::min(item.bbox_min.x(), p.x()),
                                        std::min(item.bbox_min.y(), p.y()),
                                        std::min(item.bbox_min.z(), p.z()));
              item.bbox_max = QVector3D(std::max(item.bbox_max.x(), p.x()),
                                        std::max(item.bbox_max.y(), p.y()),
                                        std::max(item.bbox_max.z(), p.z()));
            }
        }
      items.push_back(std::move(item));
    }

  for(std::vector<ModelNode>::const_iterator i = node.children.begin(); i != node.children.end(); ++i)
    add_node(*i, index);
}

void
ModelCanvas::update_nodes()
{
  for(std::vector<DrawNode>::iterator i = nodes.begin(); i != nodes.end(); ++i)
    {
      QMatrix4x4 local = i->local;
      if (i->path)
        {
          float matrix[16];
          sample_path(*i->path, time, i->scale, matrix);
          local = QMatrix4x4(matrix).transposed();
        }
      i->world = i->parent == -1 ? local : nodes[i->parent].world * local;
    }
}

bool
ModelCanvas::fits(const DrawItem& item, const ModelAnimation& animation_) const
{
  return item.mesh->bones.size() == animation_.tracks.size() &&
    item.mesh->bone_parents.size() == animation_.tracks.size() &&
    int(animation_.tracks.size()) <= MAX_BONES;
}

bool
ModelCanvas::is_animated() const
{
  if (animation)
    return true;

  for(std::vector<DrawNode>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
    if (i->path)
      return true;

  for(std::vector<DrawItem>::const_iterator i = items.begin(); i != items.end(); ++i)
    if (i->own_animation || !i->mesh->frame_times.empty())
      return true;

  return false;
}

bool
ModelCanvas::has_own_animation() const
{
  for(std::vector<DrawItem>::const_iterator i = items.begin(); i != items.end(); ++i)
    if (i->own_animation && fits(*i, *i->own_animation))
      return true;

  return false;
}

void
ModelCanvas::update_timer()
{
  if (playing && isVisible() && is_animated())
    {
      if (!animation_timer->isActive())
        {
          animation_clock.start();
          animation_timer->start();
        }
    }
  else
    {
      animation_timer->stop();
    }
}

void
ModelCanvas::set_animation(std::shared_ptr<const ModelAnimation> animation_)
{
  animation = animation_;
  time = 0.0f;
  update_timer();
  update();
}

void
ModelCanvas::set_playing(bool t)
{
  playing = t;
  update_timer();
}

void
ModelCanvas::set_time(float seconds)
{
  time = seconds;
  update();
}

void
ModelCanvas::showEvent(QShowEvent* event)
{
  QOpenGLWidget::showEvent(event);
  update_timer();
}

void
ModelCanvas::hideEvent(QHideEvent* event)
{
  QOpenGLWidget::hideEvent(event);
  update_timer();
}

void
ModelCanvas::set_scene(const ModelScene& scene)
{
  clear();
  pak = scene.pak;
  time = 0.0f;
  add_node(scene.root, -1);
  update_timer();
  reset_camera();
}

void
ModelCanvas::reset_camera()
{
  if (items.empty())
    return;

  // The sky around a location is many times the size of what is of
  // interest, so go by where most of the objects are
  std::vector<float> xs[3];
  for(std::vector<DrawItem>::const_iterator i = items.begin(); i != items.end(); ++i)
    {
      const QVector3D center = (i->bbox_min + i->bbox_max) * 0.5f;
      for(int axis = 0; axis < 3; ++axis)
        xs[axis].push_back(center[axis]);
    }

  QVector3D low, high;
  for(int axis = 0; axis < 3; ++axis)
    {
      std::sort(xs[axis].begin(), xs[axis].end());
      const size_t margin = xs[axis].size() < 16 ? 0 : xs[axis].size() / 10;
      low[axis]  = xs[axis][margin];
      high[axis] = xs[axis][xs[axis].size() - 1 - margin];
    }

  if (items.size() < 16)
    {
      // Small enough that the outline of the objects themselves matters
      for(std::vector<DrawItem>::const_iterator i = items.begin(); i != items.end(); ++i)
        for(int axis = 0; axis < 3; ++axis)
          {
            low[axis]  = std::min(low[axis],  i->bbox_min[axis]);
            high[axis] = std::max(high[axis], i->bbox_max[axis]);
          }
    }

  target = (low + high) * 0.5f;
  scene_size = std::max((high - low).length(), 0.01f);
  yaw = 0.6f;
  pitch = 0.3f;
  distance = scene_size * 1.2f;
  update();
}

void
ModelCanvas::set_camera(const QVector3D& target_, float yaw_, float pitch_, float distance_)
{
  target = target_;
  yaw = yaw_;
  pitch = pitch_;
  distance = distance_;
  update();
}

void ModelCanvas::set_show_textures(bool t)  { show_textures = t;  update(); }
void ModelCanvas::set_show_lighting(bool t)  { show_lighting = t;  update(); }
void ModelCanvas::set_show_wireframe(bool t) { show_wireframe = t; update(); }
void ModelCanvas::set_cull(bool t)           { cull = t;           update(); }

void
ModelCanvas::initializeGL()
{
  initializeOpenGLFunctions();

  std::cerr << "renderer: " << reinterpret_cast<const char*>(glGetString(GL_VENDOR))
            << ", " << reinterpret_cast<const char*>(glGetString(GL_RENDERER))
            << ", OpenGL " << reinterpret_cast<const char*>(glGetString(GL_VERSION))
            << ", GLSL " << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))
            << std::endl;

  program = new QOpenGLShaderProgram;
  program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader);
  program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader);
  if (!program->link())
    std::cout << "ModelCanvas: " << program->log().toStdString() << std::endl;

  connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, [this]
          {
            makeCurrent();
            free_gl();
            doneCurrent();
          });
}

const ModelCanvas::GpuMesh&
ModelCanvas::get_gpu_mesh(const DrawItem& item)
{
  std::map<const ModelMesh*, GpuMesh>::iterator i = gpu_meshes.find(item.mesh.get());
  if (i != gpu_meshes.end())
    return i->second;

  const ModelMesh& mesh = *item.mesh;

  GpuMesh gpu;
  glGenVertexArrays(1, &gpu.vao);
  glGenBuffers(1, &gpu.vertex_buffer);
  glGenBuffers(1, &gpu.index_buffer);
  glGenBuffers(1, &gpu.frame_buffer);

  glBindVertexArray(gpu.vao);

  glBindBuffer(GL_ARRAY_BUFFER, gpu.vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(ModelVertex), mesh.vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu.index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint16_t), mesh.indices.data(), GL_STATIC_DRAW);

  const GLsizei stride = sizeof(ModelVertex);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(ModelVertex, position)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(ModelVertex, normal)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(ModelVertex, uv)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(ModelVertex, uv2)));
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, reinterpret_cast<void*>(offsetof(ModelVertex, color)));
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(ModelVertex, weights)));
  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(ModelVertex, bones)));

  // Attribute 0, 1, 7 and 8 get pointed at the frames when drawing
  if (!mesh.frames.empty())
    {
      glBindBuffer(GL_ARRAY_BUFFER, gpu.frame_buffer);
      glBufferData(GL_ARRAY_BUFFER, mesh.frames.size() * sizeof(ModelFrameVertex), mesh.frames.data(), GL_STATIC_DRAW);
      glEnableVertexAttribArray(7);
      glEnableVertexAttribArray(8);
    }

  glBindVertexArray(0);

  gpu_mesh_owners.push_back(item.mesh);
  return gpu_meshes[item.mesh.get()] = gpu;
}

GLuint
ModelCanvas::get_texture(const std::string& name)
{
  if (name.empty())
    return 0;

  std::map<std::string, GLuint>::iterator i = textures.find(name);
  if (i != textures.end())
    return i->second;

  if (queued_textures.insert(name).second)
    pending_textures.push_back(name);
  return 0;
}

GLuint
ModelCanvas::load_texture(const std::string& name)
{
  try {
    std::vector<char> data;
    if (!data_manager.read(name, pak, data))
      return 0;

    QImage image;
    if (data.size() > 4 && memcmp(data.data(), "DDS ", 4) == 0)
      {
        std::istringstream in(std::string(data.data(), data.size()), std::ios::binary);
        DDS dds(in);
        const std::vector<char>& pixels = dds.get_data();
        image = QImage(reinterpret_cast<const uchar*>(pixels.data()), dds.width, dds.height,
                       QImage::Format_RGBA8888).copy();
      }
    else
      {
        image = QImage::fromData(reinterpret_cast<const uchar*>(data.data()), data.size());
      }

    if (image.isNull())
      return 0;

    image = image.convertToFormat(QImage::Format_RGBA8888);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image.bytesPerLine() / 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
  } catch(std::exception& err) {
    std::cout << "ModelCanvas: " << name << ": " << err.what() << std::endl;
    return 0;
  }
}

void
ModelCanvas::load_pending_textures()
{
  QElapsedTimer clock;
  clock.start();

  while(!pending_textures.empty() && clock.elapsed() < 30)
    {
      const std::string name = pending_textures.back();
      pending_textures.pop_back();
      textures[name] = load_texture(name);
    }
}

QVector3D
ModelCanvas::get_eye() const
{
  return target + distance * QVector3D(std::cos(pitch) * std::cos(yaw),
                                       std::cos(pitch) * std::sin(yaw),
                                       std::sin(pitch));
}

void
ModelCanvas::draw_item(const DrawItem& item, const QMatrix4x4& view_projection)
{
  const GpuMesh& gpu = get_gpu_mesh(item);
  const ModelMaterial& material = item.material;

  program->setUniformValue("view_projection", view_projection);
  program->setUniformValue("model", nodes[item.node].world);
  program->setUniformValue("alpha_test", show_textures ? int(material.alpha_test) : 0);
  program->setUniformValue("alpha_ref", material.alpha_ref);

  if (cull && material.cull)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);

  if (material.is_blended() && show_textures)
    {
      glEnable(GL_BLEND);
      glBlendFunc(to_gl(material.blend_src), to_gl(material.blend_dest));
    }
  else
    {
      glDisable(GL_BLEND);
    }
  glDepthMask(material.depth_write ? GL_TRUE : GL_FALSE);

  glBindVertexArray(gpu.vao);

  const ModelAnimation* bone_animation =
    (animation && fits(item, *animation)) ? animation.get() :
    (item.own_animation && fits(item, *item.own_animation)) ? item.own_animation.get() : nullptr;

  program->setUniformValue("use_bones", bone_animation != nullptr);
  if (bone_animation)
    {
      sample_skeleton(*item.mesh, *bone_animation, time, bone_rows);
      glUniform4fv(program->uniformLocation("bone_rows"), GLsizei(bone_rows.size() / 4), bone_rows.data());
    }

  float next_weight = 0.0f;
  if (!item.mesh->frames.empty())
    {
      int first, second;
      sample_frames(*item.mesh, time, first, second, next_weight);

      const size_t count = item.mesh->vertices.size();
      const size_t size  = sizeof(ModelFrameVertex);
      const auto pointer = [&](size_t frame, size_t member)
        {
          return reinterpret_cast<void*>(frame * count * size + member);
        };

      glBindBuffer(GL_ARRAY_BUFFER, gpu.frame_buffer);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, size, pointer(first, offsetof(ModelFrameVertex, position)));
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, size, pointer(first, offsetof(ModelFrameVertex, normal)));
      glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, size, pointer(second, offsetof(ModelFrameVertex, position)));
      glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, size, pointer(second, offsetof(ModelFrameVertex, normal)));
    }
  program->setUniformValue("next_weight", next_weight);

  for(std::vector<ModelStage>::const_iterator i = item.mesh->stages.begin(); i != item.mesh->stages.end(); ++i)
    {
      const GLuint base = show_textures ? get_texture(find_base_texture(i->textures)) : 0;
      const GLuint lightmap = show_lighting ? get_texture(find_lightmap(i->textures)) : 0;

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, base);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, lightmap);

      program->setUniformValue("use_base", base != 0);
      program->setUniformValue("use_lightmap", lightmap != 0);
      program->setUniformValue("use_colors", show_lighting && lightmap == 0 && item.mesh->has_colors);
      program->setUniformValue("use_headlight", lightmap == 0 && !(show_lighting && item.mesh->has_colors));

      glDrawElements(GL_TRIANGLES, i->index_count, GL_UNSIGNED_SHORT,
                     reinterpret_cast<void*>(size_t(i->first_index) * sizeof(uint16_t)));
    }
}

void
ModelCanvas::paintGL()
{
  load_pending_textures();

  glClearColor(0.2f, 0.22f, 0.25f, 1.0f);
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!program || !program->isLinked() || items.empty())
    return;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glPolygonMode(GL_FRONT_AND_BACK, show_wireframe ? GL_LINE : GL_FILL);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  update_nodes();

  const QVector3D eye = get_eye();

  QMatrix4x4 view_projection;
  const float aspect = float(width()) / std::max(height(), 1);
  view_projection.perspective(50.0f, aspect, distance * 0.01f, std::max(distance, scene_size) * 200.0f);
  view_projection.lookAt(eye, target, QVector3D(0, 0, 1));

  program->bind();
  program->setUniformValue("base_texture", 0);
  program->setUniformValue("lightmap_texture", 1);
  program->setUniformValue("eye", eye);

  std::vector<const DrawItem*> blended;
  for(std::vector<DrawItem>::const_iterator i = items.begin(); i != items.end(); ++i)
    {
      if (i->material.is_blended())
        blended.push_back(&*i);
      else
        draw_item(*i, view_projection);
    }

  // Good enough for the mostly separate glass and fog objects
  std::sort(blended.begin(), blended.end(),
            [&eye](const DrawItem* lhs, const DrawItem* rhs)
            {
              return ((lhs->bbox_min + lhs->bbox_max) * 0.5f - eye).lengthSquared() >
                     ((rhs->bbox_min + rhs->bbox_max) * 0.5f - eye).lengthSquared();
            });
  for(std::vector<const DrawItem*>::const_iterator i = blended.begin(); i != blended.end(); ++i)
    draw_item(**i, view_projection);

  glBindVertexArray(0);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  program->release();

  if (is_loading())
    update();
}

void
ModelCanvas::mousePressEvent(QMouseEvent* event)
{
  last_mouse = event->position().toPoint();
}

void
ModelCanvas::mouseMoveEvent(QMouseEvent* event)
{
  const QPoint pos   = event->position().toPoint();
  const QPoint delta = pos - last_mouse;
  last_mouse = pos;

  if (event->buttons() & Qt::LeftButton)
    {
      yaw   -= delta.x() * 0.008f;
      pitch += delta.y() * 0.008f;
      pitch = std::max(-1.55f, std::min(1.55f, pitch));
      update();
    }
  else if (event->buttons() & (Qt::MiddleButton | Qt::RightButton))
    {
      const QVector3D forward = (target - get_eye()).normalized();
      const QVector3D right   = QVector3D::crossProduct(forward, QVector3D(0, 0, 1)).normalized();
      const QVector3D up      = QVector3D::crossProduct(right, forward);
      const float scale = distance * 0.002f;
      target += (up * float(delta.y()) - right * float(delta.x())) * scale;
      update();
    }
}

void
ModelCanvas::wheelEvent(QWheelEvent* event)
{
  distance *= std::pow(0.999f, float(event->angleDelta().y()));
  update();
}

void
ModelCanvas::keyPressEvent(QKeyEvent* event)
{
  switch(event->key())
    {
    case Qt::Key_W: case Qt::Key_A: case Qt::Key_S: case Qt::Key_D: case Qt::Key_Q: case Qt::Key_E:
      if (!event->isAutoRepeat())
        {
          keys.insert(event->key());
          if (!fly_timer->isActive())
            {
              fly_clock.start();
              fly_timer->start();
            }
        }
      break;

    default:
      QOpenGLWidget::keyPressEvent(event);
      break;
    }
}

void
ModelCanvas::keyReleaseEvent(QKeyEvent* event)
{
  if (!event->isAutoRepeat())
    keys.erase(event->key());
  if (keys.empty())
    fly_timer->stop();
}

void
ModelCanvas::focusOutEvent(QFocusEvent*)
{
  keys.clear();
  fly_timer->stop();
}

void
ModelCanvas::fly()
{
  const float delta = fly_clock.restart() / 1000.0f;

  const QVector3D forward = (target - get_eye()).normalized();
  const QVector3D right   = QVector3D::crossProduct(forward, QVector3D(0, 0, 1)).normalized();

  QVector3D direction;
  if (keys.count(Qt::Key_W)) direction += forward;
  if (keys.count(Qt::Key_S)) direction -= forward;
  if (keys.count(Qt::Key_D)) direction += right;
  if (keys.count(Qt::Key_A)) direction -= right;
  if (keys.count(Qt::Key_E)) direction += QVector3D(0, 0, 1);
  if (keys.count(Qt::Key_Q)) direction -= QVector3D(0, 0, 1);

  target += direction * distance * delta;
  update();
}

namespace {

enum { ROLE_KIND = Qt::UserRole, ROLE_SMR, ROLE_MESH };
enum { ITEM_LOCATION, ITEM_SMR, ITEM_MESH, ITEM_NONE };

} // namespace

ModelView::ModelView(QWidget* parent, QMainWindow* mainwindow)
  : QWidget(parent),
    library(data_manager)
{
  toolbar = new QToolBar;
  mainwindow->addToolBar(toolbar);

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  QSplitter* splitter = new QSplitter;
  layout->addWidget(splitter);

  tree = new QTreeWidget;
  tree->setHeaderHidden(true);
  splitter->addWidget(tree);

  QWidget* right = new QWidget;
  QVBoxLayout* vbox = new QVBoxLayout(right);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);
  canvas = new ModelCanvas(right);
  vbox->addWidget(canvas, 1);
  info = new QLabel;
  info->setToolTip("Left mouse: rotate, right mouse: move, wheel: zoom, W/A/S/D/Q/E: fly");
  vbox->addWidget(info);
  splitter->addWidget(right);

  splitter->setStretchFactor(1, 1);
  splitter->setSizes(QList<int>() << 200 << 600);

  const auto add_toggle = [this](const QString& text, bool state, std::function<void (bool)> callback)
    {
      QAction* action = toolbar->addAction(text);
      action->setCheckable(true);
      action->setChecked(state);
      connect(action, &QAction::toggled, this, callback);
    };

  add_toggle("Textures",  true,  [this](bool t){ canvas->set_show_textures(t); });
  add_toggle("Lighting",  true,  [this](bool t){ canvas->set_show_lighting(t); });
  add_toggle("Wireframe", false, [this](bool t){ canvas->set_show_wireframe(t); });
  add_toggle("Culling",   true,  [this](bool t){ canvas->set_cull(t); });

  connect(toolbar->addAction("Reset Camera"), &QAction::triggered, this, [this]{ canvas->reset_camera(); });

  toolbar->addSeparator();

  play_action = toolbar->addAction("Play");
  play_action->setCheckable(true);
  play_action->setChecked(true);
  connect(play_action, &QAction::toggled, this, [this](bool t){ canvas->set_playing(t); });

  animation_box = new QComboBox;
  animation_box->setMinimumContentsLength(32);
  animation_box->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  animation_widget = toolbar->addWidget(animation_box);
  animation_widget->setVisible(false);
  connect(animation_box, &QComboBox::currentIndexChanged, this, [this](int index){ on_animation_selected(index); });

  toolbar->addSeparator();

  source_action = toolbar->addAction("Source");
  connect(source_action, &QAction::triggered, this, [this]{ if (show_source) show_source(); });

  connect(tree, &QTreeWidget::currentItemChanged, this,
          [this](QTreeWidgetItem* item, QTreeWidgetItem*){ if (item) on_item_selected(item); });
}

void
ModelView::set_show_source(std::function<void ()> callback)
{
  show_source = callback;
}

void
ModelView::show_toolbar(bool t)
{
  toolbar->setVisible(t);
}

namespace {

/** Finds the first mesh with a skeleton */
const ModelMesh* find_skinned_mesh(const ModelNode& node)
{
  if (node.mesh && !node.mesh->bone_parents.empty())
    return node.mesh.get();

  for(std::vector<ModelNode>::const_iterator i = node.children.begin(); i != node.children.end(); ++i)
    {
      const ModelMesh* mesh = find_skinned_mesh(*i);
      if (mesh)
        return mesh;
    }
  return nullptr;
}

} // namespace

void
ModelView::on_animation_selected(int index)
{
  if (index < 0)
    return;

  try {
    const std::string filename = animation_box->itemData(index).toString().toStdString();
    canvas->set_animation(filename.empty() ? nullptr : library.load_animation(filename, pak));
  } catch(std::exception& err) {
    canvas->set_animation(nullptr);
    info->setText(QString::fromStdString(err.what()));
  }
}

void
ModelView::show_scene(const ModelScene& scene, const std::string& smr)
{
  canvas->set_scene(scene);

  // Characters have their animations in a directory below them, a
  // location would just be slowed down by looking for those
  std::vector<std::string> files;
  const ModelMesh* mesh = smr.empty() ? nullptr : find_skinned_mesh(scene.root);
  if (mesh)
    files = library.find_animations(smr, int(mesh->bones.size()), pak);

  {
    const QSignalBlocker blocker(animation_box);
    animation_box->clear();
    animation_box->addItem(canvas->has_own_animation() ? "Default" : "Bind Pose", QString());

    // The plain idle if there is one, the longer names are special
    // cases and those with a "_to_" are transitions
    int selected = 0;
    int selected_length = 0;
    for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i)
      {
        const QString file = QString::fromStdString(*i);
        const QString name = file.section('/', -1);
        animation_box->addItem(name, file);

        const int length = name.contains("normal_idle") ? 0 : name.size();
        if (name.contains("idle") && !name.contains("_to_") && (selected == 0 || length < selected_length))
          {
            selected = animation_box->count() - 1;
            selected_length = length;
          }
      }
    animation_box->setCurrentIndex(selected);
    animation_widget->setVisible(!files.empty());
  }
  on_animation_selected(animation_box->currentIndex());

  play_action->setVisible(canvas->is_animated() || !files.empty());

  std::ostringstream str;
  str << scene.root.name;
  if (!scene.warnings.empty())
    {
      str << " (" << scene.warnings.size() << " objects failed to load)";
      for(std::vector<std::string>::const_iterator i = scene.warnings.begin(); i != scene.warnings.end(); ++i)
        std::cout << "ModelView: " << *i << std::endl;
    }
  info->setText(QString::fromStdString(str.str()));
}

void
ModelView::set_bundle(const std::string& name, const std::string& pak_)
{
  bundle_name = name;
  pak = pak_;

  const std::shared_ptr<ModelBundle> bundle = library.get_bundle(name);
  if (!bundle)
    throw std::runtime_error("Couldn't find bundle " + name);

  const QSignalBlocker blocker(tree);
  tree->clear();

  QTreeWidgetItem* location = new QTreeWidgetItem(tree, QStringList(QString::fromStdString(name)));
  location->setData(0, ROLE_KIND, ITEM_LOCATION);

  const std::vector<ModelBundle::SmrEntry>& smrs = bundle->get_smrs();
  for(std::vector<ModelBundle::SmrEntry>::const_iterator i = smrs.begin(); i != smrs.end(); ++i)
    {
      QTreeWidgetItem* smr = new QTreeWidgetItem(tree, QStringList(QString::fromStdString(i->name)));
      smr->setData(0, ROLE_KIND, ITEM_SMR);
      smr->setData(0, ROLE_SMR, QString::fromStdString(i->name));
      for(std::vector<ModelBundle::MeshEntry>::const_iterator j = i->meshes.begin(); j != i->meshes.end(); ++j)
        {
          QTreeWidgetItem* mesh = new QTreeWidgetItem(smr, QStringList(QString::fromStdString(j->name)));
          mesh->setData(0, ROLE_KIND, ITEM_MESH);
          mesh->setData(0, ROLE_SMR, QString::fromStdString(i->name));
          mesh->setData(0, ROLE_MESH, QString::fromStdString(j->name));
        }
    }

  source_action->setVisible(false);
  tree->setCurrentItem(location);
  on_item_selected(location);
}

void
ModelView::on_item_selected(QTreeWidgetItem* item)
{
  try {
    const std::string smr  = item->data(0, ROLE_SMR).toString().toStdString();
    const std::string mesh = item->data(0, ROLE_MESH).toString().toStdString();

    switch(item->data(0, ROLE_KIND).toInt())
      {
      case ITEM_LOCATION:
        show_scene(library.load_location(bundle_name), "");
        break;

      case ITEM_SMR:
        {
          ModelScene scene;
          try {
            scene = library.load_sir(smr.substr(0, smr.rfind('.')) + ".sir", pak);
          } catch(std::exception&) {
            // Models without a .sir of their own are shown at their origin
            const std::shared_ptr<ModelBundle> bundle = library.get_bundle(bundle_name);
            const ModelBundle::SmrEntry* entry = bundle ? bundle->find_smr(smr) : nullptr;
            scene.pak = pak;
            scene.root.name = smr;
            if (entry)
              for(std::vector<ModelBundle::MeshEntry>::const_iterator i = entry->meshes.begin();
                  i != entry->meshes.end(); ++i)
                {
                  ModelNode node;
                  node.mesh = library.load_mesh(bundle_name, smr, i->name);
                  scene.root.children.push_back(std::move(node));
                }
          }
          show_scene(scene, smr);
        }
        break;

      case ITEM_MESH:
        {
          ModelScene scene;
          scene.pak = pak;
          scene.root.name = smr + " / " + mesh;
          scene.root.mesh = library.load_mesh(bundle_name, smr, mesh);
          if (scene.root.mesh)
            show_scene(scene, smr);
          else
            {
              canvas->clear();
              info->setText(QString::fromStdString(mesh + " has no visible geometry"));
            }
        }
        break;

      default:
        break;
      }
  } catch(std::exception& err) {
    canvas->clear();
    info->setText(QString::fromStdString(err.what()));
  }
}

void
ModelView::add_scene_nodes(QTreeWidgetItem* parent, const ModelNode& node)
{
  for(std::vector<ModelNode>::const_iterator i = node.children.begin(); i != node.children.end(); ++i)
    {
      QTreeWidgetItem* item = parent
        ? new QTreeWidgetItem(parent, QStringList(QString::fromStdString(i->name)))
        : new QTreeWidgetItem(tree, QStringList(QString::fromStdString(i->name)));
      item->setData(0, ROLE_KIND, ITEM_NONE);
      add_scene_nodes(item, *i);
    }
}

bool
ModelView::set_sir(const std::string& filename, const std::string& pak_)
{
  const ModelScene scene = library.load_sir(filename, pak_);
  if (scene.root.children.empty())
    return false;

  pak = pak_;

  const QSignalBlocker blocker(tree);
  tree->clear();
  add_scene_nodes(nullptr, scene.root);
  tree->expandToDepth(0);

  source_action->setVisible(true);
  show_scene(scene, filename.substr(0, filename.rfind('.')) + ".smr");
  return true;
}

/* EOF */
