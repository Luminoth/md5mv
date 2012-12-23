#if !defined __LIGHT_H__
#define __LIGHT_H__

#include "Renderable.h"
#include "Vector.h"

class Lexer;

class Light : public Renderable
{
private:
    static Logger& logger;
    static bool enbled;
    static Color ac;

public:
    // enable/disables lighting
    static void lighting_enable(bool enable=true) { enbled = enable; }
    static bool lighting_enabled() { return enbled; }

    // global lighting values
    static void global_ambient_color(const Color& color) { ac = color; }
    static const Color& global_ambient_color() { return ac; }

public:
    virtual ~Light() throw();

public:
    void enable(bool enable=true) { _enabled = enable; }
    bool enabled() const { return _enabled; }

    // loads a colordef file
    bool load_colordef(const std::string& name);

    const Color& ambient_color() const { return _ambient; }
    void ambient_color(const Color& color) { _ambient = color; }

    const Color& diffuse_color() const { return _diffuse; }
    void diffuse_color(const Color& color) { _diffuse = color; }

    const Color& specular_color() const { return _specular; }
    void specular_color(const Color& color) { _specular = color; }

public:
    virtual bool is_transparent() const { return false; }
    virtual bool is_static() const { return true; }

private:
    bool scan_ambient(Lexer& lexer);
    bool scan_diffuse(Lexer& lexer);
    bool scan_specular(Lexer& lexer);

private:
    bool _enabled;
    Color _ambient, _diffuse, _specular;

protected:
    Light();

private:
    DISALLOW_COPY_AND_ASSIGN(Light);
};

class PositionalLight : public Light
{
public:
    PositionalLight();
    virtual ~PositionalLight() throw();

public:
    float constant_attenuation() const { return _constant_atten; }
    void constant_attenuation(float atten);

    float linear_attenuation() const { return _linear_atten; }
    void linear_attenuation(float atten);

    float quadratic_attenuation() const { return _quadratic_atten; }
    void quadratic_attenuation(float atten);

    virtual std::string str() const;

private:
    void calculate_radius();

private:
    float _constant_atten, _linear_atten, _quadratic_atten;

private:
    DISALLOW_COPY_AND_ASSIGN(PositionalLight);
};

class DirectionalLight : public Light
{
public:
    DirectionalLight();
    virtual ~DirectionalLight() throw();

public:
    Direction direction() const { return position().homogeneous_direction(); }
    void direction(const Direction& direction) { position(direction); }

    virtual std::string str() const;

private:
    DISALLOW_COPY_AND_ASSIGN(DirectionalLight);
};

class SpotLight : public PositionalLight
{
public:
    SpotLight();
    virtual ~SpotLight() throw();

public:
    const Direction& direction() const { return _direction; }
    void direction(const Direction& direction) { _direction = direction; }

    float cutoff() const { return _cutoff; }
    void cutoff(float cutoff) { _cutoff = cutoff; }

    float exponent() const { return _exponent; }
    void exponent(float exponent) { _exponent = exponent; }

    virtual std::string str() const;

private:
    Direction _direction;
    float _cutoff, _exponent;

private:
    DISALLOW_COPY_AND_ASSIGN(SpotLight);
};

#endif
