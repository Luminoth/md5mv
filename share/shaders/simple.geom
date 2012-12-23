#version 330

uniform mat4 mvp, modelview;

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec2 geom_texture_coord[3];

out vec2 frag_texture_coord;

void main()
{
    for(int i=0; i<gl_in.length(); ++i) {
        gl_Position = mvp * gl_in[i].gl_Position;
        frag_texture_coord = geom_texture_coord[i];
        EmitVertex();
    }
    EndPrimitive();
}
