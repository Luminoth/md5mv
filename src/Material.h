#if !defined __MATERIAL_H__
#define __MATERIAL_H__

#include "Vector.h"

class Lexer;

class Material
{
private:
    static Logger& logger;
    static boost::unordered_map<std::string, Material> materials;

public:
    Material();
    virtual ~Material() throw();

public:
    bool load(const boost::filesystem::path& path, const std::string& name);

    const Color& ambient_color() const { return _ambient; }
    void ambient_color(const Color& color) { _ambient = color; }

    const Color& diffuse_color() const { return _diffuse; }
    void diffuse_color(const Color& color) { _diffuse = color; }

    const Color& specular_color() const { return _specular; }
    void specular_color(const Color& color) { _specular = color; }

    const Color& emissive_color() const { return _emissive; }
    void emissive_color(const Color& color) { _emissive = color; }

    float shininess() const { return _shininess; }
    void shininess(float shininess) { _shininess = shininess; }

private:
    bool scan_ambient(Lexer& lexer);
    bool scan_diffuse(Lexer& lexer);
    bool scan_specular(Lexer& lexer);
    bool scan_emissive(Lexer& lexer);

private:
    Color _ambient, _diffuse, _specular, _emissive;
    float _shininess;
};

#endif
