#include "pch.h"
#include "math_util.h"
#include "Matrix3.h"

std::string Matrix3::str() const
{
    std::stringstream ss;
    ss << _m[0] << " " << _m[1] << " " << _m[2] << std::endl
       << _m[3] << " " << _m[4] << " " << _m[5] << std::endl
       << _m[6] << " " << _m[7] << " " << _m[8];
    return ss.str();
}
