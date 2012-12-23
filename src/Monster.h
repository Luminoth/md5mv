#if !defined __MONSTER_H__
#define __MONSTER_H__

#include "Actor.h"

class Monster : public Actor
{
public:
    explicit Monster(const std::string& name);
    virtual ~Monster() throw();

private:
    Monster();
    DISALLOW_COPY_AND_ASSIGN(Monster);
};

#endif
