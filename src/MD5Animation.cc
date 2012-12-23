#include "pch.h"
#include <iostream>
#include "common.h"
#include "Lexer.h"
#include "MD5Model.h"
#include "MD5Animation.h"

Logger& MD5Animation::logger(Logger::instance("md5mv.MD5Animation"));

MD5Animation::MD5Animation(const std::string& name)
    : Animation(name), _version(0), _account(0)
{
}

MD5Animation::~MD5Animation() throw()
{
}

void MD5Animation::build_skeletons()
{
    LOG_INFO("Building animation skeletons..." << std::endl);
    for(size_t i=0; i<frame_count(); ++i) {
        const MD5Frame& md5frame(dynamic_cast<const MD5Frame&>(frame(i)));

        boost::shared_ptr<Skeleton> skeleton(new Skeleton());
        for(size_t j=0; j<joint_count(); ++j) {
            const AnimationJoint& ajoint(_askeleton[j]);
            const Skeleton::Joint& bjoint(base_joint(j));

            // start with the base frame
            Position position(bjoint.position);
            Quaternion orientation(bjoint.orientation);

            // update the joint based on the frame data
            // TODO: find a better way to swizzle this
            int flag = ajoint.acflag, idx = ajoint.acstart;
            if(flag & 1) {
                position.x(md5frame.animated_components[idx]);
                idx++;
            }
            if(flag & 2) {
                position.z(-md5frame.animated_components[idx]);
                idx++;
            }
            if(flag & 4) {
                position.y(md5frame.animated_components[idx]);
                idx++;
            }
            if(flag & 8) {
                orientation.vector().x(md5frame.animated_components[idx]);
                idx++;
            }
            if(flag & 16) {
                orientation.vector().z(-md5frame.animated_components[idx]);
                idx++;
            }
            if(flag & 32) {
                orientation.vector().y(md5frame.animated_components[idx]);
            }

            orientation.compute_scalar();

            // joint depends on the parent unless it's a root
            Skeleton::Joint joint;
            if(ajoint.parent >= 0) {
                joint.parent = ajoint.parent;

                const Skeleton::Joint& parent(skeleton->joint(ajoint.parent));
                joint.position = parent.position + (parent.orientation * position);
                joint.orientation = parent.orientation * orientation;
                joint.orientation.normalize();
            } else {
                joint.parent = -1;
                joint.position = position;
                joint.orientation = orientation;
            }
            skeleton->add_joint(joint);
        }
        add_skeleton(skeleton);
    }
}

bool MD5Animation::on_load(const boost::filesystem::path& path)
{
    boost::filesystem::path filename(model_dir() / path / (name() + extension()));
    LOG_INFO("Loading animation from '" << filename << "'" << std::endl);

    Lexer lexer;
    if(!lexer.load(filename)) {
        return false;
    }

    if(!scan_version(lexer)) {
        return false;
    }

    if(!scan_commandline(lexer)) {
        return false;
    }

    int fcount = scan_num_frames(lexer);
    if(fcount < 0) return false;

    int jcount = scan_num_joints(lexer);
    if(jcount < 0) return false;
    _askeleton.reset(new AnimationJoint[jcount]);

    int frate = scan_frame_rate(lexer);
    if(frate <= 0) return false;
    frame_rate(frate);

    _account = scan_num_animated_components(lexer);
    if(_account < 0) return false;

    if(!scan_hierarchy(lexer, jcount)) {
        return false;
    }

    if(!scan_bounds(lexer, fcount)) {
        return false;
    }

    if(!scan_baseframe(lexer, jcount)) {
        return false;
    }

    if(!scan_frames(lexer, fcount)) {
        return false;
    }

    return true;
}

void MD5Animation::on_unload() throw()
{
    _version = 0;
    _commandline.erase();

    _askeleton.reset();
    _account = 0;
}

bool MD5Animation::scan_version(Lexer& lexer)
{
    if(!lexer.match(MD5VERSION)) {
        return false;
    }

    if(!lexer.int_literal(_version)) {
        return false;
    }

    // only version 10 is supported
    return _version == 10;
}

bool MD5Animation::scan_commandline(Lexer& lexer)
{
    if(!lexer.match(COMMANDLINE)) {
        return false;
    }
    return lexer.string_literal(_commandline);
}

int MD5Animation::scan_num_frames(Lexer& lexer)
{
    if(!lexer.match(NUM_FRAMES)) {
        return -1;
    }

    int value;
    if(!lexer.int_literal(value)) {
        return -1;
    }
    return value;
}

int MD5Animation::scan_num_joints(Lexer& lexer)
{
    if(!lexer.match(NUM_JOINTS)) {
        return -1;
    }

    int value;
    if(!lexer.int_literal(value)) {
        return -1;
    }
    return value;
}

int MD5Animation::scan_frame_rate(Lexer& lexer)
{
    if(!lexer.match(FRAME_RATE)) {
        return -1;
    }

    int value;
    if(!lexer.int_literal(value)) {
        return -1;
    }
    return value;
}

int MD5Animation::scan_num_animated_components(Lexer& lexer)
{
    if(!lexer.match(NUM_ANIMATED_COMPONENTS)) {
        return -1;
    }

    int value;
    if(!lexer.int_literal(value)) {
        return -1;
    }
    return value;
}

bool MD5Animation::scan_hierarchy(Lexer& lexer, int count)
{
    if(!lexer.match(HIERARCHY)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    for(int i=0; i<count; ++i) {
        // joint name
        std::string name;
        if(!lexer.string_literal(name)) {
            return false;
        }

        // joint parent
        int parent;
        if(!lexer.int_literal(parent)) {
            return false;
        }

        // animated component flag
        int flag;
        if(!lexer.int_literal(flag)) {
            return false;
        }

        // animated component start index
        int start;
        if(!lexer.int_literal(start)) {
            return false;
        }

        AnimationJoint& joint(_askeleton[i]);
        joint.name = name;
        joint.parent = parent;
        joint.acflag = flag;
        joint.acstart = start;
    }

    return lexer.match(CLOSE_BRACE);
}

bool MD5Animation::scan_bounds(Lexer& lexer, int count)
{
    if(!lexer.match(BOUNDS)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    for(int i=0; i<count; ++i) {
        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        // bounds min
        Point3 min;

        float value;
        if(!lexer.float_literal(value)) {
            return false;
        }
        min.x(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        min.y(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        min.z(value);

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }

        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        // bounds max
        Point3 max;

        if(!lexer.float_literal(value)) {
            return false;
        }
        max.x(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        max.y(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        max.z(value);

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }

        // TODO: can we add the frame in more reasonable place?
        boost::shared_ptr<Frame> frame(new MD5Frame());
        frame->bounds = AABB(swizzle(min), swizzle(max));
        add_frame(frame);
    }

    return lexer.match(CLOSE_BRACE);
}

bool MD5Animation::scan_baseframe(Lexer& lexer, int count)
{
    if(!lexer.match(BASE_FRAME)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    for(int i=0; i<count; ++i) {
        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        // position
        Position position;

        float value;
        if(!lexer.float_literal(value)) {
            return false;
        }
        position.x(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        position.y(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        position.z(value);

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }

        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        // orientation
        Vector3 orientation;

        if(!lexer.float_literal(value)) {
            return false;
        }
        orientation.x(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        orientation.y(value);

        if(!lexer.float_literal(value)) {
            return false;
        }
        orientation.z(value);

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }

        Skeleton::Joint joint;
        joint.position = swizzle(position);
        joint.orientation = Quaternion(swizzle(orientation));
        add_base_joint(joint);
    }

    return lexer.match(CLOSE_BRACE);
}

bool MD5Animation::scan_frames(Lexer& lexer, int count)
{
    for(int i=0; i<count; ++i) {
        if(!lexer.match(FRAME)) {
            return false;
        }

        // frame index
        int index;
        if(!lexer.int_literal(index)) {
            return false;
        }

        if(!lexer.match(OPEN_BRACE)) {
            return false;
        }

        // animated components
        boost::shared_array<float> animated_components(new float[_account]);
        for(int j=0; j<_account; ++j) {
            float value;
            if(!lexer.float_literal(value)) {
                return false;
            }
            animated_components[j] = value;
        }

        if(!lexer.match(CLOSE_BRACE)) {
            return false;
        }

        MD5Frame& md5frame(dynamic_cast<MD5Frame&>(frame(i)));
        md5frame.animated_components = animated_components;
    }

    return true;
}
