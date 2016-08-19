#version 410 core

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec4 out_color;

uniform sampler2D uniform_sourceTexture;
uniform sampler2D uniform_bloom8Texture;
uniform sampler2D uniform_bloom16Texture;
uniform sampler2D uniform_bloom32Texture;
uniform float uniform_exposure;
uniform float uniform_whitepoint;

vec3 Uncharted2Tonemap(vec3 x)
{
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;

  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main( ) {
    // Use gaussian coefficients to bias the bloom outwards
    vec3 bloom8 = vec3(texture(uniform_bloom8Texture, in_uv).rgb)   * 0.06136;
    vec3 bloom16 = vec3(texture(uniform_bloom16Texture, in_uv).rgb) * 0.24477;
    vec3 bloom32 = vec3(texture(uniform_bloom32Texture, in_uv).rgb) * 0.38774;
    
    vec3 bloom = (bloom8 + bloom16 + bloom32);
    
    vec3 in_color = texture(uniform_sourceTexture, in_uv).rgb * uniform_exposure;
    out_color.a = 1.0;
    out_color.rgb = (Uncharted2Tonemap(in_color+bloom) / Uncharted2Tonemap(vec3(uniform_whitepoint)));
}