#include "pch.h"
#include <fstream>
#include "PNG.h"

void png_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::ifstream* f = reinterpret_cast<std::ifstream*>(png_get_io_ptr(png_ptr));
    f->read(reinterpret_cast<char*>(data), length);
}

void png_user_warn(png_structp ctx, png_const_charp str)
{
    std::cerr << "libpng: warning: " << str << "\n";
}

void png_user_error(png_structp ctx, png_const_charp str)
{
    std::cerr << "libpng: error: " << str << "\n";
}

Logger& PNG::logger(Logger::instance("gled.PNG"));

PNG::PNG()
    : Texture(), _width(0), _height(0), _bpp(0)
{
}

PNG::~PNG() throw()
{
}

bool PNG::load(const boost::filesystem::path& filename)
{
    unload();

    std::ifstream f(filename.string().c_str(), std::ios::binary);
    if(!f) {
        return false;
    }

    png_byte header[8];
    f.read(reinterpret_cast<char*>(header), 8);
    if(png_sig_cmp(header, 0, 8)) {
        return false;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, png_user_error, png_user_warn);
    if(NULL == png_ptr) {
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(NULL == info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if(NULL == end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return false;
    }

    png_set_read_fn(png_ptr, &f, png_read);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    if(bit_depth == 16) {
        png_set_strip_16(png_ptr);
    } else if(bit_depth < 8) {
        /*if(PNG_COLOR_TYPE_GRAY == color_type) {
            png_set_gray_1_2_4_to_8(png_ptr);
        } else {*/
            LOG_ERROR("Unsupported bit depth: " << bit_depth << "\n");
            return false;
        //}
    }

    if(PNG_COLOR_TYPE_PALETTE == color_type) {
        png_set_palette_to_rgb(png_ptr);
    } else if(PNG_COLOR_TYPE_GRAY == color_type || PNG_COLOR_TYPE_GRAY_ALPHA == color_type) {
        png_set_gray_to_rgb(png_ptr);
    }

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    //png_set_invert_alpha(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    _width = png_get_image_width(png_ptr, info_ptr);
    _height = png_get_image_height(png_ptr, info_ptr);

    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    int channels = png_get_channels(png_ptr, info_ptr);
    _bpp = channels * bit_depth;

    size_t size = width() * height() * Bpp();
    _image.reset(new unsigned char[size]);

    png_bytep* row_pointers = (png_bytep*)png_malloc(png_ptr, height() * sizeof(png_bytep));
    for(size_t i=0; i<height(); ++i) {
        row_pointers[i] = (png_bytep)png_malloc(png_ptr, width() * Bpp());
    }

    png_read_image(png_ptr, row_pointers);

    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    for(size_t i=0; i<height(); ++i) {
        std::memcpy(_image.get() + (i * width() * Bpp()), row_pointers[i], width() * Bpp());
        png_free(png_ptr, row_pointers[i]);
    }
    png_free(png_ptr, row_pointers);

    return true;
}

void PNG::unload() throw()
{
    _width = _height = _bpp = 0;
    _image.reset();
}

bool PNG::save(const boost::filesystem::path& filename, size_t width, size_t height, size_t bpp, const unsigned char* const pixels) const
{
    return false;
}
