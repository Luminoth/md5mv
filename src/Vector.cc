#include "pch.h"
#include "Vector.h"

std::string Vector::str() const
{
    std::stringstream ss;
    ss << "Vector(x:" << std::fixed << x() << ", y:" << y() << ", z:" << z() << ", w:" << w() << ")";
    return ss.str();
}
