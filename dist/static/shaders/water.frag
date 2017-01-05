#version 410 core

layout(location = 0) in vec4 in_vertexPos;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

uniform sampler2D uniform_depthmap;
uniform sampler2D uniform_colormap;
uniform vec2 uniform_projConst; //D
uniform mat4 uniform_invViewMatrix;
uniform mat4 uniform_viewMatrix;

// Normal and height
uniform sampler2D uniform_normalmapA;
uniform sampler2D uniform_normalmapB;

uniform sampler2D uniform_heightmapA;
uniform sampler2D uniform_heightmapB;

uniform float uniform_time = 0.0;

const float const_wavespeed = 1.0 / 1000.0;

vec3 cookTorranceSpecular(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness, vec3 F0);

void main()
{
  // Calculate some wave uv shit
  float waveTimeA = const_wavespeed * 0.3 * uniform_time;
  float waveTimeB = const_wavespeed * 0.25 * uniform_time;
  
  vec2 waveUvA = (in_uv * 6.0)  + (vec2(0.12, 0.1) * waveTimeA);
  vec2 waveUvB = (in_uv * 12.0) + (vec2(-0.2, 0.12) * waveTimeB);
  
  float heightA = texture(uniform_heightmapA, waveUvA).r;
  float heightB = texture(uniform_heightmapB, waveUvB).r;
  
  float height = mix(heightA, heightB, 0.5);
 
 // World-space normal
  vec3 wNormal = ((texture(uniform_normalmapA, waveUvA).rgb * 2.0) - 1.0);
  wNormal += ((texture(uniform_normalmapB, waveUvB).rgb * 2.0) - 1.0);
  wNormal = normalize(wNormal);
  
  vec3 normalTex = wNormal;
  
  //wNormal = vec3(0.0, 0.0, 1.0);
  
  mat3 normalRotation = mat3(mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(0.0, -1.0, 0.0)));
  
  wNormal =  normalRotation * wNormal;
 
  vec3 vNormal = normalize(vec3(uniform_viewMatrix * vec4(wNormal, 0.0)));
 
  // Sample accumulation copy
  vec2 texStep = vec2(1.0) / textureSize(uniform_depthmap, 0);
  vec2 texCoord = gl_FragCoord.xy * texStep;
  float oldDepth = texture(uniform_depthmap, texCoord).r;
  
  // Depthextracted position
  vec3 viewRay = vec3(in_vertexPos.xy / in_vertexPos.z, 1.0);//D
  float linearOldDepth = uniform_projConst.x / (uniform_projConst.y - oldDepth);//D
  vec3 oldPosition = viewRay * linearOldDepth;//D

  vec3 oldWP = vec3(uniform_invViewMatrix * vec4(oldPosition, 1.0));
  vec3 waterWP = vec3(uniform_invViewMatrix * in_vertexPos);
  
  float refractionStrength = 100.0;
  
  vec2 refTexCoord = texCoord + texStep * normalTex.xy * refractionStrength;
  float oldRefDepth = texture(uniform_depthmap, refTexCoord).r;

  if(oldRefDepth > gl_FragCoord.z)
  {
    texCoord += texStep * normalTex.xy * refractionStrength;
  }
  vec3 prevColor = texture(uniform_colormap, texCoord).rgb;
  
  // water distance
  float shoreLength = 0.5;
  float shoreCoeff = clamp(distance(waterWP.y, oldWP.y)* (1.0 / shoreLength), 0.0, 1.0);

  // View-direction in viewspace
  vec3 vViewDir = normalize( -oldPosition );

  // View-direction in worldspace
  vec3 wViewDir = normalize(vec3(uniform_invViewMatrix * vec4(vViewDir, 0.0)));
  
      // Config variables for murk
  float waterClarity = 0.2;
  vec3 baseColor = vec3(0.67, 0.8, 0.9);

  // water color 
  vec3 waterColor = prevColor* baseColor * shoreCoeff;
  
  vec3 result = mix(prevColor, waterColor, shoreCoeff);
  out_color = vec4(result, 1.0);
}

  
