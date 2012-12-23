#include "pch.h"
#include <iostream>
#include "common.h"
#include "util.h"
#include "math_util.h"
#include "Actor.h"
#include "Camera.h"
#include "D3Map.h"
#include "Lexer.h"
#include "Light.h"
#include "ModelManager.h"
#include "Player.h"
#include "Renderer.h"
#include "State.h"
#include "Static.h"
#include "Scene.h"

//#include "Q3BSP.h"

Logger& Scene::logger(Logger::instance("md5mv.Scene"));

Scene::Scene()
    : _loaded(false), _camera(new Camera())
{
}

Scene::~Scene() throw()
{
    unload();
}

bool Scene::load(const std::string& name)
{
    unload();

    boost::filesystem::path filename(scene_dir() / (name + ".scene"));
    LOG_INFO("Loading scene from '" << filename << "'" << std::endl);

    Lexer lexer;
    if(!lexer.load(filename)) {
        return false;
    }

    State::instance().display_text("Loading map...");
    State::instance().render();

    if(!scan_map(lexer)) {
        return false;
    }

    Position position(_map->player_spawn_position());
    LOG_INFO("Moving player to " << position.str() << std::endl);
    State::instance().player()->position(position);

    float angle(_map->player_spawn_angle());
    LOG_INFO("Rotating " << angle << " degrees" << std::endl);
    State::instance().player()->rotate(DEG_RAD(angle), Vector3(0.0f, 1.0f, 0.0f));

/*State::instance().display_text("Loading Q3BSP...");
State::instance().render();
_bsp.reset(new Q3BSP("sq3t2"));
if(!_bsp->load("")) {
    LOG_ERROR("Error loading Q3BSP!" << std::endl);
}

if(!_bsp->load_material(material_dir(), "scene")) {
    LOG_ERROR("Error loading Q3BSP material!" << std::endl);
}*/

    if(!scan_global_ambient_color(lexer)) {
        return false;
    }

    State::instance().display_text("Loading models...");
    State::instance().render();

    if(!scan_models(lexer)) {
        return false;
    }

    State::instance().display_text("Loading actors...");
    State::instance().render();

    if(!scan_renderables(lexer)) {
        return false;
    }

    State::instance().display_text("Loading lights...");
    State::instance().render();

    if(!scan_lights(lexer)) {
        return false;
    }

    _loaded = true;
    return true;
}

void Scene::unload()
{
    _loaded = false;

    _map.reset();
//_bsp.reset();

    _renderables.clear();
}

void Scene::update(double dt)
{
    State::instance().player()->think(dt);
    BOOST_FOREACH(boost::shared_ptr<Renderable> renderable, _renderables) {
        if(!renderable->is_static()) {
            renderable->think(dt);
        }
    }
}

void Scene::render()
{
    _camera->look();
    if(!_loaded) {
        Renderer::instance().render(*_camera);
        return;
    }

    // TODO: we should call map->render() here and let it decide which actors to register
    BOOST_FOREACH(boost::shared_ptr<Renderable> renderable, _renderables) {
        if(!renderable->is_static()) {
            boost::dynamic_pointer_cast<Actor, Renderable>(renderable)->animate();
        }
        Renderer::instance().register_renderable(*_camera, renderable);
    }

    // TODO: same for the lights
    /*if(State::instance().render_lights()) {
        BOOST_FOREACH(boost::shared_ptr<Light> light, _map->lights()) {
            if(light->enabled()) {
                Renderer::instance().register_renderable(*_camera, light);
            }
        }
    }*/

    // TODO: and obviously this would change as well
    Renderer::instance().render(*_camera, *_map);
}

bool Scene::scan_map(Lexer& lexer)
{
    if(!lexer.match(MAP)) {
        return false;
    }

    std::string name;
    if(!lexer.string_literal(name)) {
        return false;
    }

    _map.reset(new D3Map(name));
    if(!_map->load("")) {
        return false;
    }

    if(!_map->load_material(material_dir(), "scene")) {
        return false;
    }

    return true;
}

bool Scene::scan_global_ambient_color(Lexer& lexer)
{
    if(!lexer.match(GLOBAL_AMBIENT_COLOR)) {
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

    Color global_ambient_color(r, g, b, a);
    LOG_DEBUG("Setting global ambient color to " << global_ambient_color.str() << std::endl);
    Light::global_ambient_color(global_ambient_color);

    return true;
}

bool Scene::scan_models(Lexer& lexer)
{
    if(!lexer.match(MODELS)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    while(!lexer.check_token(CLOSE_BRACE)) {
        std::string path;
        if(!lexer.string_literal(path)) {
            return false;
        }

        std::string name;
        if(!lexer.string_literal(name)) {
            return false;
        }

        if(!ModelManager::instance().load_model(path, name)) {
            return false;
        }

        size_t acount;
        if(!lexer.size_literal(acount)) {
            return false;
        }

        for(size_t i=0; i<acount; ++i) {
            std::string anim;
            if(!lexer.string_literal(anim)) {
                return false;
            }

            if(!ModelManager::instance().load_animation(path, name, anim)) {
                return false;
            }
        }
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    return true;
}

bool Scene::scan_renderables(Lexer& lexer)
{
    if(!lexer.match(RENDERABLES)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    while(!lexer.check_token(CLOSE_BRACE)) {
        std::string type;
        if(!lexer.string_literal(type)) {
            return false;
        }

        if("static" == type) {
            if(!scan_static(lexer)) {
                return false;
            }
        } else {
            if(!scan_actor(lexer, type)) {
                return false;
            }
        }
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    return true;
}

bool Scene::scan_static(Lexer& lexer)
{
    std::string model;
    if(!lexer.string_literal(model)) {
        return false;
    }

    std::string name;
    if(!lexer.string_literal(name)) {
        return false;
    }

    boost::shared_ptr<Static> renderable(new Static(name));
    renderable->init();

    renderable->model(ModelManager::instance().model(model));
    if(!renderable->has_model()) {
        return false;
    }

    float x;
    if(!lexer.float_literal(x)) {
        return false;
    }

    float y;
    if(!lexer.float_literal(y)) {
        return false;
    }

    float z;
    if(!lexer.float_literal(z)) {
        return false;
    }

    renderable->position(Position(x, y, z));

    if(!renderable->load_material()) {
        return false;
    }

    _renderables.push_back(renderable);
    return true;
}

bool Scene::scan_actor(Lexer& lexer, const std::string& type)
{
    std::string model;
    if(!lexer.string_literal(model)) {
        return false;
    }

    std::string name;
    if(!lexer.string_literal(name)) {
        return false;
    }

    boost::shared_ptr<Actor> actor(Actor::new_actor(type, name));
    if(!actor) {
        return false;
    }
    actor->init();

    actor->model(ModelManager::instance().model(model));
    if(!actor->has_model()) {
        return false;
    }

    float x;
    if(!lexer.float_literal(x)) {
        return false;
    }

    float y;
    if(!lexer.float_literal(y)) {
        return false;
    }

    float z;
    if(!lexer.float_literal(z)) {
        return false;
    }

    actor->position(Position(x, y, z));

    std::string anim;
    if(!lexer.string_literal(anim)) {
        return false;
    }

    boost::shared_ptr<Animation> animation(ModelManager::instance().animation(model, anim));
    if(!animation) {
        return false;
    }
    actor->animation(animation);

    size_t cframe;
    if(!lexer.size_literal(cframe)) {
        return false;
    }
    actor->current_frame(cframe);

    if(!actor->load_material()) {
        return false;
    }

    LOG_INFO("Pick id=" << actor->pick_id() << ", color=" << actor->pick_color().str() << std::endl);
    _renderables.push_back(actor);
    return true;
}

bool Scene::scan_lights(Lexer& lexer)
{
    if(!lexer.match(LIGHTS)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    while(!lexer.check_token(CLOSE_BRACE)) {
        std::string type;
        if(!lexer.string_literal(type)) {
            return false;
        }

        float x;
        if(!lexer.float_literal(x)) {
            return false;
        }

        float y;
        if(!lexer.float_literal(y)) {
            return false;
        }

        float z;
        if(!lexer.float_literal(z)) {
            return false;
        }

        std::string colordef;
        if(!lexer.string_literal(colordef)) {
            return false;
        }

        boost::shared_ptr<Light> light;
        if(type == "directional") {
            boost::shared_ptr<DirectionalLight> directional(new DirectionalLight());
            directional->direction(Direction(x, y, z));

            light = directional;
        } else if(type == "positional") {
            boost::shared_ptr<PositionalLight> positional(new PositionalLight());
            positional->position(Position(x, y, z));

            if(!scan_light_positional(lexer, positional)) {
                return false;
            }
            light = positional;
        } else if(type == "spot") {
            boost::shared_ptr<SpotLight> spot(new SpotLight());
            spot->position(Position(x, y, z));

            if(!scan_light_spotlight(lexer, spot)) {
                return false;
            }
            light = spot;
        } else {
            LOG_ERROR("Invalid light type '" << type << "'" << std::endl);
            return false;
        }

        if(!light->load_colordef(colordef)) {
            return false;
        }

        light->enable();
        _map->add_light(light);
    }

    if(_map->light_count() > Map::MAX_LIGHTS) {
        LOG_CRITICAL("Only " << Map::MAX_LIGHTS << " lights allowed per-scene!" << std::endl);
        return false;
    }

    // fill in any remaining lights and disable them
    for(size_t i=_map->light_count(); i<Map::MAX_LIGHTS; ++i) {
        boost::shared_ptr<Light> light(new DirectionalLight());
        light->enable(false);
        _map->add_light(light);
    }

    return true;
}

bool Scene::scan_light_positional(Lexer& lexer, boost::shared_ptr<PositionalLight> positional)
{
    float constant_atten;
    if(!lexer.float_literal(constant_atten)) {
        return false;
    }
    positional->constant_attenuation(constant_atten);

    float linear_atten;
    if(!lexer.float_literal(linear_atten)) {
        return false;
    }
    positional->linear_attenuation(linear_atten);

    float quadratic_atten;
    if(!lexer.float_literal(quadratic_atten)) {
        return false;
    }
    positional->quadratic_attenuation(quadratic_atten);

    return true;
}

bool Scene::scan_light_spotlight(Lexer& lexer, boost::shared_ptr<SpotLight> spot)
{
    float x;
    if(!lexer.float_literal(x)) {
        return false;
    }

    float y;
    if(!lexer.float_literal(y)) {
        return false;
    }

    float z;
    if(!lexer.float_literal(z)) {
        return false;
    }
    spot->direction(Direction(x, y, z));

    float cutoff;
    if(!lexer.float_literal(cutoff)) {
        return false;
    }
    spot->cutoff(cutoff);

    float exponent;
    if(!lexer.float_literal(exponent)) {
        return false;
    }
    spot->exponent(exponent);

    return scan_light_positional(lexer, spot);
}
