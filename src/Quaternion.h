#if !defined __QUATERNION_H__
#define __QUATERNION_H__

#include "Matrix4.h"
#include "Vector.h"

class Quaternion
{
public:
    static Quaternion new_euler(float pitch, float yaw, float roll);

    // NOTE: this normalizes the axis
    static Quaternion new_axis(float angle, const Vector3& axis);

public:
    // multiplication identity quaternion
    Quaternion()
        : _scalar(1.0f)
    {
    }

    // computes the scalar based on the vector value
    explicit Quaternion(const Vector3& vector)
    {
        _vector = vector;
        compute_scalar();
    }

    explicit Quaternion(const Matrix4& matrix);

    virtual ~Quaternion() throw() {}

public:
    float scalar() const { return _scalar; }

    Vector3& vector() { return _vector; }
    const Vector3& vector() const { return _vector; }

    // computes (or recomputes) the scalar based on the vector value
    void compute_scalar()
    {
        const float w = 1.0f - _vector.length_squared();
        _scalar = w < 0.0f ? 0.0f : -std::sqrt(w);
    }

    Quaternion& normalize() { return (*this) *= invsqrt(length_squared()); }
    Quaternion normalized() const { return (*this) * invsqrt(length_squared()); }

    float length_squared() const
    {
        return (*this) ^ (*this);
    }

    float length() const
    {
        float x = length_squared();
#if defined USE_SSE
        __m128 V = _mm_set_ss(x);
        _mm_store_ss(&x, _mm_sqrt_ss(V));
        return x;
#else
        return std::sqrt(x);
#endif
    }

    Quaternion rotate(const Quaternion& quaternion) const { return (*this) * quaternion * ~(*this); }
    Quaternion rotate(float angle, const Vector3& vector) const { return new_axis(angle, vector).rotate(*this); }

    float angle() const { return 2.0f * std::acos(_scalar); }

    Vector3 axis() const;

    // NOTE: if the quaternion is normalized
    // then this is equivalent to the conjugate
    Quaternion inverse() const { return ~(*this) / length_squared(); }

    // these assume all quaternions involved are normalized
    // t must be in [0, 1]
    Quaternion lerp(const Quaternion& rhs, double t) const { return ((1.0 - t) * (*this) + t * rhs).normalized(); }
    Quaternion slerp(const Quaternion& rhs, double t) const;

    std::string str() const;
    Matrix4 matrix() const;

public:
    // NOTE: these apply indices in this order - (x, y, z, w)
    float& operator[](int index) { return index > 2 ? _scalar : _vector[index]; }
    const float& operator[](int index) const { return index > 2 ? _scalar : _vector[index]; }

    bool operator==(const Quaternion& rhs) const { return _scalar == rhs._scalar && _vector == rhs._vector; }
    bool operator!=(const Quaternion& rhs) const { return !((*this) == rhs); }

    bool operator==(float rhs) const { return length() == rhs; }
    bool operator!=(float rhs) const { return !((*this) == rhs); }

    Quaternion operator-() const { return Quaternion(-_scalar, -_vector); }

    // NOTE: these do *not* normalize the quaternion
    Quaternion operator+(const Quaternion& rhs) const { return Quaternion(_scalar + rhs._scalar, _vector + rhs._vector); }
    Quaternion& operator+=(const Quaternion& rhs) { return ((*this) = (*this) + rhs); }

    Quaternion operator-(const Quaternion& rhs) const { return (*this) + (-rhs); }
    Quaternion& operator-=(const Quaternion& rhs) { return (*this) += (-rhs); }

    // TODO: this needs to be seriously optimized
    Vector3 operator*(const Vector3& vector) const { return rotate(Quaternion(0.0f, vector))._vector; }

    Quaternion operator*(const Quaternion& rhs) const { return Quaternion((_scalar * rhs._scalar) - (_vector * rhs._vector), (_scalar * rhs._vector) + (rhs._scalar * _vector) + (_vector ^ rhs._vector)); }
    Quaternion& operator*=(const Quaternion& rhs) { return ((*this) = (*this) * rhs); }

    Quaternion operator*(float rhs) const { return Quaternion(_scalar * rhs, _vector * rhs); }
    Quaternion& operator*=(float rhs) { return ((*this) = (*this) * rhs); }

    Quaternion operator/(float rhs) const { return Quaternion(_scalar / rhs, _vector / rhs); }
    Quaternion& operator/=(float rhs) { return ((*this) = (*this) / rhs); }

    // dot-product
    float operator^(const Quaternion& rhs) const { return (_scalar * rhs._scalar) + (_vector * rhs._vector); }

    Quaternion& operator=(const Quaternion& rhs) { _scalar = rhs._scalar; _vector = rhs._vector; return *this; }

    // conjugate
    Quaternion operator~() const { return Quaternion(_scalar, -_vector); }

public:
    friend bool operator==(float lhs, const Quaternion& rhs) { return lhs == rhs.length(); }
    friend bool operator!=(float lhs, const Quaternion& rhs) { return !(lhs == rhs); }

    friend Quaternion operator*(float lhs, const Quaternion& rhs) { return Quaternion(lhs * rhs._scalar, lhs * rhs._vector); }

private:
    float _scalar;
    Vector3 _vector;

private:
    Quaternion(float w, float x, float y, float z)
        : _scalar(w), _vector(x, y, z)
    {
    }

    Quaternion(float scalar, const Vector3& vector)
        : _scalar(scalar), _vector(vector)
    {
    }
};

#endif
