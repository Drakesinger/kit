#version 410 core

layout(location = 0) in vec3 in_vertexPos;
uniform mat4 uniform_MVPMatrix;

void main()
{
  gl_Position = uniform_MVPMatrix * vec4(in_vertexPos, 1.0);
}