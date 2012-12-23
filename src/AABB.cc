#include "pch.h"
#include "AABB.h"
#include "Sphere.h"
#include "math_util.h"

AABB::AABB()
    : BoundingVolume(),
        _minimum(FLT_MAX, FLT_MAX, FLT_MAX),
        _maximum(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
    calculate_center();
}

AABB::AABB(const Point3& minimum, const Point3& maximum)
    : BoundingVolume(), _minimum(minimum), _maximum(maximum)
{
    calculate_center();
}

AABB::AABB(const Point3& center, float radius)
    : BoundingVolume(), _center(center), _radius(radius)
{
    Point3 rv(radius, radius, radius);
    _minimum = Point3(center - rv);
    _maximum = center + rv;
}

AABB::~AABB() throw()
{
}

void AABB::update(const AABB& bounds)
{
    if(bounds.minimum().x() < _minimum.x()) {
        _minimum.x(bounds.minimum().x());
    }
    if(bounds.minimum().y() < _minimum.y()) {
        _minimum.y(bounds.minimum().y());
    }
    if(bounds.minimum().z() < _minimum.z()) {
        _minimum.z(bounds.minimum().z());
    }

    if(bounds.maximum().x() > _maximum.x()) {
        _maximum.x(bounds.maximum().x());
    }
    if(bounds.maximum().y() > _maximum.y()) {
        _maximum.y(bounds.maximum().y());
    }
    if(bounds.maximum().z() > _maximum.z()) {
        _maximum.z(bounds.maximum().z());
    }

    calculate_center();
}

void AABB::update(const Point3& point)
{
    if(point.x() < _minimum.x()) {
        _minimum.x(point.x());
    }
    if(point.y() < _minimum.y()) {
        _minimum.y(point.y());
    }
    if(point.z() < _minimum.z()) {
        _minimum.z(point.z());
    }

    if(point.x() > _maximum.x()) {
        _maximum.x(point.x());
    }
    if(point.y() > _maximum.y()) {
        _maximum.y(point.y());
    }
    if(point.z() > _maximum.z()) {
        _maximum.z(point.z());
    }

    calculate_center();
}

float AABB::distance(const Point3& other) const
{
    return other.distance(closest_point(other));
}

float AABB::distance(const Sphere& other) const
{
    return other.distance(closest_point(other.center()));
}

Point3 AABB::closest_point(const Point3& point) const
{
    float x = point.x();
    if(point.x() < _minimum.x()) {
        x = _minimum.x();
    } else if(point.x() > _maximum.x()) {
        x = _maximum.x();
    }

    float y = point.y();
    if(point.y() < _minimum.y()) {
        y = _minimum.y();
    } else if(point.y() > _maximum.y()) {
        y = _maximum.y();
    }

    float z = point.z();
    if(point.z() < _minimum.z()) {
        z = _minimum.z();
    } else if(point.z() > _maximum.z()) {
        z = _maximum.z();
    }

    return Point3(x, y, z);
}

std::string AABB::str() const
{
    std::stringstream ss;
    ss << "AABB(" << _minimum.str() << ", " << _maximum.str() << ")";
    return ss.str();
}

void AABB::calculate_center()
{
    _center = _minimum + ((_maximum - _minimum) / 2.0f);
    _radius = center().distance(_minimum);
}
