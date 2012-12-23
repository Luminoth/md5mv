#include "pch.h"
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "common.h"
#include "Camera.h"
#include "Renderer.h"
#include "Font.h"

Logger& TextFont::logger(Logger::instance("md5mv.TextFont"));
FT_Library TextFont::freetype = NULL;
GLuint TextFont::texture = 0;
GLuint TextFont::vbo[VBOCount] = { 0 };
Shader TextFont::shader("text");

bool TextFont::init()
{
    if(FT_Init_FreeType(&freetype)) {
        LOG_CRITICAL("Could not init freetype library!" << std::endl);
        return false;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenBuffers(VBOCount, vbo);

    shader.create();
    shader.read_shader(shader_dir() / "no-geom.vert");
    shader.read_shader(shader_dir() / "text.frag");
    shader.bind_fragment_data_location(0, "fragment_color");
    shader.link();

    return true;
}

void TextFont::shutdown()
{
    glDeleteBuffers(VBOCount, vbo);
    glDeleteTextures(1, &texture);
}

TextFont::TextFont()
{
}

TextFont::~TextFont() throw()
{
}

bool TextFont::load(const std::string& name, size_t height)
{
    boost::filesystem::path filename(font_dir() / (name + ".ttf"));
    if(FT_New_Face(freetype, filename.string().c_str(), 0, &_face)) {
        LOG_ERROR("Could not open font '" << filename << "'!" << std::endl);
        return false;
    }
    FT_Set_Pixel_Sizes(_face, 0, height);

    return true;
}

void TextFont::render(const std::string& text, const Position& position, const Vector2& scale, bool center) const
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    shader.begin();
    Renderer::instance().init_shader_matrices(shader);

    shader.uniform4f("color", _color);

    // setup the detail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.uniform1i("detail_texture", 0);

    // get the attribute locations
    GLint vloc = shader.attrib_location("vertex");
    GLint tloc = shader.attrib_location("texture_coord");

    glEnableVertexAttribArray(vloc);
    glEnableVertexAttribArray(tloc);

    float x=position.x(), y=position.y();
    if(center) {
        float width, height;
        calculate_width_height(text, width, height);
        x -= width / 2.0f;
    }

    BOOST_FOREACH(const char ch, text) {
        // render the glyph to a texture
        if(FT_Load_Char(_face, ch, FT_LOAD_RENDER)) {
            continue;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, _face->glyph->bitmap.width, _face->glyph->bitmap.rows,
            0, GL_ALPHA, GL_UNSIGNED_BYTE, _face->glyph->bitmap.buffer);

        const float x2 = x + _face->glyph->bitmap_left * scale.x();
        const float y2 = y + _face->glyph->bitmap_top * scale.y();
        const float w = _face->glyph->bitmap.width * scale.x();
        const float h = _face->glyph->bitmap.rows * scale.y();

        // combined vertex and texture coord
        const float vertices[] = {
            x2,     y2 - h, 0.0f,
            x2 + w, y2 - h, 0.0f,
            x2,     y2,     0.0f,
            x2 + w, y2,     0.0f
        };

        static const float texture_coords[] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
        };

        // render the glyph
        glBindBuffer(GL_ARRAY_BUFFER, vbo[VertexArray]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[TextureArray]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), texture_coords, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(tloc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        x += (_face->glyph->advance.x >> 6) * scale.x();
        y += (_face->glyph->advance.y >> 6) * scale.y();
    }

    glDisableVertexAttribArray(tloc);
    glDisableVertexAttribArray(vloc);

    shader.end();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glDisable(GL_BLEND);
}

/*void TextFont::render(const std::string& text, const Camera& camera, const Position& position, const Vector2& scale, bool center) const
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    shader.begin();
    Renderer::instance().init_shader_matrices(shader);

    shader.uniform4f("color", _color);

    // setup the detail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.uniform1i("detail_texture", 0);

    // get the attribute locations
    GLint vloc = shader.attrib_location("vertex");
    GLint tloc = shader.attrib_location("texture_coord");

    glEnableVertexAttribArray(vloc);
    glEnableVertexAttribArray(tloc);

    float x=position.x(), y=position.y();
    if(center) {
        float width, height;
        calculate_width_height(text, width, height);
        x -= width / 2.0f;
    }

    BOOST_FOREACH(const char ch, text) {
        // render the glyph to a texture
        if(FT_Load_Char(_face, ch, FT_LOAD_RENDER)) {
            continue;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, _face->glyph->bitmap.width, _face->glyph->bitmap.rows,
            0, GL_ALPHA, GL_UNSIGNED_BYTE, _face->glyph->bitmap.buffer);

        const float x2 = x + _face->glyph->bitmap_left * scale.x();
        const float y2 = y + _face->glyph->bitmap_top * scale.y();
        const float w = _face->glyph->bitmap.width * scale.x();
        const float h = _face->glyph->bitmap.rows * scale.y();

        const Position P(x2, y2, position.z());

        // vector from the quad to the camera
        const Vector3 Z((camera.position() - P).normalized());

        // vector in the billboard plane
        const Vector3 A((camera.up() ^ Z).normalized());

        // vector orthogonal to A
        const Vector3 B(Z ^ A);

        const Vector3 X((w / 2.0f) * A), Y((h / 2.0f) * B);
        const Vector3 Q1(P + X + Y), Q2(P - X + Y), Q3(P - X - Y), Q4(P + X - Y);

        // combined vertex and texture coord
        const float vertices[] = {
            Q1.x(), Q1.y(), Q1.z(),
            Q2.x(), Q2.y(), Q2.z(),
            Q3.x(), Q3.y(), Q3.z(),
            Q4.x(), Q4.y(), Q4.z()
        };

        static const float texture_coords[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
        };

        // render the glyph
        glBindBuffer(GL_ARRAY_BUFFER, vbo[VertexArray]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[TextureArray]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), texture_coords, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(tloc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_QUADS, 0, 4);

        x += (_face->glyph->advance.x >> 6) * scale.x();
        y += (_face->glyph->advance.y >> 6) * scale.y();
    }

    glDisableVertexAttribArray(tloc);
    glDisableVertexAttribArray(vloc);

    shader.end();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glDisable(GL_BLEND);
}*/

void TextFont::calculate_width_height(const std::string& text, float& width, float& height) const
{
    width = 0.0f; height = 0.0f;
    BOOST_FOREACH(const char ch, text) {
        if(FT_Load_Char(_face, ch, FT_LOAD_RENDER)) {
            continue;
        }
        width += _face->glyph->bitmap.width;
        height += _face->glyph->bitmap.rows;
    }
}
