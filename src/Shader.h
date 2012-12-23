#if !defined __SHADER_H__
#define __SHADER_H__

#include "Vector.h"

class ShaderError
{
public:
    explicit ShaderError(const std::string& what) throw() : _what(what) {}
    virtual ~ShaderError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

private:
    std::string _what;
};

class Shader
{
private:
    static void print_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

private:
    static Logger& logger;

public:
    explicit Shader(const std::string& name);
    virtual ~Shader() throw();

public:
    const std::string& name() const { return _name; }

    void create() throw(ShaderError);

    void read_shader(const boost::filesystem::path& filename) throw(ShaderError);

    void compile_vertex_shader(const char* source) throw(ShaderError);
    void compile_geometry_shader(const char* source) throw(ShaderError);
    void compile_fragment_shader(const char* source) throw(ShaderError);
    void link() throw(ShaderError);

    GLint uniform_location(const std::string& name);

    void uniform1f(const std::string& name, GLfloat v0);
    void uniform1fv(const std::string& name, size_t count, const GLfloat* value);

    void uniform2f(const std::string& name, GLfloat v0, GLfloat v1);
    void uniform2f(const std::string& name, const Vector2& value);
    void uniform2fv(const std::string& name, size_t count, const GLfloat* value);

    void uniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2);
    void uniform3f(const std::string& name, const Vector3& value);
    void uniform3fv(const std::string& name, size_t count, const GLfloat* value);

    void uniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void uniform4f(const std::string& name, const Vector4& value);
    void uniform4fv(const std::string& name, size_t count, const GLfloat* value);

    void uniform1i(const std::string& name, GLint v0);

    void uniform_matrix3fv(const std::string& name, const GLfloat* v, bool transpose=true);
    void uniform_matrix4fv(const std::string& name, const GLfloat* v, bool transpose=true);

    GLint attrib_location(const std::string& name);
    void bind_attrib(GLuint index, const std::string& name) const;

    void bind_fragment_data_location(GLuint color_number, const std::string& name) const;

    void begin() const;
    void end() const;

private:
    boost::shared_array<char> read_shader_file(const boost::filesystem::path& filename) throw(ShaderError);
    GLuint compile_shader(GLenum type, const char* source) throw(ShaderError);

private:
    std::string _name;
    GLint _max_geometry_vertices;
    std::vector<GLuint> _shaders;
    GLuint _program;

    boost::unordered_map<std::string, GLint> _attrib_map;
    boost::unordered_map<std::string, GLint> _uniform_map;

private:
    Shader();
    DISALLOW_COPY_AND_ASSIGN(Shader);
};

#endif
