#version 410 core

layout (location = 0) in vec2 in_texCoords;
layout (location = 1) in vec3 in_vertexPos;
layout (location = 0) out vec4 out_color;

vec3 decodeNormal (vec2 enc);
vec3 cookTorranceSpecular(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness, vec3 F0);

uniform sampler2D uniform_textureA;
uniform sampler2D uniform_textureB;
uniform sampler2D uniform_textureC;
uniform sampler2D uniform_textureDepth;//D

uniform vec2 uniform_projConst; //D

uniform vec3 uniform_lightDir;
uniform vec3 uniform_lightColor;

// shadowsampler
uniform sampler2DShadow uniform_shadowmap;

uniform mat4 uniform_lightViewProjMatrix;
uniform mat4 uniform_invViewMatrix;

const int SHADOWSAMPLES = 9;
vec2 sampleOffsets[SHADOWSAMPLES] = vec2[]
(
   vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0),
   vec2(-1.0, 0.0), vec2(0.0, 0.0), vec2(1.0, 0.0),
   vec2(-1.0, 1.0), vec2(0.0, 1.0), vec2(1.0, 1.0)
);

float calcShadow(vec3 viewPosition)
{
  float returner = 0.0;
  
  // Get the current fragment position in world-space
  vec4 worldPosition = uniform_invViewMatrix * vec4(viewPosition, 1); 
  
  // Multiply this position by the VPMatrix used by the shadow-map-rendering. Also, normalize the perspective by dividing by w
  vec4 lightPosition = uniform_lightViewProjMatrix * worldPosition; 
  lightPosition /= lightPosition.w;
  
  // Convert these coordinates from (-1 to 1) range to  (0 to 1) range
  vec2 shadowUV = lightPosition.xy * vec2(0.5) + vec2(0.5);

  // Use the (biased) z coordinate as the comparative depth to pass to the texture sampling function.
  const float bias = 0.005;
  float depthComp = (lightPosition.z * 0.5 + 0.5) - bias;
  
  // If the shadow UV is out of bounds, return 1.0. Needed for directional lights
  if(shadowUV.x < 0 || shadowUV.y < 0 ||shadowUV.x > 1 ||shadowUV.y > 1)
  {
    return 1.0;
  }

  // Prepare sampling
  ivec2 shadowSize = textureSize(uniform_shadowmap, 0);
  vec2 shadowFragSize = vec2(1.0) / vec2(shadowSize.x, shadowSize.y);

  // 9-tap shadow sampling
  for(int i = 0; i < SHADOWSAMPLES; i++)
  {
    vec2 offsetUV = shadowUV + (sampleOffsets[i] * shadowFragSize);
    float shadowDepth = texture(uniform_shadowmap, vec3(offsetUV.x, offsetUV.y, depthComp));
    returner += shadowDepth;
  }
  returner /= float(SHADOWSAMPLES);
  
  return returner;
}


void main()
{
  float depth = texture(uniform_textureDepth, in_texCoords).r;//D
  
  if(depth == 1.0)
  {
    discard;
  }
  
  vec4 texValueA = texture(uniform_textureA, in_texCoords).rgba;
  vec4 texValueB = texture(uniform_textureB, in_texCoords).rgba;
  vec4 texValueC = texture(uniform_textureC, in_texCoords).rgba;

  vec3 albedo = texValueA.xyz;
  float roughness = clamp(texValueA.a, 0.01, 1.0);
  vec3 emissive = texValueB.rgb;
  float metalness = texValueB.a;
  vec3 normal = normalize(texValueC.xyz);
  float occlusion = texValueC.a;
  
  // Depthextracted position
  vec3 viewRay = in_vertexPos;//D
  float linearDepth = uniform_projConst.x / (uniform_projConst.y - depth);//D
  vec3 position = viewRay * linearDepth;//D
  
  vec3 viewDir = normalize( -position );

  float diffuse = max(0.0, dot(normal, -uniform_lightDir));
  vec3 lightDiffuse = (uniform_lightColor) * albedo * diffuse * (1.0 - metalness);

  float ior = 1.3;
  vec3 F0 = vec3(abs ((1.0 - ior) / (1.0 + ior)));
  F0 = mix(F0, albedo, metalness) ;
  vec3 specular = cookTorranceSpecular(normalize(-uniform_lightDir), viewDir, normal, roughness, F0);
  vec3 lightSpecular =  (uniform_lightColor) * specular;

  out_color = vec4(lightDiffuse + lightSpecular, 1.0) * calcShadow(position);
}
