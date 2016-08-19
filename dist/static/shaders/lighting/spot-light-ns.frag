#version 410 core

layout (location = 0) in vec3 in_vertexPos;
layout (location = 0) out vec4 out_color;

vec3 decodeNormal (vec2 enc);
float calcAttenuation(vec3 in_lightPos, vec3 in_surfacePos, vec4 in_lightFalloff);
float calcSpotAttenuation(vec3 in_lightPos, vec3 in_surfacePos, vec2 in_lightConeAngle, vec3 in_lightDirection);
vec3 cookTorranceSpecular(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness, vec3 F0);

uniform sampler2D uniform_textureA;
uniform sampler2D uniform_textureB;
uniform sampler2D uniform_textureC;
uniform sampler2D uniform_textureDepth;//D

uniform vec3 uniform_lightPosition;
uniform vec3 uniform_lightColor;
uniform vec3 uniform_lightDirection;
uniform vec4 uniform_lightFalloff;
uniform vec2 uniform_coneAngle;

uniform vec2 uniform_projConst; //D

void main()
{
  vec2 texCoord = gl_FragCoord.xy / textureSize(uniform_textureA, 0);
  
  float depth = texture(uniform_textureDepth, texCoord).r;//D
  
  if(depth == 1.0)
  {
    discard;
  }
  
  vec4 texValueA = texture(uniform_textureA, texCoord).rgba;
  vec4 texValueB = texture(uniform_textureB, texCoord).rgba;
  vec4 texValueC = texture(uniform_textureC, texCoord).rgba;

  vec3 albedo = texValueA.xyz;
  float roughness = clamp(texValueA.a, 0.0001, 1.0);
  vec3 emissive = texValueB.rgb;
  float metalness = texValueB.a;
    vec3 normal = normalize(texValueC.xyz);
  float occlusion = texValueC.a;
  
  // Hardcoded position
  //vec3 position = texValueD.xyz;
  
  // Depthextracted position
  vec3 viewRay = vec3(in_vertexPos.xy / in_vertexPos.z, 1.0);//D
  float linearDepth = uniform_projConst.x / (uniform_projConst.y - depth);//D
  vec3 position = viewRay * linearDepth;//D

  vec3 viewDir = normalize( -position );
  vec3 lightDir = normalize(uniform_lightPosition - position);
  float satten =  calcSpotAttenuation(uniform_lightPosition, position, uniform_coneAngle, uniform_lightDirection);
  float atten =  calcAttenuation(uniform_lightPosition, position, uniform_lightFalloff);

  float diffuse = max(0.0, dot(normal, lightDir));
  vec3 lightDiffuse = (uniform_lightColor) * albedo * diffuse * (1.0 - metalness);

  float ior = 1.3;
  vec3 F0 = vec3(abs ((1.0 - ior) / (1.0 + ior)));
  F0 = F0 * F0;
  F0 = mix(F0, albedo, metalness);
  vec3 specular = cookTorranceSpecular(lightDir, viewDir, normal, roughness, F0);
  vec3 lightSpecular =  (uniform_lightColor) * specular;

  out_color = vec4(lightDiffuse + lightSpecular, 1.0) * (satten * atten);
}