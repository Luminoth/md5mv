#version 330

uniform sampler2D detail_texture, normal_map, specular_map;
uniform vec4 light_position, light_ambient, light_diffuse, light_specular;
uniform float light_constant_attenuation, light_linear_attenuation, light_quadratic_attenuation, light_spotlight_cutoff, light_spotlight_exponent;
uniform vec4 material_ambient, material_diffuse, material_specular;
uniform float material_shininess;

// these are in tangent-space
in vec3 frag_L, frag_SD, frag_V;

// these are in object-space
in vec3 frag_N, frag_T;

// distance from the light to the vertex
in float distance;

in vec2 frag_texture_coord;

out vec4 fragment_color;

void main()
{
    // light vector from the surface to the light
    vec3 L = normalize(frag_L);

    // tangent-space normal (shifted to [-1, 1])
    vec4 normal = texture2D(normal_map, frag_texture_coord);
    vec3 N = normalize(2.0 * normal.xyz - 1.0);

    // view-vector
    vec3 V = normalize(frag_V);

    // half-vector (L - V is not "correct", but produces better highlights)
    vec3 H = normalize(L - V);

    float attenuation = light_position.w > 0.5
        ? (1.0 / (light_constant_attenuation
            + (light_linear_attenuation * distance)
            + (light_quadratic_attenuation * distance * distance)))
        : 1.0;

    // spotlight factor
    vec3 SD = normalize(frag_SD);
    float spotlight = max(dot(-SD, L), 0.0);
    spotlight = spotlight <= cos(radians(light_spotlight_cutoff)) ? pow(spotlight, light_spotlight_exponent) : 1.0;

    // ambient term
    vec4 ambient = clamp(attenuation * spotlight * (light_ambient * material_ambient), 0.0, 1.0);

    // diffuse term
    float lamber = max(dot(N, L), 0.0);
    vec4 diffuse = clamp(attenuation * spotlight * (light_diffuse * material_diffuse * lamber), 0.0, 1.0);

    // Blinn specular term
    vec4 specular_color = texture2D(specular_map, frag_texture_coord);
    float speculate = pow(max(dot(N, H), 0.0), material_shininess);
    vec4 specular = clamp(attenuation * spotlight * (light_specular * specular_color * speculate), 0.0, 1.0);

    vec4 texture_color = texture2D(detail_texture, frag_texture_coord);
    fragment_color = ((ambient + diffuse) * texture_color) + specular;
}
