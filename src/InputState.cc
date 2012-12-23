#include "pch.h"
#include "InputState.h"

InputState::MouseState::MouseState()
    : x(0), y(0), wheel_x(0), wheel_y(0)
{
    for(unsigned int i=0; i<MouseButton_max; ++i) {
        buttons[i] = false;
    }
}

InputState::MouseState::~MouseState() throw()
{
}

InputState& InputState::instance()
{
    static boost::shared_ptr<InputState> input_state;
    if(!input_state) {
        input_state.reset(new InputState());
    }
    return *input_state;
}

InputState::InputState()
{
    ZeroMemory(_keyboard_state, InputKeySym_last * sizeof(bool));
}

InputState::~InputState() throw()
{
}

void InputState::update(const SDL_Event& event)
{
    switch(event.type)
    {
    case SDL_MOUSEMOTION:
        _mouse_state.x = event.motion.x;
        _mouse_state.y = event.motion.y;
        break;
    case SDL_MOUSEBUTTONDOWN:
        _mouse_state.x = event.button.x;
        _mouse_state.y = event.button.y;

        if(event.button.button == SDL_BUTTON_WHEELDOWN) {
            _mouse_state.wheel_y -= 1.0f;
        } else {
            // NOTE: this will be broken if SDL changes things
            _mouse_state.buttons[event.button.button - 1] = true;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        _mouse_state.x = event.button.x;
        _mouse_state.y = event.button.y;

        if(event.button.button == SDL_BUTTON_WHEELUP) {
            _mouse_state.wheel_y += 1.0f;
        } else {
            // NOTE: this will be broken if SDL changes things
            _mouse_state.buttons[event.button.button - 1] = false;
        }
        break;
    /*case SDL_MOUSEWHEEL:
        _mouse_state.wheel_x = event.wheel.x;
        _mouse_state.wheel_y = event.wheel.y;
        break;*/
    case SDL_KEYDOWN:
        _keyboard_state[event.key.keysym.sym] = true;
        break;
    case SDL_KEYUP:
        _keyboard_state[event.key.keysym.sym] = false;
        break;
    default:
        break;
    }
}

void InputState::reset_mouse(int x, int y)
{
    _mouse_state.x = x;
    _mouse_state.y = y;
    SDL_WarpMouse(x, y);
}

void InputState::reset_mouse_wheel()
{
    _mouse_state.wheel_x = 0;
    _mouse_state.wheel_y = 0;
}
