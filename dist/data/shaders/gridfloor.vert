#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec4 out_color;

uniform mat4 uniform_mvpMatrix;

void main()
{
  gl_Position = uniform_mvpMatrix * vec4(in_position, 1.0);
  out_position = in_position;
  out_color = in_color;
}