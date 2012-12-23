#if !defined __MD5MODEL_H__
#define __MD5MODEL_H__

#include "Mesh.h"
#include "Model.h"

class Lexer;

class MD5Mesh : public Mesh
{
public:
    MD5Mesh(const std::string& shader, size_t vcount, boost::shared_array<Vertex> vertices, size_t tcount, boost::shared_array<Triangle> triangles, size_t wcount, boost::shared_array<Weight> weights);
    virtual ~MD5Mesh() throw();

public:
    const std::string& shader_name() const { return _shader_name; }

private:
    std::string _shader_name;

private:
    MD5Mesh();
    DISALLOW_COPY_AND_ASSIGN(MD5Mesh);
};

class MD5Model : public Model
{
public:
    static std::string extension() { return ".md5mesh"; }

private:
    static Logger& logger;

public:
    explicit MD5Model(const std::string& name);
    virtual ~MD5Model() throw();

public:
    int version() const { return _version; }
    const std::string& commandline() const { return _commandline; }

private:
    virtual bool on_load(const boost::filesystem::path& path);
    void on_unload() throw();

private:
    bool scan_version(Lexer& lexer);
    bool scan_commandline(Lexer& lexer);
    int scan_num_joints(Lexer& lexer);
    int scan_num_meshes(Lexer& lexer);
    bool scan_joints(Lexer& lexer, int count);
    bool scan_joint(Lexer& lexer);
    bool scan_meshes(Lexer& lexer, int count);
    bool scan_mesh(Lexer& lexer);
    bool scan_vertices(Lexer& lexer, boost::shared_array<Vertex>& vertices, int count);
    bool scan_triangles(Lexer& lexer, boost::shared_array<Vertex>& vertices, boost::shared_array<Triangle>& triangles, int count);
    bool scan_weights(Lexer& lexer, boost::shared_array<Weight>& weights, int count);

private:
    int _version;
    std::string _commandline;

private:
    MD5Model();
    DISALLOW_COPY_AND_ASSIGN(MD5Model);
};

#endif
