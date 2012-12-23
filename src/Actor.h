#if !defined __ACTOR_H__
#define __ACTOR_H__

#include "Model.h"
#include "Nameplate.h"
#include "Renderable.h"

// TODO: rather than having the Actor subclassed
// make an AIState or something object that the Actor
// can hold and subclass that instead

class Animation;

class Actor : public Renderable
{
private:
    enum
    {
        SkeletonVertexArray,
        SkeletonVBOCount
    };

public:
    static boost::shared_ptr<Actor> new_actor(const std::string& type, const std::string& name);

private:
    static Logger& logger;

public:
    virtual ~Actor() throw();

public:
    // NOTE: model must be set first
    void animation(boost::shared_ptr<Animation> animation);
    const boost::shared_ptr<const Animation> animation() const { return _animation; }

    void current_frame(size_t frame);
    void reset_animation() { _cframe = 0; _ftime = 0.0; }
    double frame_time() const { return _ftime; }
    double frame_percent() const;

public:
    virtual bool is_pickable() const { return true; }
    virtual bool is_transparent() const { return false; }
    virtual bool is_static() const { return false; }
    virtual bool has_shadow() const { return true; }

    virtual void animate();

    void render_skeleton() const;

private:
    size_t current_frame() const { return _cframe; }
    size_t next_frame() const;
    void advance_frame();

    virtual bool on_think(double dt);
    virtual void on_render_unlit(const Camera& camera) const;

private:
    boost::shared_ptr<Animation> _animation;

    size_t _cframe;
    double _ftime;

    Skeleton _skeleton;
    GLuint _skeleton_vbo[SkeletonVBOCount];

    Nameplate _nameplate;

protected:
    explicit Actor(const std::string& name);

private:
    Actor();
    DISALLOW_COPY_AND_ASSIGN(Actor);
};

#endif
