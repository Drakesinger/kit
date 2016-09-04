#version 410 core

layout (location = 0) in vec3 in_worldPosition;
layout (location = 1) in vec2 in_texCoords;

layout (location = 0) out vec3 out_worldPosition;
layout (location = 1) out vec2 out_texCoords;
    
uniform mat4 uniform_mvpMatrix;
uniform float uniform_scaleCoeff;
uniform bool uniform_doScale;

void main()
{
  if(uniform_doScale)
  {
    float w = (uniform_mvpMatrix * vec4(0.0, 0.0, 0.0, 1.0)).w;
    w *= uniform_scaleCoeff;
    gl_Position = uniform_mvpMatrix * vec4(in_worldPosition * w, 1.0);
  }
  else
  {
    gl_Position = uniform_mvpMatrix * vec4(in_worldPosition, 1.0);
  }
  out_worldPosition = in_worldPosition;
  out_texCoords = in_texCoords;
}