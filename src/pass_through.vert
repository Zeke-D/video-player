#version 330

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_uv;

out vec2 frag_uv;

void main() {
    gl_Position = vec4(in_pos, 1.);
    frag_uv = in_uv; // pass to frag shader
}