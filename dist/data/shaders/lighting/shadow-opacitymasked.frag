#version 410 core
layout(location = 0) in vec2 in_uv;
uniform sampler2D uniform_opacityMask;

void main()
{
  if(texture(uniform_opacityMask, in_uv).r < 0.5)
  {
    discard;
  }
  gl_FragDepth = gl_FragCoord.z;
}
