#include "pch.h"
#include "Character.h"

Character::Character(const std::string& name)
    : Actor(name)
{
}

Character::~Character() throw()
{
}
