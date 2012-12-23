#include "pch.h"
#include "Engine.h"
#include "InputState.h"
#include "UIController.h"

boost::shared_ptr<UIController> UIController::current_controller;
std::stack<boost::shared_ptr<UIController> > UIController::controller_stack;

UIController::UIController()
{
}

UIController::~UIController() throw()
{
}

void UIController::handle_events()
{
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        InputState::instance().update(event);
        switch(event.type)
        {
        case SDL_ACTIVEEVENT:
            // TODO: handle this
            break;
        case SDL_KEYDOWN:
            key_down(event.key.keysym.sym);
            break;
        case SDL_QUIT:
            Engine::instance().quit();
            break;
        }
    }
}

void UIController::handle_input()
{
    if(InputState::instance().keyboard_key(InputKeySym_escape)) {
        Engine::instance().quit();
        return;
    }
    InputState::instance().reset_mouse_wheel();
}
