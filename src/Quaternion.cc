#include "pch.h"
#include "Quaternion.h"

Quaternion Quaternion::new_euler(float pitch, float yaw, float roll)
{
    const Quaternion qpitch(std::cos(pitch / 2.0f), Vector3(std::sin(pitch / 2.0f), 0.0f, 0.0f));
    const Quaternion qyaw(std::cos(yaw / 2.0f), Vector3(0.0f, std::sin(yaw / 2.0f), 0.0f));
    const Quaternion qroll(std::cos(roll / 2.0f), Vector3(0.0f, 0.0f, std::sin(roll / 2.0f)));
    return qpitch * qyaw * qroll;
}

Quaternion Quaternion::new_axis(float angle, const Vector3& axis)
{
    return Quaternion(std::cos(angle / 2.0f), std::sin(angle / 2.0f) * axis.normalized()).normalized();
}

Quaternion::Quaternion(const Matrix4& matrix)
{
    // Game Engine Architecture, section 4.4
    const float trace = matrix[0] + matrix[5] + matrix[10];
    if(trace > 0.0f) {
        // diagonal is positive
        const float s = std::sqrt(trace + 1.0f);
        _scalar = s * 0.5f;

        const float t = 0.5f / s;
        _vector.x((matrix[9] - matrix[6]) * t);
        _vector.y((matrix[2] - matrix[8]) * t);
        _vector.z((matrix[4] - matrix[1]) * t);
    } else {
        // diagonal is negative
        int i=0;
        if(matrix[5] > matrix[0]) i = 1;
        if(matrix[10] > matrix(i, i)) i = 2;

        static const int NEXT[3] = { 1, 2, 0 };
        const int j = NEXT[i];
        const int k = NEXT[j];

        const float s = std::sqrt((matrix(i, i) - (matrix(j, j) + matrix(k, k))) + 1.0f);
        (*this)[i] = s * 0.5f;

        float t = 0.0f;
        if(0.0f != s) t = 0.5f / s;
        else t = s;

        (*this)[3] = (matrix(k, j) - matrix(j, k)) * t;
        (*this)[j] = (matrix(j, i) + matrix(i, j)) * t;
        (*this)[k] = (matrix(k, i) + matrix(i, k)) * t;
    }
}

Vector3 Quaternion::axis() const
{
    const float s = std::sqrt(1.0f - (_scalar * _scalar));
    if(std::fabs(s) >= 0.001f) {
        return _vector / s;
    }
    return Vector3(1.0f, 0.0f, 0.0f);
}

Quaternion Quaternion::slerp(const Quaternion& rhs, double t) const
{
    if(t <= 0.0) {
        return *this;
    } else if(t >= 1.0) {
        return rhs;
    }

    // see if rotating the other direction is a shorter path
    Quaternion temp(rhs);
    float angle = (*this) ^ rhs;
    if(angle < 0.0f) {
        temp = -temp;
        angle = -angle;
    }

    if(std::abs(angle) >= 1.0f) {
        // angle between is 0, so nothing to do here
        return *this;
    }

    // sin(angle)
    const float sangle = std::sqrt(1.0f - angle * angle);
    if(std::fabs(sangle) < 0.001f) {
        // angle is 180 degrees
        return Quaternion(_scalar * 0.5f + temp._scalar * 0.5f, _vector * 0.5f + temp._vector * 0.5f);
    }

    const float half_angle = std::acos(angle);
    const float ratioa = std::sin((1.0 - t) * half_angle) / sangle;
    const float ratiob = std::sin(t * half_angle) / sangle;
    return Quaternion(_scalar * ratioa + temp._scalar * ratiob, _vector * ratioa + temp._vector * ratiob).normalize();
}

Matrix4 Quaternion::matrix() const
{
    const Quaternion n(normalized());

    const float xw = n._vector.x() * n._scalar;
    const float xx = n._vector.x() * n._vector.x();
    const float xy = n._vector.x() * n._vector.y();
    const float xz = n._vector.x() * n._vector.z();

    const float yw = n._vector.y() * n._scalar;
    const float yy = n._vector.y() * n._vector.y();
    const float yz = n._vector.y() * n._vector.z();

    const float zw = n._vector.z() * n._scalar;
    const float zz = n._vector.z() * n._vector.z();

    Matrix4 m;
    m[0] = 1.0f - 2.0f * (yy + zz);
    m[1] = 2.0f * (xy + zw);
    m[2] = 2.0f * (xz - yw);

    m[4] = 2.0f * (xy - zw);
    m[5] = 1.0f - 2.0f * (xx + zz);
    m[6] = 2.0f * (yz + xw);

    m[8] = 2.0f * (xz + yw);
    m[9] = 2.0f * (yz - xw);
    m[10] = 1.0f - 2.0f * (xx + yy);
    return m;
}

std::string Quaternion::str() const
{
    std::stringstream ss;
    ss << "Quaternion(" << _scalar << ", " << _vector.str() << ")";
    return ss.str();
}
