#version 330
precision highp float;

in vec2 frag_uv;
out vec4 color;

uniform sampler2D frame_tex;

void main() {
    // color = .2 * vec4(frag_uv.x, frag_uv.y, 0.1, 1.);
    color = texture(frame_tex, frag_uv);
    // color = vec4(gl_FragCoord.x, gl_FragCoord.y, .1, 1.);
}
