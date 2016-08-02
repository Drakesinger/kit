#version 410 core

layout(location = 0) in vec3 in_vertexPos;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec2 out_uv;

uniform mat4 uniform_mvpMatrix;

void main()
{
  vec4 position = vec4(in_vertexPos, 1.0);
  out_uv = in_uv;
  gl_Position = uniform_mvpMatrix * position;
}