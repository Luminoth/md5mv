#include "pch.h"
#include "Texture.h"

Texture::Texture()
{
}

Texture::~Texture() throw()
{
}

void Texture::flip_vertical()
{
    boost::scoped_array<unsigned char> scratch(new unsigned char[size()]);
    for(size_t i=0; i<height(); ++i) {
        const size_t dst = i * byte_width();
        const size_t src = (height() - i - 1) * byte_width();
        std::memcpy(scratch.get() + dst, _image.get() + src, byte_width());
    }
    std::memcpy(_image.get(), scratch.get(), size());
}
