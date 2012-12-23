#if !defined __MAP_H__
#define __MAP_H__

#include "Material.h"

class Camera;
class Shader;

class Light;
typedef std::vector<boost::shared_ptr<Light> > Lights;

class Map
{
public:
    static const size_t MAX_LIGHTS;

private:
    static Logger& logger;

public:
    virtual ~Map() throw();

public:
    const std::string& name() const { return _name; }

    virtual Position player_spawn_position() const = 0;
    virtual float player_spawn_angle() const = 0;

    size_t light_count() const { return _lights.size(); }
    const Lights& lights() const { return _lights; }
    void add_light(boost::shared_ptr<Light> light) { _lights.push_back(light); }

    const Material& material() const { return _material; }

    virtual bool load(const boost::filesystem::path& path) = 0;
    void unload();

    bool load_material(const boost::filesystem::path& path, const std::string& name);

    virtual void render(const Camera& camera, Shader& shader) const = 0;
    virtual void render(const Camera& camera, Shader& shader, const Light& light) const = 0;
    virtual void render_normals(const Camera& camera) const = 0;

private:
    std::string _name;

    Lights _lights;

    Material _material;

protected:
    explicit Map(const std::string& name);

    virtual void on_unload() = 0;

private:
    Map();
    DISALLOW_COPY_AND_ASSIGN(Map);
};

#endif
