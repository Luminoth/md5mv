#include "pch.h"
#include <fstream>
#include <iostream>
#include "gl_defs.h"
#include "math_util.h"
#include "AABB.h"
#include "Actor.h"
#include "Camera.h"
#include "ClientConfiguration.h"
#include "Light.h"
#include "Map.h"
#include "Mesh.h"
#include "PNG.h"
#include "Renderable.h"
#include "Shader.h"
#include "Sphere.h"
#include "State.h"
#include "TextureManager.h"
#include "Renderer.h"

// TODO: just here for temp bsp rendering
/*#include "Q3BSP.h"
#include "Scene.h"*/

Logger& Renderer::logger(Logger::instance("md5mv.Renderer"));

Renderer& Renderer::instance()
{
    static boost::shared_ptr<Renderer> renderer;
    if(!renderer) {
        renderer.reset(new Renderer());
    }
    return *renderer;
}

Renderer::Renderer()
    : _window(NULL), _near_plane(0.0f), _far_plane(0.0f), _aspect_ratio(0.0f), _fov(0.0f)
{
    ZeroMemory(_fbo, BufferCount * sizeof(GLuint));
    ZeroMemory(_rbo, BufferCount * sizeof(GLuint));
    ZeroMemory(_tbo, BufferCount * sizeof(GLuint));
    ZeroMemory(_vbo, VBOCount * sizeof(GLuint));
}

Renderer::~Renderer() throw()
{
    glDeleteFramebuffers(BufferCount, _fbo);
    glDeleteRenderbuffers(BufferCount, _rbo);
    glDeleteTextures(BufferCount, _tbo);
    glDeleteBuffers(VBOCount, _vbo);
}

void Renderer::push_projection_matrix()
{
    _projection_stack.push(_projection);
    _fov_stack.push(_fov);
    _aspect_ratio_stack.push(_aspect_ratio);
    _near_plane_stack.push(_near_plane);
    _far_plane_stack.push(_far_plane);
}

void Renderer::pop_projection_matrix()
{
    _projection = _projection_stack.top();
    _projection_stack.pop();

    _fov = _fov_stack.top();
    _fov_stack.pop();

    _aspect_ratio = _aspect_ratio_stack.top();
    _aspect_ratio_stack.pop();

    _near_plane = _near_plane_stack.top();
    _near_plane_stack.pop();

    _far_plane  = _far_plane_stack.top();
    _far_plane_stack.pop();
}

void Renderer::projection_identity()
{
    _projection.identity();
    _fov = 0.0f;
    _aspect_ratio = 0.0f;
    _near_plane = 0.0f;
    _far_plane = 0.0f;
}

void Renderer::perspective(float fov, float aspect, float n, float f)
{
    _fov = DEG_RAD(fov);
    _aspect_ratio = aspect;
    _near_plane = n;
    _far_plane = f;

    _projection.identity();
    _projection *= Matrix4::perspective(fov, aspect, n, f);
}

void Renderer::orthographic(float left, float right, float bottom, float top, float n, float f)
{
    _fov = 0.0f;
    _aspect_ratio = std::abs((right - left) / (top - bottom));
    _near_plane = n;
    _far_plane = f;

    _projection.identity();
    _projection *= Matrix4::orthographic(left, right, bottom, top, n, f);
}

void Renderer::frustum(float left, float right, float bottom, float top, float n, float f)
{
    _fov = 0.0f;
    _aspect_ratio = std::abs((right - left) / (top - bottom));
    _near_plane = n;
    _far_plane = f;

    _projection.identity();
    _projection *= Matrix4::frustum(left, right, bottom, top, n, f);
}

void Renderer::infinite_frustum(float left, float right, float bottom, float top, float n)
{
    _fov = 0.0f;
    _aspect_ratio = std::abs((right - left) / (top - bottom));
    _near_plane = n;
    _far_plane = -1;

    _projection.identity();
    _projection *= Matrix4::infinite_frustum(left, right, bottom, top, n);
}

void Renderer::frustum(Point4& left, Point4& right, Point4& bottom, Point4& top, Point4& n, Point4& f) const
{
    const float e = 1.0f / std::tan(_fov / 2.0f);
    const float e2 = e * e;
    const float a = _aspect_ratio;
    const float a2 = a * a;
    const float np = _near_plane;
    const float fp = _far_plane;

    left = Point4(e * invsqrt(e2 + 1), 0.0f, -invsqrt(e2 + 1), 0.0f);
    right = Point4(-e * invsqrt(e2 + 1), 0.0f, -invsqrt(e2 + 1), 0.0f);
    bottom = Point4(0.0f, e * invsqrt(e2 + a2), -a * invsqrt(e2 + a2), 0.0f);
    top = Point4(0.0f, -e * invsqrt(e2 + a2), -a * invsqrt(e2 + a2), 0.0f);
    n = Point4(0.0f, 0.0f, -1.0f, -np);
    f = Point4(0.0f, 0.0f, 1.0f, fp);
}

void Renderer::lookat(const Position& eye, const Position& lookat, const Direction& up)
{
    const Vector3 f((lookat - eye).normalize());
    const Direction un(up.normalized());

    const Vector3 s(f ^ un);
    const Vector3 u(s ^ f);

    Matrix4 m;
    m[0] = s.x(); m[1] = s.y(); m[2] = s.z();
    m[4] = u.x(); m[5] = u.y(); m[6] = u.z();
    m[8] = -f.x(); m[9] = -f.y(); m[10] = -f.z();

    _view.identity();
    _view *= m;
    _view.translate(-eye);
}

void Renderer::register_renderable(const Camera& camera, boost::shared_ptr<Renderable> renderable)
{
    // TODO: add transparent renderables to a separate list
    if(renderable->is_transparent()) {
        LOG_ERROR("TODO: Transparent renderables not supported!" << std::endl);
        return;
    }

    if(camera.visible(renderable->absolute_bounds())) {
        _visible_renderables.push_back(renderable);
        if(renderable->is_pickable()) {
            _pickable_renderables.push_back(renderable);
        }
    }

    // TODO: this should cull objects based on whether it would cast a shadow, based on the light
    // TODO: we need to do this for every light that can affect the visible scene
    _light_renderables.push_back(renderable);
}

void Renderer::render(const Camera& camera)
{
    // render the ambient
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[AmbientBuffer]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render the detail
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[DetailBuffer]);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render picking
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[PickBuffer]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // combine it all together
    render_deferred();
}

void Renderer::render(const Camera& camera, Map& map)
{
    // sort the renderables for "efficient" rendering (lol)
    _visible_renderables.sort(CompareRenderablesOpaque(camera.position()));
    BOOST_FOREACH(boost::shared_ptr<Light> light, map.lights()) {
        if(typeid(*light) == typeid(DirectionalLight)) {
            boost::shared_ptr<DirectionalLight> directional(boost::dynamic_pointer_cast<DirectionalLight, Light>(light));
// TODO: sort them in the direction of the light
            //_light_renderables.sort(CompareRenderablesOpaque()));
        } else if(typeid(*light) == typeid(PositionalLight)) {
            boost::shared_ptr<PositionalLight> positional(boost::dynamic_pointer_cast<PositionalLight, Light>(light));
            _light_renderables.sort(CompareRenderablesOpaque(positional->position()));
        } else if(typeid(*light) == typeid(SpotLight)) {
            boost::shared_ptr<SpotLight> spot(boost::dynamic_pointer_cast<SpotLight, Light>(light));
            _light_renderables.sort(CompareRenderablesOpaque(spot->position()));
        }
    }

    // render the ambient (filling the depth buffer)
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[AmbientBuffer]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        render_ambient(camera, map);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render the detail
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[DetailBuffer]);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_STENCIL_TEST);

    const ClientConfiguration& config(ClientConfiguration::instance());
    BOOST_FOREACH(boost::shared_ptr<Light> light, map.lights()) {
        if(!light->enabled()) {
            continue;
        }

        glClear(GL_STENCIL_BUFFER_BIT);

        // fill the stencil buffer with shadows
        if(Light::lighting_enabled() && config.render_shadows()) {
            render_shadows(*light, camera);
        }

        // only render where the stencil is 0 and the depth is equal (only modify the color buffer)
        glDepthFunc(GL_EQUAL);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_EQUAL, 0, ~0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
            render_detail(camera, map, *light);
        glDisable(GL_BLEND);
        glStencilFunc(GL_ALWAYS, 0, ~0);
        glDepthFunc(GL_LEQUAL);
    }

    glDisable(GL_STENCIL_TEST);

    // render things that are not lit
    render_unlit(camera, map);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render picking
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[PickBuffer]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_picking();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // combine it all together
    render_deferred();

    // transparency last
    render_transparent();

    // cleanup
    _visible_renderables.clear();
    _light_renderables.clear();
}

void Renderer::render_triangle() const
{
    Vertex vertices[3];
    vertices[0].position = Position(-200.0f, -200.0f, 0.0f); vertices[0].texture_coords = Vector2(0.0f, 0.0f);
    vertices[1].position = Position( 200.0f, -200.0f, 0.0f); vertices[1].texture_coords = Vector2(1.0f, 0.0f);
    vertices[2].position = Position( 0.0f,    200.0f, 0.0f); vertices[2].texture_coords = Vector2(0.5f, 1.0f);

    Shader& shader(State::instance().simple_shader());
    shader.begin();
    init_shader_matrices(shader);

    // setup the detail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureManager::instance().default_detail_texture());
    shader.uniform1i("detail_texture", 0);

    // setup the normal map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, TextureManager::instance().default_normal_map());
    shader.uniform1i("normal_map", 1);

    RenderableBuffers buffers(vertices, 3);
    render_buffers(buffers, shader);

    shader.end();
}

void Renderer::render_cube(const AABB& cube) const
{
// TODO: this seems to be rendering in the wrong order?
    static const size_t vcount = 6 * 2 * 3;
    Vertex vertices[vcount];

    const float minx = cube.minimum().x(), miny = cube.minimum().y(), minz = cube.minimum().z();
    const float maxx = cube.maximum().x(), maxy = cube.maximum().y(), maxz = cube.maximum().z();

    // front
    vertices[0].position = Position(minx, miny, maxz); vertices[0].texture_coords = Vector2(0.0f, 0.0f);
    vertices[1].position = Position(maxx, miny, maxz); vertices[1].texture_coords = Vector2(1.0f, 0.0f);
    vertices[2].position = Position(minx, maxy, maxz); vertices[2].texture_coords = Vector2(0.0f, 1.0f);

    vertices[3].position = Position(maxx, miny, maxz); vertices[3].texture_coords = Vector2(1.0f, 0.0f);
    vertices[4].position = Position(maxx, maxy, maxz); vertices[4].texture_coords = Vector2(1.0f, 1.0f);
    vertices[5].position = Position(minx, maxy, maxz); vertices[5].texture_coords = Vector2(0.0f, 1.0f);

    // back
    vertices[6].position = Position(maxx, miny, minz); vertices[6].texture_coords = Vector2(1.0f, 0.0f);
    vertices[7].position = Position(minx, miny, minz); vertices[7].texture_coords = Vector2(0.0f, 0.0f);
    vertices[8].position = Position(maxx, maxy, minz); vertices[8].texture_coords = Vector2(1.0f, 1.0f);

    vertices[9].position = Position(minx, miny, minz); vertices[9].texture_coords = Vector2(0.0f, 0.0f);
    vertices[10].position = Position(minx, maxy, minz); vertices[10].texture_coords = Vector2(0.0f, 1.0f);
    vertices[11].position = Position(maxx, maxy, minz); vertices[11].texture_coords = Vector2(1.0f, 1.0f);

    // left
    vertices[12].position = Position(minx, miny, minz); vertices[12].texture_coords = Vector2(0.0f, 0.0f);
    vertices[13].position = Position(minx, miny, maxz); vertices[13].texture_coords = Vector2(0.0f, 1.0f);
    vertices[14].position = Position(minx, maxy, minz); vertices[14].texture_coords = Vector2(1.0f, 0.0f);

    vertices[15].position = Position(minx, miny, maxz); vertices[15].texture_coords = Vector2(0.0f, 1.0f);
    vertices[16].position = Position(minx, maxy, maxz); vertices[16].texture_coords = Vector2(1.0f, 1.0f);
    vertices[17].position = Position(minx, maxy, minz); vertices[17].texture_coords = Vector2(1.0f, 0.0f);

    // right
    vertices[18].position = Position(maxx, miny, maxz); vertices[18].texture_coords = Vector2(0.0f, 1.0f);
    vertices[19].position = Position(maxx, miny, minz); vertices[19].texture_coords = Vector2(0.0f, 0.0f);
    vertices[20].position = Position(maxx, maxy, maxz); vertices[20].texture_coords = Vector2(1.0f, 1.0f);

    vertices[21].position = Position(maxx, miny, minz); vertices[21].texture_coords = Vector2(0.0f, 0.0f);
    vertices[22].position = Position(maxx, maxy, minz); vertices[22].texture_coords = Vector2(1.0f, 0.0f);
    vertices[23].position = Position(maxx, maxy, maxz); vertices[23].texture_coords = Vector2(1.0f, 1.0f);

    // top
    vertices[24].position = Position(minx, maxy, maxz); vertices[24].texture_coords = Vector2(0.0f, 1.0f);
    vertices[25].position = Position(maxx, maxy, maxz); vertices[25].texture_coords = Vector2(1.0f, 1.0f);
    vertices[26].position = Position(minx, maxy, minz); vertices[26].texture_coords = Vector2(0.0f, 0.0f);

    vertices[27].position = Position(maxx, maxy, maxz); vertices[27].texture_coords = Vector2(1.0f, 1.0f);
    vertices[28].position = Position(maxx, maxy, minz); vertices[28].texture_coords = Vector2(1.0f, 0.0f);
    vertices[29].position = Position(minx, maxy, minz); vertices[29].texture_coords = Vector2(0.0f, 0.0f);

    // bottom
    vertices[30].position = Position(minx, miny, minz); vertices[30].texture_coords = Vector2(0.0f, 0.0f);
    vertices[31].position = Position(maxx, miny, minz); vertices[31].texture_coords = Vector2(1.0f, 0.0f);
    vertices[32].position = Position(minx, miny, maxz); vertices[32].texture_coords = Vector2(0.0f, 1.0f);

    vertices[33].position = Position(maxx, miny, minz); vertices[33].texture_coords = Vector2(1.0f, 0.0f);
    vertices[34].position = Position(maxx, miny, maxz); vertices[34].texture_coords = Vector2(1.0f, 1.0f);
    vertices[35].position = Position(minx, miny, maxz); vertices[35].texture_coords = Vector2(0.0f, 1.0f);

    Shader& shader(State::instance().simple_shader());
    shader.begin();
    init_shader_matrices(shader);

    // setup the detail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureManager::instance().default_detail_texture());
    shader.uniform1i("detail_texture", 0);

    // setup the normal map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, TextureManager::instance().default_normal_map());
    shader.uniform1i("normal_map", 1);

    RenderableBuffers buffers(vertices, vcount);
    render_buffers(buffers, shader);

    shader.end();
}

void Renderer::render_cube() const
{
    render_cube(AABB(Point3(-0.5f, -0.5f, -0.5f), Point3(0.5f, 0.5f, 0.5f)));
}

void Renderer::render_sphere(const Sphere& sphere) const
{
// TODO: this does not work
    static const size_t xskip = 45;
    static const size_t yskip = 10;

    static const size_t slices = 360 / xskip;
    static const size_t stacks = 360 / yskip;

    static const size_t vcount = stacks * slices * 3;
    Vertex vertices[vcount];

    for(size_t i=0; i<stacks; ++i) {
        for(size_t j=0; j<slices; ++j) {
            const size_t idx = (i * slices * 3) + (j * 3);

            float phi = DEG_RAD(i * xskip);
            float theta = DEG_RAD(j * yskip);
            float cstack = std::cos(phi);
            float sstack = std::sin(phi);
            float cslice = std::cos(theta);
            float sslice = std::sin(theta);

            vertices[idx + 0].position = sphere.center() + sphere.radius() * Position(cstack * cslice, cstack * sslice, sstack);

            phi = DEG_RAD((i+1) * xskip);
            theta = DEG_RAD((j+1) * yskip);
            cstack = std::cos(phi);
            sstack = std::sin(phi);
            cslice = std::cos(theta);
            sslice = std::sin(theta);

            vertices[idx + 1].position = sphere.center() + sphere.radius() * Position(cstack * cslice, cstack * sslice, sstack);

            phi = DEG_RAD((i+2) * xskip);
            theta = DEG_RAD((j+2) * yskip);
            cstack = std::cos(phi);
            sstack = std::sin(phi);
            cslice = std::cos(theta);
            sslice = std::sin(theta);

            vertices[idx + 2].position = sphere.center() + sphere.radius() * Position(cstack * cslice, cstack * sslice, sstack);
        }
    }

    Shader& shader(State::instance().simple_shader());
    shader.begin();
    init_shader_matrices(shader);

    // setup the detail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureManager::instance().default_detail_texture());
    shader.uniform1i("detail_texture", 0);

    // setup the normal map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, TextureManager::instance().default_normal_map());
    shader.uniform1i("normal_map", 1);

    RenderableBuffers buffers(vertices, vcount);
    render_buffers(buffers, shader);

    shader.end();
}

void Renderer::render_sphere() const
{
    render_sphere(Sphere());
}

void Renderer::render_buffers(const RenderableBuffers& buffers, Shader& shader) const
{
    GLuint vbo[4];
    glGenBuffers(4, vbo);

    // setup the vertex array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, buffers.vertex_buffer_size() * sizeof(float), buffers.vertex_buffer().get(), GL_STATIC_DRAW);

    // setup the normal array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, buffers.normal_buffer_size() * sizeof(float), buffers.normal_buffer().get(), GL_STATIC_DRAW);

    // setup the tangent array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, buffers.tangent_buffer_size() * sizeof(float), buffers.tangent_buffer().get(), GL_STATIC_DRAW);

    // setup the texture array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, buffers.texture_buffer_size() * sizeof(float), buffers.texture_buffer().get(), GL_STATIC_DRAW);

    // get the attribute locations
    GLuint vloc = shader.attrib_location("vertex");
    GLint nloc = shader.attrib_location("normal");
    GLint tnloc = shader.attrib_location("tangent");
    GLuint tloc = shader.attrib_location("texture_coord");

    glEnableVertexAttribArray(vloc);
    glEnableVertexAttribArray(nloc);
    glEnableVertexAttribArray(tnloc);
    glEnableVertexAttribArray(tloc);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
        glVertexAttribPointer(tloc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
        glVertexAttribPointer(nloc, 3, GL_FLOAT, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
        glVertexAttribPointer(tnloc, 4, GL_FLOAT, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, buffers.vertex_count());
    glDisableVertexAttribArray(tloc);
    glDisableVertexAttribArray(tnloc);
    glDisableVertexAttribArray(nloc);
    glDisableVertexAttribArray(vloc);

    glDeleteBuffers(4, vbo);
}

void Renderer::render_fullscreen_quad(Shader& shader)
{
    static const float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,

         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
    };

    // setup the vertex array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VertexArray]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // get the attribute locations
    GLuint vloc = shader.attrib_location("vertex");

    // render the quad
    glEnableVertexAttribArray(vloc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[VertexArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, 2 * 3);
    glDisableVertexAttribArray(vloc);
}

bool Renderer::init()
{
    GLenum err = glewInit();
    if(GLEW_OK != err) {
        LOG_CRITICAL(glewGetErrorString(err) << std::endl);
        return false;
    }

    print_info();

    if(!check_extensions()) {
        return false;
    }
    print_video_memory_details();

    projection_identity();
    view_identity();
    model_identity();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil(0);

    // enable face culling
    glEnable(GL_CULL_FACE);

    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // blending
    glDisable(GL_BLEND);

    // old anti-aliasing (don't use these anymore)
    /*glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);*/

    // FSAA
    glEnable(GL_MULTISAMPLE);

    // the stuffs commented out here are deprecated
    //glHint(GL_FOG_HINT, GL_NICEST);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

    return init_framebuffers();
}

bool Renderer::init_framebuffers()
{
    glGenFramebuffers(BufferCount, _fbo);
    glGenRenderbuffers(BufferCount, _rbo);
    glGenTextures(BufferCount, _tbo);
    glGenBuffers(VBOCount, _vbo);

    // setup the ambient buffers
    glBindTexture(GL_TEXTURE_2D, _tbo[AmbientBuffer]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width(), window_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // NOTE: ambient and detail FBOs share depth and stencil buffers (use DetailBuffer)

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[/*AmbientBuffer*/DetailBuffer]);
    glRenderbufferStorage(GL_RENDERBUFFER, /*GL_DEPTH_COMPONENT*/GL_DEPTH24_STENCIL8, window_width(), window_height());

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[AmbientBuffer]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tbo[AmbientBuffer], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, /*GL_DEPTH_ATTACHMENT*/GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo[/*AmbientBuffer*/DetailBuffer]);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(GL_FRAMEBUFFER_COMPLETE != status) {
        LOG_CRITICAL("Incomplete ambient buffer: " << status << std::endl);
        return false;
    }

    GLint temp;
    glGetIntegerv(GL_DEPTH_BITS, &temp);
    LOG_DEBUG("Ambient depth bits: " << temp << std::endl;);

    glGetIntegerv(GL_STENCIL_BITS, &temp);
    LOG_DEBUG("Ambient stencil bits: " << temp << std::endl);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup the detail buffers
    glBindTexture(GL_TEXTURE_2D, _tbo[DetailBuffer]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width(), window_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    /*glBindRenderbuffer(GL_RENDERBUFFER, _rbo[DetailBuffer]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width(), window_height());*/

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[DetailBuffer]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tbo[DetailBuffer], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo[DetailBuffer]);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(GL_FRAMEBUFFER_COMPLETE != status) {
        LOG_CRITICAL("Incomplete detail buffer: " << status << std::endl);
        return false;
    }

    glGetIntegerv(GL_DEPTH_BITS, &temp);
    LOG_DEBUG("Detail depth bits: " << temp << std::endl);

    glGetIntegerv(GL_STENCIL_BITS, &temp);
    LOG_DEBUG("Detail stencil bits: " << temp << std::endl);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup the picking buffers
    glBindTexture(GL_TEXTURE_2D, _tbo[PickBuffer]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width(), window_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[PickBuffer]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, window_width(), window_height());

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo[PickBuffer]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tbo[PickBuffer], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbo[PickBuffer]);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(GL_FRAMEBUFFER_COMPLETE != status) {
        LOG_CRITICAL("Incomplete picking buffer: " << status << std::endl);
        return false;
    }

    glGetIntegerv(GL_DEPTH_BITS, &temp);
    LOG_DEBUG("Picking depth bits: " << temp << std::endl);

    glGetIntegerv(GL_STENCIL_BITS, &temp);
    LOG_DEBUG("Picking stencil bits: " << temp << std::endl);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

bool Renderer::create_window(int width, int height, int bpp, bool fullscreen, const std::string& caption)
{
    //SDL_putenv("SDL_VIDEO_WINDOW_POS=1000,200");

    // setup all the lame video shit
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_CRITICAL("Could not initialize SDL: " << SDL_GetError() << std::endl);
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    /*SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 16);*/

    // 2xFSAA (requires GL_ARB_multisample)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    Uint32 flags = SDL_ASYNCBLIT | SDL_ANYFORMAT | SDL_HWPALETTE | SDL_OPENGL;
    if(fullscreen) flags |= SDL_FULLSCREEN;

    _window = SDL_SetVideoMode(width, height, bpp, flags);
    if(NULL == _window) {
        LOG_CRITICAL("Unable to set video mode!" << std::endl);
        return false;
    }

    SDL_WM_SetCaption(caption.c_str(), NULL);

    if(!init()) {
        SDL_Quit();
        _window = NULL;
        return false;
    }

    resize_viewport(width, height);

    return true;
}

void Renderer::resize_viewport(int width, int height)
{
    glViewport(0, 0, width, height);

    mvp_identity();

    // infinite perspective (for shadows)
    const ClientConfiguration& config(ClientConfiguration::instance());
    perspective(config.game_fov(), static_cast<float>(width) / static_cast<float>(height), 0.1f, -1.0f);
    //perspective(config.game_fov(), static_cast<float>(width) / static_cast<float>(height), 0.1f, 1000.0f);

    // resize the render buffers
    /*glBindRenderbuffer(GL_RENDERBUFFER, _rbo[AmbientBuffer]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);*/

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[DetailBuffer]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[PickBuffer]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // resize the texture buffers
    glBindTexture(GL_TEXTURE_2D, _tbo[AmbientBuffer]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, _tbo[DetailBuffer]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, _tbo[PickBuffer]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::print_video_memory_details()
{
    if(glewIsSupported("GL_NVX_gpu_memory_info")) {
        LOG_INFO("Found GL_NVX_gpu_memory_info" << std::endl);

        GLint temp;
        glGetIntegerv(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &temp);
        LOG_INFO("Dedicated video memory: " << (temp / 1024.0f) << "Mb" << std::endl);

        glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &temp);
        LOG_INFO("Available memory: " << (temp / 1024.0f) << "Mb" << std::endl);

        glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &temp);
        LOG_INFO("Available dedicated video memory: " << (temp / 1024.0f) << "Mb" << std::endl);

        glGetIntegerv(GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &temp);
        LOG_INFO("Eviction count: " << temp << std::endl);

        glGetIntegerv(GPU_MEMORY_INFO_EVICTED_MEMORY_NVX , &temp);
        LOG_INFO("Evicted memory size: " << (temp / 1024.0f) << "Mb" << std::endl);
    } else if(glewIsSupported("GL_ATI_meminfo")) {
        LOG_INFO("Found GL_ATI_meminfo" << std::endl);

        GLint temp[4];
        glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, temp);
        LOG_INFO("Available texture memory: " << (temp[0] / 1024.0f) << "Mb" << std::endl);

        glGetIntegerv(RENDERBUFFER_FREE_MEMORY_ATI, temp);
        LOG_INFO("Available render buffer memory: " << (temp[0] / 1024.0f) << "Mb" << std::endl);
    } else {
        LOG_WARNING("No supported video memory extension!" << std::endl);
    }
}

void Renderer::screenshot(const boost::filesystem::path& filename)
{
    LOG_INFO("Saving screenshot..." << std::endl);

    SDL_Surface* screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
        window_width(), window_height(), window_bpp(),
        0x0000ff, 0x00ff00, 0xff0000, 0x000000);

    SDL_LockSurface(screen);
    glReadPixels(0, 0, window_width(), window_height(), GL_RGBA, GL_UNSIGNED_BYTE, screen->pixels);

    SDL_Surface* temp = SDL_CreateRGBSurface(SDL_SWSURFACE,
        screen->w, screen->h, screen->format->BitsPerPixel,
        0x0000ff, 0x00ff00, 0xff0000, 0x000000);

    SDL_LockSurface(temp);

    // http://www.math.psu.edu/local_doc/SDL-1.1.2/opengl/examples.html
    for(int i=0; i<screen->h; ++i) {
        std::memcpy(static_cast<char*>(temp->pixels) + (temp->format->BytesPerPixel * temp->w * i),
            static_cast<char*>(screen->pixels) + (screen->format->BytesPerPixel * screen->w * (screen->h - i - 1)),
            screen->format->BytesPerPixel * screen->w);
    }
    std::memmove(screen->pixels, temp->pixels, screen->w * screen->h * screen->format->BytesPerPixel);

    SDL_UnlockSurface(temp);
    SDL_UnlockSurface(screen);

#if defined WIN32
    SDL_SaveBMP(screen, filename.string().c_str());
#else
    save_png(filename, screen);
#endif

    SDL_FreeSurface(temp);
    SDL_FreeSurface(screen);
}

void Renderer::save_depth_stencil(const boost::filesystem::path& filename)
{
// NOTE: assuming a 24-bit depth buffer and an 8-bit stencil buffer
    SDL_Surface* screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
        window_width(), window_height(), 32,
        0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    SDL_LockSurface(screen);
    glReadPixels(0, 0, window_width(), window_height(), GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, screen->pixels);

    SDL_Surface* temp = SDL_CreateRGBSurface(SDL_SWSURFACE,
        screen->w, screen->h, 32,
        0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    SDL_LockSurface(temp);

    // http://www.math.psu.edu/local_doc/SDL-1.1.2/opengl/examples.html
    for(int i=0; i<screen->h; ++i) {
        std::memcpy(static_cast<char*>(temp->pixels) + (temp->format->BytesPerPixel * temp->w * i),
            static_cast<char*>(screen->pixels) + (screen->format->BytesPerPixel * screen->w * (screen->h - i - 1)),
            screen->format->BytesPerPixel * screen->w);
    }
    std::memmove(screen->pixels, temp->pixels, screen->w * screen->h * screen->format->BytesPerPixel);

    SDL_UnlockSurface(temp);
    SDL_UnlockSurface(screen);

    save_png(filename, screen);

    SDL_FreeSurface(temp);
    SDL_FreeSurface(screen);
}

void Renderer::save_stencil(const boost::filesystem::path& filename)
{
// NOTE: assuming an 8-bit stencil buffer
    SDL_Surface* screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
        window_width(), window_height(), 8,
        0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    SDL_LockSurface(screen);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, window_width(), window_height(), GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, screen->pixels);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    SDL_Surface* temp = SDL_CreateRGBSurface(SDL_SWSURFACE,
        screen->w, screen->h, 8,
        0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    SDL_LockSurface(temp);

    // http://www.math.psu.edu/local_doc/SDL-1.1.2/opengl/examples.html
    for(int i=0; i<screen->h; ++i) {
        std::memcpy(static_cast<char*>(temp->pixels) + (temp->format->BytesPerPixel * temp->w * i),
            static_cast<char*>(screen->pixels) + (screen->format->BytesPerPixel * screen->w * (screen->h - i - 1)),
            screen->format->BytesPerPixel * screen->w);
    }
    std::memmove(screen->pixels, temp->pixels, screen->w * screen->h * screen->format->BytesPerPixel);

    SDL_UnlockSurface(temp);
    SDL_UnlockSurface(screen);

    SDL_SaveBMP(screen, filename.string().c_str());

    SDL_FreeSurface(temp);
    SDL_FreeSurface(screen);
}

void Renderer::init_shader_matrices(Shader& shader) const
{
    // pass in the matrices
    shader.uniform_matrix4fv("mvp", mvp_matrix().array());
    shader.uniform_matrix4fv("modelview", modelview_matrix().array());
}

void Renderer::init_shader_ambient(Shader& shader, const Material& material) const
{
    Color global_ambient_color(Light::lighting_enabled() ? Light::global_ambient_color() : Color(0.0f, 0.0f, 0.0f, 1.0f));
    shader.uniform4f("global_ambient_color", global_ambient_color);

    // pass in the material parameters
    shader.uniform4f("material_ambient", Light::lighting_enabled() ? material.ambient_color() : Color(1.0f, 1.0f, 1.0f, 1.0f));
    shader.uniform4f("material_emissive", Light::lighting_enabled() ? material.emissive_color() : Color(0.0f, 0.0f, 0.0f, 1.0f));
}

void Renderer::init_shader_light(Shader& shader, const Material& material, const Light& light, const Camera& camera) const
{
    if(!Light::lighting_enabled() || !light.enabled()) {
        return;
    }

    Color light_ambient = light.ambient_color();
    Color light_diffuse = light.diffuse_color();
    Color light_specular = light.specular_color();

    Color light_position, light_spotlight_direction;
    float light_constant_attenuation=1.0f, light_linear_attenuation=0.0f, light_quadratic_attenuation=0.0f, light_spotlight_cutoff=180.0f, light_spotlight_exponent=0.0f;
    if(typeid(light) == typeid(DirectionalLight)) {
        const DirectionalLight& directional(dynamic_cast<const DirectionalLight&>(light));
        light_position = directional.direction();
    } else if(typeid(light) == typeid(PositionalLight)) {
        const PositionalLight& positional(dynamic_cast<const PositionalLight&>(light));
        light_position = positional.position().homogeneous_position();

        light_constant_attenuation = positional.constant_attenuation();
        light_linear_attenuation = positional.linear_attenuation();
        light_quadratic_attenuation = positional.quadratic_attenuation();
    } else if(typeid(light) == typeid(SpotLight)) {
        const SpotLight& spot(dynamic_cast<const SpotLight&>(light));
        light_position = spot.position().homogeneous_position();

        light_spotlight_direction = Color(spot.direction(), 0.0f);
        light_spotlight_cutoff = spot.cutoff();
        light_spotlight_exponent = spot.exponent();
        light_constant_attenuation = spot.constant_attenuation();
        light_linear_attenuation = spot.linear_attenuation();
        light_quadratic_attenuation = spot.quadratic_attenuation();
    }

    // put the light position/directions into eye space
    light_position = _view * light_position;
    light_spotlight_direction = _view * light_spotlight_direction;

    // pass in the camera position (object-space)
    shader.uniform3f("camera", (-_model * camera.position().homogeneous_position()).xyz());

    // pass in the light parameters
    shader.uniform4f("light_ambient", light_ambient);
    shader.uniform4f("light_diffuse", light_diffuse);
    shader.uniform4f("light_specular", light_specular);
    shader.uniform4f("light_position", light_position);
    shader.uniform1f("light_constant_attenuation", light_constant_attenuation);
    shader.uniform1f("light_linear_attenuation", light_linear_attenuation);
    shader.uniform1f("light_quadratic_attenuation", light_quadratic_attenuation);
    shader.uniform4f("light_spotlight_direction", light_spotlight_direction);
    shader.uniform1f("light_spotlight_cutoff", light_spotlight_cutoff);
    shader.uniform1f("light_spotlight_exponent", light_spotlight_exponent);

    // pass in the material parameters
    shader.uniform4f("material_ambient", Light::lighting_enabled() ? material.ambient_color() : Color(1.0f, 1.0f, 1.0f, 1.0f));
    shader.uniform4f("material_diffuse", Light::lighting_enabled() ? material.diffuse_color() : Color(0.0f, 0.0f, 0.0f, 1.0f));
    shader.uniform4f("material_specular",Light::lighting_enabled() ? material.specular_color() : Color(0.0f, 0.0f, 0.0f, 1.0f));
    shader.uniform1f("material_shininess", Light::lighting_enabled() ? material.shininess() : 0.0f);
}

bool Renderer::save_png(const boost::filesystem::path& filename, size_t width, size_t height, size_t Bpp, size_t pitch, const void* const pixels) const
{
    FILE* fp = fopen(filename.string().c_str(), "wb");
    if(NULL == fp) {
        return false;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_user_error, png_user_warn);
    if(NULL == png_ptr) {
        fclose(fp);
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(NULL == info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        fclose(fp);
        return false;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, Bpp == 4 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    //png_set_invert_alpha(png_ptr);

    png_write_info(png_ptr, info_ptr);
    png_set_packing(png_ptr);

    boost::shared_array<png_bytep> row_pointers(new png_bytep[height]);
    for(size_t i=0; i<height; ++i) {
        row_pointers[i] = (png_bytep)(Uint8*)pixels + i * pitch;
    }
    png_write_image(png_ptr, row_pointers.get());
    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
    return true;
}

bool Renderer::save_png(const boost::filesystem::path& filename, const SDL_Surface* const surface) const
{
    return save_png(filename, surface->w, surface->h, surface->format->BytesPerPixel, surface->pitch, surface->pixels);
}

void Renderer::print_info()
{
    LOG_INFO("GLEW version: " << glewGetString(GLEW_VERSION) << std::endl);

    LOG_INFO("OpenGL version: " << glGetString(GL_VERSION) << std::endl);
    LOG_INFO("OpenGL vendor: " << glGetString(GL_VENDOR) << std::endl);
    LOG_INFO("OpenGL renderer: " << glGetString(GL_RENDERER) << std::endl);
    LOG_DEBUG("OpenGL extensions: " << glGetString(GL_EXTENSIONS) << std::endl);

    int temp1;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &temp1);
    LOG_INFO("Double buffering: " << (temp1 > 0 ? "true" : "false") << std::endl);

    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &temp1);
    LOG_INFO("Multisample buffers: " << temp1 << std::endl);

    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &temp1);
    LOG_INFO("Multisample samples: " << temp1 << std::endl);

    GLint temp;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &temp);
    LOG_INFO("Max texture size: " << temp << std::endl);

    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &temp);
    LOG_INFO("Max shader geometry vertices: " << temp << std::endl);

    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &temp);
    LOG_INFO("Max draw buffers: " << temp << std::endl);

    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &temp);
    LOG_INFO("Max renderbuffer size: " << temp << std::endl);

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &temp);
    LOG_INFO("Max texture image units: " << temp << std::endl);

    glGetIntegerv(GL_DEPTH_BITS, &temp);
    LOG_INFO("Depth bits: " << temp << std::endl);

    glGetIntegerv(GL_STENCIL_BITS, &temp);
    LOG_INFO("Stencil bits: " << temp << std::endl);
}

bool Renderer::check_extensions()
{
    LOG_INFO("Checking OpenGL extensions..." << std::endl);
    _extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

    if(GLEW_VERSION_3_3) {
        LOG_INFO("Using OpenGL 3.3" << std::endl);
    } else {
        LOG_CRITICAL("OpenGL 3.3 required!" << std::endl);
        return false;
    }
    LOG_INFO("GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl);

    // nvidia depth clamp
    glEnable(GL_DEPTH_CLAMP);

    return true;
}

void Renderer::render_ambient(const Camera& camera, Map& map) const
{
    Shader& shader(State::instance().ambient_shader());
    BOOST_FOREACH(boost::shared_ptr<Renderable> renderable, _visible_renderables) {
        shader.begin();
        init_shader_ambient(shader, renderable->material());
        renderable->render(shader);
        shader.end();
    }

    shader.begin();
    init_shader_ambient(shader, map.material());
    map.render(camera, shader);
    shader.end();

/*Shader& bspshader(State::instance().simple_shader());
bspshader.begin();
init_shader_ambient(bspshader, map.material());
State::instance().scene()->_bsp->render(camera, bspshader);
bspshader.end();*/
}

void Renderer::render_shadows(const Light& light, const Camera& camera)
{
    /*if(typeid(light) == typeid(DirectionalLight)) {
        push_projection_matrix();
        perspective(_fov, _aspect_ratio, 0.1f, -1.0f);
    }*/

    // disable color and depth writes
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    // setup the stencil and depth functions
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, ~0);
    glDepthFunc(GL_LESS);

    // offset the shadows
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1);

    BOOST_FOREACH(boost::shared_ptr<Renderable> renderable, _light_renderables) {
        if(renderable->has_shadow()) {
            size_t vcount = renderable->compute_silhouette(light);
            if(vcount > 0) {
                render_shadow(*renderable, light, camera, vcount);
            }
        }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    glDepthFunc(GL_LEQUAL);
    glStencilFunc(GL_ALWAYS, 0, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // re-enable color and depth writes
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    /*if(typeid(light) == typeid(DirectionalLight)) {
        pop_projection_matrix();
    }*/
}

void Renderer::render_shadow(const Renderable& renderable, const Light& light, const Camera& camera, size_t vcount) const
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3.6

    glDisable(GL_CULL_FACE);

    Shader& shader(typeid(light) == typeid(DirectionalLight)
        ? State::instance().shadow_infinite_shader()
        : State::instance().shadow_point_shader());

    /*if(require_shadow_volume_cap(renderable, light)) {
//std::cout << "cap" << std::endl;
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
        renderable.render_shadow(shader, light, vcount, true);
    } else {*/
//std::cout << "no cap " << std::endl;
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
        renderable.render_shadow(shader, light, camera, vcount, false);
    //}

    glEnable(GL_CULL_FACE);
}

bool Renderer::require_shadow_volume_cap(const Renderable& renderable, const Light& light) const
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3.5

    Vector L;
    bool point = false;
    if(typeid(light) == typeid(DirectionalLight)) {
        L = dynamic_cast<const DirectionalLight&>(light).direction();
    } else if(typeid(light) == typeid(PositionalLight) || typeid(light) == typeid(SpotLight)) {
        L = dynamic_cast<const PositionalLight&>(light).position().homogeneous_position();
        point = true;
    }

    // construct the near-clip volume
    const Matrix4 invview = -_view;
    const float n = _near_plane;
    const float a = _aspect_ratio;
    const float e = 1.0f / std::tan(_fov / 2.0f);

    // world-space bounds
    const AABB bounds(renderable.absolute_bounds());
    const Position C(bounds.center(), 1.0f);
    const float r = bounds.radius();

    // distance from the light to the near plane (eye-space)
    const float d = (_view * L) * Position(0, 0, -1.0f, -n);

    // see if the light is in front of, behind, or inside of the near plane
    // epsilon may be adjusted for more or less accuracy here
    bool cap = false;
    static const float epsilon = 0.001f;
    if(d > epsilon) {
        // world-space near-rectangle vertices
        const Position r0((invview * Position( n / e,  (a * n) / e, -n, 1.0f)).xyz()),
            r1((invview * Position(-n / e,  (a * n) / e, -n, 1.0f)).xyz()),
            r2((invview * Position(-n / e, -(a * n) / e, -n, 1.0f)).xyz()),
            r3((invview * Position( n / e, -(a * n) / e, -n, 1.0f)).xyz());

        // world-space near-rectangle normals
        const Vector3 n0((r0 - r3) ^ (L.xyz() - L.w() * r0)), n1((r1 - r0) ^ (L.xyz() - L.w() * r1)),
               n2((r2 - r1) ^ (L.xyz() - L.w() * r2)), n3((r3 - r2) ^ (L.xyz() - L.w() * r3));

        // world-space near-clip volume planes
        const Vector4 k0(Vector4(n0, -n0 * r0) / n0.length()),
            k1(Vector4(n1, -n1 * r1) / n1.length()),
            k2(Vector4(n2, -n2 * r2) / n2.length()),
            k3(Vector4(n3, -n3 * r3) / n3.length()),
            k4(~_view * Vector4(0.0f, 0.0f, -1.0f, -n));

        // special case for objects behind a point light source
        if(point) {
            const Vector3 n5((~_view * Vector4(0.0f, 0.0f, -n, 1.0f) - L).xyz());
            const Vector4 k5(Vector4(n5, -n5 * L.xyz()) / n5.length());
            const float d5 = k5 * C;
            cap = !(d5 < -r);
        }

        // check to see if the bounds lie completely outside any plane
        const float d0 = k0 * C, d1 = k1 * C, d2 = k2 * C, d3 = k3 * C, d4 = k4 * C;
        cap = cap || !(d0 < -r || d1 < -r || d2 < -r || d3 < -r || d4 < -r);
    } else if(d < -epsilon) {
        // world-space near-rectangle vertices
        const Position r0((invview * Position( n / e,  (a * n) / e, -n, 1.0f)).xyz()),
            r1((invview * Position(-n / e,  (a * n) / e, -n, 1.0f)).xyz()),
            r2((invview * Position(-n / e, -(a * n) / e, -n, 1.0f)).xyz()),
            r3((invview * Position( n / e, -(a * n) / e, -n, 1.0f)).xyz());

        // world-space near-rectangle normals
        const Vector3 n0(-((r0 - r3) ^ (L.xyz() - L.w() * r0))), n1(-((r1 - r0) ^ (L.xyz() - L.w() * r1))),
               n2(-((r2 - r1) ^ (L.xyz() - L.w() * r2))), n3(-((r3 - r2) ^ (L.xyz() - L.w() * r3)));

        // world-space near-clip volume planes
        const Vector4 k0(Vector4(n0, -n0 * r0) / n0.length()),
            k1(Vector4(n1, -n1  * r1) / n1.length()),
            k2(Vector4(n2, -n2 * r2) / n2.length()),
            k3(Vector4(n3, -n3 * r3) / n3.length()),
            k4(-(~_view * Vector4(0.0f, 0.0f, -1.0f, -n)));

        // special case for objects behind a point light source
        if(point) {
            const Vector3 n5((~_view * Vector4(0.0f, 0.0f, -n, 1.0f) - L).xyz());
            const Vector4 k5(Vector4(n5, -n5 * L.xyz()) / n5.length());
            const float d5 = k5 * C;
            cap = !(d5 < -r);
        }

        // check to see if the bounds lie completely outside any plane
        const float d0 = k0 * C, d1 = k1 * C, d2 = k2 * C, d3 = k3 * C, d4 = k4 * C;
        cap = cap || !(d0 < -r || d1 < -r || d2 < -r || d3 < -r || d4 < -r);
    } else {
        // near-clip planes
        const Vector4 k0(0.0f, 0.0f, -1.0f, -n), k1(0.0f, 0.0f, 1.0f, n);

        // check to see if the bounds lie completely outside any plane
        const float d0 = k0 * C, d1 = k1 * C;
        cap = !(d0 < -r || d1 < -r);
    }

    return cap;
}

void Renderer::render_detail(const Camera& camera, Map& map, const Light& light) const
{
    const ClientConfiguration& config(ClientConfiguration::instance());
    Shader& shader(config.render_mode_vertex() ? State::instance().vertex_shader() : State::instance().bump_shader());
    BOOST_FOREACH(boost::shared_ptr<Renderable> renderable, _visible_renderables) {
        shader.begin();
        renderable->render(shader, light, camera);
        shader.end();
    }

    shader.begin();
    map.render(camera, shader, light);
    shader.end();
}

void Renderer::render_unlit(const Camera& camera, const Map& map) const
{
    BOOST_FOREACH(boost::shared_ptr<Renderable> renderable, _visible_renderables) {
        renderable->render_unlit(camera);
    }

    // TODO: move this into a render_unlit() Map method
    if(State::instance().render_normals()) {
        map.render_normals(camera);
    }
}

void Renderer::render_picking() const
{
    Shader& shader(State::instance().pick_shader());
    BOOST_FOREACH(boost::shared_ptr<Renderable> renderable, _pickable_renderables) {
        shader.begin();
        shader.uniform4f("color", renderable->pick_color());
        renderable->render(shader);
        shader.end();
    }
}

void Renderer::render_deferred()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    Shader& shader(State::instance().deferred_shader());
    shader.begin();

    // setup the ambient texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _tbo[AmbientBuffer]);
    shader.uniform1i("ambient_texture", 0);

    // setup the detail texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _tbo[DetailBuffer]);
    shader.uniform1i("detail_texture", 1);

    render_fullscreen_quad(shader);

    shader.end();
}

void Renderer::render_transparent() const
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // TODO: render transparent renderables here

    glDisable(GL_BLEND);
}
