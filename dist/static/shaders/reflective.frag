#version 410 core

layout (location = 0) in vec2 in_texCoords;
layout (location = 1) in float in_height;
layout (location = 2) in vec3 in_viewPosition;
layout (location = 3) in vec3 in_normal;

layout(location = 0) out vec4 out_color;

uniform sampler2D uniform_arCache;
uniform sampler2D uniform_eoCache;
uniform sampler2D uniform_nmCache;
uniform float uniform_emissiveStrength;

uniform mat4 uniform_invViewMatrix;


/*
 IBL light
*/

uniform vec3 uniform_lightColor;
uniform samplerCube uniform_lightIrradiance;
uniform samplerCube uniform_lightRadiance;
uniform sampler2D uniform_brdf;

void main()
{
  if(in_height < 0.0) discard;

  vec4 arData = texture(uniform_arCache, in_texCoords).rgba;
  vec4 eoData = texture(uniform_eoCache, in_texCoords).rgba;
  vec4 nmData = texture(uniform_nmCache, in_texCoords).rgba;
  
  vec3 albedo = arData.xyz;
  float roughness = clamp(arData.a, 0.0001, 1.0);
  vec3 emissive = eoData.rgb;
  float occlusion = eoData.a;
  float metalness = nmData.a;

  
  vec3 viewDir = normalize( - in_viewPosition );

  vec4 rotatedNormal = uniform_invViewMatrix * vec4(normalize(in_normal.xyz), 0.0);
  vec4 rotatedViewDir = uniform_invViewMatrix * vec4(viewDir, 0.0);
  vec3 refVec = normalize(reflect(-rotatedViewDir.xyz, rotatedNormal.xyz));
  float ndotv =  clamp(dot(rotatedNormal.xyz, rotatedViewDir.xyz), 0.0, 1.0);  
  
  
  float ior = 1.333; // Water
  vec3 F0 = vec3(abs ((1.0 - ior) / (1.0 + ior)));
  F0 = F0 * F0;
  F0 = mix(F0, albedo.rgb, metalness) ;

  vec3 diffuse = texture(uniform_lightIrradiance, refVec).xyz;
  vec3 lightDiffuse = (uniform_lightColor) * albedo  * diffuse * (1.0 - metalness) * occlusion;

  vec3 radiance = textureLod(uniform_lightRadiance, refVec, roughness*5.0).xyz;
  vec2 brdf = texture(uniform_brdf, vec2(roughness, ndotv)).xy;  
  vec3 lightSpecular = (radiance * brdf.x + brdf.y) * uniform_lightColor * F0 * occlusion;

  //out_color = vec4((lightSpecular + lightDiffuse), 1.0);
  out_color.w = 1.0;
  //out_color.xyz = in_normal;
  out_color.xyz = lightSpecular + lightDiffuse;
  out_color.xyz += emissive * uniform_emissiveStrength;
}
