#version 410 core

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec3 out_color;

uniform sampler2D uniform_sourceTexture;
uniform sampler3D uniform_lut;


vec3 apply_lut(vec3 color)
{
    const float lut_size = 32.0;
    const float lut_start = 0.5 / lut_size;
    const float lut_end = 1.0 - lut_start;
    color = color * (lut_end - lut_start) + lut_start;
    return texture(uniform_lut, color, 0).xyz;
}

void main()
{
  out_color = apply_lut(texture(uniform_sourceTexture, in_uv).rgb);
  //out_color.yz = vec2(1.0) - out_color.yz;
}