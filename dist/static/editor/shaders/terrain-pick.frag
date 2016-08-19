#version 410 core

// In attributes
layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texCoords;

// Out attributes
layout (location = 0) out vec4 out_A;
layout (location = 1) out vec4 out_B;

void main()
{
  // Write output
  out_A = vec4(in_position.xyz, 1.0);
  out_B = vec4(in_texCoords, 1.0, 1.0);
} 
