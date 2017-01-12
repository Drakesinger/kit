#version 410 core

layout(location = 0) in vec2 in_position;

layout(location = 0) out vec4 out_vertexPos;
layout(location = 1) out vec2 out_uv;

uniform mat4 uniform_mvpMatrix;
uniform mat4 uniform_mvMatrix;

void main()
{
  vec4 worldPosition = vec4(in_position.x, 0.0, in_position.y, 1.0);
  
  gl_Position = uniform_mvpMatrix * worldPosition;
  
  vec4 scaleVec = uniform_mvMatrix * vec4(1.0, 1.0, 1.0, 1.0);
  float scale = length(scaleVec.xyz / scaleVec.w);
  
  out_vertexPos = uniform_mvMatrix * worldPosition;
  out_uv = vec2(in_position.x, in_position.y) * scale;
}
