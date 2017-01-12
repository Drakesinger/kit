#version 410 core

layout (location = 0) in vec2 in_texCoords;
layout (location = 1) in vec3 in_vertexPos;
layout (location = 0) out vec4 out_color;

uniform sampler2D uniform_textureDepth;//D

uniform vec2 uniform_projConst; //D

void main()
{
  float depth = texture(uniform_textureDepth, in_texCoords).r;//D
  
  // Depthextracted position
  vec3 viewRay = in_vertexPos;//D
  float linearDepth = uniform_projConst.x / (uniform_projConst.y - depth);//D
  vec3 position = viewRay * linearDepth;//D
  
  out_color = vec4(position, 1.0);
}
