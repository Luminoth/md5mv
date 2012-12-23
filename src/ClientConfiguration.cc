#include "pch.h"
#include "common.h"
#include "ClientConfiguration.h"

ClientConfiguration& ClientConfiguration::instance()
{
    static boost::shared_ptr<ClientConfiguration> configuration;
    if(!configuration) {
        configuration.reset(new ClientConfiguration());
    }
    return *configuration;
}

ClientConfiguration::ClientConfiguration()
    : Configuration()
{
    set_default("renderer", "mode", "bump");
    set_default("renderer", "shadows", "true");

    set_default("video", "width", "1280");
    set_default("video", "height", "720");
    set_default("video", "depth", "32");
    set_default("video", "fullscreen", "false");
    set_default("video", "sync", "false");
    set_default("video", "maxfps", "-1");

    set_default("game", "fov", "75.0");

    set_default("input", "sensitivity", "1.0");

    set_default("logging", "filename", (home_conf_dir() / "md5mv.log").string());

    load_defaults();

    generate_header("Client configuration file");
}

ClientConfiguration::~ClientConfiguration() throw()
{
}

bool ClientConfiguration::load_config()
{
    return load(client_conf());
}

bool ClientConfiguration::save_config()
{
    return save(client_conf());
}

bool ClientConfiguration::config_exists() const
{
    return exists(client_conf());
}

void ClientConfiguration::validate() const throw(ConfigurationError)
{
    Configuration::validate();

    if(!is_int(get("video", "width"))) {
        throw ConfigurationError("Video width must be an integer");
    }

    if(!is_int(get("video", "height"))) {
        throw ConfigurationError("Video height must be an integer");
    }

    if(!is_int(get("video", "depth"))) {
        throw ConfigurationError("Video depth must be an integer");
    }

    if(!is_int(get("video", "maxfps"))) {
        throw ConfigurationError("Video maxfps must be an integer");
    }

    if(!is_double(get("game", "fov"))) {
        throw ConfigurationError("Game fov must be a float");
    }

    if(!is_double(get("input", "sensitivity"))) {
        throw ConfigurationError("Input sensitivity must be a float");
    }

    if(video_maxfps() > 0 && video_maxfps() < 30) {
        throw ConfigurationError("Video maxfps must be at least 30!");
    }

    if(!render_mode_vertex() && !render_mode_bump()) {
        throw ConfigurationError("Invalid render mode: " + render_mode());
    }
}

void ClientConfiguration::on_save()
{
}
