#version 410 core

layout(location = 0) in vec3 in_vertexPos;
layout(location = 1) in vec2 in_uv;
layout (location = 4) in ivec4 in_boneids;
layout (location = 5) in vec4  in_boneweights;
layout(location = 0) out vec2 out_uv;

uniform mat4 uniform_mvpMatrix;

uniform mat4 uniform_bones[128];

void main()
{
  mat4 boneTransform = uniform_bones[in_boneids[0]] * in_boneweights[0];
  boneTransform += uniform_bones[in_boneids[1]] * in_boneweights[1];
  boneTransform += uniform_bones[in_boneids[2]] * in_boneweights[2];
  boneTransform += uniform_bones[in_boneids[3]] * in_boneweights[3];
  vec4 position = boneTransform * vec4(in_vertexPos, 1.0);

  out_uv = in_uv;
  gl_Position = uniform_mvpMatrix * position;
}
