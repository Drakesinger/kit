#version 410 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 0) out vec2 out_uv;
layout (location = 1) out vec3 out_viewRay; //D

uniform vec2 uniform_position;
uniform vec2 uniform_size;

uniform mat4 uniform_invProjMatrix;

void main()
{
  vec2 finalpos = uniform_position + (in_position.xy * uniform_size);
  gl_Position = vec4(finalpos, in_position.z, 1.0);
  gl_Position.xy *= 2.0;
  gl_Position.y = 1.0 - gl_Position.y;
  gl_Position.x -= 1.0;
  out_uv = in_uv;
  
  vec3 viewPosition = (uniform_invProjMatrix * vec4(gl_Position.xy, 1.0, 1.0)).xyz;//D
  out_viewRay = vec3(viewPosition.xy / viewPosition.z, 1.0);//D
}
