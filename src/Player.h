#if !defined __PLAYER_H__
#define __PLAYER_H__

#include "Actor.h"

class Player : public Actor
{
private:
    static const float MOVE_SPEED;

public:
    Player();
    virtual ~Player() throw();

private:
    virtual bool on_think(double dt);

private:
    float _pitch;

private:
    DISALLOW_COPY_AND_ASSIGN(Player);
};

#endif
