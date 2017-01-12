#version 410 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 0) out vec2 out_uv;

uniform vec2 uniform_position;
uniform vec2 uniform_size;

void main()
{
  vec2 finalpos = uniform_position + (in_position.xy * uniform_size);
  gl_Position = vec4(finalpos, in_position.z, 1.0);
  gl_Position.xy *= 2.0;
  gl_Position.y = 1.0 - gl_Position.y;
  gl_Position.x -= 1.0;
  out_uv = in_uv;
}
