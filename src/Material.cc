#include "pch.h"
#include "common.h"
#include "Lexer.h"
#include "Material.h"

Logger& Material::logger(Logger::instance("md5mv.Material"));
boost::unordered_map<std::string, Material> Material::materials;

Material::Material()
    : _ambient(0.2f, 0.2f, 0.2f, 1.0f), _diffuse(0.8f, 0.8f, 0.8f, 1.0f),
        _specular(0.0f, 0.0f, 0.0f, 1.0f), _emissive(0.0f, 0.0f, 0.0f, 1.0f),
        _shininess(128.0f)
{
}

Material::~Material() throw()
{
}

bool Material::load(const boost::filesystem::path& path, const std::string& name)
{
    try {
        (*this) = materials.at(name);
        return true;
    } catch(std::out_of_range&) {
    }

    boost::filesystem::path filename(material_dir() / (name + ".material"));
    LOG_INFO("Loading material from '" << filename << "'" << std::endl);

    Lexer lexer;
    if(!lexer.load(filename)) {
        return false;
    }

    if(!scan_ambient(lexer)) {
        return false;
    }

    if(!scan_diffuse(lexer)) {
        return false;
    }

    if(!scan_specular(lexer)) {
        return false;
    }

    if(!scan_emissive(lexer)) {
        return false;
    }

    if(!lexer.match(SHININESS)) {
        return false;
    }

    float s;
    if(!lexer.float_literal(s)) {
        return false;
    }
    shininess(s);

    materials[name] = (*this);
    return true;
}

bool Material::scan_ambient(Lexer& lexer)
{
    if(!lexer.match(AMBIENT)) {
        return false;
    }

    float r;
    if(!lexer.float_literal(r)) {
        return false;
    }

    float g;
    if(!lexer.float_literal(g)) {
        return false;
    }

    float b;
    if(!lexer.float_literal(b)) {
        return false;
    }

    float a;
    if(!lexer.float_literal(a)) {
        return false;
    }

    ambient_color(Color(r, g, b, a));
    return true;
}

bool Material::scan_diffuse(Lexer& lexer)
{
    if(!lexer.match(DIFFUSE)) {
        return false;
    }

    float r;
    if(!lexer.float_literal(r)) {
        return false;
    }

    float g;
    if(!lexer.float_literal(g)) {
        return false;
    }

    float b;
    if(!lexer.float_literal(b)) {
        return false;
    }

    float a;
    if(!lexer.float_literal(a)) {
        return false;
    }

    diffuse_color(Color(r, g, b, a));
    return true;
}

bool Material::scan_specular(Lexer& lexer)
{
    if(!lexer.match(SPECULAR)) {
        return false;
    }

    float r;
    if(!lexer.float_literal(r)) {
        return false;
    }

    float g;
    if(!lexer.float_literal(g)) {
        return false;
    }

    float b;
    if(!lexer.float_literal(b)) {
        return false;
    }

    float a;
    if(!lexer.float_literal(a)) {
        return false;
    }

    specular_color(Color(r, g, b, a));
    return true;
}

bool Material::scan_emissive(Lexer& lexer)
{
    if(!lexer.match(EMISSIVE)) {
        return false;
    }

    float r;
    if(!lexer.float_literal(r)) {
        return false;
    }

    float g;
    if(!lexer.float_literal(g)) {
        return false;
    }

    float b;
    if(!lexer.float_literal(b)) {
        return false;
    }

    float a;
    if(!lexer.float_literal(a)) {
        return false;
    }

    emissive_color(Color(r, g, b, a));
    return true;
}
