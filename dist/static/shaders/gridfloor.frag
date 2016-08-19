#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 out_color;

uniform float uniform_whitepoint;

void main()
{
  out_color = in_color;
  out_color.rgb *= uniform_whitepoint;
  out_color.a *= max(0.0, 1.0 - ( length(in_position) / 10.0));
}
