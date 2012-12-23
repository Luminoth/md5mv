#if !defined __MESH_H__
#define __MESH_H__

#include "AABB.h"
#include "Geometry.h"
#include "TextureManager.h"

class RenderableBuffers;
class Skeleton;

class Mesh
{
private:
    static Logger& logger;

public:
    Mesh(int vcount, boost::shared_array<Vertex> vertices, int tcount, boost::shared_array<Triangle> triangles, int wcount, boost::shared_array<Weight> weights);
    virtual ~Mesh() throw();

public:
    int vertex_count() const { return _vcount; }
    const Vertex& vertex(size_t idx) const { return _vertices[idx]; }

    int triangle_count() const { return _tcount; }
    const Triangle& triangle(size_t idx) const { return _triangles[idx]; }

    bool has_weights() const { return static_cast<bool>(_weights); }

    size_t edge_count() const { return _edges.size(); }
    const Edge& edge(size_t idx) const { return _edges[idx]; }

    GLuint texture(TextureManager::TextureType idx) const { return _textures[idx]; }

    // pose-position bounds
    const AABB& bounds() const { return _bounds; }

public:
    bool load_textures(const boost::filesystem::path& path, const std::string& name);

    // NOTE: this must be called before doing any calculations
    // that require the vertices to be positioned correctly
    void pose(const Skeleton& skeleton);

    void compute_normals(const Skeleton& skeleton, bool smooth=false);
    void weld_vertices();
    void compute_edges();

    // puts the vertices for this mesh into the given buffers
    // vstart is the vertex-based index into vertices
    // tstart is the triangle-based buffer index
    void calculate_vertices(const Skeleton& skeleton, boost::shared_array<Vertex> vertices, size_t vstart, RenderableBuffers& buffers, size_t tstart) const;

private:
    void position_vertices(const Skeleton& skeleton, boost::shared_array<Vertex> vertices, size_t vstart) const;
    void weld_vertices(const boost::unordered_map<int, int>& vertices);
    void fix_triangles(int old_index, int new_index);
    void calculate_edge(const Triangle& triangle, int t);
    void find_matching_edges();
    void find_matching_edge(int v1, int v2, int t);

private:
    int _vcount;
    boost::shared_array<Vertex> _vertices;

    int _tcount;
    boost::shared_array<Triangle> _triangles;

    int _wcount;
    boost::shared_array<Weight> _weights;

    std::vector<Edge> _edges;

    GLuint _textures[TextureManager::TextureCount];

    // pose-position bounds
    AABB _bounds;

private:
    Mesh();
    DISALLOW_COPY_AND_ASSIGN(Mesh);
};

#endif
