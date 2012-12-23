#include "pch.h"
#include "Geometry.h"

Vertex::Vertex()
    : index(-1), weight_start(0), weight_count(0)
{
}

std::string Vertex::str() const
{
    std::stringstream ss;
    ss << "Vertex(index: " << index << ", position:" << position.str() << ", texture_coords: " << texture_coords.str() << ")";
    return ss.str();
}

Triangle::Triangle()
    : index(-1), v1(-1), v2(-1), v3(-1)
{
}

Weight::Weight()
    : index(-1), joint(-1), weight(0.0f)
{
}

Edge::Edge()
    : v1(-1), v2(-1), t1(-1), t2(-1)
{
}

std::string Edge::str() const
{
    std::stringstream ss;
    ss << "Edge(v1: " << v1 << ", v2:" << v2 << ", t1: " << t1 << ", t2: " << t2 << ")";
    return ss.str();
}

void compute_tangents(boost::shared_array<Triangle> triangles, size_t triangle_count, boost::shared_array<Vertex> vertices, size_t vertex_count, bool smooth)
{
    boost::shared_array<Vector3> narray(new Vector3[vertex_count]);
    boost::shared_array<Vector3> tarray(new Vector3[vertex_count]);
    boost::shared_array<Vector3> btarray(new Vector3[vertex_count]);

    // calculate the vertex normals and tangents (sum of each face normal/tangent)
    // Mathematics for 3D Game Programming and Computer Graphics, section 7.8.3
    for(size_t i=0; i<triangle_count; ++i) {
        Triangle& triangle(triangles[i]);
        const Vertex &p0(vertices[triangle.v1]), &p1(vertices[triangle.v2]), &p2(vertices[triangle.v3]);

        const int idx0 = p0.index, idx1 = p1.index, idx2 = p2.index;

        // face normal
        const Vector3 q1(p1.position - p0.position), q2(p2.position - p0.position);
        Vector3 normal(q1 ^ q2);
        if(smooth) {
            normal.normalize();
        }
        triangle.normal = normal.normalized();

        narray[idx0] += normal;
        narray[idx1] += normal;
        narray[idx2] += normal;

        const float s1 = p1.texture_coords.x() - p0.texture_coords.x(), t1 = p1.texture_coords.y() - p0.texture_coords.y();
        const float s2 = p2.texture_coords.x() - p0.texture_coords.x(), t2 = p2.texture_coords.y() - p0.texture_coords.y();

        // face tangent (coefficient times the texture matrix times the position matrix)
        const float coef = 1.0f / (s1 * t2 - s2 * t1);
        Vector3 tangent(coef * Vector3(t2 * q1.x() - t1 * q2.x(), t2 * q1.y() - t1 * q2.y(), t2 * q1.z() - t1 * q2.z()));
        if(smooth) {
            tangent.normalize();
        }

        // Gram-Schmidt orthogonalize the tangent
        Vector3 object_tangent(tangent - (normal * tangent) * normal);
        if(smooth) {
            object_tangent.normalize();
        }

        tarray[idx0] += object_tangent;
        tarray[idx1] += object_tangent;
        tarray[idx2] += object_tangent;

        Vector3 bitangent(coef * Vector3(s1 * q2.x() - s2 * q1.x(), s1 * q2.y() - s2 * q1.y(), s1 * q2.z() - s2 * q1.z()));
        if(smooth) {
            bitangent.normalize();
        }

        btarray[idx0] += bitangent;
        btarray[idx1] += bitangent;
        btarray[idx2] += bitangent;
    }

    // store the vertex data
    for(size_t i=0; i<vertex_count; ++i) {
        Vertex& vertex(vertices[i]);
        vertex.normal = narray[i].normalized();
        vertex.tangent = tarray[i].normalized();
        vertex.bitangent = btarray[i].normalized();
    }
}
