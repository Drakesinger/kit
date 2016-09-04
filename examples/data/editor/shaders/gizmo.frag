#version 410 core

layout (location = 0) out vec4 out_color;
layout (location = 0) in vec3 in_normal;

uniform vec4 uniform_color;

void main()
{
  const float ambience = 0.5;
  float lighting = 1.0 - ambience;
  float lambert = dot(in_normal, vec3(0.0, 0.0, 1.0));
  out_color = (uniform_color*ambience) + (uniform_color*lambert);
}