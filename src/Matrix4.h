#if !defined __MATRIX4_H__
#define __MATRIX4_H__

#include "Matrix3.h"
#include "Vector.h"

class Matrix4
{
public:
    static Matrix4 perspective(float fov, float aspect, float near=0.1f, float far=1000.0f);
    static Matrix4 orthographic(float left, float right, float bottom, float top, float near=-1.0f, float far=1.0f);
    static Matrix4 frustum(float left, float right, float bottom, float top, float near=0.1f, float far=1000.0f);
    static Matrix4 infinite_frustum(float left, float right, float bottom, float top, float near=0.1f);

public:
    Matrix4() { identity(); }
    Matrix4(const Matrix4& matrix) { std::memcpy(_m, matrix._m, 16 * sizeof(float)); }

    // matrix should be a 16 float array
    // NOTE: no bounds checking or anything is done here
    explicit Matrix4(const float* const matrix) { std::memcpy(_m, matrix, 16 * sizeof(float)); }

    virtual ~Matrix4() throw() {}

public:
    const float* array() const { return _m; }

    Vector4 row(size_t r) const { return Vector4(_m[(r * 4) + 0], _m[(r * 4) + 1], _m[(r * 4) + 2], _m[(r * 4) + 3]); }

    Matrix4& identity()
    {
        _m[0]  = 1.0f; _m[1]  = 0.0f; _m[2]  = 0.0f; _m[3]  = 0.0f;
        _m[4]  = 0.0f; _m[5]  = 1.0f; _m[6]  = 0.0f; _m[7]  = 0.0f;
        _m[8]  = 0.0f; _m[9]  = 0.0f; _m[10] = 1.0f; _m[11] = 0.0f;
        _m[12] = 0.0f; _m[13] = 0.0f; _m[14] = 0.0f; _m[15] = 1.0f;
        return *this;
    }

    float determinant() const
    {
        return _m[3] * _m[6] * _m[9] * _m[12] - _m[2] * _m[7] * _m[9] * _m[12] - _m[3] * _m[5] * _m[10] * _m[12]
            + _m[1] * _m[7] * _m[10] * _m[12] + _m[2] * _m[5] * _m[11] * _m[12] - _m[1] * _m[6] * _m[11] * _m[12]
            - _m[3] * _m[6] * _m[8] * _m[13] + _m[2] * _m[7] * _m[8] * _m[13] + _m[3] * _m[4] * _m[10] * _m[13]
            - _m[0] * _m[7] * _m[10] * _m[13] - _m[2] * _m[4] * _m[11] * _m[13] + _m[0] * _m[6] * _m[11] * _m[13]
            + _m[3] * _m[5] * _m[8] * _m[14] - _m[1] * _m[7] * _m[8] * _m[14] - _m[3] * _m[4] * _m[9] * _m[14]
            + _m[0] * _m[7] * _m[9] * _m[14] + _m[1] * _m[4] * _m[11] * _m[14] - _m[0] * _m[5] * _m[11] * _m[14]
            - _m[2] * _m[5] * _m[8] * _m[15] + _m[1] * _m[6] * _m[8] * _m[15] + _m[2] * _m[4] * _m[9] * _m[15]
            - _m[0] * _m[6] * _m[9] * _m[15] - _m[1] * _m[4] * _m[10] * _m[15] + _m[0] * _m[5] * _m[10] * _m[15];
    }

    // transpose of the inverse of
    // the upper leftmost 3x3 of this matrix
    Matrix3 normal_matrix() const
    {
        float m[9];
        m[0] = _m[0];
        m[1] = _m[1];
        m[2] = _m[2];
        m[3] = _m[4];
        m[4] = _m[5];
        m[5] = _m[6];
        m[6] = _m[8];
        m[7] = _m[9];
        m[8] = _m[10];

        Matrix3 mat(m);
        return ~(-mat);
    }

    Matrix4& translate(const Position& position)
    {
        Matrix4 matrix;
        matrix._m[3] = position.x();
        matrix._m[7] = position.y();
        matrix._m[11] = position.z();
        return (*this) *= matrix;
    }

    // angle is in radians
    Matrix4& rotate(float angle, const Vector3& around);
    Matrix4& pitch(float angle) { return rotate(angle, Vector3(1.0f, 0.0f, 0.0f)); }
    Matrix4& yaw(float angle) { return rotate(angle, Vector3(0.0f, 1.0f, 0.0f)); }
    Matrix4& roll(float angle) { return rotate(angle, Vector3(0.0f, 0.0f, 1.0f)); }
    Matrix4& rotation_identity()
    {
        _m[0] = 1.0f; _m[1] = 0.0f; _m[2] = 0.0f;
        _m[4] = 0.0f; _m[5] = 1.0f; _m[6] = 0.0f;
        _m[8] = 0.0f; _m[9] = 0.0f; _m[10] = 0.0f;
        return *this;
    }

    /*Matrix4& scale(const Vector3& scale)
    {
        Matrix4 matrix;
        matrix._m[0] = scale.x();
        matrix._m[5] = scale.y();
        matrix._m[10] = scale.z();
        return (*this) *= matrix;
    }*/

    Matrix4& uniform_scale(float scale)
    {
        Matrix4 matrix;
        matrix._m[0] = scale;
        matrix._m[5] = scale;
        matrix._m[10] = scale;
        return (*this) *= matrix;
    }

    std::string str() const;

public:
    float& operator[](int index) { return _m[index]; }
    const float& operator[](int index) const { return _m[index]; }
    float& operator()(unsigned int r, unsigned int c) { return _m[(r * 4) + c]; }
    const float& operator()(unsigned int r, unsigned int c) const { return _m[(r * 4) + c]; }

    Matrix4& operator=(const Matrix4& rhs) { std::memcpy(_m, rhs._m, 16 * sizeof(float)); return *this; }

    Matrix4 operator+(const Matrix4& rhs) const
    {
        Matrix4 m;
#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);

        __m128 B1 = _mm_loadu_ps(rhs._m + 0);
        __m128 B2 = _mm_loadu_ps(rhs._m + 4);
        __m128 B3 = _mm_loadu_ps(rhs._m + 8);
        __m128 B4 = _mm_loadu_ps(rhs._m + 12);

        _mm_storeu_ps(m._m + 0, _mm_add_ps(A1, B1));
        _mm_storeu_ps(m._m + 4, _mm_add_ps(A2, B2));
        _mm_storeu_ps(m._m + 8, _mm_add_ps(A3, B3));
        _mm_storeu_ps(m._m + 12, _mm_add_ps(A4, B4));
#else
        m._m[0] = _m[0] + rhs._m[0];
        m._m[1] = _m[1] + rhs._m[1];
        m._m[2] = _m[2] + rhs._m[2];
        m._m[3] = _m[3] + rhs._m[3];
        m._m[4] = _m[4] + rhs._m[4];
        m._m[5] = _m[5] + rhs._m[5];
        m._m[6] = _m[6] + rhs._m[6];
        m._m[7] = _m[7] + rhs._m[7];
        m._m[8] = _m[8] + rhs._m[8];
        m._m[9] = _m[9] + rhs._m[9];
        m._m[10] = _m[10] + rhs._m[10];
        m._m[11] = _m[11] + rhs._m[11];
        m._m[12] = _m[12] + rhs._m[12];
        m._m[13] = _m[13] + rhs._m[13];
        m._m[14] = _m[14] + rhs._m[14];
        m._m[15] = _m[15] + rhs._m[15];
#endif
        return m;
    }

    Matrix4& operator+=(const Matrix4& rhs)
    {
#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);

        __m128 B1 = _mm_loadu_ps(rhs._m + 0);
        __m128 B2 = _mm_loadu_ps(rhs._m + 4);
        __m128 B3 = _mm_loadu_ps(rhs._m + 8);
        __m128 B4 = _mm_loadu_ps(rhs._m + 12);

        _mm_storeu_ps(_m + 0, _mm_add_ps(A1, B1));
        _mm_storeu_ps(_m + 4, _mm_add_ps(A2, B2));
        _mm_storeu_ps(_m + 8, _mm_add_ps(A3, B3));
        _mm_storeu_ps(_m + 12, _mm_add_ps(A4, B4));
        return *this;
#else
        return ((*this) = (*this) + rhs);
#endif
    }

    Matrix4 operator-(const Matrix4& rhs) const
    {
        Matrix4 m;
#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);

        __m128 B1 = _mm_loadu_ps(rhs._m + 0);
        __m128 B2 = _mm_loadu_ps(rhs._m + 4);
        __m128 B3 = _mm_loadu_ps(rhs._m + 8);
        __m128 B4 = _mm_loadu_ps(rhs._m + 12);

        _mm_storeu_ps(m._m + 0, _mm_sub_ps(A1, B1));
        _mm_storeu_ps(m._m + 4, _mm_sub_ps(A2, B2));
        _mm_storeu_ps(m._m + 8, _mm_sub_ps(A3, B3));
        _mm_storeu_ps(m._m + 12, _mm_sub_ps(A4, B4));
#else
        m._m[0] = _m[0] - rhs._m[0];
        m._m[1] = _m[1] - rhs._m[1];
        m._m[2] = _m[2] - rhs._m[2];
        m._m[3] = _m[3] - rhs._m[3];
        m._m[4] = _m[4] - rhs._m[4];
        m._m[5] = _m[5] - rhs._m[5];
        m._m[6] = _m[6] - rhs._m[6];
        m._m[7] = _m[7] - rhs._m[7];
        m._m[8] = _m[8] - rhs._m[8];
        m._m[9] = _m[9] - rhs._m[9];
        m._m[10] = _m[10] - rhs._m[10];
        m._m[11] = _m[11] - rhs._m[11];
        m._m[12] = _m[12] - rhs._m[12];
        m._m[13] = _m[13] - rhs._m[13];
        m._m[14] = _m[14] - rhs._m[14];
        m._m[15] = _m[15] - rhs._m[15];
#endif
        return m;
    }

    Matrix4& operator-=(const Matrix4& rhs)
    {
#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);

        __m128 B1 = _mm_loadu_ps(rhs._m + 0);
        __m128 B2 = _mm_loadu_ps(rhs._m + 4);
        __m128 B3 = _mm_loadu_ps(rhs._m + 8);
        __m128 B4 = _mm_loadu_ps(rhs._m + 12);

        _mm_storeu_ps(_m + 0, _mm_sub_ps(A1, B1));
        _mm_storeu_ps(_m + 4, _mm_sub_ps(A2, B2));
        _mm_storeu_ps(_m + 8, _mm_sub_ps(A3, B3));
        _mm_storeu_ps(_m + 12, _mm_sub_ps(A4, B4));
        return *this;
#else
        return ((*this) = (*this) - rhs);
#endif
    }

    Matrix4 operator*(const Matrix4& rhs) const
    {
        Matrix4 n;
        n._m[0]  = _m[0]  * rhs._m[0] + _m[1]  * rhs._m[4] + _m[2]  * rhs._m[8]  + _m[3]  * rhs._m[12];
        n._m[1]  = _m[0]  * rhs._m[1] + _m[1]  * rhs._m[5] + _m[2]  * rhs._m[9]  + _m[3]  * rhs._m[13];
        n._m[2]  = _m[0]  * rhs._m[2] + _m[1]  * rhs._m[6] + _m[2]  * rhs._m[10] + _m[3]  * rhs._m[14];
        n._m[3]  = _m[0]  * rhs._m[3] + _m[1]  * rhs._m[7] + _m[2]  * rhs._m[11] + _m[3]  * rhs._m[15];
        n._m[4]  = _m[4]  * rhs._m[0] + _m[5]  * rhs._m[4] + _m[6]  * rhs._m[8]  + _m[7]  * rhs._m[12];
        n._m[5]  = _m[4]  * rhs._m[1] + _m[5]  * rhs._m[5] + _m[6]  * rhs._m[9]  + _m[7]  * rhs._m[13];
        n._m[6]  = _m[4]  * rhs._m[2] + _m[5]  * rhs._m[6] + _m[6]  * rhs._m[10] + _m[7]  * rhs._m[14];
        n._m[7]  = _m[4]  * rhs._m[3] + _m[5]  * rhs._m[7] + _m[6]  * rhs._m[11] + _m[7]  * rhs._m[15];
        n._m[8]  = _m[8]  * rhs._m[0] + _m[9]  * rhs._m[4] + _m[10] * rhs._m[8]  + _m[11] * rhs._m[12];
        n._m[9]  = _m[8]  * rhs._m[1] + _m[9]  * rhs._m[5] + _m[10] * rhs._m[9]  + _m[11] * rhs._m[13];
        n._m[10] = _m[8]  * rhs._m[2] + _m[9]  * rhs._m[6] + _m[10] * rhs._m[10] + _m[11] * rhs._m[14];
        n._m[11] = _m[8]  * rhs._m[3] + _m[9]  * rhs._m[7] + _m[10] * rhs._m[11] + _m[11] * rhs._m[15];
        n._m[12] = _m[12] * rhs._m[0] + _m[13] * rhs._m[4] + _m[14] * rhs._m[8]  + _m[15] * rhs._m[12];
        n._m[13] = _m[12] * rhs._m[1] + _m[13] * rhs._m[5] + _m[14] * rhs._m[9]  + _m[15] * rhs._m[13];
        n._m[14] = _m[12] * rhs._m[2] + _m[13] * rhs._m[6] + _m[14] * rhs._m[10] + _m[15] * rhs._m[14];
        n._m[15] = _m[12] * rhs._m[3] + _m[13] * rhs._m[7] + _m[14] * rhs._m[11] + _m[15] * rhs._m[15];
        return n;
    }

    Matrix4& operator*=(const Matrix4& rhs)
    {
        return ((*this) = (*this) * rhs);
    }

    Vector4 operator*(const Vector4& rhs) const
    {
        Vector4 v;
/*#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);

        __m128 X = _mm_set1_ps(rhs.x());
        __m128 Y = _mm_set1_ps(rhs.y());
        __m128 Z = _mm_set1_ps(rhs.z());
        __m128 W = _mm_set1_ps(rhs.w());

        __m128 R11 = _mm_mul_ps(A1, X);
        __m128 R21 = _mm_mul_ps(A2, Y);
        __m128 R31 = _mm_mul_ps(A3, Z);
        __m128 R41 = _mm_mul_ps(A4, W);

        __m128 R12 = _mm_hadd_ps(R11, R11);
        __m128 R22 = _mm_hadd_ps(R21, R21);
        __m128 R32 = _mm_hadd_ps(R31, R31);
        __m128 R42 = _mm_hadd_ps(R41, R41);

        _mm_storeu_ps(v._value + 0, _mm_hadd_ps(R12, R12));
        _mm_storeu_ps(v._value + 1, _mm_hadd_ps(R22, R22));
        _mm_storeu_ps(v._value + 2, _mm_hadd_ps(R32, R32));
        _mm_storeu_ps(v._value + 3, _mm_hadd_ps(R42, R42));
#else*/
        v.x(rhs * (_m + 0));
        v.y(rhs * (_m + 4));
        v.z(rhs * (_m + 8));
        v.w(rhs * (_m + 12));
//#endif
        return v;
    }

    Matrix4 operator*(float rhs) const
    {
        Matrix4 m;
#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);
        __m128 S = _mm_set1_ps(rhs);

        _mm_storeu_ps(m._m + 0, _mm_mul_ps(A1, S));
        _mm_storeu_ps(m._m + 4, _mm_mul_ps(A2, S));
        _mm_storeu_ps(m._m + 8, _mm_mul_ps(A3, S));
        _mm_storeu_ps(m._m + 12, _mm_mul_ps(A4, S));
#else
        m._m[0] = _m[0] * rhs;
        m._m[1] = _m[1] * rhs;
        m._m[2] = _m[2] * rhs;
        m._m[3] = _m[3] * rhs;
        m._m[4] = _m[4] * rhs;
        m._m[5] = _m[5] * rhs;
        m._m[6] = _m[6] * rhs;
        m._m[7] = _m[7] * rhs;
        m._m[8] = _m[8] * rhs;
        m._m[9] = _m[9] * rhs;
        m._m[10] = _m[10] * rhs;
        m._m[11] = _m[11] * rhs;
        m._m[12] = _m[12] * rhs;
        m._m[13] = _m[13] * rhs;
        m._m[14] = _m[14] * rhs;
        m._m[15] = _m[15] * rhs;
#endif
        return m;
    }

    Matrix4& operator*=(float rhs)
    {
#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);
        __m128 S = _mm_set1_ps(rhs);

        _mm_storeu_ps(_m + 0, _mm_mul_ps(A1, S));
        _mm_storeu_ps(_m + 4, _mm_mul_ps(A2, S));
        _mm_storeu_ps(_m + 8, _mm_mul_ps(A3, S));
        _mm_storeu_ps(_m + 12, _mm_mul_ps(A4, S));
        return *this;
#else
        return ((*this) = (*this) * rhs);
#endif
    }

    Matrix4 operator/(float rhs) const
    {
        Matrix4 m;
#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);
        __m128 S = _mm_set1_ps(rhs);

        _mm_storeu_ps(m._m + 0, _mm_div_ps(A1, S));
        _mm_storeu_ps(m._m + 4, _mm_div_ps(A2, S));
        _mm_storeu_ps(m._m + 8, _mm_div_ps(A3, S));
        _mm_storeu_ps(m._m + 12, _mm_div_ps(A4, S));
#else
        m._m[0] = _m[0] / rhs;
        m._m[1] = _m[1] / rhs;
        m._m[2] = _m[2] / rhs;
        m._m[3] = _m[3] / rhs;
        m._m[4] = _m[4] / rhs;
        m._m[5] = _m[5] / rhs;
        m._m[6] = _m[6] / rhs;
        m._m[7] = _m[7] / rhs;
        m._m[8] = _m[8] / rhs;
        m._m[9] = _m[9] / rhs;
        m._m[10] = _m[10] / rhs;
        m._m[11] = _m[11] / rhs;
        m._m[12] = _m[12] / rhs;
        m._m[13] = _m[13] / rhs;
        m._m[14] = _m[14] / rhs;
        m._m[15] = _m[15] / rhs;
#endif
        return m;
    }

    Matrix4& operator/=(float rhs)
    {
#if defined USE_SSE
        __m128 A1 = _mm_loadu_ps(_m + 0);
        __m128 A2 = _mm_loadu_ps(_m + 4);
        __m128 A3 = _mm_loadu_ps(_m + 8);
        __m128 A4 = _mm_loadu_ps(_m + 12);
        __m128 S = _mm_set1_ps(rhs);

        _mm_storeu_ps(_m + 0, _mm_div_ps(A1, S));
        _mm_storeu_ps(_m + 4, _mm_div_ps(A2, S));
        _mm_storeu_ps(_m + 8, _mm_div_ps(A3, S));
        _mm_storeu_ps(_m + 12, _mm_div_ps(A4, S));
        return *this;
#else
        return ((*this) = (*this) / rhs);
#endif
    }

    bool operator==(const Matrix4& rhs) const
    {
        return _m[0] == rhs._m[0] && _m[1] == rhs._m[1] && _m[2] == rhs._m[2] && _m[3] == rhs._m[3]
            && _m[4] == rhs._m[4] && _m[5] == rhs._m[5] && _m[6] == rhs._m[6] && _m[7] == rhs._m[7]
            && _m[8] == rhs._m[8] && _m[9] == rhs._m[9] && _m[10] == rhs._m[10] && _m[11] == rhs._m[11]
            && _m[12] == rhs._m[12] && _m[13] == rhs._m[13] && _m[14] == rhs._m[14] && _m[15] == rhs._m[15];
    }

    bool operator!=(const Matrix4& rhs) const { return !((*this) == rhs); }

    // inverse
    // NOTE: this does not verify that determinant() != 0!!!
    Matrix4 operator-() const
    {
        Matrix4 m;
        m._m[0] = _m[6] * _m[11] * _m[13] - _m[7] * _m[10] * _m[13] + _m[7] * _m[9] * _m[14]
            - _m[5] * _m[11] * _m[14] - _m[6] * _m[9] * _m[15] + _m[5] * _m[10] * _m[15];
        m._m[1] = _m[3] * _m[10] * _m[13] - _m[2] * _m[11] * _m[13] - _m[3] * _m[9] * _m[14]
            + _m[1] * _m[11] * _m[14] + _m[2] * _m[9] * _m[15] - _m[1] * _m[10] * _m[15];
        m._m[2] = _m[2] * _m[7] * _m[13] - _m[3] * _m[6] * _m[13] + _m[3] * _m[5] * _m[14]
            - _m[1] * _m[7] * _m[14] - _m[2] * _m[5] * _m[15] + _m[1] * _m[6] * _m[15];
        m._m[3] = _m[3] * _m[6] * _m[9] - _m[2] * _m[7] * _m[9] - _m[3] * _m[5] * _m[10]
            + _m[1] * _m[7] * _m[10] + _m[2] * _m[5] * _m[11] - _m[1] * _m[6] * _m[11];
        m._m[4] = _m[7] * _m[10] * _m[12] - _m[6] * _m[11] * _m[12] - _m[7] * _m[8] * _m[14]
            + _m[4] * _m[11] * _m[14] + _m[6] * _m[8] * _m[15] - _m[4] * _m[10] * _m[15];
        m._m[5] = _m[2] * _m[11] * _m[12] - _m[3] * _m[10] * _m[12] + _m[3] * _m[8] * _m[14]
            - _m[0] * _m[11] * _m[14] - _m[2] * _m[8] * _m[15] + _m[0] * _m[10] * _m[15];
        m._m[6] = _m[3] * _m[6] * _m[12] - _m[2] * _m[7] * _m[12] - _m[3] * _m[4] * _m[14]
            + _m[0] * _m[7] * _m[14] + _m[2] * _m[4] * _m[15] - _m[0] * _m[6] * _m[15];
        m._m[7] = _m[2] * _m[7] * _m[8] - _m[3] * _m[6] * _m[8] + _m[3] * _m[4] * _m[10]
            - _m[0] * _m[7] * _m[10] - _m[2] * _m[4] * _m[11] + _m[0] * _m[6] * _m[11];
        m._m[8] = _m[5] * _m[11] * _m[12] - _m[7] * _m[9] * _m[12] + _m[7] * _m[8] * _m[13]
            - _m[4] * _m[11] * _m[13] - _m[5] * _m[8] * _m[15] + _m[4] * _m[9] * _m[15];
        m._m[9] = _m[3] * _m[9] * _m[12] - _m[1] * _m[11] * _m[12] - _m[3] * _m[8] * _m[13]
            + _m[0] * _m[11] * _m[13] + _m[1] * _m[8] * _m[15] - _m[0] * _m[9] * _m[15];
        m._m[10] = _m[1] * _m[7] * _m[12] - _m[3] * _m[5] * _m[12] + _m[3] * _m[4] * _m[13]
            - _m[0] * _m[7] * _m[13] - _m[1] * _m[4] * _m[15] + _m[0] * _m[5] * _m[15];
        m._m[11] = _m[3] * _m[5] * _m[8] - _m[1] * _m[7] * _m[8] - _m[3] * _m[4] * _m[9]
            + _m[0] * _m[7] * _m[9] + _m[1] * _m[4] * _m[11] - _m[0] * _m[5] * _m[11];
        m._m[12] = _m[6] * _m[9] * _m[12] - _m[5] * _m[10] * _m[12] - _m[6] * _m[8] * _m[13]
            + _m[4] * _m[10] * _m[13] + _m[5] * _m[8] * _m[14] - _m[4] * _m[9] * _m[14];
        m._m[13] = _m[1] * _m[10] * _m[12] - _m[2] * _m[9] * _m[12] + _m[2] * _m[8] * _m[13]
            - _m[0] * _m[10] * _m[13] - _m[1] * _m[8] * _m[14] + _m[0] * _m[9] * _m[14];
        m._m[14] = _m[2] * _m[5] * _m[12] - _m[1] * _m[6] * _m[12] - _m[2] * _m[4] * _m[13]
            + _m[0] * _m[6] * _m[13] + _m[1] * _m[4] * _m[14] - _m[0] * _m[5] * _m[14];
        m._m[15] = _m[1] * _m[6] * _m[8] - _m[2] * _m[5] * _m[8] + _m[2] * _m[4] * _m[9]
            - _m[0] * _m[6] * _m[9] - _m[1] * _m[4] * _m[10] + _m[0] * _m[5] * _m[10];
        return (1.0f/determinant()) * m;
    }

    // transposition
    Matrix4 operator~() const
    {
        Matrix4 t;
        t._m[0] = _m[0];
        t._m[4] = _m[1];
        t._m[8] = _m[2];
        t._m[12] = _m[3];
        t._m[1] = _m[4];
        t._m[5] = _m[5];
        t._m[9] = _m[6];
        t._m[13] = _m[7];
        t._m[2] = _m[8];
        t._m[6] = _m[9];
        t._m[10] = _m[10];
        t._m[14] = _m[11];
        t._m[3] = _m[12];
        t._m[7] = _m[13];
        t._m[11] = _m[14];
        t._m[15] = _m[15];
        return t;
    }

public:
    friend Matrix4 operator*(float lhs, const Matrix4& rhs) { return rhs * lhs; }

private:
    ALIGN(16) float _m[16];
};

#endif
