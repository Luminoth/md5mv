#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "BoundingVolume.h"

class AABB;

class Sphere : public BoundingVolume
{
public:
    explicit Sphere(const Point3& center=Point3(), float radius=1.0f);
    explicit Sphere(const AABB& aabb);
    virtual ~Sphere() throw();

public:
    void center(const Point3& center) { _center = center; }
    virtual const Point3& center() const { return _center; }

    void radius(float radius) { _radius = radius; }
    virtual float radius() const { return _radius; }
    virtual float radius_squared() const { return std::pow(_radius, 2); }

    float distance_squared(const Point3& p) const;
    float distance(const Point3& p) const;
    float distance_squared(const Sphere& s) const;
    float distance(const Sphere& s) const;
    float distance(const AABB& other) const;

    virtual std::string str() const;

protected:
    Point3 _center;
    float _radius;
};

#endif
