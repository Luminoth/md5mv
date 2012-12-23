#include "pch.h"
#include "common.h"
#include "Model.h"
#include "Renderable.h"
#include "Mesh.h"

Logger& Mesh::logger(Logger::instance("md5mv.Mesh"));

Mesh::Mesh(int vcount, boost::shared_array<Vertex> vertices, int tcount, boost::shared_array<Triangle> triangles, int wcount, boost::shared_array<Weight> weights)
    : _vcount(vcount), _vertices(vertices),
        _tcount(tcount), _triangles(triangles),
        _wcount(wcount), _weights(weights)
{
    ZeroMemory(_textures, sizeof(GLuint) * TextureManager::TextureCount);
}

Mesh::~Mesh() throw()
{
    glDeleteTextures(TextureManager::TextureCount, _textures);
}

bool Mesh::load_textures(const boost::filesystem::path& path, const std::string& name)
{
    boost::filesystem::path filename(model_dir() / path / name);

    _textures[TextureManager::DetailTexture] = TextureManager::instance().load_texture(filename.string() + "_d.tga");
    if(0 == _textures[TextureManager::DetailTexture]) {
        return false;
    }

    _textures[TextureManager::NormalMap] = TextureManager::instance().load_texture(filename.string() + "_n.tga");
    if(0 == _textures[TextureManager::NormalMap]) {
        return false;
    }

    _textures[TextureManager::SpecularMap] = TextureManager::instance().load_texture(filename.string() + "_s.tga");
    if(0 == _textures[TextureManager::SpecularMap]) {
        return false;
    }

    /*_textures[TextureManager::EmissionMap] = TextureManager::instance().load_texture(filename.stem() + "_e.tga");
    if(0 == _textures[TextureManager::EmissionMap]) {
        return false;
    }*/

    return true;
}

void Mesh::pose(const Skeleton& skeleton)
{
    for(int i=0; i<_vcount; ++i) {
        Vertex& vertex(_vertices[i]);

        Position position;
        if(has_weights()) {
            for(int j=0; j<vertex.weight_count; ++j) {
                const Weight& weight(_weights[vertex.weight_start + j]);
                const Skeleton::Joint& joint(skeleton.joint(weight.joint));

                // convert the joint to object space and weight the vertex
                const Position wpos(joint.orientation * weight.position);
                position += ((wpos + joint.position) * weight.weight);
            }
            vertex.position = position;
        } else {
            position = vertex.position;
        }

        // also update bounds while we're here
        _bounds.update(position);
    }
}

void Mesh::compute_normals(const Skeleton& skeleton, bool smooth)
{
    compute_tangents(_triangles, _tcount, _vertices, _vcount);

    if(has_weights()) {
        // store the weighted vertex data in joint-space
        for(int i=0; i<_vcount; ++i) {
            Vertex& vertex(_vertices[i]);

            // put the normals and tangents into joint space
            for(int j=0; j<vertex.weight_count; ++j) {
                Weight& weight(_weights[vertex.weight_start + j]);
                const Skeleton::Joint& joint(skeleton.joint(weight.joint));

                // convert to joint-space and store
                Quaternion inv(joint.orientation.inverse());
                weight.normal += inv * vertex.normal;
                weight.tangent += inv * vertex.tangent;
                weight.bitangent += inv * vertex.bitangent;
            }

            // normalize everything
            for(int j=0; j<vertex.weight_count; ++j) {
                Weight& weight(_weights[vertex.weight_start + j]);
                weight.normal.normalize();
                weight.tangent.normalize();
                weight.bitangent.normalize();
            }
        }
    }
}

void Mesh::weld_vertices()
{
    // find the vertices that need to be welded
    boost::unordered_map<int, int> vertices;
    for(int i=0; i<_vcount; ++i) {
        for(int j=i+1; j<_vcount; ++j) {
            const Vertex &v1(_vertices[i]), &v2(_vertices[j]);
            if(v1.position.distance_squared(v2.position) < 0.001f
                && v1.texture_coords.distance_squared(v2.texture_coords) < 0.001f)
            {
                vertices[v1.index] = v2.index;
            }
        }
    }

    // weld them
    weld_vertices(vertices);
}

void Mesh::compute_edges()
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3.3
    for(int i=0; i<_tcount; ++i) {
        calculate_edge(_triangles[i], i);
    }
    find_matching_edges();
}

void Mesh::calculate_vertices(const Skeleton& skeleton, boost::shared_array<Vertex> vertices, size_t vstart, RenderableBuffers& buffers, size_t tstart) const
{
    position_vertices(skeleton, vertices, vstart);
    buffers.copy_triangles(_triangles.get(), _tcount, vertices.get() + vstart, _vcount, tstart * 3);
}

void Mesh::position_vertices(const Skeleton& skeleton, boost::shared_array<Vertex> vertices, size_t vstart) const
{
    for(int i=0; i<_vcount; ++i) {
        const Vertex& meshvertex(_vertices[i]);

        Position position;
        Vector3 normal, tangent, bitangent;
        if(has_weights()) {
            for(int j=0; j<meshvertex.weight_count; ++j) {
                const Weight& weight(_weights[meshvertex.weight_start + j]);
                const Skeleton::Joint& joint(skeleton.joint(weight.joint));

                // convert the joint to object space and weight the vertex attributes
                const Position wpos(joint.orientation * weight.position);
                position += ((wpos + joint.position) * weight.weight);
                normal += (joint.orientation * weight.normal);
                tangent += (joint.orientation * weight.tangent);
                bitangent += (joint.orientation * weight.bitangent);
            }
        } else {
            position = meshvertex.position;
            normal = meshvertex.normal;
            tangent = meshvertex.tangent;
            bitangent = meshvertex.bitangent;
        }

        Vertex& vertex(vertices[vstart + i]);
        vertex.index = meshvertex.index;
        vertex.position = position;
        vertex.normal = normal.normalized();
        vertex.tangent = tangent.normalize();
        vertex.bitangent = bitangent.normalized();
        vertex.texture_coords = meshvertex.texture_coords;
    }
}

void Mesh::weld_vertices(const boost::unordered_map<int, int>& vertices)
{
    if(0 == vertices.size()) {
        return;
    }

    LOG_INFO("Welding " << vertices.size() << " vertices" << std::endl);

    const size_t vcount = _vcount - vertices.size();
    boost::shared_array<Vertex> v(new Vertex[vcount]);

    size_t j=0;
    for(int i=0; i<_vcount; ++i) {
        const Vertex& vertex(_vertices[i]);
        if(vertices.end() == vertices.find(vertex.index)) {
            // copy the vertex and update it's index
            Vertex& new_vertex(v[j]);
            new_vertex = vertex;
            new_vertex.index = j;
            fix_triangles(vertex.index, new_vertex.index);
            j++;
        } else {
            // point triangles at the new vertex
            fix_triangles(i, vertices.at(i));
        }
    }

    _vcount = vcount;
    _vertices = v;
}

void Mesh::fix_triangles(int old_index, int new_index)
{
    for(int i=0; i<_tcount; ++i) {
        Triangle& triangle(_triangles[i]);
        if(triangle.v1 == old_index) {
            triangle.v1 = new_index;
        } else if(triangle.v2 == old_index) {
            triangle.v2 = new_index;
        } else if(triangle.v3 == old_index) {
            triangle.v3 = new_index;
        }
    }
}

void Mesh::calculate_edge(const Triangle& triangle, int t)
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3.3
    if(triangle.v1 < triangle.v2) {
        Edge edge;
        edge.v1 = triangle.v1;
        edge.v2 = triangle.v2;
        edge.t1 = t;
        _edges.push_back(edge);
    }

    if(triangle.v2 < triangle.v3) {
        Edge edge;
        edge.v1 = triangle.v2;
        edge.v2 = triangle.v3;
        edge.t1 = t;
        _edges.push_back(edge);
    }

    if(triangle.v3 < triangle.v1) {
        Edge edge;
        edge.v1 = triangle.v3;
        edge.v2 = triangle.v1;
        edge.t1 = t;
        _edges.push_back(edge);
    }
}

void Mesh::find_matching_edges()
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3.3
    for(int i=0; i<_tcount; ++i) {
        const Triangle& triangle(_triangles[i]);

        if(triangle.v1 > triangle.v2) {
            find_matching_edge(triangle.v2, triangle.v1, i);
        }

        if(triangle.v2 > triangle.v3) {
            find_matching_edge(triangle.v3, triangle.v2, i);
        }

        if(triangle.v3 > triangle.v1) {
            find_matching_edge(triangle.v1, triangle.v3, i);
        }
    }
}

void Mesh::find_matching_edge(int v1, int v2, int t)
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3.3
    BOOST_FOREACH(Edge& edge, _edges) {
        if(edge.v1 == v1 && edge.v2 == v2 && edge.t2 < 0) {
            edge.t2 = t;
            return;
        }
    }

    // didn't find a match, so this is a one-winged edge
    // TODO: not sure if I'm setting the v1/v2 properties correctly
    Edge edge;
    edge.v1 = v1;
    edge.v2 = v2;
    edge.t1 = t;
    _edges.push_back(edge);
}
