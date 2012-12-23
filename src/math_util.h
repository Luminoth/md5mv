#if !defined __MATHUTIL_H__
#define __MATHUTIL_H__

#if defined WIN32
#define _USE_MATH_DEFINES
#endif

#include <cmath>

// determine if a number is odd
#define IS_ODD(n) ((n) & 0x01)

// determine if a number is even
#define IS_EVEN(n) (!IS_ODD((n)))

// converts between radians and degrees
// these could be faster by pre-computing
#define RAD_DEG(r) ((r) * 57.2957795)   // 180.0 / M_PI is about 57.2957795
#define DEG_RAD(d) ((d) * 0.0174532925) // M_PI / 180.0 is about 0.0174532925

// some semi-useful values
#define RAD_360 DEG_RAD(360)
#define RAD_60 DEG_RAD(60)

// min/max
#if !defined MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#if !defined MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#if !defined M_PI
#define M_PI 3.14159265358979323846
#endif

#if !defined M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#if !defined M_PHI
#define M_PHI 1.61803399
#endif

#if !defined M_E
#define M_E 2.7182818284590451
#endif

inline long manhattan_distance(long x1, long y1, long x2, long y2)
{
    return static_cast<long>(std::abs(static_cast<float>(x2 - x1)) + std::abs(static_cast<float>(y2 - y1)));
}

// modulus division by 1 << s
// n is the numerator
inline unsigned int modulus_division(unsigned int n, unsigned int s)
{
    return n & ((1 << s) - 1);
}

// modulus division by (1 << s) - 1
// n is the numerator
inline unsigned int modulus_division_1(unsigned int n, unsigned int s)
{
    register const unsigned int d = (1 << s) - 1;
    register unsigned int m;
    for(m=n; n>d; n=m) {
        for(m = 0; n; n >>= s) {
            m += n & d;
        }
    }
    return m == d ? 0 : m;
}

// log base 2 of v
inline unsigned int ilog2(unsigned int v)
{
    register unsigned int c = 0;
    if(v & 0xFFFF0000) {
        v >>= 16;
        c |= 16;
    }

    if(v & 0xFF00) {
        v >>= 8;
        c |= 8;
    }

    if(v & 0xF0) {
        v >>= 4;
        c |= 4;
    }

    if(v & 0xC) {
        v >>= 2;
        c |= 2;
    }

    if(v & 0x2) {
        v >>= 1;
        c |= 1;
    }

    return c;
}

// next highest power of 2
inline unsigned int power_of_2(unsigned int i)
{
    i--;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    return i + 1;
}

inline float invsqrt(float x)
{
#if defined USE_SSE
    __m128 V = _mm_set_ss(x);
    _mm_store_ss(&x, _mm_rsqrt_ss(V));
    return x;
#elif 0
    // taken from http://www.beyond3d.com/content/articles/8/
    // TODO: this breaks strict-aliasing, is there a better way to do it?
    float xhalf = 0.5f*x;
    int i = *(reinterpret_cast<int*>(&x));
    i = 0x5f3759df - (i>>1);
    x = *(reinterpret_cast<float*>(&i));
    x = x*(1.5f - xhalf*x*x);
    return x;
#else
    return 1.0f / std::sqrt(x);
#endif
}

#endif
