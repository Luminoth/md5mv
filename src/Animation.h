#if !defined __ANIMATION_H__
#define __ANIMATION_H__

#include "AABB.h"
#include "Model.h"

class Lexer;

class Animation
{
public:
    class Frame
    {
    public:
        Frame() {}
        virtual ~Frame() throw() {}

    public:
        AABB bounds;
    };

public:
    static std::string extension() { return ".mdlanim"; }

private:
    static Logger& logger;

public:
    explicit Animation(const std::string& name);
    virtual ~Animation() throw();

public:
    const std::string& name() const { return _name; }

    size_t frame_count() const { return _frames.size(); }
    Frame& frame(size_t idx) { return *(_frames[idx]); }
    const Frame& frame(size_t idx) const { return *(_frames[idx]); }

    size_t joint_count() const { return _skeleton.joint_count(); }
    const Skeleton& skeleton(size_t idx) const { return *(_skeletons[idx]); }

    const Skeleton::Joint& base_joint(size_t idx) const { return _skeleton.joint(idx); }

    double frame_rate() const { return _frate; }
    double frame_duration() const { return _fduration; }

public:
    bool load(const boost::filesystem::path& path);
    void unload() throw();

    virtual void build_skeletons() {}

    void interpolate_skeleton(size_t current_frame, size_t next_frame, Skeleton& skeleton, double frame_percent) const;

protected:
    void add_frame(boost::shared_ptr<Frame> frame) { _frames.push_back(frame); }
    void add_skeleton(boost::shared_ptr<Skeleton> skeleton) { _skeletons.push_back(skeleton); }
    void add_base_joint(const Skeleton::Joint& joint) { _skeleton.add_joint(joint); }

    void frame_rate(int rate);

    virtual bool on_load(const boost::filesystem::path& path);
    virtual void on_unload() throw() {}

private:
    bool scan_header(Lexer& lexer);

private:
    std::string _name;

    std::vector<boost::shared_ptr<Frame> > _frames;
    std::vector<boost::shared_ptr<Skeleton> > _skeletons;
    Skeleton _skeleton;

    int _frate;
    double _fduration;

private:
    Animation();
    DISALLOW_COPY_AND_ASSIGN(Animation);
};

#endif
