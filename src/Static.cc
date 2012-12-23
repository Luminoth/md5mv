#include "pch.h"
#include "State.h"
#include "Static.h"

Static::Static(const std::string& name)
    : Renderable(name)
{
}

Static::~Static() throw()
{
}

void Static::on_render_unlit(const Camera& camera) const
{
    const AABB& bounds(absolute_bounds());
    const Position& center(bounds.center());
    _nameplate.render(name(), Vector3(center.x(), bounds.maximum().y(), center.z()), camera);
}