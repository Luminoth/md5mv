#include "pch.h"
#include <fstream>
#include <iostream>
#include "common.h"
#include "Camera.h"
#include "Renderer.h"
#include "Shader.h"
#include "TextureManager.h"
#include "Q3BSP.h"

// http://www.mralligator.com/q3/
// http://graphics.cs.brown.edu/games/quake/quake3.html

// TODO: vertices and what not need to be swizzled

Logger& Q3BSP::logger(Logger::instance("md5mv.Q3BSP"));

Q3BSP::Q3BSP(const std::string& name)
    : Map(name), _leaf_count(0)
{
    ZeroMemory(_vbo, sizeof(GLuint) * VBOCount);
    ZeroMemory(&_header, sizeof(Header));

    _vis_data.n_vecs = 0;
    _vis_data.sz_vecs = 0;
}

Q3BSP::~Q3BSP() throw()
{
    unload();
}

bool Q3BSP::load(const boost::filesystem::path& path)
{
    unload();

    boost::filesystem::path filename(map_dir() / path / (name() + ".bsp"));
    LOG_INFO("Loading q3bsp from '" << filename << "'" << std::endl);

    std::ifstream f(filename.string().c_str(), std::ios::binary);
    if(!f) {
        return false;
    }

    f.read(reinterpret_cast<char*>(&_header), sizeof(Header));
    if(strncmp(_header.magic, "IBSP", 4)) {
        LOG_ERROR("Invalid magic!" << std::endl);
        return false;
    }

    if(0x2e != _header.version) {
        LOG_ERROR("Invalid version!" << std::endl);
        return false;
    }

    if(!read_entities(f)) {
        return false;
    }

    if(!read_textures(f)) {
        return false;
    }

    if(!read_planes(f)) {
        return false;
    }

    if(!read_nodes(f)) {
        return false;
    }

    if(!read_leaves(f)) {
        return false;
    }

    if(!read_leaf_faces(f)) {
        return false;
    }

    if(!read_leaf_brushes(f)) {
        return false;
    }

    if(!read_models(f)) {
        return false;
    }

    if(!read_brushes(f)) {
        return false;
    }

    if(!read_brush_sides(f)) {
        return false;
    }

    if(!read_vertices(f)) {
        return false;
    }

    if(!read_mesh_verts(f)) {
        return false;
    }

    if(!read_effects(f)) {
        return false;
    }

    if(!read_faces(f)) {
        return false;
    }

    if(!read_light_maps(f)) {
        return false;
    }

    if(!read_light_vols(f)) {
        return false;
    }

    if(!read_vis_data(f)) {
        return false;
    }

    glGenBuffers(VBOCount, _vbo);

    return true;
}

void Q3BSP::on_unload()
{
    glDeleteBuffers(VBOCount, _vbo);

    ZeroMemory(&_header, sizeof(Header));

    _entities.ents.erase();

    _textures.reset();
    _planes.reset();
    _nodes.reset();

    _leaf_count = 0;
    _leaves.reset();

    _leaf_faces.reset();
    _models.reset();
    _brushes.reset();
    _brush_sides.reset();
    _vertices.reset();
    _mesh_verts.reset();
    _effects.reset();
    _faces.reset();
    _light_maps.reset();
    _light_vols.reset();

    _vis_data.n_vecs = 0;
    _vis_data.sz_vecs = 0;
    _vis_data.vecs.reset();
}

void Q3BSP::render(const Camera& camera, Shader& shader) const
{
    std::vector<int> faces;
    visible_faces(camera, faces);

    Renderer::instance().init_shader_matrices(shader);

    BOOST_FOREACH(int face, faces) {
        //render_face(_faces[face], shader);
    }
}

void Q3BSP::render(const Camera& camera, Shader& shader, const Light& light) const
{
    std::vector<int> faces;
    visible_faces(camera, faces);

    Renderer::instance().init_shader_matrices(shader);
    Renderer::instance().init_shader_light(shader, material(), light, camera);

    BOOST_FOREACH(int face, faces) {
        //render_face(_faces[face], shader);
    }
}

void Q3BSP::render_normals(const Camera& camera) const
{
// TODO: write meh
}

void Q3BSP::visible_faces(const Camera& camera, std::vector<int>& faces) const
{
    int leaf = find_leaf(camera.position());
    for(size_t i=0; i<_leaf_count; ++i) {
        const Leaf& tl(_leaves[i]);
        const AABB bounds(Point3(tl.mins), Point3(tl.maxs));
        if(!cluster_visible(leaf, tl.cluster) || !camera.visible(bounds)) {
            continue;
        }

        for(int j=0; j<tl.n_leaffaces; ++j) {
// TODO: can we avoid this find check by using a std::set?
            const int f = _leaf_faces[tl.leafface + j].face;
            if(faces.end() == std::find(faces.begin(), faces.end(), f)) {
                faces.push_back(f);
            }
        }
    }
}

int Q3BSP::find_leaf(const Position& pos) const
{
    int idx = 0;
    while(idx >= 0) {
        const Node& node = _nodes[idx];
        const Plane& plane = _planes[node.plane];

        // distance from pos to the plane along the plane normal
        const float d = (Vector3(plane.normal) * pos) - plane.distance;
// TODO: is 0 == front and 1 == back?
        if(d >= 0) {
            // pos is in front of the plane
            idx = node.children[0];
        } else {
            // pos is behind the plane
            idx = node.children[1];
        }
    }
    return -idx - 1;
}

bool Q3BSP::cluster_visible(int from, int cluster) const
{
    if(from < 0 || cluster < 0) {
        return true;
    }

    // vis data bitset index based on the from and the cluster
    const int idx = (from * _vis_data.sz_vecs) + (cluster >> 3);

    // check bit ('from' mod 8)
    return (_vis_data.vecs[idx] & (1 << (cluster & 7))) != 0;
}

void Q3BSP::render_face(const Face& face, Shader& shader) const
{
    size_t vcount = 0;
    boost::shared_array<float> vertices;
    boost::shared_array<float> textures;
    switch(face.type)
    {
    case 1:
    case 3:
        {
            vcount = face.n_meshverts;
            vertices.reset(new float[vcount * 3]);
            textures.reset(new float[vcount * 2]);

            // setup the vertex buffer
            float *va = vertices.get(), *ta = textures.get();
            for(int i=0; i<face.n_meshverts; ++i) {
                const MeshVert& meshvert(_mesh_verts[face.meshvert + i]);
                const BSPVertex& vertex(_vertices[face.vertex + meshvert.offset]);

                const size_t idx = i * 3;

                *(va + idx + 0) = vertex.position[0];
                *(va + idx + 1) = vertex.position[1];
                *(va + idx + 2) = vertex.position[2];

                const size_t tidx = i * 2;

                *(ta + tidx + 0) = vertex.texcoord[0][0];
                *(ta + tidx + 1) = vertex.texcoord[0][1];
            }
        }
        break;
    case 2:
        // TODO: handle patches
        return;
    case 4:
        // TODO: handle billboards
        return;
    default:
        LOG_WARNING("Unknown face type: " << face.type << std::endl);
        return;
    }

    // setup the vertex array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VertexArray]);
    glBufferData(GL_ARRAY_BUFFER, vcount * 3 * sizeof(float), vertices.get(), GL_DYNAMIC_DRAW);

    // setup the texture array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[TextureArray]);
    glBufferData(GL_ARRAY_BUFFER, vcount * 2 * sizeof(float), textures.get(), GL_DYNAMIC_DRAW);

    // setup the detail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureManager::instance().default_detail_texture());
    shader.uniform1i("detail_texture", 0);

    // setup the normal map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, TextureManager::instance().default_normal_map());
    shader.uniform1i("normal_map", 1);

    // get the attribute locations
    GLint vloc = shader.attrib_location("vertex");
    GLint tloc = shader.attrib_location("texture_coord");

    // render the mesh
    glEnableVertexAttribArray(vloc);
    glEnableVertexAttribArray(tloc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[TextureArray]);
        glVertexAttribPointer(tloc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[VertexArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, vcount);
    glDisableVertexAttribArray(tloc);
    glDisableVertexAttribArray(vloc);
}

bool Q3BSP::read_entities(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryEntities].length;
    LOG_DEBUG("Reading entities (" << len << ")..." << std::endl);

    boost::shared_array<char> ents(new char[len+1]);
    f.seekg(_header.direntries[DirEntryEntities].offset);
    f.read(ents.get(), len);
    ents[len] = '\0';

    _entities.ents = ents.get();
    return true;
}

bool Q3BSP::read_textures(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryTextures].length;
    LOG_DEBUG("Reading textures (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(Texture);
    _textures.reset(new Texture[count]);
    f.seekg(_header.direntries[DirEntryTextures].offset);
    f.read(reinterpret_cast<char*>(_textures.get()), len);

    return true;
}

bool Q3BSP::read_planes(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryPlanes].length;
    LOG_DEBUG("Reading planes (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(Plane);
    _planes.reset(new Plane[count]);
    f.seekg(_header.direntries[DirEntryPlanes].offset);
    f.read(reinterpret_cast<char*>(_planes.get()), len);

    return true;
}

bool Q3BSP::read_nodes(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryNodes].length;
    LOG_DEBUG("Reading nodes (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(Node);
    _nodes.reset(new Node[count]);
    f.seekg(_header.direntries[DirEntryNodes].offset);
    f.read(reinterpret_cast<char*>(_nodes.get()), len);

    return true;
}

bool Q3BSP::read_leaves(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryLeaves].length;
    LOG_DEBUG("Reading leaves (" << len << ")..." << std::endl);

    _leaf_count = len / sizeof(Leaf);
    _leaves.reset(new Leaf[_leaf_count]);
    f.seekg(_header.direntries[DirEntryLeaves].offset);
    f.read(reinterpret_cast<char*>(_leaves.get()), len);

    return true;
}

bool Q3BSP::read_leaf_faces(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryLeafFaces].length;
    LOG_DEBUG("Reading leaf faces (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(LeafFace);
    _leaf_faces.reset(new LeafFace[count]);
    f.seekg(_header.direntries[DirEntryLeafFaces].offset);
    f.read(reinterpret_cast<char*>(_leaf_faces.get()), len);

    return true;
}

bool Q3BSP::read_leaf_brushes(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryLeafBrushes].length;
    LOG_DEBUG("Reading leaf brushes (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(LeafBrush);
    _leaf_brushes.reset(new LeafBrush[count]);
    f.seekg(_header.direntries[DirEntryLeafBrushes].offset);
    f.read(reinterpret_cast<char*>(_leaf_brushes.get()), len);

    return true;
}

bool Q3BSP::read_models(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryModels].length;
    LOG_DEBUG("Reading models (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(Model);
    _models.reset(new Model[count]);
    f.seekg(_header.direntries[DirEntryModels].offset);
    f.read(reinterpret_cast<char*>(_models.get()), len);

    return true;
}

bool Q3BSP::read_brushes(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryBrushes].length;
    LOG_DEBUG("Reading brushes (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(Brush);
    _brushes.reset(new Brush[count]);
    f.seekg(_header.direntries[DirEntryBrushes].offset);
    f.read(reinterpret_cast<char*>(_brushes.get()), len);

    return true;
}

bool Q3BSP::read_brush_sides(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryBrushSides].length;
    LOG_DEBUG("Reading brush sides (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(BrushSides);
    _brush_sides.reset(new BrushSide[count]);
    f.seekg(_header.direntries[DirEntryBrushSides].offset);
    f.read(reinterpret_cast<char*>(_brush_sides.get()), len);

    return true;
}

bool Q3BSP::read_vertices(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryVertices].length;
    LOG_DEBUG("Reading vertices (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(BSPVertex);
    _vertices.reset(new BSPVertex[count]);
    f.seekg(_header.direntries[DirEntryVertices].offset);
    f.read(reinterpret_cast<char*>(_vertices.get()), len);

    return true;
}

bool Q3BSP::read_mesh_verts(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryMeshVerts].length;
    LOG_DEBUG("Reading mesh vertices (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(MeshVert);
    _mesh_verts.reset(new MeshVert[count]);
    f.seekg(_header.direntries[DirEntryMeshVerts].offset);
    f.read(reinterpret_cast<char*>(_mesh_verts.get()), len);

    return true;
}

bool Q3BSP::read_effects(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryEffects].length;
    LOG_DEBUG("Reading effects (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(Effect);
    _effects.reset(new Effect[count]);
    f.seekg(_header.direntries[DirEntryEffects].offset);
    f.read(reinterpret_cast<char*>(_effects.get()), len);

    return true;
}

bool Q3BSP::read_faces(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryFaces].length;
    LOG_DEBUG("Reading faces (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(Face);
    _faces.reset(new Face[count]);
    f.seekg(_header.direntries[DirEntryFaces].offset);
    f.read(reinterpret_cast<char*>(_faces.get()), len);

    return true;
}

bool Q3BSP::read_light_maps(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryLightMaps].length;
    LOG_DEBUG("Reading light maps (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(LightMap);
    _light_maps.reset(new LightMap[count]);
    f.seekg(_header.direntries[DirEntryLightMaps].offset);
    f.read(reinterpret_cast<char*>(_light_maps.get()), len);

    return true;
}

bool Q3BSP::read_light_vols(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryLightVols].length;
    LOG_DEBUG("Reading light volumes (" << len << ")..." << std::endl);

    const size_t count = len / sizeof(LightVol);
    _light_vols.reset(new LightVol[count]);
    f.seekg(_header.direntries[DirEntryLightVols].offset);
    f.read(reinterpret_cast<char*>(_light_vols.get()), len);

    return true;
}

bool Q3BSP::read_vis_data(std::ifstream& f)
{
    const size_t len = _header.direntries[DirEntryVisData].length;
    LOG_DEBUG("Reading vis data (" << len << ")..." << std::endl);

    f.seekg(_header.direntries[DirEntryVisData].offset);
    f.read(reinterpret_cast<char*>(&_vis_data.n_vecs), sizeof(int));
    f.read(reinterpret_cast<char*>(&_vis_data.sz_vecs), sizeof(int));

    const size_t count = _vis_data.n_vecs * _vis_data.sz_vecs;
    _vis_data.vecs.reset(new unsigned char[count]);
    f.read(reinterpret_cast<char*>(_vis_data.vecs.get()), count);

    return true;
}
