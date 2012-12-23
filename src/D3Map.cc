#include "pch.h"
#include "common.h"
#include "Camera.h"
#include "Lexer.h"
#include "Renderable.h"
#include "Renderer.h"
#include "State.h"
#include "D3Map.h"

D3Map::Surface::Surface()
{
    glGenBuffers(Renderable::VBOCount, vbo);
}

D3Map::Surface::~Surface() throw()
{
    glDeleteBuffers(Renderable::VBOCount, vbo);
    glDeleteTextures(TextureManager::TextureCount, textures);
}

void D3Map::Surface::init()
{
    compute_tangents(triangles, triangle_count, vertices, vertex_count);
    buffers.copy_triangles(triangles.get(), triangle_count, vertices.get(), vertex_count, 0, true);

    // setup the vertex array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[Renderable::VertexArray]);
    glBufferData(GL_ARRAY_BUFFER, buffers.vertex_buffer_size() * sizeof(float), buffers.vertex_buffer().get(), GL_STATIC_DRAW);

    // setup the normal array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[Renderable::NormalArray]);
    glBufferData(GL_ARRAY_BUFFER, buffers.normal_buffer_size() * sizeof(float), buffers.normal_buffer().get(), GL_STATIC_DRAW);

    // setup the tangent array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[Renderable::TangentArray]);
    glBufferData(GL_ARRAY_BUFFER, buffers.tangent_buffer_size() * sizeof(float), buffers.tangent_buffer().get(), GL_STATIC_DRAW);

    // setup the texture array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[Renderable::TextureArray]);
    glBufferData(GL_ARRAY_BUFFER, buffers.texture_buffer_size() * sizeof(float), buffers.texture_buffer().get(), GL_STATIC_DRAW);

    // setup the normal line array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[Renderable::NormalLineArray]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 2 * 3 * sizeof(float), buffers.normal_line_buffer().get(), GL_STATIC_DRAW);

    // setup the tangent line array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[Renderable::TangentLineArray]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 2 * 3 * sizeof(float), buffers.tangent_line_buffer().get(), GL_STATIC_DRAW);
}

bool D3Map::Surface::load_textures()
{
    // TODO: move this into a load_textures() method of the Surface class
    textures[TextureManager::DetailTexture] = TextureManager::instance().load_texture(data_dir() / (material + ".tga"));
    if(0 == textures[TextureManager::DetailTexture]) {
        return false;
    }

    textures[TextureManager::NormalMap] = TextureManager::instance().load_texture(data_dir() / (material + "_local.tga"));
    if(0 == textures[TextureManager::NormalMap]) {
        return false;
    }

    textures[TextureManager::SpecularMap] = TextureManager::instance().load_texture(data_dir() / (material + "_s.tga"));
    if(0 == textures[TextureManager::SpecularMap]) {
        return false;
    }

    /*textures[TextureManager::EmissionMap] = TextureManager::instance().load_texture(data_dir() / (material + "_e.tga"));
    if(0 == textures[TextureManager::EmissionMap]) {
        return false;
    }*/

    return true;
}

Logger& D3Map::logger(Logger::instance("md5mv.D3Map"));

D3Map::D3Map(const std::string& name)
    : Map(name), _version(0), _acount(0)
{
}

D3Map::~D3Map() throw()
{
    unload();
}

Position D3Map::player_spawn_position() const
{
    Position position;
    try {
        boost::shared_ptr<Entity> player_start(_entities.at("info_player_start_1"));
        const std::string data(player_start->properties.at("origin"));

        Lexer lexer(data);

        float value;
        if(!lexer.float_literal(value)) {
            return position;
        }
        position.x(value);

        if(!lexer.float_literal(value)) {
            return position;
        }
        position.y(value);

        if(!lexer.float_literal(value)) {
            return position;
        }
        position.z(value);
    } catch(const std::out_of_range&) {
        LOG_WARNING("Missing info_player_start origin!" << std::endl);
    }

    return swizzle(position);
}

float D3Map::player_spawn_angle() const
{
    float angle = 0.0f;
    try {
        boost::shared_ptr<Entity> player_start(_entities.at("info_player_start_1"));
        const std::string data(player_start->properties.at("angle"));

        Lexer lexer(data);
        if(!lexer.float_literal(angle)) {
            return angle;
        }
    } catch(const std::out_of_range&) {
        LOG_WARNING("Missing info_player_start angle!" << std::endl);
    }

    return angle;
}

bool D3Map::load(const boost::filesystem::path& path)
{
    unload();

    if(!load_map(path)) {
        return false;
    }

    if(!load_proc(path)) {
        return false;
    }

    return true;
}

void D3Map::on_unload()
{
    _version = 0;

    _acount = 0;
    _models.clear();

    _entities.clear();
    _worldspawn.reset();
}

void D3Map::render(const Camera& camera, Shader& shader) const
{
    Matrix4 matrix;
    matrix.translate(Position());

    Renderer::instance().push_model_matrix();
    Renderer::instance().multiply_model_matrix(matrix);

    Renderer::instance().init_shader_matrices(shader);

    // TODO: BSP and portal this shit
    BOOST_FOREACH(boost::shared_ptr<Model> model, _models) {
        if(model->is_area()) {
            render_area(camera, *model, shader);
        }
    }

    Renderer::instance().pop_model_matrix();
}


void D3Map::render(const Camera& camera, Shader& shader, const Light& light) const
{
    Renderer::instance().init_shader_matrices(shader);
    Renderer::instance().init_shader_light(shader, material(), light, camera);

    // TODO: BSP and portal this shit
    // for now we'll just do a bounds check
    BOOST_FOREACH(boost::shared_ptr<Model> model, _models) {
        if(model->is_area() && camera.visible(model->bounds)) {
            render_area(camera, *model, shader);
        }
    }
}

void D3Map::render_normals(const Camera& camera) const
{
    // TODO: BSP and portal this shit
    // for now we'll just do a bounds check
    BOOST_FOREACH(boost::shared_ptr<Model> model, _models) {
        if(model->is_area() && camera.visible(model->bounds)) {
            render_area_normals(camera, *model);
        }
    }
}

void D3Map::render_area(const Camera& camera, const Model& area, Shader& shader) const
{
    for(int i=0; i<area.surface_count; ++i) {
        const Surface& surface(*(area.surfaces[i]));
        if(camera.visible(surface.bounds)) {
            render_surface(surface, shader);
        }
    }
}

void D3Map::render_area_normals(const Camera& camera, const Model& area) const
{
    for(int i=0; i<area.surface_count; ++i) {
        const Surface& surface(*(area.surfaces[i]));
        if(camera.visible(surface.bounds)) {
            render_surface_normals(surface);
        }
    }
}

void D3Map::render_surface(const Surface& surface, Shader& shader) const
{
    // setup the detail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, surface.textures[TextureManager::DetailTexture]);
    shader.uniform1i("detail_texture", 0);

    // setup the normal map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, surface.textures[TextureManager::NormalMap]);
    shader.uniform1i("normal_map", 1);

    // setup the specular map
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, surface.textures[TextureManager::SpecularMap]);
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
        glBindBuffer(GL_ARRAY_BUFFER, surface.vbo[Renderable::TextureArray]);
        glVertexAttribPointer(tloc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, surface.vbo[Renderable::NormalArray]);
        glVertexAttribPointer(nloc, 3, GL_FLOAT, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, surface.vbo[Renderable::TangentArray]);
        glVertexAttribPointer(tnloc, 4, GL_FLOAT, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, surface.vbo[Renderable::VertexArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, surface.triangle_count * 3);
    glDisableVertexAttribArray(tloc);
    glDisableVertexAttribArray(tnloc);
    glDisableVertexAttribArray(nloc);
    glDisableVertexAttribArray(vloc);
}

void D3Map::render_surface_normals(const Surface& surface) const
{
    // render the normals
    Shader& rshader(State::instance().red_shader());
    rshader.begin();
    Renderer::instance().init_shader_matrices(rshader);

    // get the attribute locations
    GLint vloc = rshader.attrib_location("vertex");

    // render the mesh
    glEnableVertexAttribArray(vloc);
        glBindBuffer(GL_ARRAY_BUFFER, surface.vbo[Renderable::NormalLineArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_LINES, 0, surface.vertex_count * 2);
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
        glBindBuffer(GL_ARRAY_BUFFER, surface.vbo[Renderable::TangentLineArray]);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_LINES, 0, surface.vertex_count * 2);
    glDisableVertexAttribArray(vloc);

    gshader.end();
}

bool D3Map::load_map(const boost::filesystem::path& path)
{
    boost::filesystem::path filename = map_dir() / path / (name() + ".map");
    LOG_INFO("Loading map from '" << filename << "'" << std::endl);

    Lexer lexer;
    if(!lexer.load(filename)) {
        return false;
    }

    if(!scan_map_version(lexer)) {
        return false;
    }

    // map is just a list of entities
    while(!lexer.check_token(END)) {
        if(!scan_map_entity(lexer)) {
            return false;
        }
        lexer.skip_whitespace();
    }

    if(!_worldspawn) {
        LOG_ERROR("Map missing worldspawn!" << std::endl);
        return false;
    }

    LOG_INFO("Read " << _entities.size() << " non-worldspawn entities" << std::endl);

    return true;
}

bool D3Map::scan_map_version(Lexer& lexer)
{
    if(!lexer.match(VERSION)) {
        return false;
    }

    if(!lexer.int_literal(_version)) {
        return false;
    }

    // only version 2 is supported
    return _version == 2;
}

bool D3Map::scan_map_entity(Lexer& lexer)
{
    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    boost::shared_ptr<Entity> entity(new Entity());

    // read the key/value pairs
    while(!lexer.check_token(OPEN_BRACE) && !lexer.check_token(CLOSE_BRACE)) {
        std::string key;
        if(!lexer.string_literal(key)) {
            return false;
        }

        std::string value;
        if(!lexer.string_literal(value)) {
            return false;
        }

        entity->properties[key] = value;
    }

    // scan brushes
    if(!lexer.check_token(CLOSE_BRACE)) {
        if(!scan_map_brushes(lexer, entity)) {
            return false;
        }
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    try {
// TODO: make a mapping of property keys or something
        const std::string& classname(entity->properties.at("classname"));
        if(classname == "worldspawn") {
            if(_worldspawn) {
                LOG_ERROR("Only one worldspawn allowed!" << std::endl);
                return false;
            }
            _worldspawn = entity;
            LOG_INFO("World has " << entity->brushes.size() << " brushes and " << entity->patches.size() << " patches" << std::endl);
        } else {
            const std::string& name(entity->properties.at("name"));
            _entities[name] = entity;
        }
    } catch(std::out_of_range&) {
        return false;
    }

    return true;
}

bool D3Map::scan_map_brushes(Lexer& lexer, boost::shared_ptr<Entity> entity)
{
    while(!lexer.check_token(CLOSE_BRACE)) {
        if(!lexer.match(OPEN_BRACE)) {
            return false;
        }

        // either a brush or a patch
        if(lexer.check_token(BRUSHDEF3)) {
            if(!scan_map_brushdef3(lexer, entity)) {
                return false;
            }
        } else if(lexer.check_token(PATCHDEF2)) {
            if(!scan_map_patchdef2(lexer, entity)) {
                return false;
            }
        } else if(lexer.check_token(PATCHDEF3)) {
            if(!scan_map_patchdef3(lexer, entity)) {
                return false;
            }
        } else {
            LOG_ERROR("Unexpected brush token!" << std::endl);
            return false;
        }

        if(!lexer.match(CLOSE_BRACE)) {
            return false;
        }
    }

    return true;
}

bool D3Map::scan_map_brushdef3(Lexer& lexer, boost::shared_ptr<Entity> entity)
{
    if(!lexer.match(BRUSHDEF3)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    while(!lexer.check_token(CLOSE_BRACE)) {
        if(!scan_map_brush(lexer, entity)) {
            return false;
        }
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    return true;
}

bool D3Map::scan_map_brush(Lexer& lexer, boost::shared_ptr<Entity> entity)
{
    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    // normal
    Vector3 normal;

    float value;
    if(!lexer.float_literal(value)) {
        return false;
    }
    normal.x(value);

    if(!lexer.float_literal(value)) {
        return false;
    }
    normal.y(value);

    if(!lexer.float_literal(value)) {
        return false;
    }
    normal.z(value);

    // distance
    float d;
    if(!lexer.float_literal(d)) {
        return false;
    }

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    boost::shared_ptr<Brush> brush(new Brush());
    brush->plane = Plane(swizzle(normal), d);

    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    float xxscale;
    if(!lexer.float_literal(xxscale)) {
        return false;
    }

    float xyscale;
    if(!lexer.float_literal(xyscale)) {
        return false;
    }

    float xoffset;
    if(!lexer.float_literal(xoffset)) {
        return false;
    }

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    float yxscale;
    if(!lexer.float_literal(yxscale)) {
        return false;
    }

    float yyscale;
    if(!lexer.float_literal(yyscale)) {
        return false;
    }

    float yoffset;
    if(!lexer.float_literal(yoffset)) {
        return false;
    }

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    Matrix3 texture_matrix;
    texture_matrix(0, 0) = xxscale;
    texture_matrix(0, 1) = xyscale;
    texture_matrix(0, 2) = xoffset;
    texture_matrix(1, 0) = yxscale;
    texture_matrix(1, 1) = yyscale;
    texture_matrix(1, 2) = yoffset;
    brush->texture_matrix = texture_matrix;

    std::string material;
    if(!lexer.string_literal(material)) {
        return false;
    }

    brush->material = material;

    // swallow the unknown numbers that may follow the brushdef
    lexer.advance_line();

    entity->brushes.push_back(brush);

    return true;
}

bool D3Map::scan_map_patchdef2(Lexer& lexer, boost::shared_ptr<Entity> entity)
{
    if(!lexer.match(PATCHDEF2)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    std::string material;
    if(!lexer.string_literal(material)) {
        return false;
    }

    boost::shared_ptr<Patch2> patch(new Patch2());
    patch->material = material;

    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    float x;
    if(!lexer.float_literal(x)) {
        return false;
    }

    float y;
    if(!lexer.float_literal(y)) {
        return false;
    }

    // TODO: swizzle??
    patch->offset = Vector2(x, y);

    float rotation;
    if(!lexer.float_literal(rotation)) {
        return false;
    }
    patch->rotation = rotation;

    if(!lexer.float_literal(x)) {
        return false;
    }

    if(!lexer.float_literal(y)) {
        return false;
    }

    // TODO: swizzle?
    patch->scale = Vector2(x, y);

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    if(!scan_patch_control_grid(lexer, patch)) {
        return false;
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    entity->patches.push_back(patch);

    return true;
}

bool D3Map::scan_map_patchdef3(Lexer& lexer, boost::shared_ptr<Entity> entity)
{
    if(!lexer.match(PATCHDEF3)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    std::string material;
    if(!lexer.string_literal(material)) {
        return false;
    }

    boost::shared_ptr<Patch3> patch(new Patch3());
    patch->material = material;

    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    float x;
    if(!lexer.float_literal(x)) {
        return false;
    }

    float y;
    if(!lexer.float_literal(y)) {
        return false;
    }

    float z;
    if(!lexer.float_literal(z)) {
        return false;
    }

    // TODO: swizzle??
    patch->offset = Vector3(x, y, z);

    float rotation;
    if(!lexer.float_literal(rotation)) {
        return false;
    }
    patch->rotation = rotation;

    if(!lexer.float_literal(x)) {
        return false;
    }

    if(!lexer.float_literal(y)) {
        return false;
    }

    if(!lexer.float_literal(z)) {
        return false;
    }

    // TODO: swizzle?
    patch->scale = Vector3(x, y, z);

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    if(!scan_patch_control_grid(lexer, patch)) {
        return false;
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    entity->patches.push_back(patch);

    return true;
}

bool D3Map::scan_patch_control_grid(Lexer& lexer, boost::shared_ptr<Patch> patch)
{
    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    while(!lexer.check_token(CLOSE_PAREN)) {
        if(!scan_patch_control_grid_row(lexer, patch)) {
            return false;
        }
    }

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    return true;
}

bool D3Map::scan_patch_control_grid_row(Lexer& lexer, boost::shared_ptr<Patch> patch)
{
    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    while(!lexer.check_token(CLOSE_PAREN)) {
        if(!scan_patch_control_point(lexer, patch)) {
            return false;
        }
    }

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    return true;
}

bool D3Map::scan_patch_control_point(Lexer& lexer, boost::shared_ptr<Patch> patch)
{
    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    float x;
    if(!lexer.float_literal(x)) {
        return false;
    }

    float y;
    if(!lexer.float_literal(y)) {
        return false;
    }

    float z;
    if(!lexer.float_literal(z)) {
        return false;
    }

    float u;
    if(!lexer.float_literal(u)) {
        return false;
    }

    float v;
    if(!lexer.float_literal(v)) {
        return false;
    }

//LOG_ERROR("TODO: handle patch defs!" << std::endl);

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    return true;
}

bool D3Map::load_proc(const boost::filesystem::path& path)
{
    boost::filesystem::path filename(map_dir() / path / (name() + ".proc"));
    LOG_INFO("Loading geometry from '" << filename << "'" << std::endl);

    Lexer lexer;
    if(!lexer.load(filename)) {
        return false;
    }

    if(!scan_proc_header(lexer)) {
        return false;
    }

    while(!lexer.check_token(END)) {
        if(lexer.check_token(MODEL)) {
            if(!scan_proc_model(lexer)) {
                return false;
            }
        } else if(lexer.check_token(PORTALS)) {
            if(!scan_proc_portals(lexer)) {
                return false;
            }
        } else if(lexer.check_token(NODES)) {
            if(!scan_proc_nodes(lexer)) {
                return false;
            }
        } else if(lexer.check_token(SHADOW_MODEL)) {
            if(!scan_proc_shadow_model(lexer)) {
                return false;
            }
        }
    }

    return true;
}

bool D3Map::scan_proc_header(Lexer& lexer)
{
    if(!lexer.match(MAP_PROC_FILE)) {
        return false;
    }

    return true;
}

bool D3Map::scan_proc_model(Lexer& lexer)
{
    if(!lexer.match(MODEL)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    std::string name;
    if(!lexer.string_literal(name)) {
        return false;
    }

    int surface_count;
    if(!lexer.int_literal(surface_count)) {
        return false;
    }

    boost::shared_ptr<Model> model(new Model());
    model->name = name;
    model->surface_count = surface_count;

    model->surfaces.reset(new boost::shared_ptr<Surface>[model->surface_count]);
    for(int i=0; i<model->surface_count; ++i) {
        boost::shared_ptr<Surface> surface(new Surface());
        if(!scan_proc_surface(lexer, surface)) {
            return false;
        }
        model->surfaces[i] = surface;
        model->bounds.update(surface->bounds);
    }
    _models.push_back(model);

    if(model->is_area()) {
        _acount++;
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    return true;
}

bool D3Map::scan_proc_surface(Lexer& lexer, boost::shared_ptr<Surface> surface)
{
    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    std::string material;
    if(!lexer.string_literal(material)) {
        return false;
    }

    int vertex_count;
    if(!lexer.int_literal(vertex_count)) {
        return false;
    }

    int index_count;
    if(!lexer.int_literal(index_count)) {
        return false;
    }

    // indices define triangles, so we need a multiple of 3
    if(0 != index_count % 3) {
        return false;
    }

    surface->material = material;

    surface->vertex_count = vertex_count;
    surface->vertices.reset(new Vertex[surface->vertex_count]);

    surface->triangle_count = index_count / 3;
    surface->triangles.reset(new Triangle[surface->triangle_count]);

    for(int i=0; i<surface->vertex_count; ++i) {
        if(!scan_proc_vertex(lexer, surface, i)) {
            return false;
        }
    }

    for(int i=0; i<surface->triangle_count; ++i) {
        int a;
        if(!lexer.int_literal(a)) {
            return false;
        }

        int b;
        if(!lexer.int_literal(b)) {
            return false;
        }

        int c;
        if(!lexer.int_literal(c)) {
            return false;
        }

        // TODO: find a better way to swizzle this
        Triangle& triangle(surface->triangles[i]);
        triangle.index = i;
        triangle.v1 = c;
        triangle.v2 = b;
        triangle.v3 = a;
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    surface->init();
    if(!surface->load_textures()) {
        return false;
    }

    return true;
}

bool D3Map::scan_proc_vertex(Lexer& lexer, boost::shared_ptr<Surface> surface, int index)
{
    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

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

    float texture_u;
    if(!lexer.float_literal(texture_u)) {
        return false;
    }

    float texture_v;
    if(!lexer.float_literal(texture_v)) {
        return false;
    }

    Vector3 normal;

    if(!lexer.float_literal(value)) {
        return false;
    }
    normal.x(value);

    if(!lexer.float_literal(value)) {
        return false;
    }
    normal.y(value);

    if(!lexer.float_literal(value)) {
        return false;
    }
    normal.z(value);

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    Vertex& vertex(surface->vertices[index]);
    vertex.index = index;
    vertex.position = swizzle(position);
    vertex.normal = swizzle(normal);
    vertex.texture_coords = Vector2(texture_u, texture_v);
    surface->bounds.update(vertex.position);

    return true;
}

bool D3Map::scan_proc_portals(Lexer& lexer)
{
    if(!lexer.match(PORTALS)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    int area_count;
    if(!lexer.int_literal(area_count)) {
        return false;
    }

    if(area_count != _acount) {
        LOG_ERROR("Area count mismatch!" << std::endl);
        return false;
    }

    int portal_count;
    if(!lexer.int_literal(portal_count)) {
        return false;
    }

    for(int i=0; i<portal_count; ++i) {
        if(!scan_proc_portal(lexer)) {
            return false;
        }
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    return true;
}

bool D3Map::scan_proc_portal(Lexer& lexer)
{
    int vertex_count;
    if(!lexer.int_literal(vertex_count)) {
        return false;
    }

    int positive_area;
    if(!lexer.int_literal(positive_area)) {
        return false;
    }

    int negative_area;
    if(!lexer.int_literal(negative_area)) {
        return false;
    }

    for(int i=0; i<vertex_count; ++i) {
        if(!lexer.match(OPEN_PAREN)) {
            return false;
        }

        float x;
        if(!lexer.float_literal(x)) {
            return false;
        }

        float y;
        if(!lexer.float_literal(y)) {
            return false;
        }

        float z;
        if(!lexer.float_literal(z)) {
            return false;
        }

//LOG_ERROR("TODO: store portal!");

        if(!lexer.match(CLOSE_PAREN)) {
            return false;
        }
    }

    return true;
}

bool D3Map::scan_proc_nodes(Lexer& lexer)
{
    if(!lexer.match(NODES)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    int node_count;
    if(!lexer.int_literal(node_count)) {
        return false;
    }

    for(int i=0; i<node_count; ++i) {
        if(!scan_proc_node(lexer)) {
            return false;
        }
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

    return true;
}

bool D3Map::scan_proc_node(Lexer& lexer)
{
    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    float x;
    if(!lexer.float_literal(x)) {
        return false;
    }

    float y;
    if(!lexer.float_literal(y)) {
        return false;
    }

    float z;
    if(!lexer.float_literal(z)) {
        return false;
    }

    float d;
    if(!lexer.float_literal(d)) {
        return false;
    }

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    int pos_child;
    if(!lexer.int_literal(pos_child)) {
        return false;
    }

    int neg_child;
    if(!lexer.int_literal(neg_child)) {
        return false;
    }

//LOG_ERROR("TODO: store node!");

    return true;
}

bool D3Map::scan_proc_shadow_model(Lexer& lexer)
{
    if(!lexer.match(SHADOW_MODEL)) {
        return false;
    }

    if(!lexer.match(OPEN_BRACE)) {
        return false;
    }

    std::string name;
    if(!lexer.string_literal(name)) {
        return false;
    }

    int vertex_count;
    if(!lexer.int_literal(vertex_count)) {
        return false;
    }

    int cap_count;
    if(!lexer.int_literal(cap_count)) {
        return false;
    }

    int front_cap_count;
    if(!lexer.int_literal(front_cap_count)) {
        return false;
    }

    int index_count;
    if(!lexer.int_literal(index_count)) {
        return false;
    }

    int plane_bits;
    if(!lexer.int_literal(plane_bits)) {
        return false;
    }

    for(int i=0; i<vertex_count; ++i) {
        if(!scan_proc_shadow_vertex(lexer)) {
            return false;
        }
    }

    for(int i=0; i<index_count; ++i) {
        int index;
        if(!lexer.int_literal(index)) {
            return false;
        }
    }

    if(!lexer.match(CLOSE_BRACE)) {
        return false;
    }

//LOG_ERROR("TODO: store shadow model info!" << std::endl);

    return true;
}

bool D3Map::scan_proc_shadow_vertex(Lexer& lexer)
{
    if(!lexer.match(OPEN_PAREN)) {
        return false;
    }

    float value;
    if(!lexer.float_literal(value)) {
        return false;
    }

    if(!lexer.float_literal(value)) {
        return false;
    }

    if(!lexer.float_literal(value)) {
        return false;
    }

    if(!lexer.match(CLOSE_PAREN)) {
        return false;
    }

    return true;
}
