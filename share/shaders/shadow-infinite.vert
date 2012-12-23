#version 330

uniform mat4 mvp, modelview;

// these are in eye-space
uniform vec4 light_position;

in vec4 vertex;

void main()
{
    // Mathematics for 3D Game Programming and Computer Graphics, section 10.3.4

    // convert the light to object-space
    vec4 L = inverse(modelview) * light_position;

    float t = vertex.w < 0.5 ? 1.0 : 0.0;
    vec4 extruded = vertex - t * vec4(vertex.xyz + L.xyz, vertex.w);
    gl_Position = mvp * extruded;
}
