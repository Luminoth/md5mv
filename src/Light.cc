#include "pch.h"
#include <iostream>
#include "common.h"
#include "math_util.h"
#include "Lexer.h"
#include "Renderer.h"
#include "State.h"
#include "Light.h"

Logger& Light::logger(Logger::instance("md5mv.Light"));
bool Light::enbled = true;
Color Light::ac(0.2f, 0.2f, 0.2f, 1.0f);

Light::Light()
    : Renderable("light"), _enabled(false), _ambient(0.0f, 0.0f, 0.0f, 1.0f),
        _diffuse(0.0f, 0.0f, 0.0f, 1.0f), _specular(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Light::~Light() throw()
{
}

bool Light::load_colordef(const std::string& name)
{
    boost::filesystem::path filename(light_dir() / (name + ".light"));
    LOG_INFO("Loading colordef '" << filename << "'..." << std::endl);

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

    return true;
}

bool Light::scan_ambient(Lexer& lexer)
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

bool Light::scan_diffuse(Lexer& lexer)
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

bool Light::scan_specular(Lexer& lexer)
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

PositionalLight::PositionalLight()
    : Light(), /*_bounds(Point3(0.0f, 0.0f, 1.0f)),*/
        _constant_atten(1.0f), _linear_atten(0.0f), _quadratic_atten(0.0f)
{
}

PositionalLight::~PositionalLight() throw()
{
}

void PositionalLight::constant_attenuation(float atten)
{
    _constant_atten = atten;
    calculate_radius();
}

void PositionalLight::linear_attenuation(float atten)
{
    _linear_atten = atten;
    calculate_radius();
}

void PositionalLight::quadratic_attenuation(float atten)
{
    _quadratic_atten = atten;
    calculate_radius();
}

std::string PositionalLight::str() const
{
    std::stringstream ss;
    ss << "PositionalLight(position: " << std::fixed << position().str()
        << ", constant_attenuation: " << _constant_atten
        << ", linear_attenuation: " << _linear_atten
        << ", quadratic_attenuation: " << _quadratic_atten
        << ")";
    return ss.str();
}

/*void PositionalLight::render(Shader& shader) const
{
    //Renderer::instance().render_sphere(_bounds);
}*/

void PositionalLight::calculate_radius()
{
    /*float radius = 1.0f;
    if(_constant_atten > 0.0f) {
        radius = MAX(radius, 1.0f / _constant_atten);
    }

    if(_linear_atten > 0.0f) {
        radius = MAX(radius, 1.0f / _linear_atten);
    }

    if(_quadratic_atten > 0.0f) {
        radius = MAX(radius, invsqrt(_quadratic_atten));
    }

    _bounds.radius(MAX(radius, 1.0f));*/
//_bounds.radius(10.0f);
}

DirectionalLight::DirectionalLight()
    : Light()
{
    position(Position(0.0f, 0.0f, 1.0f));
}

DirectionalLight::~DirectionalLight() throw()
{
}

std::string DirectionalLight::str() const
{
    std::stringstream ss;
    ss << "DirectionalLight(direction: " << std::fixed << position().str() << ")";
    return ss.str();
}

SpotLight::SpotLight()
    : PositionalLight(), _direction(0.0f, 0.0f, -1.0f),
        _cutoff(180.0f), _exponent(0.0f)
{
}

SpotLight::~SpotLight() throw()
{
}

std::string SpotLight::str() const
{
    std::stringstream ss;
    ss << "SpotLight(position: " << std::fixed << position().str()
        << ", direction: " << _direction.str()
        << ", constant_attenuation: " << constant_attenuation()
        << ", linear_attenuation: " << linear_attenuation()
        << ", quadratic_attenuation: " << quadratic_attenuation()
        << ", cutoff: " << _cutoff
        << ", exponent: " << _exponent
        << ")";
    return ss.str();
}
