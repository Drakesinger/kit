#version 410 core

layout(location = 0) in vec2 in_position;

layout(location = 0) out vec4 out_vertexPos;
layout(location = 1) out vec2 out_uv;

uniform mat4 uniform_mvpMatrix;
uniform mat4 uniform_mvMatrix;

uniform sampler2D uniform_heightmapA;
uniform sampler2D uniform_heightmapB;

uniform float uniform_time = 0.0;

const float const_wavespeed = 1.0 / 1000.0;

void main()
{

  float waveTimeA = const_wavespeed * 0.3 * uniform_time;
  float waveTimeB = const_wavespeed * 0.25 * uniform_time;
  
  
  vec2 waveUvA = (in_position * 6.0)  + (vec2(0.12, 0.1) * waveTimeA);
  vec2 waveUvB = (in_position * 12.0) + (vec2(-0.2, 0.12) * waveTimeB);
  
  float heightA = texture(uniform_heightmapA, waveUvA).r;
  float heightB = texture(uniform_heightmapB, waveUvB).r;
  
  float height = mix(heightA, heightB, 0.5);

  vec4 worldPosition = vec4(in_position.x, height, in_position.y, 1.0);
  gl_Position = uniform_mvpMatrix * worldPosition;
  out_vertexPos = (uniform_mvMatrix * worldPosition);
  
  out_uv = in_position;
}