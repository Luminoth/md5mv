#if !defined __PNG_H__
#define __PNG_H__

#include <png.h>
#include "Texture.h"

void png_user_warn(png_structp ctx, png_const_charp str);
void png_user_error(png_structp ctx, png_const_charp str);

class PNG : public Texture
{
private:
    static Logger& logger;

public:
    PNG();
    virtual ~PNG() throw();

public:
    virtual size_t width() const { return _width; }
    virtual size_t height() const { return _height; }
    virtual size_t bpp() const { return _bpp; }

public:
    virtual bool load(const boost::filesystem::path& filename);
    virtual void unload() throw();

    virtual bool save(const boost::filesystem::path& filename, size_t width, size_t height, size_t bpp, const unsigned char* const pixels) const;

private:
    size_t _width, _height, _bpp;

private:
    DISALLOW_COPY_AND_ASSIGN(PNG);
};

#endif
