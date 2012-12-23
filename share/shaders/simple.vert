#version 330

in vec4 tangent;
in vec3 vertex, normal;
in vec2 texture_coord;

out vec4 geom_tangent;
out vec3 geom_normal;
out vec2 geom_texture_coord;

void main()
{
    gl_Position = vec4(vertex, 1.0);
    geom_normal = normal;
    geom_tangent = tangent;
    geom_texture_coord = texture_coord;
}
