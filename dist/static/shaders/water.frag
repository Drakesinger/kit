#version 410 core

layout(location = 0) in vec4 in_vertexPos;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

uniform sampler2D uniform_depthmap;
uniform sampler2D uniform_positionmap;
uniform sampler2D uniform_colormap;
uniform mat4 uniform_invViewMatrix;
uniform mat4 uniform_viewMatrix;


// Normal and height
uniform sampler2D uniform_normalmapA;
uniform sampler2D uniform_normalmapB;

uniform sampler2D uniform_heightmapA;
uniform sampler2D uniform_heightmapB;
uniform sampler2D uniform_reflection;

uniform float uniform_time = 0.0;

// Layer one - Clear surface
uniform vec3 uniform_surfaceColor = vec3(0.67, 0.8, 0.9)*0.5;
uniform float uniform_surfaceDepth = 1.0;
//uniform vec3 uniform_surfaceColor = vec3(1.0, 1.0, 1.0);

// Layer two - Murk
uniform vec3 uniform_murkColor = vec3(0.2, 0.6, 0.7)*0.5;
//uniform vec3 uniform_murkColor = vec3(0.67, 0.8, 0.9);
//uniform vec3 uniform_murkColor = vec3(1.0, 0.0, 0.0);
uniform float uniform_murkDepth = 10.0;
uniform float uniform_murkLength = -10.0;
uniform float uniform_murkClarity = 1.0;

// Layer three - Deep
uniform vec3 uniform_deepColor = vec3(0.1, 0.45, 0.4) * 0.25;
//uniform vec3 uniform_deepColor = vec3(0.0, 1.0, 0.0);
uniform float uniform_deepDepth = 50.0;
uniform float uniform_deepClarity = 0.0;

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
  
  float heightA = texture(uniform_heightmapA, waveUvA).r;
  float heightB = texture(uniform_heightmapB, waveUvB).r;
  
  float height = mix(heightA, heightB, 0.5);
 
 // World-space normal
  vec3 normalA = ((texture(uniform_normalmapA, waveUvA).rbg * 2.0) - 1.0);
  vec3 normalB = ((texture(uniform_normalmapB, waveUvB).rbg * 2.0) - 1.0);
  //vec3 worldNormal = normalize(mix(normalA, normalB, sin(uniform_time * 0.001)));
  vec3 worldNormal =normalA + normalB;
  worldNormal.xz *= 2.0;
  worldNormal = normalize(worldNormal);
  //vec3 worldNormal = normalize(normalB);

 
  vec2 texStep = vec2(1.0) / textureSize(uniform_depthmap, 0);
  vec2 texCoord = gl_FragCoord.xy * texStep;
  vec2 refCoord = texCoord + texStep * worldNormal.xz * uniform_refractionStrength;
  
  vec3 oldVP = texture(uniform_positionmap, texCoord).rgb;
  vec3 oldWP = vec3(uniform_invViewMatrix * vec4(oldVP, 1.0));
  float oldHeight = oldWP.y;
   
  vec3 refOldVP = texture(uniform_positionmap, refCoord).rgb;
  vec3 refOldWP = vec3(uniform_invViewMatrix * vec4(refOldVP, 1.0));
  float refOldHeight = refOldWP.y;
  
  vec3 waterWP = vec3(uniform_invViewMatrix * in_vertexPos);
  float waterHeight = waterWP.y;
  
  vec3 cameraWp = (uniform_invViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  float cameraHeight = cameraWp.y;
  
  float oldDepth = distance(waterHeight, oldHeight);
  float refOldDepth = distance(waterHeight, refOldHeight);

 
  vec3 prevColor = texture(uniform_colormap, texCoord).rgb;
  vec3 reflection = texture(uniform_reflection, refCoord).rgb;
  
  float surfaceCoeff = clamp(oldDepth / uniform_surfaceDepth, 0.0, 1.0);
  float murkCoeff = clamp(oldDepth / uniform_murkDepth, 0.0, 1.0);
  float deepCoeff = clamp( ((oldDepth - uniform_murkDepth - uniform_murkLength) / (uniform_deepDepth - uniform_murkDepth - uniform_murkLength)), 0.0, 1.0);
  
  if((cameraHeight > waterHeight && refOldHeight < waterHeight)
    || (cameraHeight < waterHeight && refOldHeight > waterHeight))
  {
    prevColor = texture(uniform_colormap, refCoord).rgb;
    surfaceCoeff = clamp(refOldDepth / uniform_surfaceDepth, 0.0, 1.0);
    murkCoeff = clamp(refOldDepth / uniform_murkDepth, 0.0, 1.0);
    deepCoeff = clamp( ((refOldDepth - uniform_murkDepth - uniform_murkLength) / (uniform_deepDepth - uniform_murkDepth - uniform_murkLength)), 0.0, 1.0);
  }
  // Layer one
  vec3 surfaceColor = prevColor * uniform_surfaceColor;
  surfaceColor = mix(prevColor, surfaceColor, surfaceCoeff );
  vec3 result = surfaceColor;
  
  // Layer two
  murkCoeff = clamp(pow(murkCoeff, 1.0 / 2.2), 0.0, 1.0);
  vec3 murkColor = prevColor * uniform_murkColor;
  vec3 murk = mix(uniform_murkColor, murkColor, uniform_murkClarity);
  result = mix(result, murk, murkCoeff);
  
  // Layer three
  deepCoeff = clamp(pow(deepCoeff, 1.0 / 2.2), 0.0, 1.0);
  //vec3 deepColor = mix(uniform_deepColor*0.5, uniform_deepColor, height);
  result = mix(result, uniform_deepColor, clamp(deepCoeff * (1.0 - uniform_deepClarity), 0.0, 1.0) );
  
  // Reflection
  
  vec3 viewNormal = vec3(uniform_viewMatrix * vec4(worldNormal, 0.0));
  vec3 viewDir = -normalize(vec3(in_vertexPos));
  float refCoeff = 1.0 - clamp(dot(viewDir, viewNormal), 0.0, 1.0);
  result += reflection * (refCoeff * 0.2);
  
  //out_color = vec4(worldNormal / 2.0 + 0.5, 1.0);
  out_color = vec4(result, 1.0);
}

  
