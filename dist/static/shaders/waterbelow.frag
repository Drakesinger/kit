#version 410 core

layout(location = 0) in vec4 in_vertexPos;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

uniform sampler2D uniform_depthmap;
uniform sampler2D uniform_positionmap;
uniform sampler2D uniform_colormap;
uniform mat4 uniform_invViewMatrix;
uniform mat4 uniform_viewMatrix;
uniform vec3 uniform_camerawp;

// Normal and height
uniform sampler2D uniform_normalmapA;
uniform sampler2D uniform_normalmapB;

//uniform sampler2D uniform_heightmapA;
//uniform sampler2D uniform_heightmapB;

uniform float uniform_time = 0.0;

// Layer one - Clear surface
uniform vec3 uniform_surfaceColor = vec3(0.67, 0.8, 0.9);
uniform float uniform_surfaceDepth = 128.0;
//uniform vec3 uniform_surfaceColor = vec3(1.0, 1.0, 1.0);


uniform float uniform_refractionStrength = 20.0;

void main()
{
  float waveScaleA = 128.0;
  float waveScaleB = 128.0;
  
  float waveSpeedA = 1.0 / 1000.0 / waveScaleA;
  float waveSpeedB = 4.0 / 1000.0 / waveScaleB;
  
  vec2 waveDirA  = normalize(vec2(1.0, 1.0));
  vec2 waveDirB  = normalize(vec2(0.25, -1.0));

  float waveTimeA = waveSpeedA * uniform_time;
  float waveTimeB = waveSpeedB * uniform_time;
  
  vec2 waveUvA = (in_uv / waveScaleA) + (waveDirA * waveTimeA);
  vec2 waveUvB = (in_uv / waveScaleB) + (waveDirB * waveTimeB);
  
  
  //float heightA = texture(uniform_heightmapA, waveUvA).r;
  //float heightB = texture(uniform_heightmapB, waveUvB).r;
  
  //float height = mix(heightA, heightB, 0.5);
 
 // World-space normal
  vec3 normalA = ((texture(uniform_normalmapA, waveUvA).rbg * 2.0) - 1.0);
  vec3 normalB = ((texture(uniform_normalmapB, waveUvB).rbg * 2.0) - 1.0);
  //vec3 worldNormal = normalize(mix(normalA, normalB, sin(uniform_time * 0.001)));
  vec3 worldNormal =normalA + normalB;
  worldNormal.xz *= 2.0;
  worldNormal = normalize(worldNormal);
 
 /*
  mat3 normalRotation = mat3(mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(0.0, -1.0, 0.0)));
  vec3 wNormal =  normalRotation * normalTex;
  vec3 vNormal = normalize(vec3(uniform_viewMatrix * vec4(wNormal, 0.0)));
  
  vec3 vViewDir = normalize( -oldPosition );
  vec3 wViewDir = normalize(vec3(uniform_invViewMatrix * vec4(vViewDir, 0.0)));  
 */
 
  vec2 texStep = vec2(1.0) / textureSize(uniform_depthmap, 0);
  vec2 texCoord = gl_FragCoord.xy * texStep;
  vec2 refCoord = texCoord + texStep * worldNormal.xz * uniform_refractionStrength;
  
  //vec3 oldVP = texture(uniform_positionmap, texCoord).rgb;
  //vec3 oldWP = vec3(uniform_invViewMatrix * vec4(oldVP, 1.0));
  //float oldHeight = oldWP.y;
   
  vec3 refOldVP = texture(uniform_positionmap, refCoord).rgb;
  vec3 refOldWP = vec3(uniform_invViewMatrix * vec4(refOldVP, 1.0));
  float refOldHeight = refOldWP.y;
  
  vec3 waterWP = vec3(uniform_invViewMatrix * in_vertexPos);
  float waterHeight = waterWP.y;
  
  //float dist = length(in_vertexPos.xyz / in_vertexPos.w);
  
  float cameraHeight = uniform_camerawp.y;
  
  //float oldDepth = distance(waterHeight, oldHeight);
  //float refOldDepth = distance(waterHeight, refOldHeight);
  
  //float cameraDepth = distance(cameraHeight, waterHeight);

  vec2 waterXz = waterWP.xz;
  vec2 cameraXz = uniform_camerawp.xz;
  float planarDistance = distance(waterXz, cameraXz);
  
  vec3 prevColor = texture(uniform_colormap, texCoord).rgb;
  
  if((cameraHeight > waterHeight && refOldHeight < waterHeight)
    || (cameraHeight < waterHeight && refOldHeight > waterHeight))
  {
    prevColor = texture(uniform_colormap, refCoord).rgb;
  }
  
  vec3 surfaceColor = prevColor * uniform_surfaceColor;
    
  out_color = vec4(surfaceColor, 1.0);
}

  
