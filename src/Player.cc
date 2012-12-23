#include "pch.h"
#include "ClientConfiguration.h"
#include "InputState.h"
#include "Renderer.h"
#include "State.h"
#include "Player.h"

const float Player::MOVE_SPEED = 100.0f;

Player::Player()
    : Actor("player"), _pitch(0.0f)
{
}

Player::~Player() throw()
{
}

bool Player::on_think(double dt)
{
    const ClientConfiguration& config(ClientConfiguration::instance());

    velocity(Vector3(0.0f, 0.0f, 0.0f));

    // TODO: this is FPS dependant and shouldn't be
    int mx, my, hw=Renderer::instance().window_width() >> 1, hh=Renderer::instance().window_height() >> 1;
    SDL_GetMouseState(&mx, &my);

    float p = (hh - my) * config.input_sensitivity() * dt;
    _pitch += p;

    /*// 60 degrees in radians
    static const float dr60 = 1.04719755;
std::cout << "p: " << p << "_pitch: " << _pitch << std::endl;
    if(_pitch > dr60) {
        p -= _pitch - dr60;
        _pitch = dr60;
    } else if(_pitch < -dr60) {
        p += _pitch + dr60;
        _pitch = -dr60;
    }*/

    pitch(p);
    yaw((hw - mx) * config.input_sensitivity() * dt);
    SDL_WarpMouse(hw, hh);

    Vector3 v;
    if(InputState::instance().keyboard_key(InputKeySym_w)) {
        v.z(v.z() - MOVE_SPEED);
    }

    if(InputState::instance().keyboard_key(InputKeySym_s)) {
        v.z(v.z() + MOVE_SPEED);
    }

    if(InputState::instance().keyboard_key(InputKeySym_a)) {
        v.x(v.x() - MOVE_SPEED);
    }

    if(InputState::instance().keyboard_key(InputKeySym_d)) {
        v.x(v.x() + MOVE_SPEED);
    }

    // rotate the velocity
    v = orientation() * v;

    // constrain movement along the y-axis
    v.y(0.0f);

    if(InputState::instance().keyboard_key(InputKeySym_space)) {
        v.y(v.y() + MOVE_SPEED);
    }

    if(InputState::instance().keyboard_key(InputKeySym_lshift)) {
        v.y(v.y() - MOVE_SPEED);
    }

    velocity(v);
    return true;
}
