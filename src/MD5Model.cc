#include "pch.h"
#include <iostream>
#include "common.h"
#include "Lexer.h"
#include "MD5Model.h"

// NOTE: MD5s are *not* guaranteed to be closed - one-winged edges require the *first* triangle to face the light

MD5Mesh::MD5Mesh(const std::string& shader, size_t vcount, boost::shared_array<Vertex> vertices, size_t tcount, boost::shared_array<Triangle> triangles, size_t wcount, boost::shared_array<Weight> weights)
    : Mesh(vcount, vertices, tcount, triangles, wcount, weights),
        _shader_name(shader)
{
}

MD5Mesh::~MD5Mesh() throw()
{
}

Logger& MD5Model::logger(Logger::instance("md5mv.MD5Model"));

MD5Model::MD5Model(const std::string& name)
    : Model(name), _version(0)
{
}

MD5Model::~MD5Model() throw()
{
}

bool MD5Model::on_load(const boost::filesystem::path& path)
{
    boost::filesystem::path filename(model_dir() / path / (name() + extension()));
    LOG_INFO("Loading model from '" << filename << "'" << std::endl);

    Lexer lexer;
    if(!lexer.load(filename)) {
        return false;
    }

    if(!scan_version(lexer)) {
        return false;
    }

    if(!scan_commandline(lexer)) {
        return false;
    }

    int jcount = scan_num_joints(lexer);
    if(jcount < 0) return false;

    int mcount = scan_num_meshes(lexer);
    if(mcount < 0) return false;

    if(!scan_joints(lexer, jcount)) {
        return false;
    }

    if(!scan_meshes(lexer, mcount)) {
        return false;
    }

    return true;
}

void MD5Model::on_unload() throw()
{
    _version = 0;
    _commandline.erase();
}

bool MD5Model::scan_version(Lexer& lexer)
{
    if(!lexer.match(MD5VERSION)) {
        return false;
    }

    if(!lexer.int_literal(_version)) {
        return false;
    }

    // only version 10 is supported
    return _version == 10;
}

bool MD5Model::scan_commandline(Lexer& lexer)
{
    if(!lexer.match(COMMANDLINE)) {
        return false;
    }
    return lexer.string_literal(_commandline);
}

int MD5Model::scan_num_joints(Lexer& lexer)
{
    if(!lexer.match(NUM_JOINTS)) {
        return -1;
    }

    int value;
    if(!lexer.int_literal(value)) {
        return -1;
    }
    return value;
}

int MD5Model::scan_num_meshes(Lexer& lexer)
{
    if(!lexer.match(NUM_MESHES)) {
        return -1;
    }

    int value;
    if(!lexer.int_literal(value)) {
        return -1;
    }
    return value;
}

bool MD5Model::scan_joints(Lexer& lexer, int count)
{
    if(!lexer.match(JOINTS)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    for(int i=0; i<count; ++i) {
        if(!scan_joint(lexer)) {
            return false;
        }
    }

    return lexer.match(CLOSE_BRACE);
}

bool MD5Model::scan_joint(Lexer& lexer)
{
    std::string name;
    if(!lexer.string_literal(name)) {
        return false;
    }

    int parent;
    if(!lexer.int_literal(parent)) {
        return false;
    }

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

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    // orientation
    Vector3 orientation;

    if(!lexer.float_literal(value)) {
        return false;
    }
    orientation.x(value);

    if(!lexer.float_literal(value)) {
        return false;
    }
    orientation.y(value);

    if(!lexer.float_literal(value)) {
        return false;
    }
    orientation.z(value);

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    Skeleton::Joint joint;
    joint.name = name;
    joint.parent = parent;
    joint.position = swizzle(position);
    joint.orientation = Quaternion(swizzle(orientation));
    skeleton().add_joint(joint);

    return true;
}

bool MD5Model::scan_meshes(Lexer& lexer, int count)
{
    LOG_INFO("Reading " << count << " meshes..." << std::endl);
    for(int i=0; i<count; ++i) {
        if(!scan_mesh(lexer)) {
            return false;
        }
    }

    return true;
}

bool MD5Model::scan_mesh(Lexer& lexer)
{
    if(!lexer.match(MESH)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    if(!lexer.match(SHADER)) {
        return false;
    }

    std::string shader;
    if(!lexer.string_literal(shader)) {
        return false;
    }

    if(!lexer.match(NUM_VERTS)) {
        return false;
    }

    // vertices
    int vcount;
    if(!lexer.int_literal(vcount)) {
        return false;
    }

    boost::shared_array<Vertex> vertices(new Vertex[vcount]);
    if(!scan_vertices(lexer, vertices, vcount)) {
        return false;
    }

    // triangles
    if(!lexer.match(NUM_TRIS)) {
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

    // weights
    if(!lexer.match(NUM_WEIGHTS)) {
        return false;
    }

    int wcount;
    if(!lexer.int_literal(wcount)) {
        return false;
    }

    boost::shared_array<Weight> weights(new Weight[wcount]);
    if(!scan_weights(lexer, weights, wcount)) {
        return false;
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    add_mesh(boost::shared_ptr<Mesh>(new MD5Mesh(shader, vcount, vertices, tcount, triangles, wcount, weights)), false, false);
    return true;
}

bool MD5Model::scan_vertices(Lexer& lexer, boost::shared_array<Vertex>& vertices, int count)
{
    LOG_INFO("Reading " << count << " vertices..." << std::endl);
    for(int i=0; i<count; ++i) {
        if(!lexer.match(VERT)) {
            return false;
        }

        int index;
        if(!lexer.int_literal(index)) {
            return false;
        }

        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        // texture coords
        Vector2 texture_coords;

        float value;
        if(!lexer.float_literal(value)) {
            return false;
        }
        texture_coords.x(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        texture_coords.y(value);

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }

        // start weight
        int ws;
        if(!lexer.int_literal(ws)) {
            return false;
        }

        // weight count
        int wc;
        if(!lexer.int_literal(wc)) {
            return false;
        }

        // add the vertex
        Vertex& vertex(vertices[index]);
        vertex.index = index;
        vertex.texture_coords = texture_coords;
        vertex.weight_start = ws;
        vertex.weight_count = wc;
    }

    return true;
}

bool MD5Model::scan_triangles(Lexer& lexer, boost::shared_array<Vertex>& vertices, boost::shared_array<Triangle>& triangles, int count)
{
    LOG_INFO("Reading " << count << " triangles..." << std::endl);
    for(int i=0; i<count; ++i) {
        if(!lexer.match(TRI)) {
            return false;
        }

        // triangle index
        int index;
        if(!lexer.int_literal(index)) {
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

        // add the triangle
        // TODO: find a better way to swizzle this
        Triangle& triangle(triangles[index]);
        triangle.index = index;
        triangle.v1 = c;
        triangle.v2 = b;
        triangle.v3 = a;
    }

    return true;
}

bool MD5Model::scan_weights(Lexer& lexer, boost::shared_array<Weight>& weights, int count)
{
    for(int i=0; i<count; ++i) {
        if(!lexer.match(WEIGHT)) {
            return false;
        }

        // weight index
        int index;
        if(!lexer.int_literal(index)) {
            return false;
        }

        // joint index
        int ji;
        if(!lexer.int_literal(ji)) {
            return false;
        }

        // weight
        float w;
        if(!lexer.float_literal(w)) {
            return false;
        }

        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        // position
        Position pv;

        float value;
        if(!lexer.float_literal(value)) {
            return false;
        }
        pv.x(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        pv.y(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        pv.z(value);

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }

        // add the weight
        Weight& weight(weights[index]);
        weight.index = index;
        weight.joint = ji;
        weight.weight = w;
        weight.position = swizzle(pv);
    }

    return true;
}
