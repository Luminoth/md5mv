#if !defined __MD5ANIMATION_H__
#define __MD5ANIMATION_H__

#include "Animation.h"
#include "Model.h"

class Lexer;

class MD5Animation : public Animation
{
private:
    struct AnimationJoint : public Skeleton::Joint
    {
        int acflag, acstart;
    };
    typedef boost::shared_array<AnimationJoint> AnimationSkeleton;

    class MD5Frame : public Animation::Frame
    {
    public:
        MD5Frame() : Animation::Frame() {}
        virtual ~MD5Frame() throw() {}

    public:
        boost::shared_array<float> animated_components;
    };

public:
    static std::string extension() { return ".md5anim"; }

private:
    static Logger& logger;

public:
    explicit MD5Animation(const std::string& name);
    virtual ~MD5Animation() throw();

public:
    virtual void build_skeletons();

private:
    virtual bool on_load(const boost::filesystem::path& path);
    virtual void on_unload() throw();

private:
    bool scan_version(Lexer& lexer);
    bool scan_commandline(Lexer& lexer);
    int scan_num_frames(Lexer& lexer);
    int scan_num_joints(Lexer& lexer);
    int scan_frame_rate(Lexer& lexer);
    int scan_num_animated_components(Lexer& lexer);
    bool scan_hierarchy(Lexer& lexer, int count);
    bool scan_bounds(Lexer& lexer, int count);
    bool scan_baseframe(Lexer& lexer, int count);
    bool scan_frames(Lexer& lexer, int count);

private:
    int _version;
    std::string _commandline;

    AnimationSkeleton _askeleton;
    int _account;

private:
    MD5Animation();
    DISALLOW_COPY_AND_ASSIGN(MD5Animation);
};

#endif
