#version 410 core

float beckmannDistribution(float x, float roughness)
{
  float NdotH = max(x, 0.0001);
  float cos2Alpha = NdotH * NdotH;
  float tan2Alpha = (cos2Alpha - 1.0) / cos2Alpha;
  float roughness2 = roughness * roughness;
  float denom = 3.141592653589793 * roughness2 * cos2Alpha * cos2Alpha;
  return exp(tan2Alpha / roughness2) / denom;
}

vec3 cookTorranceSpecular(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness, vec3 F0)
{
  //Half angle vector
  vec3 H = normalize(lightDirection + viewDirection);

  float VdotN = max(dot(viewDirection, surfaceNormal), 0.0001);
  float LdotN = max(dot(lightDirection, surfaceNormal), 0.0001);

  //Geometric term
  float NdotH = max(dot(surfaceNormal, H), 0.0001);
  float VdotH = max(dot(viewDirection, H), 0.0001);
  float LdotH = max(dot(lightDirection, H), 0.0001);
  float G1 = (2.0 * NdotH * VdotN) / VdotH;
  float G2 = (2.0 * NdotH * LdotN) / LdotH;
  float G = min(1.0, min(G1, G2));

  //Distribution term
  float D = beckmannDistribution(NdotH, roughness);

  //Fresnel term
  vec3 F = (F0 + (1.0 - F0)) * pow(1.0 - VdotN, 5.0);

  //Multiply terms and done
  return  F * G * D / max(3.14159265 * VdotN, 0.0001);
}
