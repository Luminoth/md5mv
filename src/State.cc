#include "pch.h"
#include <iostream>
#include "common.h"
#include "Player.h"
#include "Renderer.h"
#include "Scene.h"
#include "State.h"

Logger& State::logger(Logger::instance("md5mv.State"));

State& State::instance()
{
    static boost::shared_ptr<State> state;
    if(!state) {
        state.reset(new State());
    }
    return *state;
}

State::State()
    : _scene(new Scene()), _player(new Player()),
        _ambient_shader("ambient"), _vertex_shader("vertex"), _bump_shader("bump"),
        _pick_shader("pick"), _deferred_shader("deferred"),
        _shadow_point_shader("shadow_point"), _shadow_infinite_shader("shadow_infinite"),
        _simple_shader("simple"), _gray_shader("gray"), _red_shader("red"), _green_shader("green"), _blue_shader("blue"),
        _render_wireframe(false), _render_skeleton(false), _render_normals(false), _render_bounds(false), _render_lights(true),
_rotate_actors(false)
{
    _player->init();
}

State::~State() throw()
{
}

bool State::load_font(const std::string& name, size_t height, const Color& color)
{
    if(!_font.load(name, height)) {
        return false;
    }
    _font.color(color);
    return true;
}

bool State::load_shaders()
{
    try {
        _ambient_shader.create();
        _ambient_shader.read_shader(shader_dir() / "simple.vert");
        _ambient_shader.read_shader(shader_dir() / "simple.geom");
        _ambient_shader.read_shader(shader_dir() / "ambient.frag");
        _ambient_shader.bind_fragment_data_location(0, "fragment_color");
        _ambient_shader.link();

        _vertex_shader.create();
        _vertex_shader.read_shader(shader_dir() / "simple.vert");
        _vertex_shader.read_shader(shader_dir() / "vertex.geom");
        _vertex_shader.read_shader(shader_dir() / "vertex.frag");
        _vertex_shader.bind_fragment_data_location(0, "fragment_color");
        _vertex_shader.link();

        _bump_shader.create();
        _bump_shader.read_shader(shader_dir() / "simple.vert");
        _bump_shader.read_shader(shader_dir() / "bump.geom");
        _bump_shader.read_shader(shader_dir() / "bump.frag");
        _bump_shader.bind_fragment_data_location(0, "fragment_color");
        _bump_shader.link();

        _shadow_point_shader.create();
        _shadow_point_shader.read_shader(shader_dir() / "shadow-point.vert");
        _shadow_point_shader.read_shader(shader_dir() / "shadow.frag");
        _shadow_point_shader.bind_fragment_data_location(0, "fragment_color");
        _shadow_point_shader.link();

        _shadow_infinite_shader.create();
        _shadow_infinite_shader.read_shader(shader_dir() / "shadow-infinite.vert");
        _shadow_infinite_shader.read_shader(shader_dir() / "shadow.frag");
        _shadow_infinite_shader.bind_fragment_data_location(0, "fragment_color");
        _shadow_infinite_shader.link();

        _pick_shader.create();
        _pick_shader.read_shader(shader_dir() / "no-geom.vert");
        _pick_shader.read_shader(shader_dir() / "pick.frag");
        _pick_shader.bind_fragment_data_location(0, "fragment_color");
        _pick_shader.link();

        _deferred_shader.create();
        _deferred_shader.read_shader(shader_dir() / "deferred.vert");
        _deferred_shader.read_shader(shader_dir() / "deferred.frag");
        _deferred_shader.bind_fragment_data_location(0, "fragment_color");
        _deferred_shader.link();

        _simple_shader.create();
        _simple_shader.read_shader(shader_dir() / "simple.vert");
        _simple_shader.read_shader(shader_dir() / "simple.geom");
        _simple_shader.read_shader(shader_dir() / "simple-texture.frag");
        _simple_shader.bind_fragment_data_location(0, "fragment_color");
        _simple_shader.link();

        _gray_shader.create();
        _gray_shader.read_shader(shader_dir() / "no-geom.vert");
        _gray_shader.read_shader(shader_dir() / "simple-gray.frag");
        _gray_shader.bind_fragment_data_location(0, "fragment_color");
        _gray_shader.link();

        _red_shader.create();
        _red_shader.read_shader(shader_dir() / "no-geom.vert");
        _red_shader.read_shader(shader_dir() / "simple-red.frag");
        _red_shader.bind_fragment_data_location(0, "fragment_color");
        _red_shader.link();

        _green_shader.create();
        _green_shader.read_shader(shader_dir() / "no-geom.vert");
        _green_shader.read_shader(shader_dir() / "simple-green.frag");
        _green_shader.bind_fragment_data_location(0, "fragment_color");
        _green_shader.link();

        _blue_shader.create();
        _blue_shader.read_shader(shader_dir() / "no-geom.vert");
        _blue_shader.read_shader(shader_dir() / "simple-blue.frag");
        _blue_shader.bind_fragment_data_location(0, "fragment_color");
        _blue_shader.link();
    } catch(const ShaderError& e) {
        LOG_ERROR(e.what() << std::endl);
        return false;
    }

    return true;
}

bool State::load_scene(const std::string& name)
{
    if(!State::instance().scene()->load(name)) {
        LOG_ERROR("Error loading the scene!" << std::endl);
        return false;
    }

    return true;
}

void State::render() const
{
    _scene->render();
    render_2d();

    SDL_GL_SwapBuffers();
}

void State::render_2d() const
{
    glDisable(GL_DEPTH_TEST);

    Renderer::instance().push_mvp_matrix();
    Renderer::instance().mvp_identity();
    Renderer::instance().orthographic(0.0f, Renderer::instance().window_width(), 0.0f, Renderer::instance().window_height());

    _font.render(_display_text, Vector2(10.0f, 10.0f));

    Renderer::instance().pop_mvp_matrix();

    glEnable(GL_DEPTH_TEST);
}
