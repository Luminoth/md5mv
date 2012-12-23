#if !defined __LEXER_H__
#define __LEXER_H__

enum Token
{
    // keywords
    VERSION,
    MD5VERSION,
    COMMANDLINE,
    NUM_JOINTS,
    NUM_MESHES,
    JOINTS,
    MESH,
    SHADER,
    NUM_VERTS,
    VERT,
    NUM_TRIS,
    TRI,
    NUM_WEIGHTS,
    WEIGHT,
    NUM_FRAMES,
    FRAME_RATE,
    NUM_ANIMATED_COMPONENTS,
    HIERARCHY,
    BOUNDS,
    BASE_FRAME,
    FRAME,
    MAP,
    GLOBAL_AMBIENT_COLOR,
    MODELS,
    RENDERABLES,
    LIGHTS,
    AMBIENT,
    DIFFUSE,
    SPECULAR,
    EMISSIVE,
    SHININESS,
    BRUSHDEF3,
    PATCHDEF2,
    PATCHDEF3,
    MAP_PROC_FILE,
    MODEL,
    PORTALS,
    NODES,
    SHADOW_MODEL,
    MESHES,
    HAS_NORMALS,
    HAS_EDGES,
    VERTICES,
    TRIANGLES,

    // delimiters
    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACE,
    CLOSE_BRACE,

    // end of file
    END,

    // scan error
    LEX_ERROR
};

class Lexer
{
private:
    struct KeywordMap
    {
        boost::unordered_map<std::string, Token> keywords;
        KeywordMap();
    };
    static KeywordMap keyword_map;

public:
    Lexer();
    explicit Lexer(const std::string& data);
    virtual ~Lexer() throw();

public:
    bool load(const boost::filesystem::path& filename);

    int position() const { return _current; }
    size_t length() const { return _data.length(); }
    void clear() { _current = 0; _data.erase(); }
    void reset() { _current = 0; }

    void skip_whitespace();
    bool check_token(Token token);
    bool match(Token token);
    bool int_literal(int& value);
    bool size_literal(size_t& value);
    bool float_literal(float& value);
    bool string_literal(std::string& value);
    bool bool_literal(bool& value);

    // swallows the rest of the current line
    void advance_line();

private:
    void skip_comments();

    char advance();
    Token lex();
    bool check_keyword(Token token);
    Token keyword();

private:
    std::string _data;
    int _current;

private:
    DISALLOW_COPY_AND_ASSIGN(Lexer);
};

#endif
