#include "pch.h"
#include "fs_util.h"
#include "Lexer.h"

Lexer::KeywordMap Lexer::keyword_map;

Lexer::KeywordMap::KeywordMap()
{
    keywords["Version"] = VERSION;
    keywords["MD5Version"] = MD5VERSION;
    keywords["commandline"] = COMMANDLINE;
    keywords["numJoints"] = NUM_JOINTS;
    keywords["numMeshes"] = NUM_MESHES;
    keywords["joints"] = JOINTS;
    keywords["mesh"] = MESH;
    keywords["shader"] = SHADER;
    keywords["numverts"] = NUM_VERTS;
    keywords["vert"] = VERT;
    keywords["numtris"] = NUM_TRIS;
    keywords["tri"] = TRI;
    keywords["numweights"] = NUM_WEIGHTS;
    keywords["weight"] = WEIGHT;
    keywords["numFrames"] = NUM_FRAMES;
    keywords["frameRate"] = FRAME_RATE;
    keywords["numAnimatedComponents"] = NUM_ANIMATED_COMPONENTS;
    keywords["hierarchy"] = HIERARCHY;
    keywords["bounds"] = BOUNDS;
    keywords["baseframe"] = BASE_FRAME;
    keywords["frame"] = FRAME;
    keywords["map"] = MAP;
    keywords["global_ambient_color"] = GLOBAL_AMBIENT_COLOR;
    keywords["models"] = MODELS;
    keywords["renderables"] = RENDERABLES;
    keywords["lights"] = LIGHTS;
    keywords["ambient"] = AMBIENT;
    keywords["diffuse"] = DIFFUSE;
    keywords["specular"] = SPECULAR;
    keywords["emissive"] = EMISSIVE;
    keywords["shininess"] = SHININESS;
    keywords["brushDef3"] = BRUSHDEF3;
    keywords["patchDef2"] = PATCHDEF2;
    keywords["patchDef3"] = PATCHDEF3;
    keywords["mapProcFile003"] = MAP_PROC_FILE;
    keywords["model"] = MODEL;
    keywords["interAreaPortals"] = PORTALS;
    keywords["nodes"] = NODES;
    keywords["shadowModel"] = SHADOW_MODEL;
    keywords["meshes"] = MESHES;
    keywords["has_normals"] = HAS_NORMALS;
    keywords["has_edges"] = HAS_EDGES;
    keywords["vertices"] = VERTICES;
    keywords["triangles"] = TRIANGLES;
}

Lexer::Lexer()
    : _current(0)
{
}

Lexer::Lexer(const std::string& data)
    : _data(data), _current(0)
{
}

Lexer::~Lexer() throw()
{
}

bool Lexer::load(const boost::filesystem::path& filename)
{
    _data.erase();
    if(!file_to_string(filename, _data)) {
        return false;
    }
    return true;
}

bool Lexer::check_token(Token token)
{
    skip_whitespace();

    char ch = advance();
    _current--;
    switch(ch)
    {
    case '\0': return END == token;
    case '(': return OPEN_PAREN == token;
    case ')': return CLOSE_PAREN == token;
    case '{': return OPEN_BRACE == token;
    case '}': return CLOSE_BRACE == token;
    default:
        if(std::isalpha(ch) || '_' == ch) {
            return check_keyword(token);
        }
    }

    return false;
}

bool Lexer::match(Token token)
{
    Token next = lex();
    return next == token;
}

bool Lexer::int_literal(int& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while(ch != '\0' && (std::isdigit(ch) || ch == '-' || ch == '.')) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    char* end;
    value = strtol(scratch.c_str(), &end, 0);
    return !(*end);
}

bool Lexer::size_literal(size_t& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while(ch != '\0' && (std::isdigit(ch) || ch == '-' || ch == '.')) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    char* end;
    value = strtol(scratch.c_str(), &end, 0);
    return !(*end);
}

bool Lexer::float_literal(float& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while(ch != '\0' && (std::isdigit(ch) || ch == '-' || ch == '.')) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    char* end;
    value = strtod(scratch.c_str(), &end);
    return !(*end);
}

bool Lexer::string_literal(std::string& value)
{
    skip_whitespace();

    char ch = advance();
    if(ch != '"') return false;

    value.erase();

    ch = advance();
    while(ch != '\0' && ch != '"' && ch != '\n') {
        value += ch;
        ch = advance();
    }
    return ch != '\0' && ch != '\n';
}

bool Lexer::bool_literal(bool& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while(ch != '\0' && std::isalpha(ch)) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    value = ("true" == boost::algorithm::to_lower_copy(scratch));
    return true;
}

void Lexer::advance_line()
{
    char ch = advance();
    while(ch != '\0' && ch != '\r' && ch != '\n') {
        ch = advance();
    }
    skip_whitespace();
}

char Lexer::advance()
{
    char ret = _current < _data.length() ? _data[_current] : '\0';
    _current++;
    return ret;
}

Token Lexer::lex()
{
    skip_whitespace();

    char ch = advance();
    switch(ch)
    {
    case '\0': return END;
    case '(': return OPEN_PAREN;
    case ')': return CLOSE_PAREN;
    case '{': return OPEN_BRACE;
    case '}': return CLOSE_BRACE;
    default:
        if(std::isalnum(ch) || '_' == ch) {
            _current--;
            return keyword();
        }
    }

    return LEX_ERROR;
}

void Lexer::skip_whitespace()
{
    // skip whitespace
    char ch = advance();
    while(ch != '\0' && std::isspace(ch)) {
        ch = advance();
    }
    _current--;

    skip_comments();
}

void Lexer::skip_comments()
{
    char ch = advance();
    if(ch == '/') {
        ch = advance();
        if(ch == '/') {
            return advance_line();
        } else if(ch == '*') {
            ch = advance();
            while(true) {
                if(ch == '*') {
                    ch = advance();
                    if(ch == '/') {
                        return skip_whitespace();
                    }
                }
                ch = advance();
            }
        }
        _current--;
    }
    _current--;
}

bool Lexer::check_keyword(Token token)
{
    std::string scratch;

    char ch = advance();
    size_t len = 1;
    while(ch != '\0' && (std::isalnum(ch) || '_' == ch)) {
        scratch += ch;
        ch = advance();
        len++;
    }
    _current -= len;

    try {
        return token == keyword_map.keywords.at(scratch);
    } catch(std::out_of_range&) {
    }
    return false;
}

Token Lexer::keyword()
{
    std::string scratch;

    char ch = advance();
    while(ch != '\0' && (std::isalnum(ch) || '_' == ch)) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    try {
        return keyword_map.keywords.at(scratch);
    } catch(std::out_of_range&) {
    }
    return LEX_ERROR;
}
