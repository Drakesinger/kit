#version 410 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec3 in_normal;

layout (location = 4) in ivec4 in_boneids;
layout (location = 5) in vec4  in_boneweights;

layout (location = 0) out vec2 out_texcoord;
layout (location = 1) out float out_worldHeight;
layout (location = 2) out vec3 out_viewPosition;
layout (location = 3) out vec3 out_normal;

uniform mat4 uniform_mMatrix;
uniform mat4 uniform_mvMatrix;
uniform mat4 uniform_mvpMatrix;
uniform mat3 uniform_normalMatrix;

uniform mat4 uniform_bones[128];
uniform mat4 uniform_instanceTransform[128];

uniform int uniform_isSkinned;
uniform int uniform_isInstanced;

void main()
{
  vec4 position = vec4(in_position, 1.0);
  vec4 normal = normalize(vec4(in_normal, 0.0));
  
  if(uniform_isSkinned == 1)
  {
    mat4 boneTransform = uniform_bones[in_boneids[0]] * in_boneweights[0];
    boneTransform += uniform_bones[in_boneids[1]] * in_boneweights[1];
    boneTransform += uniform_bones[in_boneids[2]] * in_boneweights[2];
    boneTransform += uniform_bones[in_boneids[3]] * in_boneweights[3];

    position = boneTransform * position;
    normal = boneTransform * normal;
  }
  
  if(uniform_isInstanced == 1)
  {
    gl_Position = uniform_mvpMatrix * uniform_instanceTransform[gl_InstanceID] * position;
    out_viewPosition = vec4(uniform_mvMatrix * uniform_instanceTransform[gl_InstanceID] * position).xyz;
    vec4 mPosition = uniform_mMatrix * uniform_instanceTransform[gl_InstanceID] * position;
    out_worldHeight = vec3(mPosition.xyz / mPosition.w).y;
    out_normal = uniform_normalMatrix * mat3(uniform_instanceTransform[gl_InstanceID]) * normal.xyz;
  }
  else
  {
    gl_Position = uniform_mvpMatrix * position;
    out_viewPosition = vec4(uniform_mvMatrix * position).xyz;
    vec4 mPosition = uniform_mMatrix * position;
    out_worldHeight = vec3(mPosition.xyz / mPosition.w).y;
    out_normal = uniform_normalMatrix * normal.xyz;
  }
  
  out_texcoord = in_texcoord;
}
