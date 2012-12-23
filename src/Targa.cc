#include "pch.h"
#include <fstream>
#include "Targa.h"

Targa::Targa()
    : Texture(), _width(0), _height(0)
{
    ZeroMemory(&_header, sizeof(Header));
}

Targa::~Targa() throw()
{
}

bool Targa::load(const boost::filesystem::path& filename)
{
    unload();

    std::ifstream f(filename.string().c_str(), std::ios::binary);
    if(!f) {
        return false;
    }

    // NOTE: we only support RGB/RGBA
    f.read(reinterpret_cast<char*>(&_header), sizeof(Header));
    if(_header.type != RGB) {
        return false;
    }

    if(_header.bpp != 24 && _header.bpp != 32) {
        return false;
    }

    // little-endian ordering
    _width = (_header.width2 << 8) + _header.width1;
    _height = (_header.height2 << 8) + _header.height1;

    // skip TARGA junk
    f.seekg(_header.idlen, std::ios::cur);

    _image.reset(new unsigned char[size()]);
    f.read(reinterpret_cast<char*>(_image.get()), size());

    flip_vertical();
    return true;
}

void Targa::unload() throw()
{
    ZeroMemory(&_header, sizeof(Header));
    _width = _height = 0;
    _image.reset();
}

bool Targa::save(const boost::filesystem::path& filename, size_t width, size_t height, size_t bpp, const unsigned char* const pixels) const
{
// NOTE: probably have to mirror the image here
    return false;
}
