#if !defined __CAMERA_H__
#define __CAMERA_H__

#include "Physical.h"

class AABB;
class Actor;

class Camera : public Physical
{
private:
    static int check_clipping(const Vector4& clipped);

public:
    Camera();
    virtual ~Camera() throw();

public:
    const Direction& up() const { return _up; }
    const Position& lookat() const { return _lookat; }

    // attaches the camera to an actor
    // so that the camera will follow the actor around
    bool attached() const { return NULL != _attached; }
    void attach(boost::shared_ptr<Actor> actor) { _attached = actor; }
    void detach() { _attached.reset(); }

    // call every frame to adjust the view
    // matrix based on the camera values
    // also adjusts the camera position and orientation to
    // match what it's attached to (if it's attached),
    // so this needs to be called before referencing the camera position, etc
    void look();

    // returns true if some portion of the world-space
    // bounding box is within the viewing frustum
    bool visible(const AABB& bounds) const;

private:
    Direction _up;
    Position _lookat;
    boost::shared_ptr<Actor> _attached;

private:
    DISALLOW_COPY_AND_ASSIGN(Camera);
};

#endif
