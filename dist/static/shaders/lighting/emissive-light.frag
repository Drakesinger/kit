#version 410 core

layout (location = 0) in vec2 in_texCoords;
layout (location = 0) out vec4 out_color;

uniform sampler2D uniform_textureB;

void main()
{
  vec4 texValueB = texture(uniform_textureB, in_texCoords).rgba; //EM
  vec3 emissive = texValueB.rgb;
  
  out_color = vec4(emissive , 1.0);
}
