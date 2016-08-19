#version 410 core

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec4 out_color;

uniform sampler2D uniform_sourceTexture;
uniform float uniform_pow = 20.0;

void main( ) {
  vec3 source = texture(uniform_sourceTexture, in_uv).rgb;

  out_color.rgb = pow(source, vec3(uniform_pow));
  out_color.a = 1.0;
}
