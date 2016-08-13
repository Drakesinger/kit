#version 410 core

layout (location = 0) in vec2 in_texCoords;
layout (location = 1) in vec3 in_vertexPos; //D
layout (location = 0) out vec4 out_color;

uniform sampler2D uniform_textureA;
uniform sampler2D uniform_textureB;
uniform sampler2D uniform_textureC;

uniform sampler2D uniform_textureDepth;//D
uniform vec2 uniform_projConst; //D

uniform vec3 uniform_lightColor;
uniform samplerCube uniform_lightIrradiance;
uniform samplerCube uniform_lightRadiance;

uniform sampler2D uniform_brdf;
uniform mat4 uniform_invViewMatrix;
uniform float uniform_quality = 1.0;

vec3 decodeNormal (vec2 enc);

void main()
{
  float depth = texture(uniform_textureDepth, in_texCoords).r;//D
  
  if(depth == 1.0)
  {
    discard;
  }
  
  vec4 texValueA = texture(uniform_textureA, in_texCoords).rgba; //AR
  vec4 texValueB = texture(uniform_textureB, in_texCoords).rgba; //EM
  vec4 texValueC = texture(uniform_textureC, in_texCoords).rgba; //NO

  vec3 albedo = texValueA.xyz;
  float roughness = clamp(texValueA.a, 0.0001, 1.0);
  vec3 emissive = texValueB.rgb;
  float metalness = texValueB.a;
  vec3 normal = normalize(texValueC.xyz);
  float occlusion = texValueC.a;
  
  // Hardcoded position
  //vec3 position = texValueD.xyz;

  // Depthextracted position
  vec3 viewRay = in_vertexPos;//D
  float linearDepth = uniform_projConst.x / (uniform_projConst.y - depth);//D
  vec3 position = viewRay * linearDepth;//D
  
  vec3 viewDir = normalize( - position );

  vec4 rotatedNormal = uniform_invViewMatrix * vec4(normal.xyz, 0.0);
  vec4 rotatedViewDir = uniform_invViewMatrix * vec4(viewDir, 0.0);
  vec3 refVec = normalize(reflect(-rotatedViewDir.xyz, rotatedNormal.xyz));
  float ndotv =  clamp(dot(rotatedNormal.xyz, rotatedViewDir.xyz), 0.0, 1.0);
  float ior = 1.333; // Water
  vec3 F0 = vec3(abs ((1.0 - ior) / (1.0 + ior)));
  F0 = F0 * F0;
  F0 = mix(F0, albedo.rgb, metalness) ;

  vec3 diffuse = texture(uniform_lightIrradiance, refVec).xyz;
  vec3 lightDiffuse = (uniform_lightColor) * albedo  * diffuse * (1.0 - metalness) * occlusion;

  vec3 radiance = textureLod(uniform_lightRadiance, refVec, (9.0 - (9.0 / uniform_quality)) + ((roughness*9.0) / uniform_quality)).xyz;
  vec2 brdf = texture(uniform_brdf, vec2(roughness, ndotv)).xy;  
  vec3 lightSpecular = (radiance * brdf.x + brdf.y) * uniform_lightColor * F0 * occlusion;

  out_color = vec4((lightSpecular + lightDiffuse), 1.0);
}
