#version 410 core

layout(location = 0) in vec2 in_texCoords;

out vec4 out_color;

uniform sampler2D uniform_texA;
uniform sampler2D uniform_texB;
uniform sampler2D uniform_texC;
uniform sampler2D uniform_texD;

uniform int uniform_useA;
uniform int uniform_useB;
uniform int uniform_useC;
uniform int uniform_useD;

void main()
{
  out_color = vec4(1.0, 1.0, 1.0, 1.0);
  
  if(uniform_useA == 1)
  {
    out_color.r = texture(uniform_texA, in_texCoords).r;
  }
  if(uniform_useB == 1)
  {
    out_color.g = texture(uniform_texB, in_texCoords).r;
  }
  
  if(uniform_useC == 1)
  {
    out_color.b = texture(uniform_texC, in_texCoords).r;
  }
  
  if(uniform_useD == 1)
  {
    out_color.a = texture(uniform_texD, in_texCoords).r;
  }
}
