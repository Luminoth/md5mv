#if !defined __D3MAP_H__
#define __D3MAP_H__

#include "AABB.h"
#include "Map.h"
#include "Matrix3.h"
#include "Mesh.h"
#include "Plane.h"
#include "Renderable.h"
#include "TextureManager.h"

class Camera;

class D3Map : public Map
{
private:
    struct Surface
    {
        std::string material;

        int vertex_count;
        boost::shared_array<Vertex> vertices;

        int triangle_count;
        boost::shared_array<Triangle> triangles;

        GLuint textures[TextureManager::TextureCount];

        GLuint vbo[Renderable::VBOCount];
        RenderableBuffers buffers;

        AABB bounds;

        Surface();
        virtual ~Surface() throw();

        void init();
        bool load_textures();
    };
    typedef boost::shared_array<boost::shared_ptr<Surface> > Surfaces;

    struct Model
    {
        std::string name;

        int surface_count;
        Surfaces surfaces;
        AABB bounds;

        bool is_area() const { return 0 == name.compare(0, 5, "_area"); }
    };
    typedef std::vector<boost::shared_ptr<Model> > Models;

    struct Brush
    {
        Plane plane;
        Matrix3 texture_matrix;
        std::string material;
    };
    typedef std::vector<boost::shared_ptr<Brush> > Brushes;

    struct Patch
    {
        std::string material;
    };
    typedef std::vector<boost::shared_ptr<Patch> > Patches;

    struct Patch2 : public Patch
    {
        Vector2 offset, scale;
        float rotation;
    };

    struct Patch3 : public Patch
    {
        Vector3 offset, scale;
        float rotation;
    };

    struct Entity
    {
        boost::unordered_map<std::string, std::string> properties;
        Brushes brushes;
        Patches patches;
    };
    typedef boost::unordered_map<std::string, boost::shared_ptr<Entity> > Entities;

private:
    static Logger& logger;

public:
    explicit D3Map(const std::string& name);
    virtual ~D3Map() throw();

public:
    virtual Position player_spawn_position() const;
    virtual float player_spawn_angle() const;

    virtual bool load(const boost::filesystem::path& path);

    virtual void render(const Camera& camera, Shader& shader) const;
    virtual void render(const Camera& camera, Shader& shader, const Light& light) const;
    virtual void render_normals(const Camera& camera) const;

private:
    void render_area(const Camera& camera, const Model& area, Shader& shader) const;
    void render_area_normals(const Camera& camera, const Model& area) const;
    void render_surface(const Surface& surface, Shader& shader) const;
    void render_surface_normals(const Surface& surface) const;

private:
    bool load_map(const boost::filesystem::path& path);
    bool scan_map_version(Lexer& lexer);
    bool scan_map_entity(Lexer& lexer);
    bool scan_map_brushes(Lexer& lexer, boost::shared_ptr<Entity> entity);
    bool scan_map_brushdef3(Lexer& lexer, boost::shared_ptr<Entity> entity);
    bool scan_map_brush(Lexer& lexer, boost::shared_ptr<Entity> entity);
    bool scan_map_patchdef2(Lexer& lexer, boost::shared_ptr<Entity> entity);
    bool scan_map_patchdef3(Lexer& lexer, boost::shared_ptr<Entity> entity);
    bool scan_patch_control_grid(Lexer& lexer, boost::shared_ptr<Patch> patch);
    bool scan_patch_control_grid_row(Lexer& lexer, boost::shared_ptr<Patch> patch);
    bool scan_patch_control_point(Lexer& lexer, boost::shared_ptr<Patch> patch);

    bool load_proc(const boost::filesystem::path& path);
    bool scan_proc_header(Lexer& lexer);
    bool scan_proc_model(Lexer& lexer);
    bool scan_proc_surface(Lexer& lexer, boost::shared_ptr<Surface> surface);
    bool scan_proc_vertex(Lexer& lexer, boost::shared_ptr<Surface> surface, int index);
    bool scan_proc_portals(Lexer& lexer);
    bool scan_proc_portal(Lexer& lexer);
    bool scan_proc_nodes(Lexer& lexer);
    bool scan_proc_node(Lexer& lexer);
    bool scan_proc_shadow_model(Lexer& lexer);
    bool scan_proc_shadow_vertex(Lexer& lexer);

private:
    virtual void on_unload();

private:
    int _version;

    int _acount;
    Models _models;

    Entities _entities;
    boost::shared_ptr<Entity> _worldspawn;

private:
    D3Map();
    DISALLOW_COPY_AND_ASSIGN(D3Map);
};

#endif
