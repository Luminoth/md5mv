#if !defined __STATE_H__
#define __STATE_H__

#include "Font.h"
#include "Shader.h"

class Player;
class Scene;

// TODO: replace with a configuration class
class State
{
public:
    // NOTE: need a valid OpenGL context before calling this
    static State& instance();

private:
    static Logger& logger;

public:
    virtual ~State() throw();

public:
    bool load_font(const std::string& name, size_t height, const Color& color);
    bool load_shaders();
    bool load_scene(const std::string& name);

    void render() const;

    const std::string& display_text() const { return _display_text; }
    void display_text(const std::string& text) { _display_text = text; }
    void append_display_text(const std::string& text) { _display_text += text; }

public:
    boost::shared_ptr<Scene> scene() const { return _scene; }
    boost::shared_ptr<Player> player() const { return _player; }

    Shader& ambient_shader() { return _ambient_shader; }

    Shader& vertex_shader() { return _vertex_shader; }
    Shader& bump_shader() { return _bump_shader; }

    Shader& pick_shader() { return _pick_shader; }

    Shader& deferred_shader() { return _deferred_shader; }

    Shader& shadow_point_shader() { return _shadow_point_shader; }
    Shader& shadow_infinite_shader() { return _shadow_infinite_shader; }

    Shader& simple_shader() { return _simple_shader; }
    Shader& gray_shader() { return _gray_shader; }
    Shader& red_shader() { return _red_shader; }
    Shader& green_shader() { return _green_shader; }
    Shader& blue_shader() { return _blue_shader; }

    // debugging
    bool render_wireframe() const { return _render_wireframe; }
    void render_wireframe(bool enable) { _render_wireframe = enable; }

    bool render_skeleton() const { return _render_skeleton; }
    void render_skeleton(bool enable) { _render_skeleton = enable; }

    bool render_normals() const { return _render_normals; }
    void render_normals(bool enable) { _render_normals = enable; }

    bool render_bounds() const { return _render_bounds; }
    void render_bounds(bool enable) { _render_bounds = enable; }

    bool render_lights() const { return _render_lights; }
    void render_lights(bool enable) { _render_lights = enable; }

    const TextFont& font() const { return _font; }

bool rotate_actors() const { return _rotate_actors; }
void rotate_actors(bool enable) { _rotate_actors = enable; }

private:
    void render_2d() const;

private:
    boost::shared_ptr<Scene> _scene;
    boost::shared_ptr<Player> _player;

    Shader _ambient_shader, _vertex_shader, _bump_shader, _pick_shader, _deferred_shader;
    Shader _shadow_point_shader, _shadow_infinite_shader;
    Shader _simple_shader, _gray_shader, _red_shader, _green_shader, _blue_shader;

    TextFont _font;
    std::string _display_text;

    bool _render_wireframe, _render_skeleton, _render_normals, _render_bounds, _render_lights;
bool _rotate_actors;

private:
    State();
    DISALLOW_COPY_AND_ASSIGN(State);
};

#endif
