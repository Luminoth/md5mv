#include "pch.h"
#include "common.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Model.h"
#include "Plane.h"
#include "Renderer.h"
#include "Shader.h"
#include "State.h"
#include "TextureManager.h"
#include "Renderable.h"

RenderableBuffers::RenderableBuffers()
    : _vcount(), _vbsize(0), _nbsize(0), _tnbsize(0), _tbsize(0)
{
}

RenderableBuffers::RenderableBuffers(size_t vertex_count)
    : _vcount(0), _vbsize(0), _nbsize(0), _tnbsize(0), _tbsize(0)
{
    allocate_buffers(vertex_count);
}

RenderableBuffers::RenderableBuffers(const Vertex* const vertices, size_t vertex_count)
    : _vcount(0), _vbsize(0), _nbsize(0), _tnbsize(0), _tbsize(0)
{
    copy_vertices(vertices, vertex_count, 0, true);
}

RenderableBuffers::RenderableBuffers(size_t triangle_count, size_t vertex_count)
    : _vcount(0), _vbsize(0), _nbsize(0), _tnbsize(0), _tbsize(0)
{
    allocate_buffers(triangle_count, vertex_count);
}

RenderableBuffers::RenderableBuffers(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count)
    : _vcount(0), _vbsize(0), _nbsize(0), _tnbsize(0), _tbsize(0)
{
    copy_triangles(triangles, triangle_count, vertices, vertex_count, 0, true);
}

RenderableBuffers::~RenderableBuffers() throw()
{
}

void RenderableBuffers::allocate_buffers(size_t vertex_count)
{
    _vcount = vertex_count;

    _vbsize = _vcount * 3;
    _vertex_buffer.reset(new float[_vbsize]);

    _nbsize = _vcount * 3;
    _normal_buffer.reset(new float[_nbsize]);
    _normal_line_buffer.reset(new float[_vcount * 2 * 3]);

    _tnbsize = _vcount * 4;
    _tangent_buffer.reset(new float[_tnbsize]);
    _tangent_line_buffer.reset(new float[_vcount * 2 * 3]);

    _tbsize = _vcount * 2;
    _texture_buffer.reset(new float[_tbsize]);
}

void RenderableBuffers::allocate_buffers(size_t triangle_count, size_t vertex_count)
{
    _vcount = triangle_count * 3;

    _vbsize = _vcount * 3;
    _vertex_buffer.reset(new float[_vbsize]);

    _nbsize = _vcount * 3;
    _normal_buffer.reset(new float[_nbsize]);
    _normal_line_buffer.reset(new float[vertex_count * 2 * 3]);

    _tnbsize = _vcount * 4;
    _tangent_buffer.reset(new float[_tnbsize]);
    _tangent_line_buffer.reset(new float[vertex_count * 2 * 3]);

    _tbsize = _vcount * 2;
    _texture_buffer.reset(new float[_tbsize]);
}

void RenderableBuffers::copy_vertices(const Vertex* const vertices, size_t vertex_count, size_t start, bool allocate)
{
    if(allocate) {
        allocate_buffers(vertex_count);
    }

    // fill the normal/tangent line buffers (for debugging)
    float *nlb = _normal_line_buffer.get(), *tnlb = _tangent_line_buffer.get();
    for(size_t i=0; i<_vcount; ++i) {
        const Vertex& vertex(vertices[i]);

        const size_t idx = (start * 2 * 3) + (i * 2 * 3);
        const Position& p(vertex.position);
        const Vector3 &n(vertex.normal), &t(vertex.tangent);

        *(nlb + idx + 0) = p.x(); *(nlb + idx + 3) = p.x() + n.x();
        *(nlb + idx + 1) = p.y(); *(nlb + idx + 4) = p.y() + n.y();
        *(nlb + idx + 2) = p.z(); *(nlb + idx + 5) = p.z() + n.z();

        *(tnlb + idx + 0) = p.x(); *(tnlb + idx + 3) = p.x() + t.x();
        *(tnlb + idx + 1) = p.y(); *(tnlb + idx + 4) = p.y() + t.y();
        *(tnlb + idx + 2) = p.z(); *(tnlb + idx + 5) = p.z() + t.z();
    }

    // fill the vertex buffers
    float *va = _vertex_buffer.get(), *na = _normal_buffer.get(), *tna = _tangent_buffer.get(), *ta = _texture_buffer.get();
    for(size_t i=0; i<_vcount; ++i) {
        const Vertex& vertex(vertices[i]);

        const size_t vidx = (start * 3) + (i * 3);
        *(va + vidx + 0) = vertex.position.x();
        *(va + vidx + 1) = vertex.position.y();
        *(va + vidx + 2) = vertex.position.z();

        const Vector3& normal(vertex.normal);
        *(na + vidx + 0) = normal.x();
        *(na + vidx + 1) = normal.y();
        *(na + vidx + 2) = normal.z();

        const size_t tnidx = (start * 4) + (i * 4);

        // Mathematics for 3D Game Programming and Computer Graphics, section 7.8.3
        const Vector3& tangent(vertex.tangent);
        const Vector3& bitangent(normal ^ tangent);
        *(tna + tnidx + 0) = tangent.x();
        *(tna + tnidx + 1) = tangent.y();
        *(tna + tnidx + 2) = tangent.z();
        *(tna + tnidx + 3) = bitangent.opposite_direction(vertex.bitangent) ? -1.0f : 1.0f;

        const size_t tidx = (start * 2) + (i * 2);
        *(ta + tidx + 0) = vertex.texture_coords.x();
        *(ta + tidx + 1) = vertex.texture_coords.y();
    }
}

void RenderableBuffers::copy_triangles(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count, size_t start, bool allocate)
{
    if(allocate) {
        allocate_buffers(triangle_count, vertex_count);
    }

    // fill the normal/tangent line buffers (for debugging)
    float *nlb = _normal_line_buffer.get(), *tnlb = _tangent_line_buffer.get();
    for(size_t i=0; i<vertex_count; ++i) {
        const Vertex& vertex(vertices[i]);

        const size_t idx = (start * 2 * 3) + i * 2 * 3;
        const Position& p(vertex.position);
        const Vector3 &n(vertex.normal), &t(vertex.tangent);

        *(nlb + idx + 0) = p.x(); *(nlb + idx + 3) = p.x() + n.x();
        *(nlb + idx + 1) = p.y(); *(nlb + idx + 4) = p.y() + n.y();
        *(nlb + idx + 2) = p.z(); *(nlb + idx + 5) = p.z() + n.z();

        *(tnlb + idx + 0) = p.x(); *(tnlb + idx + 3) = p.x() + t.x();
        *(tnlb + idx + 1) = p.y(); *(tnlb + idx + 4) = p.y() + t.y();
        *(tnlb + idx + 2) = p.z(); *(tnlb + idx + 5) = p.z() + t.z();
    }

    // fill the vertex buffers
    float *vb = _vertex_buffer.get(), *nb = _normal_buffer.get(), *tnb = _tangent_buffer.get(), *tb = _texture_buffer.get();
    for(size_t i=0; i<triangle_count; ++i) {
        const Triangle& triangle(triangles[i]);
        const Vertex &v1(vertices[triangle.v1]), &v2(vertices[triangle.v2]), &v3(vertices[triangle.v3]);

        const size_t idx = (start * 3) + (i * 3 * 3);

        *(vb + idx + 0) = v1.position.x();
        *(vb + idx + 1) = v1.position.y();
        *(vb + idx + 2) = v1.position.z();
        *(vb + idx + 3) = v2.position.x();
        *(vb + idx + 4) = v2.position.y();
        *(vb + idx + 5) = v2.position.z();
        *(vb + idx + 6) = v3.position.x();
        *(vb + idx + 7) = v3.position.y();
        *(vb + idx + 8) = v3.position.z();

        const Vector3 &n1(v1.normal), &n2(v2.normal), &n3(v3.normal);
        *(nb + idx + 0) = n1.x();
        *(nb + idx + 1) = n1.y();
        *(nb + idx + 2) = n1.z();
        *(nb + idx + 3) = n2.x();
        *(nb + idx + 4) = n2.y();
        *(nb + idx + 5) = n2.z();
        *(nb + idx + 6) = n3.x();
        *(nb + idx + 7) = n3.y();
        *(nb + idx + 8) = n3.z();

        const size_t tnidx = (start * 4) + (i * 3 * 4);

        // Mathematics for 3D Game Programming and Computer Graphics, section 7.8.3
        const Vector3 &t1(v1.tangent), &t2(v2.tangent), &t3(v3.tangent);
        const Vector3 b1(n1 ^ t1), b2(n2 ^ t2), b3(n3 ^ t3);
        *(tnb + tnidx + 0)  = t1.x();
        *(tnb + tnidx + 1)  = t1.y();
        *(tnb + tnidx + 2)  = t1.z();
        *(tnb + tnidx + 3)  = b1.opposite_direction(v1.bitangent) ? -1.0f : 1.0f;
        *(tnb + tnidx + 4)  = t2.x();
        *(tnb + tnidx + 5)  = t2.y();
        *(tnb + tnidx + 6)  = t2.z();
        *(tnb + tnidx + 7)  = b2.opposite_direction(v2.bitangent) ? -1.0f : 1.0f;
        *(tnb + tnidx + 8)  = t3.x();
        *(tnb + tnidx + 9)  = t3.y();
        *(tnb + tnidx + 10) = t3.z();
        *(tnb + tnidx + 11) = b3.opposite_direction(v3.bitangent) ? -1.0f : 1.0f;

        const size_t tidx = (start * 2) + (i * 3 * 2);

        *(tb + tidx + 0) = v1.texture_coords.x();
        *(tb + tidx + 1) = v1.texture_coords.y();
        *(tb + tidx + 2) = v2.texture_coords.x();
        *(tb + tidx + 3) = v2.texture_coords.y();
        *(tb + tidx + 4) = v3.texture_coords.x();
        *(tb + tidx + 5) = v3.texture_coords.y();
    }
}

uint32_t Renderable::pick_ids = 0;

uint32_t Renderable::next_pick_id()
{
    return ++pick_ids;
}

Renderable::Renderable(const std::string& name)
    : Physical(), _name(name), _pick_id(0)
{
    ZeroMemory(_vbo, sizeof(GLuint) * VBOCount);
    glGenBuffers(VBOCount, _vbo);

    ZeroMemory(_shadow_vbo, sizeof(GLuint) * ShadowVBOCount);
    glGenBuffers(ShadowVBOCount, _shadow_vbo);
}

Renderable::~Renderable() throw()
{
    glDeleteBuffers(ShadowVBOCount, _shadow_vbo);
    glDeleteBuffers(VBOCount, _vbo);
}

void Renderable::init()
{
    // picking
    if(is_pickable()) {
        _pick_id = next_pick_id();
        _pick_color = Color(
            _pick_id & 0x000000ff,
            (_pick_id >> 8) & 0x000000ff,
            (_pick_id >> 16) & 0x000000ff,
            (_pick_id >> 24) & 0x000000ff
        );
    }
}

void Renderable::model(boost::shared_ptr<Model> model)
{
    _model = model;
    _buffers.allocate_buffers(_model->triangle_count() * 3);

    _vertices.reset(new Vertex[model->vertex_count()]);
    calculate_vertices(_model->skeleton());
}

bool Renderable::load_material()
{
    if(!_material.load(material_dir(), "actor")) {
        return false;
    }
    return true;
}

size_t Renderable::compute_silhouette(const Light& light)
{
    Matrix4 matrix;
    transform(matrix);

    // allocate enough space for every edge
    const size_t vsize = model().edge_count() * 4 * 4;
    boost::shared_array<float> varray(new float[vsize]);

    size_t vcount = 0;
    if(typeid(light) == typeid(DirectionalLight)) {
        const DirectionalLight& directional(dynamic_cast<const DirectionalLight&>(light));
        vcount = compute_silhouette_directional(-matrix * directional.direction(), varray);
    } else if(typeid(light) == typeid(PositionalLight) || typeid(light) == typeid(SpotLight)) {
        const PositionalLight& positional(dynamic_cast<const PositionalLight&>(light));
        vcount = compute_silhouette_positional(-matrix * positional.position().homogeneous_position(), varray);
    }

    if(0 == vcount) {
        return 0;
    }

    // setup the vertex array
    glBindBuffer(GL_ARRAY_BUFFER, _shadow_vbo[ShadowVertexArray]);
    glBufferData(GL_ARRAY_BUFFER, vcount * 4 * sizeof(float), varray.get(), GL_DYNAMIC_DRAW);

    return vcount;
}

bool Renderable::is_silhouette_edge(const Mesh& mesh, const Edge& edge, const Vector4& light_position, size_t vstart, bool& faces_light1) const
{
    Plane p1;
    if(edge.t1 >= 0) {
        const Triangle& t(mesh.triangle(edge.t1));
        p1 = Plane(vertex(vstart + t.v1).position, vertex(vstart + t.v2).position, vertex(vstart + t.v3).position);
    }

    Plane p2;
    if(edge.t2 >= 0) {
        const Triangle& t(mesh.triangle(edge.t2));
        p2 = Plane(vertex(vstart + t.v1).position, vertex(vstart + t.v2).position, vertex(vstart + t.v3).position);
    }

    // sort out which of the edges, if any, are facing the light
    faces_light1 = p1 * light_position > 0.0f;
    const bool faces_light2 = p2 * light_position > 0.0f;

    // two-winged edges are silhouette edges if one triangle faces towards the light and the other way
    return ((edge.t1 >= 0 && edge.t2 >= 0 && ((faces_light1 && !faces_light2) || (!faces_light1 && faces_light2)))
        // one-winged edges are only a silhouette edge if the *first* triangle faces the light
        || ((edge.t1 < 0 || edge.t2 < 0) && faces_light1));
}

size_t Renderable::compute_silhouette_directional(const Direction& light_direction, boost::shared_array<float> varray)
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3
    size_t vstart = 0, ecount = 0;
    float *v = varray.get();
    for(size_t i=0; i<model().mesh_count(); ++i) {
        const Mesh& mesh(model().mesh(i));

        for(size_t j=0; j<mesh.edge_count(); ++j) {
            const Edge& edge(mesh.edge(j));

            bool faces_light1;
            if(is_silhouette_edge(mesh, edge, light_direction, vstart, faces_light1)) {
                const Vertex& v1(vertex(vstart + (faces_light1 ? edge.v2 : edge.v1)));
                const Vertex& v2(vertex(vstart + (faces_light1 ? edge.v1 : edge.v2)));

                const size_t idx = ecount * 3 * 4;

                *(v + idx + 0) = v1.position.x();
                *(v + idx + 1) = v1.position.y();
                *(v + idx + 2) = v1.position.z();
                *(v + idx + 3) = 1.0f;

                *(v + idx + 4) = v2.position.x();
                *(v + idx + 5) = v2.position.y();
                *(v + idx + 6) = v2.position.z();
                *(v + idx + 7) = 1.0f;

                // third vertex is at infinity
                *(v + idx + 8) = 0.0f;
                *(v + idx + 9) = 0.0f;
                *(v + idx + 10) = 0.0f;
                *(v + idx + 11) = 0.0f;

                ecount++;
            }
        }
        vstart += mesh.vertex_count();
    }

    return ecount * 3;
}

size_t Renderable::compute_silhouette_positional(const Position& light_position, boost::shared_array<float> varray)
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3
    size_t vstart = 0, ecount = 0;
    float *v = varray.get();
    for(size_t i=0; i<model().mesh_count(); ++i) {
        const Mesh& mesh(model().mesh(i));

        for(size_t j=0; j<mesh.edge_count(); ++j) {
            const Edge& edge(mesh.edge(j));

            bool faces_light1;
            if(is_silhouette_edge(mesh, edge, light_position, vstart, faces_light1)) {
                const Vertex& v1(vertex(vstart + (faces_light1 ? edge.v2 : edge.v1)));
                const Vertex& v2(vertex(vstart + (faces_light1 ? edge.v1 : edge.v2)));

                const size_t idx = ecount * 4 * 4;

                *(v + idx + 0) = v1.position.x();
                *(v + idx + 1) = v1.position.y();
                *(v + idx + 2) = v1.position.z();
                *(v + idx + 3) = 1.0f;

                *(v + idx + 4) = v2.position.x();
                *(v + idx + 5) = v2.position.y();
                *(v + idx + 6) = v2.position.z();
                *(v + idx + 7) = 1.0f;

                // last two vertices are at infinity
                *(v + idx + 8) = v2.position.x();
                *(v + idx + 9) = v2.position.y();
                *(v + idx + 10) = v2.position.z();
                *(v + idx + 11) = 0.0f;

                *(v + idx + 12) = v1.position.x();
                *(v + idx + 13) = v1.position.y();
                *(v + idx + 14) = v1.position.z();
                *(v + idx + 15) = 0.0f;

                ecount++;
            }
        }
        vstart += mesh.vertex_count();
    }

    return ecount * 4;
}

void Renderable::render(Shader& shader) const
{
    Matrix4 matrix;
    transform(matrix);

    Renderer::instance().push_model_matrix();
    Renderer::instance().multiply_model_matrix(matrix);

    Renderer::instance().init_shader_matrices(shader);

    // render the meshes
    size_t tcount = 0;
    for(size_t i=0; i<model().mesh_count(); ++i) {
        const Mesh& mesh(model().mesh(i));
        render_mesh(mesh, tcount, shader);
        tcount += mesh.triangle_count();
    }

    Renderer::instance().pop_model_matrix();
}

void Renderable::render(Shader& shader, const Light& light, const Camera& camera) const
{
    Matrix4 matrix;
    transform(matrix);

    Renderer::instance().push_model_matrix();
    Renderer::instance().multiply_model_matrix(matrix);

    Renderer::instance().init_shader_matrices(shader);
    Renderer::instance().init_shader_light(shader, material(), light, camera);

    // render the meshes
    size_t tcount = 0;
    for(size_t i=0; i<model().mesh_count(); ++i) {
        const Mesh& mesh(model().mesh(i));
        render_mesh(mesh, tcount, shader);
        tcount += mesh.triangle_count();
    }

    Renderer::instance().pop_model_matrix();
}

void Renderable::render_shadow(Shader& shader, const Light& light, const Camera& camera, size_t vcount, bool cap) const
{
    Matrix4 matrix;
    transform(matrix);

    Renderer::instance().push_model_matrix();
    Renderer::instance().multiply_model_matrix(matrix);

    shader.begin();

    Renderer::instance().init_shader_matrices(shader);
    Renderer::instance().init_shader_light(shader, Material(), light, camera);

    if(typeid(light) == typeid(DirectionalLight)) {
        render_shadow_directional(shader, dynamic_cast<const DirectionalLight&>(light), vcount);
    } else if(typeid(light) == typeid(PositionalLight) || typeid(light) == typeid(SpotLight)) {
        render_shadow_positional(shader, dynamic_cast<const PositionalLight&>(light), vcount, cap);
    }

    shader.end();

    Renderer::instance().pop_model_matrix();
}

void Renderable::render_unlit(const Camera& camera)
{
    Matrix4 matrix;
    transform(matrix);

    Renderer::instance().push_model_matrix();
    Renderer::instance().multiply_model_matrix(matrix);

    if(State::instance().render_normals()) {
        render_normals();
    }

    if(State::instance().render_bounds()) {
        Renderer::instance().render_cube(relative_bounds());
    }

    Renderer::instance().pop_model_matrix();

    on_render_unlit(camera);
}

void Renderable::render_shadow_directional(Shader& shader, const DirectionalLight& light, size_t vcount) const
{
    // get the attribute locations
    GLint vloc = shader.attrib_location("vertex");

    // render the silhouette
    glEnableVertexAttribArray(vloc);
        glBindBuffer(GL_ARRAY_BUFFER, _shadow_vbo[ShadowVertexArray]);
        glVertexAttribPointer(vloc, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, vcount);
    glDisableVertexAttribArray(vloc);
}

void Renderable::render_shadow_positional(Shader& shader, const PositionalLight& light, size_t vcount, bool cap) const
{
    shader.uniform1i("cap", cap);

    // get the attribute locations
    GLint vloc = shader.attrib_location("vertex");

    // render the silhouette
    glEnableVertexAttribArray(vloc);
        glBindBuffer(GL_ARRAY_BUFFER, _shadow_vbo[ShadowVertexArray]);
        glVertexAttribPointer(vloc, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_QUADS, 0, vcount);
    glDisableVertexAttribArray(vloc);
}

void Renderable::render_mesh(const Mesh& mesh, size_t start, Shader& shader) const
{
    // setup the detail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh.texture(TextureManager::DetailTexture));
    shader.uniform1i("detail_texture", 0);

    // setup the normal map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh.texture(TextureManager::NormalMap));
    shader.uniform1i("normal_map", 1);

    // setup the specular map
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mesh.texture(TextureManager::SpecularMap));
    shader.uniform1i("specular_map", 2);

    // setup the emission map
    /*glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mesh.texture(TextureManager::EmissionMap));
    shader.uniform1i("emission_map", 3);*/

    // get the attribute locations
    GLint vloc = shader.attrib_location("vertex");
    GLint nloc = shader.attrib_location("normal");
    GLint tnloc = shader.attrib_location("tangent");
    GLint tloc = shader.attrib_location("texture_coord");

    // render the mesh
    glEnableVertexAttribArray(vloc);
    glEnableVertexAttribArray(nloc);
    glEnableVertexAttribArray(tnloc);
    glEnableVertexAttribArray(tloc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[TextureArray]);
        glVertexAttribPointer(tloc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[NormalArray]);
        glVertexAttribPointer(nloc, 3, GL_FLOAT, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[TangentArray]);
        glVertexAttribPointer(tnloc, 4, GL_FLOAT, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[VertexArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, start * 3, mesh.triangle_count() * 3);
    glDisableVertexAttribArray(tloc);
    glDisableVertexAttribArray(tnloc);
    glDisableVertexAttribArray(nloc);
    glDisableVertexAttribArray(vloc);
}

void Renderable::render_normals() const
{
    // render the mesh normals
    size_t tcount = 0;
    for(size_t i=0; i<model().mesh_count(); ++i) {
        const Mesh& mesh(model().mesh(i));
        render_normals(mesh, tcount);
        tcount += mesh.triangle_count();
    }
}

void Renderable::render_normals(const Mesh& mesh, size_t start) const
{
    const size_t vstart = start * 3 * 2 * 3;

    // setup the normal line array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[NormalLineArray]);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertex_count() * 2 * 3 * sizeof(float),
        _buffers.normal_line_buffer().get() + vstart, GL_DYNAMIC_DRAW);

    // setup the tangent line array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[TangentLineArray]);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertex_count() * 2 * 3 * sizeof(float),
        _buffers.tangent_line_buffer().get() + vstart, GL_DYNAMIC_DRAW);

    // render the normals
    Shader& rshader(State::instance().red_shader());
    rshader.begin();
    Renderer::instance().init_shader_matrices(rshader);

    // get the attribute locations
    GLint vloc = rshader.attrib_location("vertex");

    // render the mesh
    glEnableVertexAttribArray(vloc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[NormalLineArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_LINES, 0, mesh.vertex_count() * 2);
    glDisableVertexAttribArray(vloc);

    rshader.end();

    // render the tangents
    Shader& gshader(State::instance().green_shader());
    gshader.begin();
    Renderer::instance().init_shader_matrices(gshader);

    // get the attribute locations
    vloc = gshader.attrib_location("vertex");

    // render the mesh
    glEnableVertexAttribArray(vloc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[TangentLineArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_LINES, 0, mesh.vertex_count() * 2);
    glDisableVertexAttribArray(vloc);

    gshader.end();
}

void Renderable::calculate_vertices(const Skeleton& skeleton)
{
    _model->calculate_vertices(skeleton, _vertices, _buffers);

    // setup the vertex array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VertexArray]);
    glBufferData(GL_ARRAY_BUFFER, _buffers.vertex_buffer_size() * sizeof(float),
        _buffers.vertex_buffer().get(), is_static() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

    // setup the normal array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[NormalArray]);
    glBufferData(GL_ARRAY_BUFFER, _buffers.normal_buffer_size() * sizeof(float),
        _buffers.normal_buffer().get(), is_static() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

    // setup the tangent array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[TangentArray]);
    glBufferData(GL_ARRAY_BUFFER, _buffers.tangent_buffer_size() * sizeof(float),
        _buffers.tangent_buffer().get(), is_static() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

    // setup the texture array
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[TextureArray]);
    glBufferData(GL_ARRAY_BUFFER, _buffers.texture_buffer_size() * sizeof(float),
        _buffers.texture_buffer().get(), is_static() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
}
