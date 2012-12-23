#if !defined __PLANE_H__
#define __PLANE_H__

#include "Vector.h"

class Plane
{
public:
    // NOTE: this creates an invalid plane
    // because the normal is set to (0, 0, 0)
    Plane() /*: _plane(0.0f, 0.0f, 1.0f, 0.0f)*/ { }

    // this normalizes N
    Plane(const Vector3& n, float d) : _plane(n.normalized(), d) { }

    Plane(const Point3& p1, const Point3& p2, const Point3& p3);
    virtual ~Plane() throw() {}

public:
    Vector3 normal() const { return Vector3(_plane.x(), _plane.y(), _plane.z()); }
    float distance() const { return _plane.w(); }

    // NOTE: these assume that the plane normal is normalized and the w coordinate is 1.0
    float distance(const Vector4& q) const { return _plane * q; }
    bool is_coplanar(const Vector4& q) const { return distance(q) < 0.001f; }

public:
    float operator*(const Vector4& rhs) const { return _plane * rhs; }

private:
    Vector4 _plane;
};

#endif
