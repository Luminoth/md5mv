#if !defined __AABB_H__
#define __AABB_H__

#include "BoundingVolume.h"

class Sphere;

class AABB : public BoundingVolume
{
public:
    AABB();
    AABB(const Point3& minimum, const Point3& maximum);
    AABB(const Point3& center, float radius);
    virtual ~AABB() throw();

public:
    const Point3& minimum() const { return _minimum; }
    const Point3& maximum() const { return _maximum; }

    virtual const Point3& center() const { return _center; }
    virtual float radius() const { return _radius; }
    virtual float radius_squared() const { return std::pow(_radius, 2); }

    // updates the current bounds to encompass the given point(s)
    void update(const AABB& bounds);
    void update(const Point3& point);

    float distance(const Point3& other) const;
    float distance(const Sphere& other) const;

    Point3 closest_point(const Point3& point) const;

    virtual std::string str() const;

public:
    friend AABB operator+(const Point3& lhs, const AABB& rhs) { return AABB(lhs + rhs._minimum, lhs + rhs._maximum); }

private:
    void calculate_center();

private:
    Point3 _minimum, _maximum, _center;
    float _radius;
};

#endif
