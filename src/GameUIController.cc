#include "pch.h"
#include "ClientConfiguration.h"
#include "Light.h"
#include "Renderer.h"
#include "Scene.h"
#include "State.h"
#include "GameUIController.h"

Logger& GameUIController::logger(Logger::instance("md5mv.GameUIController"));

boost::shared_ptr<GameUIController> GameUIController::new_controller()
{
    return boost::shared_ptr<GameUIController>(new GameUIController());
}

GameUIController::GameUIController()
{
}

GameUIController::~GameUIController() throw()
{
}

void GameUIController::key_down(SDLKey key)
{
    ClientConfiguration& config(ClientConfiguration::instance());

    switch(key)
    {
    case SDLK_b:
        State::instance().render_bounds(!State::instance().render_bounds());
        break;
    case SDLK_f:
        State::instance().render_wireframe(!State::instance().render_wireframe());
        glPolygonMode(GL_FRONT_AND_BACK, State::instance().render_wireframe() ? GL_LINE : GL_FILL);
        break;
    case SDLK_h:
        config.render_shadows(!config.render_shadows());
        break;
    case SDLK_k:
        State::instance().render_skeleton(!State::instance().render_skeleton());
        break;
    case SDLK_l:
        Light::lighting_enable(!Light::lighting_enabled());
        break;
    case SDLK_m:
        Renderer::instance().print_video_memory_details();
        break;
    case SDLK_n:
        State::instance().render_normals(!State::instance().render_normals());
        break;
    case SDLK_r:
        State::instance().rotate_actors(!State::instance().rotate_actors());
        break;
    case SDLK_v:
        config.render_mode(config.render_mode_bump() ? "vertex" : "bump");
        break;
    case SDLK_F12:
        Renderer::instance().screenshot("screenshot.png");
        break;
    default:
        break;
    }
}

void GameUIController::update(double elapsed)
{
    State::instance().scene()->update(elapsed);
}

void GameUIController::render() const
{
    State::instance().render();
}