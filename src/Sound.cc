#include "pch.h"
#include "Sound.h"

Logger& Sound::logger(Logger::instance("md5mv.Sound"));

bool Sound::init(int* argc, char** argv)
{
    LOG_INFO("Initializing audio..." << std::endl);

    alutInit(argc, argv);

    return true;
}

void Sound::shutdown()
{
    alutExit();
}

Sound::~Sound() throw()
{
}
