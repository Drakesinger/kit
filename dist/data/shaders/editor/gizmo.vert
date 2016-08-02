#version 410 core

layout (location = 0) in vec3 in_worldPosition;
layout (location = 2) in vec3 in_normal;
layout (location = 0) out vec3 out_normal;

uniform float uniform_scaleCoeff;
uniform mat4 uniform_mvpMatrix;
uniform mat4 uniform_normalMatrix;

void main()
{
  float w = (uniform_mvpMatrix * vec4(0.0, 0.0, 0.0, 1.0)).w;
  w *= uniform_scaleCoeff;
  gl_Position = uniform_mvpMatrix * vec4(in_worldPosition * w, 1.0);
  out_normal = normalize(uniform_normalMatrix * vec4(in_normal, 0.0)).xyz;
}