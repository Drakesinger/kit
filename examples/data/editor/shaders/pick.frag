#version 410 core

layout (location = 0) in vec3 in_worldPosition;
layout (location = 1) in vec2 in_texCoords;

layout (location = 0) out uvec4 out_identifier;
layout (location = 1) out vec4 out_extra;

uniform uint uniform_targetId = 0;

uniform uint uniform_lightId = 0;
uniform uint uniform_modelId = 0;
uniform uint uniform_terrainId = 0;

uniform uint uniform_submeshId = 0;

uniform uint uniform_transformableType = 0;
uniform uint uniform_transformableId = 0;

void main()
{
  // zero initialize
  out_identifier = uvec4(0, 0, 0, 0);
  out_extra = vec4(0.0, 0.0, 0.0, 0.0);
  
  out_identifier.r = uniform_targetId;
  
  if(uniform_targetId > 0 && uniform_targetId < 10)
  {
    out_identifier.g = uniform_transformableType;
    out_identifier.b = uniform_transformableId;
    out_extra = vec4(1.0, 0.0, 0.0, 1.0);
  }
  
  // Model
  if(uniform_targetId == 12)
  {
    out_identifier.g = uniform_modelId;
    out_identifier.b = uniform_submeshId;
    out_extra.z = in_texCoords.x;
    out_extra.w = in_texCoords.y;
  }
  
  // Terrain
  if(uniform_targetId == 13)
  {
    out_identifier.g = uniform_terrainId;
    out_extra.y = in_worldPosition.z;
    out_extra.z = in_texCoords.x;
    out_extra.w = in_texCoords.y;
  }
  
  // Light handle
  if(uniform_targetId == 14)
  {
    out_identifier.g = uniform_lightId;
  }
  
}