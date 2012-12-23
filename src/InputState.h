#if !defined __INPUT_H__
#define __INPUT_H__

#include "InputSym.h"

class InputState
{
private:
    struct MouseState
    {
        bool buttons[MouseButton_max];
        int x, y;
        int wheel_x, wheel_y;

        MouseState();
        virtual ~MouseState() throw();
    };

public:
    static InputState& instance();

public:
    virtual ~InputState() throw();

public:
// TODO: move this
void update(const SDL_Event& event);

    // returns true if the keyboard key is pressed
    bool keyboard_key(const InputKeySym& key) const { return _keyboard_state[key]; }

    // gets the mouse position
    int mouse_x() const { return _mouse_state.x; }
    int mouse_y() const { return _mouse_state.y; }
    void reset_mouse(int x, int y);

    // gets the mousewheel scroll amount
    int mouse_wheel_x() const { return _mouse_state.wheel_x; }
    int mouse_wheel_y() const { return _mouse_state.wheel_y; }
    void reset_mouse_wheel();

    // returns true if the mouse button is pressed
    bool mouse_button(const MouseSym& button) const { return _mouse_state.buttons[button]; }

private:
    bool _keyboard_state[InputKeySym_last];
    MouseState _mouse_state;

private:
    InputState();
    DISALLOW_COPY_AND_ASSIGN(InputState);
};

#endif
