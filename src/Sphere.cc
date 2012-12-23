#include "pch.h"
#include "AABB.h"
#include "Sphere.h"

Sphere::Sphere(const Point3& center, float radius)
    : BoundingVolume(), _center(center), _radius(fabs(radius))
{
}

Sphere::Sphere(const AABB& aabb)
    : BoundingVolume(), _center(aabb.center()), _radius(aabb.radius())
{
}

Sphere::~Sphere() throw()
{
}

float Sphere::distance_squared(const Point3& p) const
{
    return _center.distance_squared(p) - std::pow(_radius, 2);
}

float Sphere::distance(const Point3& p) const
{
    return _center.distance(p) - _radius;
}

float Sphere::distance_squared(const Sphere& s) const
{
    return _center.distance_squared(s._center) - std::pow(_radius + s._radius, 2);
}

float Sphere::distance(const Sphere& s) const
{
    return _center.distance(s._center) - (_radius + s._radius);
}

float Sphere::distance(const AABB& other) const
{
    return other.distance(*this);
}

std::string Sphere::str() const
{
    std::stringstream ss;
    ss << "Sphere(" << _center.str() << ", " << _radius << ")";
    return ss.str();
}
