#include "pch.h"
#include "Animation.h"
#include "Character.h"
#include "Monster.h"
#include "Renderer.h"
#include "State.h"
#include "Actor.h"

Logger& Actor::logger(Logger::instance("md5mv.Actor"));

boost::shared_ptr<Actor> Actor::new_actor(const std::string& type, const std::string& name)
{
    std::string scratch(boost::algorithm::to_lower_copy(type));
    if("character" == type) {
        return boost::shared_ptr<Actor>(new Character(name));
    } else if("monster" == type) {
        return boost::shared_ptr<Actor>(new Monster(name));
    }
    return boost::shared_ptr<Actor>();
}

Actor::Actor(const std::string& name)
    : Renderable(name), _cframe(0), _ftime(0.0)
{
    ZeroMemory(_skeleton_vbo, sizeof(GLuint) * SkeletonVBOCount);
    glGenBuffers(SkeletonVBOCount, _skeleton_vbo);
}

Actor::~Actor() throw()
{
    glDeleteBuffers(SkeletonVBOCount, _skeleton_vbo);
}

void Actor::animation(boost::shared_ptr<Animation> animation)
{
    // TODO: this needs to return a bool or something

    if(!has_model()) {
        return;
    }

    if(animation->joint_count() != model().joint_count()) {
        LOG_ERROR("Animation joint_count=" << animation->joint_count()
            << ", model joint_count=" << model().joint_count() << std::endl);
        return;
    }

    _animation = animation;
}

void Actor::current_frame(size_t frame)
{
    _cframe = frame % _animation->frame_count();
}

double Actor::frame_percent() const
{
    return _ftime / _animation->frame_duration();
}

void Actor::animate()
{
    _skeleton.reset();
    _animation->interpolate_skeleton(current_frame(), next_frame(), _skeleton, frame_percent());
    calculate_vertices(_skeleton);
}

size_t Actor::next_frame() const
{
    if(_cframe == _animation->frame_count() - 1) {
        return 0;
    }
    return _cframe + 1;
}

void Actor::advance_frame()
{
    _cframe++;
    if(_cframe >= _animation->frame_count()) {
        _cframe = 0;
    }
}

void Actor::render_skeleton() const
{
    Matrix4 matrix;
    transform(matrix);

    Renderer::instance().push_model_matrix();
    Renderer::instance().multiply_model_matrix(matrix);

    // build the vertex buffer
    const size_t vcount = _skeleton.nonroot_joint_count() * 3 * 2;
    boost::shared_array<float> v(new float[vcount]);
    for(size_t i=0, j=0; i<model().joint_count(); ++i) {
        const Skeleton::Joint& joint(_skeleton.joint(i));
        if(joint.parent < 0) {
            continue;
        }

        const Position& pp(_skeleton.joint(joint.parent).position);
        const Position& p(joint.position);

        const size_t idx = j * 3 * 2;
        v[idx + 0] = pp.x();
        v[idx + 1] = pp.y();
        v[idx + 2] = pp.z();
        v[idx + 3] = p.x();
        v[idx + 4] = p.y();
        v[idx + 5] = p.z();

        j++;
    }

    // setup the vertex array
    glBindBuffer(GL_ARRAY_BUFFER, _skeleton_vbo[SkeletonVertexArray]);
    glBufferData(GL_ARRAY_BUFFER, vcount * sizeof(float), v.get(), GL_DYNAMIC_DRAW);

    Shader& shader(State::instance().gray_shader());
    shader.begin();
    Renderer::instance().init_shader_matrices(shader);

    // get the attribute locations
    GLint vloc = shader.attrib_location("vertex");

    // render the skeleton
    glEnableVertexAttribArray(vloc);
        glBindBuffer(GL_ARRAY_BUFFER, _skeleton_vbo[SkeletonVertexArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_LINES, 0, _skeleton.nonroot_joint_count() * 2);
    glDisableVertexAttribArray(vloc);

    shader.end();

    Renderer::instance().pop_model_matrix();
}

bool Actor::on_think(double dt)
{
// TODO: this should run the current animation until it's done
// then update the position
// and switch to the idle animation

    if(State::instance().rotate_actors()) {
        rotate(dt, Vector3(0.0f, 1.0f, 0.0f));
    }

    if(_animation) {
        _ftime += dt;
        while(_ftime > _animation->frame_duration()) {
            _ftime -= _animation->frame_duration();
            advance_frame();
        }
        bounds(_animation->frame(_cframe).bounds);
    } else {
        bounds(model().bounds());
    }

    return true;
}

void Actor::on_render_unlit(const Camera& camera) const
{
    Matrix4 matrix;
    transform(matrix);

    Renderer::instance().push_modelview_matrix();
    Renderer::instance().multiply_model_matrix(matrix);
    Renderer::instance().modelview_rotation_identity();

    const AABB& bounds(relative_bounds());
    const Position& center(bounds.center());
    _nameplate.render(name(), Vector3(center.x(), bounds.maximum().y(), center.z()), camera);

    Renderer::instance().pop_modelview_matrix();

    Renderer::instance().push_model_matrix();
    Renderer::instance().multiply_model_matrix(matrix);

    if(State::instance().render_skeleton()) {
        render_skeleton();
    }

    Renderer::instance().pop_model_matrix();
}
