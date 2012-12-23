#if !defined __SOUND_H__
#define __SOUND_H__

class Sound
{
public:
    static bool init(int* argc, char** argv);
    static void shutdown();

private:
    static Logger& logger;

public:
    virtual ~Sound() throw();

private:
    Sound();
    DISALLOW_COPY_AND_ASSIGN(Sound);
};

#endif
