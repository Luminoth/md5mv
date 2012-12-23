#version 330

uniform sampler2D ambient_texture, detail_texture;

in vec2 frag_texture_coord;

out vec4 fragment_color;

void main()
{
    vec4 ambient_color = texture2D(ambient_texture, frag_texture_coord);
    vec4 detail_color = texture2D(detail_texture, frag_texture_coord);
    fragment_color = ambient_color + detail_color;
}
