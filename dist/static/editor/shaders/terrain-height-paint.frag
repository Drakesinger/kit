#version 410 core

layout(location = 0) in vec2 in_texCoords;

out float out_color;

uniform sampler2D uniform_heightmap;
uniform sampler2D uniform_brush;
uniform vec2 uniform_brushSize;
uniform vec2 uniform_brushPos;
uniform int uniform_opMode; // Add, subtract, set, smooth
uniform float uniform_yScale;
uniform float uniform_strength;

const int opAdd = 0;
const int opSubtract = 1;
const int opSet = 2;
const int opSmooth = 3;

float sampleSmoothHeight()
{
  vec2 uvStep = 1.0 / textureSize(uniform_heightmap, 0);
  
  vec2 uv   = in_texCoords;
  vec2 uvN  = in_texCoords + (vec2( 0.0, -1.0) * uvStep);
  vec2 uvS  = in_texCoords + (vec2( 0.0,  1.0) * uvStep);
  vec2 uvW  = in_texCoords + (vec2(-1.0,  0.0) * uvStep);
  vec2 uvE  = in_texCoords + (vec2( 1.0,  0.0) * uvStep);
  vec2 uvNW = in_texCoords + (vec2(-1.0, -1.0) * uvStep);
  vec2 uvNE = in_texCoords + (vec2( 1.0, -1.0) * uvStep);
  vec2 uvSW = in_texCoords + (vec2(-1.0,  1.0) * uvStep);
  vec2 uvSE = in_texCoords + (vec2( 1.0,  1.0) * uvStep);
  
  float h  = texture(uniform_heightmap,   uv).r;
        h += texture(uniform_heightmap,  uvN).r;
        h += texture(uniform_heightmap,  uvS).r;
        h += texture(uniform_heightmap,  uvW).r;
        h += texture(uniform_heightmap,  uvE).r;
        h += texture(uniform_heightmap, uvNW).r;
        h += texture(uniform_heightmap, uvNE).r;
        h += texture(uniform_heightmap, uvSW).r;
        h += texture(uniform_heightmap, uvSE).r;

  h /= 9.0;

  return h;
}

void main()
{
  vec2 fullUv = in_texCoords;
  float currHeight = texture(uniform_heightmap, fullUv).r;

  // Only act if brush is overlapping this pixel
  if(  fullUv.x > uniform_brushPos.x && fullUv.x < uniform_brushPos.x + uniform_brushSize.x
    && fullUv.y > uniform_brushPos.y && fullUv.y < uniform_brushPos.y + uniform_brushSize.y)
  {
    vec2 brushUv;
    brushUv = (fullUv / uniform_brushSize) - (uniform_brushPos / uniform_brushSize);
    float currBrush = (1.0 - texture(uniform_brush, brushUv ).r);
    
    switch(uniform_opMode)
    {
      case opAdd:
        out_color = currHeight + ((currBrush * uniform_strength) / uniform_yScale); 
        break;
      case opSet:
        out_color = mix(currHeight, uniform_strength, currBrush); 
        break;
      case opSubtract:
        out_color = currHeight - ((currBrush * uniform_strength) / uniform_yScale); 
        break;
      case opSmooth:
        out_color = mix(currHeight, sampleSmoothHeight(), currBrush);
        break;
      default:
         out_color = currHeight;
         break;
    }
  }
  else
  {
   out_color = currHeight; 
  }
  
  // Clamp the output since we're working with a R16F buffer
  out_color = clamp(out_color, -1.0, 1.0);
}
