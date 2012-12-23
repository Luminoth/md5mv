#if !defined __FONT_H__
#define __FONT_H__

#include "Shader.h"
#include "Vector.h"

typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;

class Camera;

// TODO: this doesn't handle non-printable characters
class TextFont
{
private:
    enum FontVBO
    {
        VertexArray,
        TextureArray,
        VBOCount
    };

private:
    static Logger& logger;
    static FT_Library freetype;
    static GLuint texture;
    static GLuint vbo[VBOCount];
    static Shader shader;

public:
    // NOTE: must be called *after* a GL context is available
    static bool init();
    static void shutdown();

public:
    TextFont();
    virtual ~TextFont() throw();

public:
    const Color& color() const { return _color; }
    void color(const Color& color) { _color = color; }

    bool load(const std::string& name, size_t height);
    void render(const std::string& text, const Position& position=Position(), const Vector2& scale=Vector2(1.0f, 1.0f), bool center=false) const;

private:
    void calculate_width_height(const std::string& text, float& width, float& height) const;

private:
    FT_Face _face;
    Color _color;

private:
    DISALLOW_COPY_AND_ASSIGN(TextFont);
};

#endif
