#version 330

uniform sampler2D detail_texture;
uniform vec4 color;

in vec2 frag_texture_coord;

out vec4 fragment_color;

void main()
{
    vec4 texture_color = texture2D(detail_texture, frag_texture_coord);
    fragment_color = vec4(1.0, 1.0, 1.0, texture_color.a) * color;
}
