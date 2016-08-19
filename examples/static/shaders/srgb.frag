#version 410 core

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec3 out_color;

uniform sampler2D uniform_sourceTexture;


vec3 apply_srgb(vec3 color)
{
    return pow(color, vec3(1.0/2.2));
}

void main()
{
  out_color = apply_srgb(texture(uniform_sourceTexture, in_uv).rgb);
}