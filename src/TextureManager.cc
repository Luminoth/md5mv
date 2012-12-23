#include "pch.h"
#include "common.h"
#include "PNG.h"
#include "Renderer.h"
#include "Targa.h"
#include "TextureManager.h"

Logger& TextureManager::logger(Logger::instance("md5mv.TextureManager"));

TextureManager& TextureManager::instance()
{
    static boost::shared_ptr<TextureManager> texture_manager;
    if(!texture_manager) {
        texture_manager.reset(new TextureManager());
    }
    return *texture_manager;
}

TextureManager::TextureManager()
{
    ZeroMemory(_default_textures, TextureCount * sizeof(GLuint));
}

TextureManager::~TextureManager() throw()
{
    glDeleteTextures(1, &_default_textures[DetailTexture]);
    glDeleteTextures(1, &_default_textures[NormalMap]);
}

bool TextureManager::init()
{
    _default_textures[DetailTexture] = load_texture(texture_dir() / "no-texture.png");
    if(0 == _default_textures[DetailTexture]) {
        return false;
    }

    _default_textures[NormalMap] = load_texture(texture_dir() / "no-texture_n.png");
    if(0 == _default_textures[NormalMap]) {
        return false;
    }

    return true;
}

GLuint TextureManager::load_texture(const boost::filesystem::path& filename, bool mipmaps, bool compress, bool repeat)
{
    try {
        return _textures.at(filename.string());
    } catch(const std::out_of_range&) {
    }

    LOG_INFO("Loading texture from '" << filename << "'" << std::endl);

    GLuint texture = 0;
    if(".png" == filename.extension()) {
        PNG tex;
        if(!tex.load(filename)) {
            LOG_ERROR("Could not load " << filename << "!\n");
            texture = default_texture(filename);
        } else {
            // TODO: this may not need to be BGR anymore
            texture = bind_texture(tex.width(), tex.height(), tex.Bpp(),
                tex.Bpp() == 4 ? GL_BGRA : GL_BGR, mipmaps, compress, repeat, tex.image());
        }
    } else if(".tga" == filename.extension()) {
        Targa tex;
        if(!tex.load(filename)) {
            LOG_ERROR("Could not load " << filename << "!\n");
            texture = default_texture(filename);
        } else {
            texture = bind_texture(tex.width(), tex.height(), tex.Bpp(),
                tex.Bpp() == 4 ? GL_BGRA : GL_BGR, mipmaps, compress, repeat, tex.image());
        }
    } else {
        LOG_ERROR("Filetype " << filename.extension() << " is not supported!\n");
        texture = default_texture(filename);
    }

    _textures[filename.string()] = texture;
    return texture;
}

GLuint TextureManager::bind_texture(size_t width, size_t height, size_t Bpp, GLenum format, bool mipmaps, bool compress, bool repeat, void* pixels)
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, Bpp == 4 ? (compress ? GL_COMPRESSED_RGBA : GL_RGBA) : (compress ? GL_COMPRESSED_RGB : GL_RGB),
        width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

    if(mipmaps) {
        // this requires opengl 3.0
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void TextureManager::save_texture(GLuint texture, size_t width, size_t height, size_t Bpp, const boost::filesystem::path& filename)
{
    glBindTexture(GL_TEXTURE_2D, texture);

    SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE,
        width, height, Bpp * 8,
        0x0000ff, 0x00ff00, 0xff0000, 0x000000);

    SDL_LockSurface(image);
    glGetTexImage(GL_TEXTURE_2D, 0, Bpp == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    SDL_Surface* temp = SDL_CreateRGBSurface(SDL_SWSURFACE,
        width, height, Bpp * 8,
        0x0000ff, 0x00ff00, 0xff0000, 0x000000);

    SDL_LockSurface(temp);

    // http://www.math.psu.edu/local_doc/SDL-1.1.2/opengl/examples.html
    for(int i=0; i<image->h; ++i) {
        std::memcpy(static_cast<char*>(temp->pixels) + (temp->format->BytesPerPixel * temp->w * i),
            static_cast<char*>(image->pixels) + (image->format->BytesPerPixel * image->w * (image->h - i - 1)),
            image->format->BytesPerPixel * image->w);
    }
    std::memmove(image->pixels, temp->pixels, image->w * image->h * image->format->BytesPerPixel);

    SDL_UnlockSurface(temp);
    SDL_UnlockSurface(image);

    Renderer::instance().save_png(filename, image);

    SDL_FreeSurface(temp);
    SDL_FreeSurface(image);

    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint TextureManager::default_texture(const boost::filesystem::path& filename) const
{
    if(std::string::npos != filename.string().find("_n.") || std::string::npos != filename.string().find("_local.")) {
        return _default_textures[NormalMap];
    }
    return _default_textures[DetailTexture];
}
