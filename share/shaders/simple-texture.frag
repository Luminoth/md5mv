#version 330

uniform sampler2D detail_texture;

in vec2 frag_texture_coord;

out vec4 fragment_color;

void main()
{
    vec4 texture_color = texture2D(detail_texture, frag_texture_coord);
    fragment_color = texture_color;
}
