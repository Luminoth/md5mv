#include "pch.h"
#include "Plane.h"

Plane::Plane(const Point3& p1, const Point3& p2, const Point3& p3)
{
    const Vector3 q1(p2 - p1), q2(p3 - p1);
    const Vector3 n((q1 ^ q2).normalize());
    _plane = Vector4(n, -n * p1);
}
