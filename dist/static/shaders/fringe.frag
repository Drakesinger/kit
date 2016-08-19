#version 410 core

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec4 out_color;

uniform sampler2D uniform_sourceTexture;
uniform float uniform_exponential = 2.0;
uniform float uniform_scale = 0.02;

const float pi = 3.14159265358979323846;
const float epsilon = 1e-6;
const float startAngle = pi;
const float angleStep = pi * 2.0 / 3.0;

void main()
{
  ivec2 resolution = textureSize(uniform_sourceTexture, 0);
  float radius = length((in_uv - vec2(0.5, 0.5)) * (float(resolution.y) / float(resolution.x)));
  float fringing = uniform_scale * pow(radius, uniform_exponential);

  float angle = startAngle;
  vec2 dir = vec2(sin(angle), cos(angle));
  vec4 redPlane = texture(uniform_sourceTexture, in_uv + fringing * dir);

  angle += angleStep;
  dir = vec2(sin(angle), cos(angle));
  vec4 greenPlane = texture(uniform_sourceTexture, in_uv + fringing * dir);

  angle += angleStep;
  dir = vec2(sin(angle), cos(angle));
  vec4 bluePlane = texture(uniform_sourceTexture, in_uv + fringing * dir);

  out_color = vec4(redPlane.r, greenPlane.g, bluePlane.b, 1.0);
}