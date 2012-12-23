#if !defined __MATRIX3_H__
#define __MATRIX3_H__

#include "Vector.h"

// TODO: this needs to use a 16 float array
// in order to work with SSE operations
class Matrix3
{
public:
    Matrix3() { identity(); }
    Matrix3(const Matrix3& matrix) { std::memcpy(_m, matrix._m, 9 * sizeof(float)); }

    // matrix should be a 9 float array
    // NOTE: no bounds checking or anything is done here
    explicit Matrix3(const float* const matrix) { std::memcpy(_m, matrix, 9 * sizeof(float)); }

    virtual ~Matrix3() throw() {}

public:
    const float* array() const { return _m; }

    Vector3 row(size_t r) const { return Vector3(_m[(r * 3) + 0], _m[(r * 3) + 1], _m[(r * 3) + 2]); }

    Matrix3& identity()
    {
        _m[0]  = 1.0f; _m[1]  = 0.0f; _m[2]  = 0.0f;
        _m[3]  = 0.0f; _m[4]  = 1.0f; _m[5]  = 0.0f;
        _m[6]  = 0.0f; _m[7]  = 0.0f; _m[8] = 1.0f;
        return *this;
    }

    float determinant() const
    {
        return _m[0] * _m[4] * _m[8] - _m[0] * _m[5] * _m[7]
            + _m[1] * _m[5] * _m[6] - _m[1] * _m[3] * _m[8]
            + _m[2] * _m[3] * _m[7] - _m[2] * _m[4] * _m[6];
    }

    std::string str() const;

public:
    float& operator[](int index) { return _m[index]; }
    const float& operator[](int index) const { return _m[index]; }
    float& operator()(unsigned int r, unsigned int c) { return _m[(r * 3) + c]; }
    const float& operator()(unsigned int r, unsigned int c) const { return _m[(r * 3) + c]; }

    Matrix3& operator=(const Matrix3& rhs) { std::memcpy(_m, rhs._m, 9 * sizeof(float)); return *this; }

    Matrix3 operator+(const Matrix3& rhs) const
    {
        Matrix3 m;
        m._m[0] = _m[0] + rhs._m[0];
        m._m[1] = _m[1] + rhs._m[1];
        m._m[2] = _m[2] + rhs._m[2];
        m._m[3] = _m[3] + rhs._m[3];
        m._m[4] = _m[4] + rhs._m[4];
        m._m[5] = _m[5] + rhs._m[5];
        m._m[6] = _m[6] + rhs._m[6];
        m._m[7] = _m[7] + rhs._m[7];
        m._m[8] = _m[8] + rhs._m[8];
        return m;
    }

    Matrix3& operator+=(const Matrix3& rhs)
    {
        return ((*this) = (*this) + rhs);
    }

    Matrix3 operator-(const Matrix3& rhs) const
    {
        Matrix3 m;
        m._m[0] = _m[0] - rhs._m[0];
        m._m[1] = _m[1] - rhs._m[1];
        m._m[2] = _m[2] - rhs._m[2];
        m._m[3] = _m[3] - rhs._m[3];
        m._m[4] = _m[4] - rhs._m[4];
        m._m[5] = _m[5] - rhs._m[5];
        m._m[6] = _m[6] - rhs._m[6];
        m._m[7] = _m[7] - rhs._m[7];
        m._m[8] = _m[8] - rhs._m[8];
        return m;
    }

    Matrix3& operator-=(const Matrix3& rhs)
    {
        return ((*this) = (*this) - rhs);
    }

    /*Matrix3 operator*(const Matrix3& rhs) const
    {
        Matrix3 n;
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

    Matrix3& operator*=(const Matrix3& rhs)
    {
        return ((*this) = (*this) * rhs);
    }

    Vector3 operator*(const Vector3& rhs) const
    {
        return Vector3(
            _m[0] * rhs.x + _m[1] * rhs.y + _m[2] * rhs.z + _m[3],
            _m[4] * rhs.x + _m[5] * rhs.y + _m[6] * rhs.z + _m[7],
            _m[8] * rhs.x + _m[9] * rhs.y + _m[10] * rhs.z + _m[11]
        );
    }*/

    Matrix3 operator*(float rhs) const
    {
        Matrix3 m;
        m._m[0] = _m[0] * rhs;
        m._m[1] = _m[1] * rhs;
        m._m[2] = _m[2] * rhs;
        m._m[3] = _m[3] * rhs;
        m._m[4] = _m[4] * rhs;
        m._m[5] = _m[5] * rhs;
        m._m[6] = _m[6] * rhs;
        m._m[7] = _m[7] * rhs;
        m._m[8] = _m[8] * rhs;
        return m;
    }

    Matrix3& operator*=(float rhs)
    {
        return ((*this) = (*this) * rhs);
    }

    Matrix3 operator/(float rhs) const
    {
        Matrix3 m;
        m._m[0] = _m[0] / rhs;
        m._m[1] = _m[1] / rhs;
        m._m[2] = _m[2] / rhs;
        m._m[3] = _m[3] / rhs;
        m._m[4] = _m[4] / rhs;
        m._m[5] = _m[5] / rhs;
        m._m[6] = _m[6] / rhs;
        m._m[7] = _m[7] / rhs;
        m._m[8] = _m[8] / rhs;
        return m;
    }

    Matrix3& operator/=(float rhs)
    {
        return ((*this) = (*this) / rhs);
    }

    bool operator==(const Matrix3& rhs) const
    {
        return _m[0] == rhs._m[0] && _m[1] == rhs._m[1] && _m[2] == rhs._m[2]
            && _m[3] == rhs._m[3] && _m[4] == rhs._m[4] && _m[5] == rhs._m[5]
            && _m[6] == rhs._m[6] && _m[7] == rhs._m[7] && _m[8] == rhs._m[8];
    }

    bool operator!=(const Matrix3& rhs) const { return !((*this) == rhs); }

    // inverse
    // NOTE: this does not verify that determinant() != 0!!!
    Matrix3 operator-() const
    {
        Matrix3 m;
        m._m[0] = _m[4] * _m[8] - _m[5] * _m[7];
        m._m[1] = _m[2] * _m[7] - _m[1] * _m[8];
        m._m[2] = _m[1] * _m[5] - _m[2] * _m[4];
        m._m[3] = _m[5] * _m[6] - _m[3] * _m[8];
        m._m[4] = _m[0] * _m[8] - _m[2] * _m[6];
        m._m[5] = _m[2] * _m[3] - _m[0] * _m[5];
        m._m[6] = _m[3] * _m[7] - _m[4] * _m[6];
        m._m[7] = _m[7] * _m[1] - _m[0] * _m[7];
        m._m[8] = _m[0] * _m[4] - _m[1] * _m[3];
        return (1.0f/determinant()) * m;
    }

    // transposition
    Matrix3 operator~() const
    {
        Matrix3 t;
        t._m[0] = _m[0];
        t._m[3] = _m[1];
        t._m[6] = _m[2];
        t._m[1] = _m[3];
        t._m[4] = _m[4];
        t._m[7] = _m[5];
        t._m[2] = _m[6];
        t._m[5] = _m[7];
        t._m[8] = _m[8];
        return t;
    }

public:
    friend Matrix3 operator*(float lhs, const Matrix3& rhs) { return rhs * lhs; }

private:
    ALIGN(16) float _m[9];
};

#endif
