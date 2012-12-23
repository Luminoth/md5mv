#include "pch.h"
#include "Monster.h"

Monster::Monster(const std::string& name)
    : Actor(name)
{
}

Monster::~Monster() throw()
{
}
