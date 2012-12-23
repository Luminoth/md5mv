#include "pch.h"
#include "common.h"
#include "State.h"
#include "Map.h"

const size_t Map::MAX_LIGHTS = 8;
Logger& Map::logger(Logger::instance("md5mv.Map"));

Map::Map(const std::string& name)
    : _name(name)
{
}

Map::~Map() throw()
{
}

void Map::unload()
{
    _lights.clear();

    on_unload();
}

bool Map::load_material(const boost::filesystem::path& path, const std::string& name)
{
    if(!_material.load(path, name)) {
        return false;
    }

    return true;
}
