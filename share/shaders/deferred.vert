#version 330

in vec3 vertex;

out vec2 frag_texture_coord;

void main()
{
    gl_Position = vec4(vertex, 1.0);
    frag_texture_coord = ((gl_Position + 1.0) / 2.0).xy;
}
