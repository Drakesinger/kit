#version 410 core

layout(location = 0) in vec2 in_texCoords;

out vec4 out_mask0;
out vec4 out_mask1;

uniform sampler2D uniform_materialMask0;
uniform sampler2D uniform_materialMask1;
uniform sampler2D uniform_brush;
uniform vec2 uniform_brushSize;
uniform vec2 uniform_brushPos;
uniform int uniform_currLayer;
uniform int uniform_opMode; // Add, subtract, set, smooth
uniform float uniform_strength;

const int opAdd = 0;
const int opSubtract = 1;
const int opSet = 2;
const int opSmooth = 3;

vec4 sampleSmooth0(vec4 mid)
{
  vec2 uvStep = 1.0 / textureSize(uniform_materialMask0, 0);

  vec2 uv   = in_texCoords;
  vec2 uvN  = in_texCoords + (vec2( 0.0, -1.0) * uvStep);
  vec2 uvS  = in_texCoords + (vec2( 0.0,  1.0) * uvStep);
  vec2 uvW  = in_texCoords + (vec2(-1.0,  0.0) * uvStep);
  vec2 uvE  = in_texCoords + (vec2( 1.0,  0.0) * uvStep);
  vec2 uvNW = in_texCoords + (vec2(-1.0, -1.0) * uvStep);
  vec2 uvNE = in_texCoords + (vec2( 1.0, -1.0) * uvStep);
  vec2 uvSW = in_texCoords + (vec2(-1.0,  1.0) * uvStep);
  vec2 uvSE = in_texCoords + (vec2( 1.0,  1.0) * uvStep);
  
  vec4  h  = mid;
        h += texture(uniform_materialMask0,  uvN);
        h += texture(uniform_materialMask0,  uvS);
        h += texture(uniform_materialMask0,  uvW);
        h += texture(uniform_materialMask0,  uvE);
        h += texture(uniform_materialMask0, uvNW);
        h += texture(uniform_materialMask0, uvNE);
        h += texture(uniform_materialMask0, uvSW);
        h += texture(uniform_materialMask0, uvSE);

  h /= 9.0;

  return h;
}

vec4 sampleSmooth1(vec4 mid)
{
  vec2 uvStep = 1.0 / textureSize(uniform_materialMask1, 0);

  vec2 uv   = in_texCoords;
  vec2 uvN  = in_texCoords + (vec2( 0.0, -1.0) * uvStep);
  vec2 uvS  = in_texCoords + (vec2( 0.0,  1.0) * uvStep);
  vec2 uvW  = in_texCoords + (vec2(-1.0,  0.0) * uvStep);
  vec2 uvE  = in_texCoords + (vec2( 1.0,  0.0) * uvStep);
  vec2 uvNW = in_texCoords + (vec2(-1.0, -1.0) * uvStep);
  vec2 uvNE = in_texCoords + (vec2( 1.0, -1.0) * uvStep);
  vec2 uvSW = in_texCoords + (vec2(-1.0,  1.0) * uvStep);
  vec2 uvSE = in_texCoords + (vec2( 1.0,  1.0) * uvStep);
  
  vec4  h  = mid;
        h += texture(uniform_materialMask1,  uvN);
        h += texture(uniform_materialMask1,  uvS);
        h += texture(uniform_materialMask1,  uvW);
        h += texture(uniform_materialMask1,  uvE);
        h += texture(uniform_materialMask1, uvNW);
        h += texture(uniform_materialMask1, uvNE);
        h += texture(uniform_materialMask1, uvSW);
        h += texture(uniform_materialMask1, uvSE);

  h /= 9.0;

  return h;
}


void main()
{
  vec2 fullUv = in_texCoords;
  
  vec4 currMask0 = texture(uniform_materialMask0, fullUv);
  vec4 currMask1 = texture(uniform_materialMask1, fullUv);
  
  int cl = uniform_currLayer;
  int mask = 0;
  vec4 currMask = currMask0;
  
  if(cl > 3)
  {
    cl -= 4;
    mask = 1;
    currMask = currMask1;
  }
  
  vec4 layerFactor = vec4(0.0);
  if(cl == 0) layerFactor.r = 1.0;
  if(cl == 1) layerFactor.g = 1.0;
  if(cl == 2) layerFactor.b = 1.0;
  if(cl == 3) layerFactor.a = 1.0;

  vec4 out_color = vec4(0.0, 0.0, 0.0, 0.0);
  
  if(  fullUv.x > uniform_brushPos.x && fullUv.x < uniform_brushPos.x + uniform_brushSize.x
    && fullUv.y > uniform_brushPos.y && fullUv.y < uniform_brushPos.y + uniform_brushSize.y)
  {
    vec2 brushUv;
    brushUv = (fullUv / uniform_brushSize) - (uniform_brushPos / uniform_brushSize);
    float currBrush = (1.0 - texture(uniform_brush, brushUv ).r);
    
    switch(uniform_opMode)
    {
      case opAdd:
        out_color = currMask + (layerFactor * vec4(currBrush * uniform_strength)); 
        break;
      case opSubtract:
        out_color = currMask - (layerFactor * vec4(currBrush * uniform_strength)); 
        break;
      case opSet:
        out_color = mix(currMask, vec4(uniform_strength),layerFactor * vec4(currBrush)); 
        break;
      case opSmooth:
        vec4 smoothedMask = (mask == 0 ? sampleSmooth0(currMask) : sampleSmooth1(currMask));
        out_color = mix(currMask, smoothedMask, (layerFactor * vec4(currBrush * uniform_strength)));
        break; 
    }
  }
  else
  {
    out_color = currMask; 
  }
  
  if(mask == 0)
  {
    out_mask0 = out_color;
    out_mask1 = currMask1;
  }
  else 
  {
    out_mask1 = out_color;
    out_mask0 = currMask0;
  }
  
}
