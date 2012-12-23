#include "pch.h"
#include "math_util.h"
#include "Matrix4.h"

Matrix4 Matrix4::perspective(float fov, float aspect, float n, float f)
{
    const float x = (DEG_RAD(fov) / 2.0f);
    const float t = std::tan(M_PI_2 - x);

    Matrix4 perspective;
    perspective[0] = t / aspect;
    perspective[5] = t;
    perspective[10] = (f + n) / (n - f);
    perspective[11] = (2.0f * f * n) / (n - f);
    perspective[14] = -1.0f;
    return perspective;
}

Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float n, float f)
{
    const float tx = -((right + left) / (right - left));
    const float ty = -((top + bottom) / (top - bottom));
    const float tz = -((f + n) / (f - n));

    Matrix4 orthographic;
    orthographic[0] = 2.0f / (right - left);
    orthographic[3] = tx;
    orthographic[5] = 2.0f / (top - bottom);
    orthographic[7] = ty;
    orthographic[10] = -2.0f / (f - n);
    orthographic[11] = tz;
    return orthographic;
}

Matrix4 Matrix4::frustum(float left, float right, float bottom, float top, float n, float f)
{
    Matrix4 frustum;
    frustum[0] = (2.0f * n) / (right - left);
    frustum[2] = (right + left) / (right - left);
    frustum[5] = (2.0f * n) / (top - bottom);
    frustum[6] = (top + bottom) / (top - bottom);
    frustum[10] = -((f + n) / (f - n));
    frustum[11] = -((2.0f * f * n) / (f - n));
    frustum[14] = -1.0f;
    return frustum;
}

Matrix4 Matrix4::infinite_frustum(float left, float right, float bottom, float top, float n)
{
    Matrix4 frustum;
    frustum[0] = (2.0f * n) / (right - left);
    frustum[2] = (right + left) / (right - left);
    frustum[5] = (2.0f * n) / (top - bottom);
    frustum[6] = (top + bottom) / (top - bottom);
    frustum[10] = -1;
    frustum[11] = -2.0f * n;
    frustum[14] = -1.0f;
    return frustum;
}

Matrix4& Matrix4::rotate(float angle, const Vector3& around)
{
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const Vector3 a(around.normalized());

    const float c1 = 1.0f - c;

    Matrix4 matrix;
    matrix._m[0] = c + c1 * a.x() * a.x();
    matrix._m[1] = c1 * a.x() * a.y() - s * a.z();
    matrix._m[2] = c1 * a.x() * a.z() + s * a.y();

    matrix._m[4] = c1 * a.x() * a.y() + s * a.z();
    matrix._m[5] = c + c1 * a.y() * a.y();
    matrix._m[6] = c1 * a.x() * a.z() - s * a.x();

    matrix._m[8] = c1 * a.x() * a.z() - s * a.y();
    matrix._m[9] = c1 * a.y() * a.z() + s * a.x();
    matrix._m[10] = c + c1 * a.z() * a.z();
    return (*this) *= matrix;
}

std::string Matrix4::str() const
{
    std::stringstream ss;
    ss << _m[0] << " " << _m[1] << " " << _m[2] << " " << _m[3] << std::endl
       << _m[4] << " " << _m[5] << " " << _m[6] << " " << _m[7] << std::endl
       << _m[8] << " " << _m[9] << " " << _m[10] << " " << _m[11] << std::endl
       << _m[12] << " " << _m[13] << " " << _m[14] << " " << _m[15];
    return ss.str();
}
