#include "pch.h"
#include <iostream>
#include <fstream>
#include "common.h"
#include "fs_util.h"
#include "Lexer.h"
#include "Shader.h"

Logger& Shader::logger(Logger::instance("md5mv.Shader"));

void Shader::print_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
    GLint log_length=0;
    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    if(log_length > 1) {
        boost::scoped_array<char> log(new char[log_length]);
        glGet__InfoLog(object, log_length, NULL, log.get());

// TODO: silence these entries:
//Vertex shader was successfully compiled to run on hardware.\n
//Geometry shader was successfully compiled to run on hardware.\n
//Fragment shader was successfully compiled to run on hardware.\n
//Fragment shader(s) linked, vertex shader(s) linked.\n

        LOG_INFO("Shader log: " << log.get() << std::endl);
    }
}

Shader::Shader(const std::string& name)
    : _name(name), _max_geometry_vertices(0), _program(0)
{
    //glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &_max_geometry_vertices);
}

Shader::~Shader() throw()
{
    glDeleteProgram(_program);
    BOOST_FOREACH(GLuint shader, _shaders) {
        glDeleteShader(shader);
    }
}

void Shader::create() throw(ShaderError)
{
    _program = glCreateProgram();
}

void Shader::read_shader(const boost::filesystem::path& filename) throw(ShaderError)
{
    boost::shared_array<char> source(read_shader_file(filename));

    std::string ext(boost::filesystem::extension(filename));
    if(".vert" == ext) {
        compile_vertex_shader(source.get());
    } else if(".geom" == ext) {
        compile_geometry_shader(source.get());
    } else if(".frag" == ext) {
        compile_fragment_shader(source.get());
    } else {
        throw ShaderError("Unknown shader type: " + filename.string());
    }
}

void Shader::compile_vertex_shader(const char* source) throw(ShaderError)
{
    _shaders.push_back(compile_shader(GL_VERTEX_SHADER, source));
}

void Shader::compile_geometry_shader(const char* source) throw(ShaderError)
{
    _shaders.push_back(compile_shader(GL_GEOMETRY_SHADER, source));
}

void Shader::compile_fragment_shader(const char* source) throw(ShaderError)
{
    _shaders.push_back(compile_shader(GL_FRAGMENT_SHADER, source));
}

void Shader::link() throw(ShaderError)
{
    BOOST_FOREACH(GLuint shader, _shaders) {
        glAttachShader(_program, shader);
    }

    // this requires an extension that isn't available at work
    /*glProgramParameteriEXT(_program, GL_GEOMETRY_INPUT_TYPE, GL_TRIANGLES);
    glProgramParameteriEXT(_program, GL_GEOMETRY_OUTPUT_TYPE, GL_TRIANGLE_STRIP);
    glProgramParameteriEXT(_program, GL_GEOMETRY_VERTICES_OUT, _max_geometry_vertices);*/

    glLinkProgram(_program);
    print_info_log(_program, glGetProgramiv, glGetProgramInfoLog);

    GLint program_ok=0;
    glGetProgramiv(_program, GL_LINK_STATUS, &program_ok);
    if(GL_TRUE != program_ok) {
        glDeleteProgram(_program);
        _program = 0;
        throw ShaderError("Failed to link shader!");
    }
}

boost::shared_array<char> Shader::read_shader_file(const boost::filesystem::path& filename) throw(ShaderError)
{
    LOG_INFO("Reading shader from " << filename << std::endl);

    std::string source;
    if(!file_to_string(filename, source)) {
        throw ShaderError("Could not read shader source!");
    }

    boost::shared_array<char> ret(new char[source.length()+1]);
    std::memcpy(ret.get(), source.c_str(), source.length());
    ret[source.length()] = 0;
    return ret;
}

GLuint Shader::compile_shader(GLenum type, const char* source) throw(ShaderError)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    print_info_log(shader, glGetShaderiv, glGetShaderInfoLog);

    GLint shader_ok=0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if(GL_TRUE != shader_ok) {
        glDeleteShader(shader);
        throw ShaderError("Failed to compile shader!");
    }
    return shader;
}

GLint Shader::uniform_location(const std::string& name)
{
    try {
        return _uniform_map.at(name);
    } catch(const std::out_of_range&) {
    }

    GLint location = glGetUniformLocation(_program, name.c_str());
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error getting uniform location '" << name << "'" << std::endl);
    }

    LOG_DEBUG("Shader '" << _name << "' uniform '" << name << "' at " << location << std::endl);
    _uniform_map[name] = location;
    return location;
}

void Shader::uniform1f(const std::string& name, GLfloat v0)
{
    glUniform1f(uniform_location(name), v0);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform1fv(const std::string& name, size_t count, const GLfloat* value)
{
    glUniform1fv(uniform_location(name), count, value);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR(Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform2f(const std::string& name, GLfloat v0, GLfloat v1)
{
    glUniform2f(uniform_location(name), v0, v1);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform2f(const std::string& name, const Vector2& value)
{
    uniform2f(name, value.x(), value.y());
}

void Shader::uniform2fv(const std::string& name, size_t count, const GLfloat* value)
{
    glUniform2fv(uniform_location(name), count, value);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    glUniform3f(uniform_location(name), v0, v1, v2);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform3f(const std::string& name, const Vector3& value)
{
    uniform3f(name, value.x(), value.y(), value.z());
}

void Shader::uniform3fv(const std::string& name, size_t count, const GLfloat* value)
{
    glUniform3fv(uniform_location(name), count, value);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    glUniform4f(uniform_location(name), v0, v1, v2, v3);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform4f(const std::string& name, const Vector4& value)
{
    uniform4f(name, value.x(), value.y(), value.z(), value.w());
}

void Shader::uniform4fv(const std::string& name, size_t count, const GLfloat* value)
{
    glUniform4fv(uniform_location(name), count, value);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform1i(const std::string& name, GLint v0)
{
    glUniform1i(uniform_location(name), v0);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform_matrix3fv(const std::string& name, const GLfloat* v, bool transpose)
{
    glUniformMatrix3fv(uniform_location(name), 1, transpose ? GL_TRUE : GL_FALSE, v);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

void Shader::uniform_matrix4fv(const std::string& name, const GLfloat* v, bool transpose)
{
    glUniformMatrix4fv(uniform_location(name), 1, transpose ? GL_TRUE : GL_FALSE, v);
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error setting uniform value '" << name << "'" << std::endl);
    }
}

GLint Shader::attrib_location(const std::string& name)
{
    try {
        return _attrib_map.at(name);
    } catch(const std::out_of_range&) {
    }

    GLint location = glGetAttribLocation(_program, name.c_str());
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error getting attribute location '" << name << "'" << std::endl);
    }

    LOG_DEBUG("Shader '" << _name << "' attribute '" << name << "' at " << location << std::endl);
    _attrib_map[name] = location;
    return location;
}

void Shader::bind_attrib(GLuint index, const std::string& name) const
{
    glBindAttribLocation(_program, index, name.c_str());
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error binding attribute '" << name << "'" << std::endl);
    }
}

void Shader::bind_fragment_data_location(GLuint color_number, const std::string& name) const
{
    glBindFragDataLocation(_program, color_number, name.c_str());
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        //LOG_ERROR("Error binding fragment data location '" << name << "'" << std::endl);
    }
}

void Shader::begin() const
{
    glUseProgram(_program);
}

void Shader::end() const
{
    glUseProgram(0);
}
