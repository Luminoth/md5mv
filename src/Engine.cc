#include "pch.h"
#include "Camera.h"
#include "ClientConfiguration.h"
#include "GameUIController.h"
#include "Font.h"
#include "Player.h"
#include "Renderer.h"
#include "Scene.h"
#include "Sound.h"
#include "State.h"
#include "TextureManager.h"
#include "Engine.h"

Logger& Engine::logger(Logger::instance("md5mv.Engine"));

Engine& Engine::instance()
{
    static boost::shared_ptr<Engine> engine;
    if(!engine) {
        engine.reset(new Engine());
    }
    return *engine;
}

Engine::Engine()
    : _quit(false), _start_time(0.0), _frame_count(0), _frame_start(0.0)
{
}

Engine::~Engine() throw()
{
}

bool Engine::init(int argc, char** argv)
{
    ClientConfiguration& config(ClientConfiguration::instance());

    std::srand(std::time(NULL));

    UIController::controller(GameUIController::new_controller());

    if(!Renderer::instance().create_window(config.video_width(), config.video_height(), config.video_depth(), config.video_fullscreen(), "MD5 Model Loader")) {
        LOG_CRITICAL("Unable to create the window!" << std::endl);
        return false;
    }

    if(!TextureManager::instance().init()) {
        return false;
    }

    if(!TextFont::init()) {
        return false;
    }

    if(!State::instance().load_font("courier", 24, Color(1.0f, 1.0f, 1.0f, 1.0f))) {
        return false;
    }

    if(!State::instance().load_shaders()) {
        return false;
    }

    State::instance().display_text("Initializing sound...");
    State::instance().render();

    if(!Sound::init(&argc, argv)) {
        LOG_CRITICAL("Unable to initialize audio!" << std::endl);
        return false;
    }

    State::instance().display_text("Loading scene...");
    State::instance().render();

    if(!State::instance().load_scene("default")) {
        return false;
    }
    State::instance().scene()->camera().attach(boost::dynamic_pointer_cast<Actor, Player>(State::instance().player()));

    State::instance().display_text("Finished loading scene!");
    State::instance().render();

    SDL_ShowCursor(SDL_DISABLE);
    //SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_WarpMouse(Renderer::instance().window_width() >> 1, Renderer::instance().window_height() >> 1);

    return true;
}

void Engine::run()
{
    UIController::controller()->handle_events();
    SDL_WarpMouse(Renderer::instance().window_width() >> 1, Renderer::instance().window_height() >> 1);

    _start_time = get_time();
    event_loop();

    LOG_INFO("Runtime statistics:" << std::endl
        << "Frames Rendered: " << _frame_count << std::endl
        << "Runtime: " << runtime() << "s" << std::endl
        << "Average FPS: " << average_fps() << std::endl);
}

void Engine::shutdown()
{
    Sound::shutdown();
    TextFont::shutdown();
    SDL_Quit();
}

double Engine::runtime() const
{
    return get_time() - _start_time;
}

double Engine::frame_time() const
{
    return get_time() - _frame_start;
}

double Engine::average_fps() const
{
    return _frame_count / runtime();
}

double Engine::current_fps() const
{
    return 1.0 / frame_time();
}

void Engine::event_loop()
{
    double last_render = get_time();
    while(!should_quit()) {
        _frame_start = get_time();

        boost::shared_ptr<UIController> controller(UIController::controller());
        controller->handle_events();
        controller->handle_input();

        controller->update(get_time() - last_render);
        controller->render();

        last_render = _frame_start;
        _frame_count++;

        std::stringstream txt;
        txt << "Current FPS: " << current_fps() << ", Average FPS: " << average_fps();
        State::instance().display_text(txt.str());

        // check for any untrapped exceptions
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            LOG_ERROR("Untrapped OpenGL error: " << gluErrorString(error) << std::endl);
            quit();
        }

        usleep(1);
        rate_limit();
    }
}

void Engine::rate_limit()
{
    ClientConfiguration& config(ClientConfiguration::instance());
    if(config.video_maxfps() <= 0) {
        return;
    }

    const double target_framespan = 1.0 / config.video_maxfps();
    const double frame_time = this->frame_time();

    // TODO: if config.video_sync() is true,
    // we should sync to the refresh rather than the framespan
    if(frame_time < target_framespan) {
        double sleep_time = (target_framespan - frame_time) * 1000000.0;
        usleep(static_cast<int>(sleep_time));
    }
}
