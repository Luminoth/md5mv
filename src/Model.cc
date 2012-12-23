#include "pch.h"
#include "common.h"
#include "Lexer.h"
#include "Mesh.h"
#include "Renderable.h"
#include "Model.h"

Skeleton::Skeleton()
    : _nonroot_joint_count(0)
{
}

Skeleton::~Skeleton() throw()
{
}

void Skeleton::add_joint(const Joint& joint)
{
    _joints.push_back(joint);
    if(joint.parent >= 0) {
        _nonroot_joint_count++;
    }
}

void Skeleton::reset()
{
    _joints.clear();
    _nonroot_joint_count = 0;
}

Logger& Model::logger(Logger::instance("md5mv.Model"));

Model::Model(const std::string& name)
    : _name(name), _vcount(0), _tcount(0), _ecount(0)
{
}

Model::~Model() throw()
{
    unload();
}

bool Model::load(const boost::filesystem::path& path)
{
    unload();
    return on_load(path);
}

bool Model::load_textures(const boost::filesystem::path& path)
{
    for(size_t i=0; i<mesh_count(); ++i) {
        if(!_meshes[i]->load_textures(path, name())) {
            return false;
        }
    }
    return true;
}

void Model::unload() throw()
{
    _meshes.clear();
    _skeleton.reset();

    _vcount = 0;
    _tcount = 0;
    _ecount = 0;

    _bounds = AABB();

    on_unload();
}

void Model::calculate_vertices(const Skeleton& skeleton, boost::shared_array<Vertex> vertices, RenderableBuffers& buffers) const
{
    size_t vstart=0, tstart=0;
    for(size_t i=0; i<_meshes.size(); ++i) {
        const Mesh& m(mesh(i));
        m.calculate_vertices(skeleton, vertices, vstart, buffers, tstart);

        vstart += m.vertex_count();
        tstart += m.triangle_count();
    }
}

void Model::add_mesh(boost::shared_ptr<Mesh> mesh, bool has_normals, bool has_edges)
{
    _meshes.push_back(mesh);

    mesh->pose(_skeleton);
    mesh->weld_vertices();

    if(!has_normals) {
        mesh->compute_normals(_skeleton);
    }

    if(!has_edges) {
        mesh->compute_edges();
    }

    // update some model-wide properties
    _vcount += mesh->vertex_count();
    _tcount += mesh->triangle_count();
    _ecount += mesh->edge_count();
    _bounds.update(mesh->bounds());
}

bool Model::on_load(const boost::filesystem::path& path)
{
    boost::filesystem::path filename(model_dir() / path / (name() + extension()));
    LOG_INFO("Loading model from '" << filename << "'" << std::endl);

    Lexer lexer;
    if(!lexer.load(filename)) {
        return false;
    }

    if(!scan_header(lexer)) {
        return false;
    }

    if(!scan_meshes(lexer)) {
        return false;
    }

    return true;
}

bool Model::scan_header(Lexer& lexer)
{
    std::string header;
    if(!lexer.string_literal(header)) {
        return false;
    }

    if("MDL1" == header) {
        return true;
    }

    return false;
}

bool Model::scan_meshes(Lexer& lexer)
{
    while(!lexer.check_token(END)) {
        if(!lexer.match(MESH)) {
            return false;
        }

        if(!lexer.match(OPEN_BRACE)) {
            return false;
        }

        while(!lexer.check_token(CLOSE_BRACE)) {
            if(!scan_mesh(lexer)) {
                return false;
            }
        }

        if(!lexer.match(CLOSE_BRACE)) {
            return false;
        }
    }

    return true;
}

bool Model::scan_mesh(Lexer& lexer)
{
    if(!lexer.match(HAS_NORMALS)) {
        return false;
    }

    // properties
    bool has_normals;
    if(!lexer.bool_literal(has_normals)) {
        return false;
    }

    if(!lexer.match(HAS_EDGES)) {
        return false;
    }

    bool has_edges;
    if(!lexer.bool_literal(has_edges)) {
        return false;
    }

    // vertices
    if(!lexer.match(VERTICES)) {
        return false;
    }

    int vcount;
    if(!lexer.int_literal(vcount)) {
        return false;
    }

    boost::shared_array<Vertex> vertices(new Vertex[vcount]);
    if(!scan_vertices(lexer, has_normals, vertices, vcount)) {
        return false;
    }

    // triangles
    if(!lexer.match(TRIANGLES)) {
        return false;
    }

    int tcount;
    if(!lexer.int_literal(tcount)) {
        return false;
    }

    boost::shared_array<Triangle> triangles(new Triangle[tcount]);
    if(!scan_triangles(lexer, vertices, triangles, tcount)) {
        return false;
    }

    // edges
    if(has_edges) {
        LOG_ERROR("TODO: READ MESH EDGES!" << std::endl);
        return false;
    }

    add_mesh(boost::shared_ptr<Mesh>(new Mesh(vcount, vertices, tcount, triangles, 0, boost::shared_array<Weight>())), has_normals, has_edges);
    return true;
}

bool Model::scan_vertices(Lexer& lexer, bool has_normals, boost::shared_array<Vertex>& vertices, int count)
{
    LOG_INFO("Reading " << count << " vertices..." << std::endl);
    for(int i=0; i<count; ++i) {
        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        // position
        Position position;

        float value;
        if(!lexer.float_literal(value)) {
            return false;
        }
        position.x(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        position.y(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        position.z(value);

        // texture coords
        Vector2 texture_coords;

        if(!lexer.float_literal(value)) {
            return false;
        }
        texture_coords.x(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        texture_coords.y(value);

        // normals
        if(has_normals) {
            LOG_ERROR("TODO: READ MESH NORMALS!" << std::endl);
            return false;
        }

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }

        // add the vertex
        Vertex& vertex(vertices[i]);
        vertex.index = i;
        vertex.position = swizzle(position);
        vertex.texture_coords = texture_coords;
    }

    return true;
}

bool Model::scan_triangles(Lexer& lexer, boost::shared_array<Vertex>& vertices, boost::shared_array<Triangle>& triangles, int count)
{
    LOG_INFO("Reading " << count << " triangles..." << std::endl);
    for(int i=0; i<count; ++i) {
        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        // vertices
        int a;
        if(!lexer.int_literal(a)) {
            return false;
        }

        int b;
        if(!lexer.int_literal(b)) {
            return false;
        }

        int c;
        if(!lexer.int_literal(c)) {
            return false;
        }

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }

        // add the triangle
        // TODO: find a better way to swizzle this
        Triangle& triangle(triangles[i]);
        triangle.index = i;
        triangle.v1 = c;
        triangle.v2 = b;
        triangle.v3 = a;
    }

    return true;
}
