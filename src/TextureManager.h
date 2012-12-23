#if !defined __TEXTURE_MANAGER_H__
#define __TEXTURE_MANAGER_H__

class TextureManager
{
public:
    enum TextureType
    {
        DetailTexture,
        NormalMap,
        SpecularMap,
        EmissionMap,
        TextureCount
    };

public:
    static TextureManager& instance();

private:
    static Logger& logger;

public:
    virtual ~TextureManager() throw();

public:
    GLuint default_detail_texture() const { return _default_textures[DetailTexture]; }
    GLuint default_normal_map() const { return _default_textures[NormalMap]; }
    GLuint default_specular_map() const { return _default_textures[SpecularMap]; }
    GLuint default_emission_map() const { return _default_textures[EmissionMap]; }

public:
    bool init();

    GLuint load_texture(const boost::filesystem::path& filename, bool mipmaps=true, bool compress=true, bool repeat=true);
    GLuint bind_texture(size_t width, size_t height, size_t Bpp, GLenum format, bool mipmaps, bool compress, bool repeat, void* pixels);
    void save_texture(GLuint texture, size_t width, size_t height, size_t Bpp, const boost::filesystem::path& filename);

private:
    GLuint default_texture(const boost::filesystem::path& filename) const;

private:
    GLuint _default_textures[TextureCount];
    boost::unordered_map<std::string, GLuint> _textures;

private:
    TextureManager();
    DISALLOW_COPY_AND_ASSIGN(TextureManager);
};

#endif
