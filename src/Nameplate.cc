#include "pch.h"
#include "State.h"
#include "Nameplate.h"

Nameplate::Nameplate()
{
}

Nameplate::~Nameplate() throw()
{
}

void Nameplate::render(const std::string& name, const Position& position, const Camera& camera) const
{
    // render the name centered on top of us
    State::instance().font().render(name, position, Vector2(1.0f, 1.0f), true);
}
