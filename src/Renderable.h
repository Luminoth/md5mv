#if !defined __RENDERABLE_H__
#define __RENDERABLE_H__

#include "Material.h"
#include "Mesh.h"
#include "Physical.h"

class Camera;
class Light;
class DirectionalLight;
class PositionalLight;
class Model;
class Shader;
class Skeleton;

class RenderableBuffers
{
public:
    RenderableBuffers();

    explicit RenderableBuffers(size_t vertex_count);
    RenderableBuffers(const Vertex* const vertices, size_t vertex_count);

    RenderableBuffers(size_t triangle_count, size_t vertex_count);
    RenderableBuffers(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count);

    virtual ~RenderableBuffers() throw();

public:
    size_t vertex_count() const { return _vcount; }

    size_t vertex_buffer_size() const { return _vbsize; }
    boost::shared_array<float> vertex_buffer() const { return _vertex_buffer; }

    size_t normal_buffer_size() const { return _nbsize; }
    boost::shared_array<float> normal_buffer() const { return _normal_buffer; }

    size_t tangent_buffer_size() const { return _tnbsize; }
    boost::shared_array<float> tangent_buffer() const { return _tangent_buffer; }

    size_t texture_buffer_size() const { return _tbsize; }
    boost::shared_array<float> texture_buffer() const { return _texture_buffer; }

    // for debugging
    boost::shared_array<float> normal_line_buffer() const { return _normal_line_buffer; }
    boost::shared_array<float> tangent_line_buffer() const { return _tangent_line_buffer; }

    void allocate_buffers(size_t vertex_count);
    void allocate_buffers(size_t triangle_count, size_t vertex_count);

    void copy_vertices(const Vertex* const vertices, size_t vertex_count, size_t start=0, bool allocate=false);
    void copy_triangles(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count, size_t start=0, bool allocate=false);

private:
    size_t _vcount;

    size_t _vbsize;
    boost::shared_array<float> _vertex_buffer;

    size_t _nbsize;
    boost::shared_array<float> _normal_buffer;

    size_t _tnbsize;
    boost::shared_array<float> _tangent_buffer;

    size_t _tbsize;
    boost::shared_array<float> _texture_buffer;

    boost::shared_array<float> _normal_line_buffer;
    boost::shared_array<float> _tangent_line_buffer;

private:
    DISALLOW_COPY_AND_ASSIGN(RenderableBuffers);
};

class Renderable : public Physical
{
public:
    enum RenderableShadowVBO
    {
        ShadowVertexArray,
        ShadowVBOCount
    };

    enum RenderableVBO
    {
        VertexArray,
        NormalArray,
        NormalLineArray,
        TangentArray,
        TangentLineArray,
        TextureArray,
        VBOCount
    };

private:
    static uint32_t pick_ids;

private:
    static uint32_t next_pick_id();

public:
    explicit Renderable(const std::string& name);
    virtual ~Renderable() throw();

public:
    // must call this every time a Renderable is instantiated
    // so that overloaded features can be setup correctly
    void init();

    const std::string& name() const { return _name; }

    void model(boost::shared_ptr<Model> model);
    const Model& model() const { return *_model; }
    bool has_model() const { return static_cast<bool>(_model); }

    bool load_material();
    const Material& material() const { return _material; }

    const Vertex& vertex(size_t idx) const { return _vertices[idx]; }

    // returns the number of vertices in the silhouette
    size_t compute_silhouette(const Light& light);

    void render(Shader& shader) const;
    void render(Shader& shader, const Light& light, const Camera& camera) const;
    void render_shadow(Shader& shader, const Light& light, const Camera& camera, size_t vcount, bool cap) const;
    void render_unlit(const Camera& camera);

    // NOTE: these are only meaningful when is_pickable() is true
    uint32_t pick_id() const { return _pick_id; }
    const Color& pick_color() const { return _pick_color; }

public:
    virtual bool is_pickable() const { return false; }
    virtual bool is_transparent() const = 0;
    virtual bool is_static() const = 0;
    virtual bool has_shadow() const { return false; }

    virtual void animate() {}

protected:
    void calculate_vertices(const Skeleton& skeleton);

    virtual void on_render_unlit(const Camera& camera) const {}

private:
    GLuint vbo(RenderableVBO idx) const { return _vbo[idx]; }
    GLuint shadow_vbo(RenderableShadowVBO idx) const { return _shadow_vbo[idx]; }

    void render_mesh(const Mesh& mesh, size_t start, Shader& shader) const;
    void render_shadow_directional(Shader& shader, const DirectionalLight& light, size_t vcount) const;
    void render_shadow_positional(Shader& shader, const PositionalLight& light, size_t vcount, bool cap) const;
    void render_normals() const;
    void render_normals(const Mesh& mesh, size_t start) const;

    bool is_silhouette_edge(const Mesh& mesh, const Edge& edge, const Vector4& light_position, size_t vstart, bool& faces_light1) const;
    size_t compute_silhouette_directional(const Direction& light_direction, boost::shared_array<float> varray);
    size_t compute_silhouette_positional(const Position& light_position, boost::shared_array<float> varray);

private:
    std::string _name;

    boost::shared_ptr<Model> _model;
    Material _material;

    boost::shared_array<Vertex> _vertices;
    RenderableBuffers _buffers;

    GLuint _vbo[VBOCount];
    GLuint _shadow_vbo[ShadowVBOCount];

    uint32_t _pick_id;
    Color _pick_color;

private:
    Renderable();
    DISALLOW_COPY_AND_ASSIGN(Renderable);
};

struct CompareRenderablesOpaque
{
    CompareRenderablesOpaque(const Position& camera) : _camera(camera) {}
    virtual ~CompareRenderablesOpaque() throw() {}

    bool operator()(const boost::shared_ptr<const Renderable> lhs, const boost::shared_ptr<const Renderable> rhs) const
    {
        // closer objects get rendered first
        // to take advantage of early-out depth testing
        return lhs->absolute_bounds().distance(_camera) < rhs->absolute_bounds().distance(_camera);
    }

private:
    Position _camera;

private:
    CompareRenderablesOpaque();
};

struct CompareRenderablesTransparent
{
    CompareRenderablesTransparent(const Position& camera) : _camera(camera) {}
    virtual ~CompareRenderablesTransparent() throw() {}

    bool operator()(const boost::shared_ptr<const Renderable> lhs, const boost::shared_ptr<const Renderable> rhs) const
    {
        // further objects get rendered first
        return lhs->absolute_bounds().distance(_camera) > rhs->absolute_bounds().distance(_camera);
    }

private:
    Position _camera;

private:
    CompareRenderablesTransparent();
};

#endif
