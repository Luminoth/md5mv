#if !defined __CLIENTCONFIGURATION_H__
#define __CLIENTCONFIGURATION_H__

#include "string_util.h"
#include "util.h"
#include "Configuration.h"

class ClientConfiguration : public Configuration
{
public:
    static ClientConfiguration& instance();

public:
    virtual ~ClientConfiguration() throw();

public:
    bool load_config();
    bool save_config();
    bool config_exists() const;

public:
    void render_mode(const std::string& mode) { set("renderer", "mode", mode); }
    std::string render_mode() const { return get("renderer", "mode"); }
    bool render_mode_vertex() const { return "vertex" == get("renderer", "mode"); }
    bool render_mode_bump() const { return "bump" == get("renderer", "mode"); }

    void render_shadows(bool enable) { set("renderer", "shadows", enable ? "true" : "false"); }
    bool render_shadows() const { return to_boolean(get("renderer", "shadows").c_str()); }

    int video_width() const { return std::atoi(get("video", "width").c_str()); }
    int video_height() const { return std::atoi(get("video", "height").c_str()); }
    int video_depth() const { return std::atoi(get("video", "depth").c_str()); }
    bool video_fullscreen() const { return to_boolean(get("video", "fullscreen").c_str()); }
    bool video_sync() const { return to_boolean(get("video", "sync").c_str()); }
    int video_maxfps() const { return std::atoi(get("video", "maxfps").c_str()); }

    float game_fov() const { return std::atof(get("game", "fov").c_str()); }

    float input_sensitivity() const { return std::atof(get("input", "sensitivity").c_str()); }

public:
    virtual void validate() const throw(ConfigurationError);
    virtual void on_save();

private:
    ClientConfiguration();
    DISALLOW_COPY_AND_ASSIGN(ClientConfiguration);
};

#endif
