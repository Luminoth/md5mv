#include "pch.h"
#include "common.h"
#include "Lexer.h"
#include "Animation.h"

Logger& Animation::logger(Logger::instance("md5mv.Animation"));

Animation::Animation(const std::string& name)
    : _name(name), _frate(0), _fduration(0.0)
{
}

Animation::~Animation() throw()
{
    unload();
}

bool Animation::load(const boost::filesystem::path& path)
{
    unload();
    return on_load(path);
}

void Animation::unload() throw()
{
    _frames.clear();
    _skeletons.clear();

    _skeleton.reset();

    _frate = 0;
    _fduration = 0.0;

    on_unload();
}

void Animation::interpolate_skeleton(size_t current_frame, size_t next_frame, Skeleton& sk, double frame_percent) const
{
    const Skeleton &cframe(skeleton(current_frame)), &nframe(skeleton(next_frame));
    for(size_t i=0; i<this->joint_count(); ++i) {
        const Skeleton::Joint& cfjoint(cframe.joint(i)), nfjoint(nframe.joint(i));

        Skeleton::Joint joint;
        joint.parent = cfjoint.parent >= 0 ? cfjoint.parent : -1;
        joint.position = cfjoint.position.lerp(nfjoint.position, frame_percent);
        joint.orientation = cfjoint.orientation.slerp(nfjoint.orientation, frame_percent);

        sk.add_joint(joint);
    }
}

void Animation::frame_rate(int rate)
{
    if(rate <= 0.0) {
        return;
    }

    _frate = rate;
    _fduration = 1.0 / rate;
}

bool Animation::on_load(const boost::filesystem::path& path)
{
    boost::filesystem::path filename(model_dir() / path / (name() + extension()));
    LOG_INFO("Loading animation from '" << filename << "'" << std::endl);

    Lexer lexer;
    if(!lexer.load(filename)) {
        return false;
    }

    if(!scan_header(lexer)) {
        return false;
    }

    return true;
}

bool Animation::scan_header(Lexer& lexer)
{
    std::string header;
    if(!lexer.string_literal(header)) {
        return false;
    }

    if("MDLANIM1" == header) {
        return true;
    }

    return false;
}
