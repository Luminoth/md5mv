#if !defined __VECTOR_H__
#define __VECTOR_H__

#include "math_util.h"

/*
In order to simplify code and avoid unnecessary duplication,
this class represents a general, size-agnostic Vector (based on a 4-dimensional vector)
with specialization where required. For less than 4-dimensional
vectors, the unused components should be constrained to 0.

Care should be taken to avoid misusing this class and to avoid
mixing sizes where it is mathematically inappropriate to do so.
*/

class Vector
{
public:
    Vector() { _value[0] = _value[1] = _value[2] = _value[3] = 0.0f; }

    Vector(float x, float y)
    {
        _value[0] = x;
        _value[1] = y;
        _value[2] = _value[3] = 0.0f;
    }

    Vector(float x, float y, float z)
    {
        _value[0] = x;
        _value[1] = y;
        _value[2] = z;
        _value[3] = 0.0f;
    }

    Vector(const Vector& v, float w)
    {
        _value[0] = v.x();
        _value[1] = v.y();
        _value[2] = v.z();
        _value[3] = w;
    }

    Vector(float x, float y, float z, float w)
    {
        _value[0] = x;
        _value[1] = y;
        _value[2] = z;
        _value[3] = w;
    }

    // NOTE: v must have at least 4 floats
    // regardless of the "size" of the vector
    Vector(const int* const v)
    {
        _value[0] = v[0];
        _value[1] = v[1];
        _value[2] = v[2];
        _value[3] = v[3];
    }

    // NOTE: v must have at least 4 floats
    // regardless of the "size" of the vector
    Vector(const float* const v)
    {
        _value[0] = v[0];
        _value[1] = v[1];
        _value[2] = v[2];
        _value[3] = v[3];
    }

    virtual ~Vector() throw() {}

public:
    void x(float x) { _value[0] = x; }
    float x() const { return _value[0]; }

    void y(float y) { _value[1] = y; }
    float y() const { return _value[1]; }

    void z(float z) { _value[2] = z; }
    float z() const { return _value[2]; }

    void w(float w) { _value[3] = w; }
    float w() const { return _value[3]; }

    const float* array() const { return _value; }

    void zero() { _value[0] = _value[1] = _value[2] = _value[3] = 0.0f; }

    bool is_zero() const
    {
#if defined USE_SSE
        float d;
        __m128 A = _mm_loadu_ps(_value);
        __m128 S = _mm_set_ss(0.0f);
        _mm_store_ss(&d, _mm_cmpeq_ss(A, S));
        return d == 0xffffffff;
#else
        return x() == 0.0f && y() == 0.0f && z() == 0.0f && w() == 0.0f;
#endif
    }

public:
    float length_squared() const
    {
#if defined USE_SSE
        float d;
        __m128 A = _mm_loadu_ps(_value);
        __m128 R1 = _mm_mul_ps(A, A);
        __m128 R2 = _mm_hadd_ps(R1, R1);
        _mm_store_ss(&d, _mm_hadd_ps(R2, R2));
        return d;
#else
        return (*this) * (*this);
#endif
    }

    float length() const
    {
#if defined USE_SSE
        float d;
        __m128 A = _mm_loadu_ps(_value);
        __m128 R1 = _mm_mul_ps(A, A);
        __m128 R2 = _mm_hadd_ps(R1, R1);
        _mm_store_ss(&d, _mm_sqrt_ss(_mm_hadd_ps(R2, R2)));
        return d;
#else
        return std::sqrt(length_squared());
#endif
    }

    Vector& normalize() { return (*this) *= invsqrt(length_squared()); }
    Vector normalized() const { return (*this) * invsqrt(length_squared()); }

    bool colinear(const Vector& rhs) const { return (*this) * rhs == (length() * rhs.length()); }
    bool colinear_opposite(const Vector& rhs) const { return (*this) * rhs == -(length() * rhs.length()); }

    // TODO: this probably should use an epsilon
    bool perpendicular(const Vector& rhs) const { return (*this) * rhs == 0.0f; }

    bool same_direction(const Vector& rhs) const { return (*this) * rhs > 0.0f; }
    bool opposite_direction(const Vector& rhs) const { return (*this) * rhs < 0.0f; }

    float manhattan_normal() const { return std::fabs(x()) + std::fabs(y()) + std::fabs(z()) + std::fabs(w()); }

    float infinite_normal() const
    {
        float ret = std::fabs(x());
        ret = MAX(ret, std::fabs(y()));
        ret = MAX(ret, std::fabs(z()));
        return MAX(ret, std::fabs(w()));
    }

    float distance(const Vector& other) const { return (other - (*this)).length(); }
    float distance_squared(const Vector& other) const { return (other - (*this)).length_squared(); }

    // angle wrt the x/y plane
    float angle() const { return std::atan(y() / x()); }

    float angle_radians(const Vector& other) const { return std::acos((*this * other) / (length() * other.length())); }
    float angle_degrees(const Vector& other) const { return RAD_DEG(angle_radians(other)); }

    // interpolates between this point and rhs
    // t must be in [0, 1]
    Vector lerp(const Vector& rhs, double t) const { return (1.0 - t) * (*this) + t * rhs; }

    // multiplies two vectors (not a dot-product)
    // useful for colors and things that are not actually mathematical vectors
    Vector multiply(const Vector& rhs)
    {
#if defined USE_SSE
        ALIGN(16) float p[4];
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs._value);
        _mm_storeu_ps(p, _mm_mul_ps(A, B));
        return Vector(p);
#else
        return Vector(x() * rhs.x(), y() * rhs.y(), z() * rhs.z(), w() * rhs.w());
#endif
    }

    // creates a homogeneous 3-dimensional vector
    Vector homogeneous_position() const { return Vector(x(), y(), z(), 1.0f); }
    Vector homogeneous_direction() const { return Vector(x(), y(), z(), 0.0f); }

    Vector xy() const { return Vector(x(), y()); }
    Vector xz() const { return Vector(x(), z()); }
    Vector yz() const { return Vector(y(), z()); }
    Vector xyz() const { return Vector(x(), y(), z()); }

    std::string str() const;

public:
    float& operator[](int index) { return _value[index]; }
    const float& operator[](int index) const { return _value[index]; }

    bool operator==(const Vector& rhs) const { return x() == rhs.x() && y() == rhs.y() && z() == rhs.z() && w() == rhs.w(); }
    bool operator!=(const Vector& rhs) const { return !((*this) == rhs); }

    /*bool operator==(float rhs) const { return length() == rhs; }
    bool operator!=(float rhs) const { return !(*this == rhs); }*/

    Vector operator+(const Vector& rhs) const
    {
#if defined USE_SSE
        ALIGN(16) float p[4];
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs._value);
        _mm_storeu_ps(p, _mm_add_ps(A, B));
        return Vector(p);
#else
        return Vector(x() + rhs.x(), y() + rhs.y(), z() + rhs.z(), w() + rhs.w());
#endif
    }

    Vector& operator+=(const Vector& rhs)
    {
#if defined USE_SSE
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs._value);
        _mm_storeu_ps(_value, _mm_add_ps(A, B));
        return *this;
#else
        return ((*this) = (*this) + rhs);
#endif
    }

    Vector operator-(const Vector& rhs) const
    {
#if defined USE_SSE
        ALIGN(16) float p[4];
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs._value);
        _mm_storeu_ps(p, _mm_sub_ps(A, B));
        return Vector(p);
#else
        return Vector(x() - rhs.x(), y() - rhs.y(), z() - rhs.z(), w() - rhs.w());
#endif
    }

    Vector& operator-=(const Vector& rhs)
    {
#if defined USE_SSE
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs._value);
        _mm_storeu_ps(_value, _mm_sub_ps(A, B));
        return *this;
#else
        return ((*this) = (*this) - rhs);
#endif
    }

    Vector operator*(float rhs) const
    {
#if defined USE_SSE
        ALIGN(16) float p[4];
        __m128 A = _mm_loadu_ps(_value);
        __m128 S = _mm_set1_ps(rhs);
        _mm_storeu_ps(p, _mm_mul_ps(A, S));
        return Vector(p);
#else
        return Vector(x() * rhs, y() * rhs, z() * rhs, w() * rhs);
#endif
    }

    Vector& operator*=(float rhs)
    {
#if defined USE_SSE
        __m128 A = _mm_loadu_ps(_value);
        __m128 S = _mm_set1_ps(rhs);
        _mm_storeu_ps(_value, _mm_mul_ps(A, S));
        return *this;
#else
        return ((*this) = (*this) * rhs);
#endif
    }

    Vector operator/(float rhs) const
    {
#if defined USE_SSE
        ALIGN(16) float p[4];
        __m128 A = _mm_loadu_ps(_value);
        __m128 S = _mm_set1_ps(rhs);
        _mm_storeu_ps(p, _mm_div_ps(A, S));
        return Vector(p);
#else
        return Vector(x() / rhs, y() / rhs, z() / rhs, w() / rhs);
#endif
    }

    Vector& operator/=(float rhs)
    {
#if defined USE_SSE
        __m128 A = _mm_loadu_ps(_value);
        __m128 S = _mm_set1_ps(rhs);
        _mm_storeu_ps(_value, _mm_div_ps(A, S));
        return *this;
#else
        return ((*this) = (*this) / rhs);
#endif
    }

    Vector operator-() const { return Vector(-x(), -y(), -z(), -w()); }

    // dot-product
    float operator*(const Vector& rhs) const
    {
#if defined USE_SSE
        float d;
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs._value);

        // multiply the vector components
        __m128 R1 = _mm_mul_ps(A, B);

        // first horizontal add gives us
        // [R1.3 + R1.2, R1.1 + R1.0, R1.3 + R1.2, R1.1 + R1.0]
        __m128 R2 = _mm_hadd_ps(R1, R1);

        // second horizontal add gives us
        // [R2.3 + R2.2, R2.1 + R2.0, R2.3 + R2.2, R2.1 + R2.0]
        // which finishes the sum
        _mm_store_ss(&d, _mm_hadd_ps(R2, R2));
        return d;
#else
        return (x() * rhs.x()) + (y() * rhs.y()) + (z() * rhs.z()) + (w() * rhs.w());
#endif
    }

    // NOTE: rhs must have at least 4 floats
    // regardless of the "size" of the vector
    float operator*(const float* const rhs) const
    {
#if defined USE_SSE
        float d;
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs);
        __m128 R1 = _mm_mul_ps(A, B);
        __m128 R2 = _mm_hadd_ps(R1, R1);
        _mm_store_ss(&d, _mm_hadd_ps(R2, R2));
        return d;
#else
        return (x() * rhs[0]) + (y() * rhs[1]) + (z() * rhs[2]) + (w() * rhs[3]);
#endif
    }

    // 3-dimensional cross-product
    Vector operator^(const Vector& rhs) const
    {
#if defined USE_SSE
        ALIGN(16) float p[4];
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs._value);

        // _MM_SHUFFLE(w, z, y, x) builds the 8-bit shuffle opcode
        // first multiply has the following operands:
        //      A1 = (y, z, x, w), B1 = (z, x, y, w) = (ay * bz, az * bx, ax * by, aw * bw)
        // second multiply has the following operands:
        //      A2 = (z, x, y, w), B2 = (y, z, x, w) = (az * by, ax * bz, ay * bx, aw * bw)
        // final subtraction gives the proper cross-product:
        //      (ay * bz - az * by, az * bx - ax * bz, ax * by - ay * bx, 0.0)

        _mm_storeu_ps(p,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(B, B, _MM_SHUFFLE(3, 1, 0, 2))),
                _mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(B, B, _MM_SHUFFLE(3, 0, 2, 1)))
            )
        );
        return Vector(p);
#else
        return Vector(
            y() * rhs.z() - z() * rhs.y(),
            z() * rhs.x() - x() * rhs.z(),
            x() * rhs.y() - y() * rhs.x()
        );
#endif
    }

    Vector& operator^=(const Vector& rhs)
    {
#if defined USE_SSE
        __m128 A = _mm_loadu_ps(_value);
        __m128 B = _mm_loadu_ps(rhs._value);

        // _MM_SHUFFLE(w, z, y, x) builds the 8-bit shuffle opcode
        // first multiply has the following operands:
        //      A1 = (y, z, x, w), B1 = (z, x, y, w) = (ay * bz, az * bx, ax * by, aw * bw)
        // second multiply has the following operands:
        //      A2 = (z, x, y, w), B2 = (y, z, x, w) = (az * by, ax * bz, ay * bx, aw * bw)
        // final subtraction gives the proper cross-product:
        //      (ay * bz - az * by, az * bx - ax * bz, ax * by - ay * bx, 0.0)

        _mm_storeu_ps(_value,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(B, B, _MM_SHUFFLE(3, 1, 0, 2))),
                _mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(B, B, _MM_SHUFFLE(3, 0, 2, 1)))
            )
        );
        return *this;
#else
        return ((*this) = (*this) ^ rhs);
#endif
    }

public:
    friend class Matrix4;
    friend Vector operator*(float lhs, const Vector& rhs) { return rhs * lhs; }

private:
    ALIGN(16) float _value[4];
};

// some useful specializations that we can
// derive from the more general Vector class
typedef Vector Vector2;
typedef Vector Point2;
typedef Vector Vector3;
typedef Vector Point3;
typedef Vector Vector4;
typedef Vector Point4;
typedef Vector Color;
typedef Vector Direction;
typedef Vector Position;

#endif
