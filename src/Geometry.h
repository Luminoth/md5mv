#if !defined __GEOMETRY_H__
#define __GEOMETRY_H__

#include "Vector.h"

struct Vertex
{
    int index;
    Position position;
    Vector3 normal, tangent, bitangent;
    Vector2 texture_coords;
    int weight_start, weight_count;

    Vertex();
    virtual ~Vertex() throw() {}

    std::string str() const;

    float distance_squared(const Vertex& other) const { return position.distance_squared(other.position); }
};

struct Triangle
{
    int index;
    int v1, v2, v3;
    Vector3 normal;

    Triangle();
    virtual ~Triangle() throw() {}
};

struct Weight
{
    int index;
    int joint;
    float weight;
    Position position;
    Vector3 normal, tangent, bitangent;

    Weight();
    virtual ~Weight() throw() {}
};

struct Edge
{
    int v1, v2;
    int t1, t2;

    Edge();
    virtual ~Edge() throw() {}

    std::string str() const;
};

void compute_tangents(boost::shared_array<Triangle> triangles, size_t triange_count, boost::shared_array<Vertex> vertices, size_t vertex_count, bool smooth=false);

#endif
