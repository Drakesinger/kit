#version 410 core

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec3 out_color;

uniform sampler2D uniform_sourceTexture;
uniform float uniform_resolution;
uniform float uniform_radius = 1.0;
uniform vec2 uniform_dir;

void main() {
    float blur = uniform_radius/uniform_resolution;
    out_color = vec3(0.0);
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x - 4.0*blur*uniform_dir.x, in_uv.y - 4.0*blur*uniform_dir.y)).rgb * 0.0162162162;
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x - 3.0*blur*uniform_dir.x, in_uv.y - 3.0*blur*uniform_dir.y)).rgb * 0.0540540541;
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x - 2.0*blur*uniform_dir.x, in_uv.y - 2.0*blur*uniform_dir.y)).rgb * 0.1216216216;
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x - 1.0*blur*uniform_dir.x, in_uv.y - 1.0*blur*uniform_dir.y)).rgb * 0.1945945946;
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x, in_uv.y)).rgb * 0.2270270270;
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x + 1.0*blur*uniform_dir.x, in_uv.y + 1.0*blur*uniform_dir.y)).rgb * 0.1945945946;
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x + 2.0*blur*uniform_dir.x, in_uv.y + 2.0*blur*uniform_dir.y)).rgb * 0.1216216216;
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x + 3.0*blur*uniform_dir.x, in_uv.y + 3.0*blur*uniform_dir.y)).rgb * 0.0540540541;
    out_color += texture(uniform_sourceTexture, vec2(in_uv.x + 4.0*blur*uniform_dir.x, in_uv.y + 4.0*blur*uniform_dir.y)).rgb * 0.0162162162;
}