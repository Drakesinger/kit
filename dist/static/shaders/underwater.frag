#version 410 core

layout (location = 0) in vec2 in_texCoords;
layout (location = 0) out vec4 out_color;

uniform sampler2D uniform_positionmap;//D
uniform sampler2D uniform_colormap;//D

uniform mat4 uniform_invViewMatrix;

uniform float uniform_refractionStrength = 10.0;
const float const_wavespeed = 1.0 / 1000.0;
uniform float uniform_time = 0.0;

uniform sampler2D uniform_normalmapA;
uniform sampler2D uniform_normalmapB;
uniform sampler2D uniform_vignette;

// Layer one - Clear surface
uniform vec3 uniform_surfaceColor = vec3(0.67, 0.8, 0.9);
uniform float uniform_surfaceDepth = 1.0;
//uniform vec3 uniform_surfaceColor = vec3(1.0, 1.0, 1.0);

// Layer two - Murk
uniform vec3 uniform_murkColor = vec3(0.2, 0.6, 0.7);
//uniform vec3 uniform_murkColor = vec3(0.67, 0.8, 0.9);
//uniform vec3 uniform_murkColor = vec3(1.0, 0.0, 0.0);
uniform float uniform_murkDepth = 64.0;
uniform float uniform_murkLength = 10.0;
uniform float uniform_murkClarity = 0.5;

// Layer three - Deep
uniform vec3 uniform_deepColor = vec3(0.1, 0.45, 0.4) * 0.5;
//uniform vec3 uniform_deepColor = vec3(0.0, 1.0, 0.0);
uniform float uniform_deepDepth = 128.0;
uniform float uniform_deepClarity = 0.0;

uniform float uniform_waterHeight = 0.0;



void main()
{
  // Calculate some wave uv shit
  float waveTimeA = const_wavespeed * 2.0 * uniform_time;
  float waveTimeB = const_wavespeed * 1.0 * uniform_time;
  
  vec2 waveUvA = (in_texCoords * 6.0)  + (vec2(0.0, 0.25) * waveTimeA);
  vec2 waveUvB = (in_texCoords * 12.0) + (vec2(0.0, 0.42) * waveTimeB);

  vec3 normalTex = ((texture(uniform_normalmapA, waveUvA / 10.0).rgb * 2.0) - 1.0);
  normalTex += ((texture(uniform_normalmapB, waveUvB / 10.0).rgb * 2.0) - 1.0);
  normalTex = normalize(normalTex);

  vec2 texStep = vec2(1.0) / textureSize(uniform_colormap, 0);
  
  vec2 refCoord = in_texCoords + (texStep * normalTex.xy * uniform_refractionStrength);
  //vec2 refCoord = in_texCoords;
  
  vec4 vignette = texture(uniform_vignette, in_texCoords).rgba;

  vec3 position = texture(uniform_positionmap, refCoord).rgb;//D
  vec3 prevColor = texture(uniform_colormap, refCoord).rgb;//D
  
  vec3 fragWp = vec3(uniform_invViewMatrix * vec4(position, 1.0));
  
  float dist = length(position);
  float height = distance(uniform_waterHeight, fragWp.y);
  float dh = (dist + height);
  
  float surfaceCoeff = clamp(height / uniform_surfaceDepth, 0.0, 1.0);
  float murkCoeff = clamp(dist / uniform_murkDepth , 0.0, 1.0);
  float deepCoeff = clamp( (dist - uniform_murkDepth - uniform_murkLength) / (uniform_deepDepth - uniform_murkDepth - uniform_murkLength), 0.0, 1.0);

  vec3 surfaceColor = prevColor * uniform_surfaceColor;
  surfaceColor = mix(prevColor, surfaceColor, surfaceCoeff);
  
  // Layer two
  murkCoeff = clamp(pow(murkCoeff, 1.0 / 2.2), 0.0, 1.0);
  vec3 murkColor = prevColor * uniform_murkColor;
  vec3 murk = mix(uniform_murkColor, murkColor, uniform_murkClarity);
  vec3 result = mix(surfaceColor, murk, murkCoeff);
  
  // Layer three
  result = mix(result, uniform_deepColor, deepCoeff);
  
  // Add vignette
  result = mix(result, vignette.rgb, vignette.a);
  
  out_color = vec4(result, 1.0);
}
