#if !defined __Q3BSP_H__
#define __Q3BSP_H__

#include "Map.h"

class Camera;

class Q3BSP : public Map
{
private:
    enum
    {
        VertexArray,
        TextureArray,
        VBOCount
    };

    enum
    {
        DirEntryEntities,
        DirEntryTextures,
        DirEntryPlanes,
        DirEntryNodes,
        DirEntryLeaves,
        DirEntryLeafFaces,
        DirEntryLeafBrushes,
        DirEntryModels,
        DirEntryBrushes,
        DirEntryBrushSides,
        DirEntryVertices,
        DirEntryMeshVerts,
        DirEntryEffects,
        DirEntryFaces,
        DirEntryLightMaps,
        DirEntryLightVols,
        DirEntryVisData,

        DirEntryCount
    };

    struct DirEntry
    {
        int offset; // lump offset
        int length; // lump length (always multiple of 4)
    };

    struct Header
    {
        char magic[4];
        int version;
        DirEntry direntries[DirEntryCount];
    };

    struct Entities
    {
        // entity descriptions
        std::string ents;
    };

    // rendering
    struct Texture
    {
        char name[64];  // texture name
        int flags;      // surface flags
        int contents;   // content flags
    };
    typedef boost::shared_array<Texture> Textures;

    // rendering
    struct Plane
    {
        float normal[3];    // plane normal
        float distance;     // distance from origin along normal
    };
    typedef boost::shared_array<Plane> Planes;

    // rendering
    struct Node
    {
        int plane;          // plane index
        int children[2];    // negative values are leaf indices
        int mins[3];        // bounding box min
        int maxs[3];        // bounding box max
    };
    typedef boost::shared_array<Node> Nodes;

    // rendering
    struct Leaf
    {
        int cluster;        // visdata cluster index
        int area;           // areaportal area
        int mins[3];        // bounding box min
        int maxs[3];        // bounding box max
        int leafface;       // first leafface
        int n_leaffaces;    // number of leaffaces
        int leafbrush;      // first leafbrush
        int n_leafbrushes;  // number of leafbrushes
    };
    typedef boost::shared_array<Leaf> Leaves;

    // rendering
    struct LeafFace
    {
        int face;   // face index
    };
    typedef boost::shared_array<LeafFace> LeafFaces;

    // collision
    struct LeafBrush
    {
        int brush;  // brush index
    };
    typedef boost::shared_array<LeafBrush> LeafBrushes;

    // rendering
    struct Model
    {
        float mins[3];  // bounding box min
        float maxs[3];  // bounding box max
        int face;       // first face
        int n_faces;    // number of faces
        int brush;      // first brush
        int n_brushes;  // number of brushes
    };
    typedef boost::shared_array<Model> Models;

    // collision
    struct Brush
    {
        int brushside;      // first brushside
        int n_brushsides;   // number of brushsides
        int texture;        // texture index
    };
    typedef boost::shared_array<Brush> Brushes;

    // collision
    struct BrushSide
    {
        int plane;      // plane index
        int texture;    // texture index
    };
    typedef boost::shared_array<BrushSide> BrushSides;

    // rendering
    struct BSPVertex
    {
        float position[3];      // vertex position
        float texcoord[2][2];   // texture coordinates (0=surface, 1=lightmap)
        float normal[3];        // vertex normal
        unsigned char color[4]; // vertex color (RGBA)
    };
    typedef boost::shared_array<BSPVertex> BSPVertices;

    // rendering
    struct MeshVert
    {
        int offset; // vertex index offset, relative to the first vertex of the corresponding face
    };
    typedef boost::shared_array<MeshVert> MeshVerts;

    struct Effect
    {
        char name[64];  // effect shader
        int brush;      // index of the brush that generates the effect
        int unknown;    // usually 5, sometimes -1
    };
    typedef boost::shared_array<Effect> Effects;

    // rendering
    struct Face
    {
        int texture;            // texture index
        int effect;             // effect index (or -1)
        int type;               // face type (1=polygon, 2=patch, 3=mesh, 4=billboard)
        int vertex;             // first vertex
        int n_vertices;         // number of vertices
        int meshvert;           // first meshvert
        int n_meshverts;        // number of meshverts
        int lm_index;           // lightmap index
        int lm_start[2];        // corner of this face's lightmap image in lightmap
        int lm_size[2];         // size of this face's lightmap image in lightmap
        float lm_origin[3];     // world-space origin of lightmap
        float lm_vecs[2][3];    // world space lightmap s and t unit vectors
        float normal[3];        // surface normal
        int size[2];            // patch dimensions
    };
    typedef boost::shared_array<Face> Faces;

    // rendering
    struct LightMap
    {
        unsigned char map[128][128][3]; // color data (RGB)
    };
    typedef boost::shared_array<LightMap> LightMaps;

    struct LightVol
    {
        unsigned char ambient[3];       // ambient color (RGB)
        unsigned char directional[3];   // directional color (RGB)
        unsigned char direction[2];     // direction to light (0=phi, 1=theta)
    };
    typedef boost::shared_array<LightVol> LightVols;

    // rendering
    struct VisData
    {
        int n_vecs;             // number of vectors
        int sz_vecs;            // size of each vector, in bytes

        // visibility data, one bit per-cluster per-vector
        boost::shared_array<unsigned char> vecs;
    };

private:
    static Logger& logger;

public:
    explicit Q3BSP(const std::string& name);
    virtual ~Q3BSP() throw();

public:
    virtual Position player_spawn_position() const { return Position(); }
    virtual float player_spawn_angle() const { return 0.0f; }

    virtual bool load(const boost::filesystem::path& path);

    virtual void render(const Camera& camera, Shader& shader) const;
    virtual void render(const Camera& camera, Shader& shader, const Light& light) const;
    virtual void render_normals(const Camera& camera) const;

private:
    void visible_faces(const Camera& camera, std::vector<int>& faces) const;

    // find the leaf that contains pos
    int find_leaf(const Position& pos) const;

    // tests to see if one cluster is visible from another
    bool cluster_visible(int from, int cluster) const;

    void render_face(const Face& face, Shader& shader) const;

private:
    bool read_entities(std::ifstream& f);
    bool read_textures(std::ifstream& f);
    bool read_planes(std::ifstream& f);
    bool read_nodes(std::ifstream& f);
    bool read_leaves(std::ifstream& f);
    bool read_leaf_faces(std::ifstream& f);
    bool read_leaf_brushes(std::ifstream& f);
    bool read_models(std::ifstream& f);
    bool read_brushes(std::ifstream& f);
    bool read_brush_sides(std::ifstream& f);
    bool read_vertices(std::ifstream& f);
    bool read_mesh_verts(std::ifstream& f);
    bool read_effects(std::ifstream& f);
    bool read_faces(std::ifstream& f);
    bool read_light_maps(std::ifstream& f);
    bool read_light_vols(std::ifstream& f);
    bool read_vis_data(std::ifstream& f);

private:
    virtual void on_unload();

private:
    GLuint _vbo[VBOCount];

    Header _header;
    Entities _entities;
    Textures _textures;
    Planes _planes;
    Nodes _nodes;

    size_t _leaf_count;
    Leaves _leaves;

    LeafFaces _leaf_faces;
    LeafBrushes _leaf_brushes;
    Models _models;
    Brushes _brushes;
    BrushSides _brush_sides;
    BSPVertices _vertices;
    MeshVerts _mesh_verts;
    Effects _effects;
    Faces _faces;
    LightMaps _light_maps;
    LightVols _light_vols;
    VisData _vis_data;

private:
    Q3BSP();
    DISALLOW_COPY_AND_ASSIGN(Q3BSP);
};

#endif
