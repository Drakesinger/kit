#version 410 core

layout (location = 0) in vec3 in_vertexPos;
layout (location = 0) out vec3 out_vertexPos; //D

uniform mat4 uniform_MVPMatrix;
uniform mat4 uniform_MVMatrix; //D

void main()
{
  gl_Position = uniform_MVPMatrix * vec4(in_vertexPos, 1.0);
  out_vertexPos = (uniform_MVMatrix * vec4(in_vertexPos, 1.0)).xyz; //D
}