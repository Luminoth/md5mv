#if !defined __COMMON_H__
#define __COMMON_H__

#include "Vector.h"

// sleep time in microseconds
inline int thread_sleep_time()
{
#if defined DEBUG
    return 1000;
#else
    return 100;
#endif
}

inline boost::filesystem::path install_dir()
{
    return "";
}

inline boost::filesystem::path bin_dir()
{
    return install_dir() / "bin";
}

inline boost::filesystem::path conf_dir()
{
    return install_dir() / "etc";
}

boost::filesystem::path home_conf_dir();

inline boost::filesystem::path client_conf()
{
    return home_conf_dir() / "md5mv.conf";
}

inline boost::filesystem::path data_dir()
{
    return /*install_dir() /*/ "share";
}

inline boost::filesystem::path scene_dir()
{
    return data_dir() / "scenes";
}

inline boost::filesystem::path light_dir()
{
    return data_dir() / "lights";
}

inline boost::filesystem::path model_dir()
{
    return data_dir() / "models";
}

inline boost::filesystem::path shader_dir()
{
    return data_dir() / "shaders";
}

inline boost::filesystem::path material_dir()
{
    return data_dir() / "materials";
}

inline boost::filesystem::path font_dir()
{
    return data_dir() / "fonts";
}

inline boost::filesystem::path map_dir()
{
    return data_dir() / "maps";
}

inline boost::filesystem::path texture_dir()
{
    return data_dir() / "textures";
}

Vector3 swizzle(const Vector3& v);

#endif
