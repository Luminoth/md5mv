#if !defined __TEXTURE_H__
#define __TEXTURE_H__

class Texture
{
public:
    Texture();
    virtual ~Texture() throw();

public:
    virtual unsigned char* image() { return _image.get(); }
    virtual const unsigned char* const image() const { return _image.get(); }

    virtual size_t width() const = 0;
    size_t byte_width() const { return width() * Bpp(); }

    virtual size_t height() const = 0;
    size_t byte_height() const { return height() * Bpp(); }

    virtual size_t bpp() const = 0;
    size_t Bpp() const { return bpp() >> 3; }

    size_t size() const { return width() * height() * Bpp(); }

public:
    virtual bool load(const boost::filesystem::path& filename) = 0;
    virtual void unload() throw() = 0;

    void flip_vertical();

    virtual bool save(const boost::filesystem::path& filename, size_t width, size_t height, size_t bpp, const unsigned char* const pixels) const = 0;

protected:
    boost::shared_array<unsigned char> _image;

private:
    DISALLOW_COPY_AND_ASSIGN(Texture);
};

#endif
