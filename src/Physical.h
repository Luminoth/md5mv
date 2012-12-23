#if !defined __PHYSICAL_H__
#define __PHYSICAL_H__

#include "AABB.h"
#include "Quaternion.h"
#include "Vector.h"

class Matrix4;

class Physical
{
public:
    virtual ~Physical() throw();

public:
    const Position& position() const { return _position; }
    void position(const Position& position) { _position = position; }

    const Quaternion& orientation() const { return _orientation; }
    void orientation(const Quaternion& orientation) { _orientation = orientation; }
    void lookat(const Position& position) { _orientation = Quaternion::new_axis(0.0f, position); }

    // rotation is in radians
    void rotate(float angle, const Position& around);
    void pitch(float angle);
    void yaw(float angle);
    void roll(float angle);

    float scale() const { return _scale; }
    void scale(float scale) { _scale = scale; }

    AABB absolute_bounds() const { return _position + _bounds; }
    const AABB& relative_bounds() const { return _bounds; }

    // applies this physicals translation, rotation, and scale to the matrix
    void transform(Matrix4& matrix) const;

    const Vector3& velocity() const { return _velocity; }
    void velocity(const Vector3& velocity) { _velocity = velocity; }

    const Vector3& acceleration() const { return _acceleration; }
    void acceleration(const Vector3& acceleration) { _acceleration = acceleration; }

    void think(double dt);

    virtual std::string str() const;

protected:
    Physical();
    Physical(const Position& position, float scale);

    // relative to the physical's position!
    void bounds(const AABB& bounds) { _bounds = bounds; }

    virtual bool on_think(double dt) { return true; }

private:
    Position _position;
    float _scale;
    Quaternion _orientation;
    Vector3 _velocity, _acceleration;
    AABB _bounds;
};

#endif
