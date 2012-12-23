#version 330

uniform sampler2D detail_texture, emission_texture;
uniform vec4 global_ambient_color;
uniform vec4 material_ambient, material_emissive;

in vec2 frag_texture_coord;

out vec4 fragment_color;

void main()
{
    vec4 texture_color = texture2D(detail_texture, frag_texture_coord);
    vec4 emission_color = texture2D(emission_texture, frag_texture_coord);
    fragment_color = (material_emissive * emission_color) + (global_ambient_color + material_ambient) * texture_color;
}
