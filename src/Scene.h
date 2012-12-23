#if !defined __SCENE_H__
#define __SCENE_H__

class Camera;
class Lexer;
class Map;
class Renderable;
class PositionalLight;
class SpotLight;

//class Q3BSP;

class Scene
{
private:
    static Logger& logger;

public:
    Scene();
    virtual ~Scene() throw();

public:
    bool load(const std::string& name);
    void unload();

    void update(double dt);

    void render();

public:
    bool loaded() const { return _loaded; }

    const Camera& camera() const { return *_camera; }
    Camera& camera() { return *_camera; }

    const Map& map() const { return *_map; }

private:
    bool scan_map(Lexer& lexer);
    bool scan_global_ambient_color(Lexer& lexer);
    bool scan_models(Lexer& lexer);
    bool scan_renderables(Lexer& lexer);
    bool scan_static(Lexer& lexer);
    bool scan_actor(Lexer& lexer, const std::string& type);
    bool scan_lights(Lexer& lexer);
    bool scan_light_positional(Lexer& lexer, boost::shared_ptr<PositionalLight> positional);
    bool scan_light_spotlight(Lexer& lexer, boost::shared_ptr<SpotLight> spot);

private:
    bool _loaded;
    boost::shared_ptr<Camera> _camera;
    boost::shared_ptr<Map> _map;
    std::vector<boost::shared_ptr<Renderable> > _renderables;

/*public:
boost::shared_ptr<Q3BSP> _bsp;*/

private:
    DISALLOW_COPY_AND_ASSIGN(Scene);
};

#endif
