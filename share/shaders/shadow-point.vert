#version 330

uniform mat4 mvp, modelview;

// true if we need to cap the shadow
uniform bool cap;

// these are in eye-space
uniform vec4 light_position;

in vec4 vertex;

void main()
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3.4

    // convert the light to object-space
    vec4 L = inverse(modelview) * light_position;

    float t = (cap || vertex.w < 0.5) ? 1.0 : 0.0;
    vec4 extruded = vec4(vertex.xyz - t * L.xyz, cap ? 0.0 : vertex.w);
    gl_Position = mvp * extruded;
}
