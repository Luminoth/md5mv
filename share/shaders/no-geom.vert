#version 330

uniform mat4 mvp, modelview;

in vec3 vertex;
in vec2 texture_coord;

out vec2 frag_texture_coord;

void main()
{
    gl_Position = mvp * vec4(vertex, 1.0);
    frag_texture_coord = texture_coord;
}
