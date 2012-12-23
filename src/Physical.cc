#include "pch.h"
#include "math_util.h"
#include "Matrix4.h"
#include "Physical.h"

Physical::Physical()
    : _scale(1.0f)
{
}

Physical::Physical(const Position& position, float scale)
    : _position(position), _scale(scale)
{
}

Physical::~Physical() throw()
{
}

void Physical::rotate(float angle, const Vector3& around)
{
    Quaternion q(Quaternion::new_axis(angle, around));
    _orientation = q * _orientation;
}

void Physical::pitch(float angle)
{
    // need to pitch against our local x-axis
    Quaternion q(Quaternion::new_axis(angle, Vector3(1.0f, 0.0f, 0.0f)));
    _orientation = _orientation * q;
}

void Physical::yaw(float angle)
{
    rotate(angle, Vector3(0.0f, 1.0f, 0.0f));
}

void Physical::roll(float angle)
{
    rotate(angle, Vector3(0.0f, 0.0f, 1.0f));
}

void Physical::transform(Matrix4& matrix) const
{
    matrix.translate(_position);
    matrix *= _orientation.matrix();
    matrix.uniform_scale(_scale);
}

void Physical::think(double dt)
{
    if(!on_think(dt)) {
        return;
    }

    // adjust our velocity by our acceleration
    _velocity += _acceleration * dt;

    // apply the velocity to our position
    _position += _velocity * dt;
}

std::string Physical::str() const
{
    std::stringstream ss;
    ss << "Physical(p:" << _position.str() << ", o:" << _orientation.str() << ", s:" << _scale << ")";
    return ss.str();
}
