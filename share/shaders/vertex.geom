#version 330

uniform mat4 mvp, modelview;

// these are in eye-space
uniform vec4 light_position, light_spotlight_direction;

// these are in object-space
uniform vec3 camera;

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

// these are in object-space
in vec3 geom_normal[3];

in vec2 geom_texture_coord[3];

// these are in eye-space
out vec3 frag_L, frag_SD, frag_V, frag_N;

// distance from the light to the vertex
out float distance;

out vec2 frag_texture_coord;

void main()
{
    for(int i=0; i<gl_in.length(); ++i) {
        // eye-space vertex position
        vec4 eye_Q = modelview * gl_in[i].gl_Position;

        // capture the distance from the light to the surface
        // while we're in eye space and before we start normalizing everything
        distance = length(light_position.xyz - eye_Q.xyz);

        // spotlight direction, towards the surface
        frag_SD = light_spotlight_direction.xyz;

        // light vector from the surface to the light
        frag_L = light_position.w > 0.5
                ? (light_position.xyz - eye_Q.xyz)
                : light_position.xyz;

        frag_V = (modelview * vec4(camera - gl_in[i].gl_Position.xyz, 1.0)).xyz;
        frag_N = (modelview * vec4(geom_normal[i], 0.0)).xyz;
        frag_texture_coord = geom_texture_coord[i];

        gl_Position = mvp * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
