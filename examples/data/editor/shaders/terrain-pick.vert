#version 410 core

// In attributes
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texcoord;

// Out attributes
layout (location = 0) out vec4 out_position;
layout (location = 1) out vec2 out_texcoord;

// Uniforms
uniform mat4 uniform_mvpMatrix;
uniform mat4 uniform_mvMatrix;
uniform sampler2D uniform_heightmap;

// Sampling functions
vec4 sampleHeight(vec2 position)
{
  vec2 tsize = vec2(" << this->m_resolution.x << ", " << this->m_resolution.y << ");
  vec2 tstep = vec2(1.0 / tsize);
  vec3 off = vec3(1.0, 1.0, 0.0);
  float cHeight = texture(uniform_heightmap, ((position + (tsize / 2.0)) *tstep) + tstep*0.5).r;
  float hL = texture(uniform_heightmap, ((position - off.xz + (tsize / 2.0)) *tstep) + tstep*0.5).r;
  float hR = texture(uniform_heightmap, ((position + off.xz + (tsize / 2.0)) *tstep) + tstep*0.5).r;
  float hD = texture(uniform_heightmap, ((position - off.zy + (tsize / 2.0)) *tstep) + tstep*0.5).r;
  float hU = texture(uniform_heightmap, ((position + off.xy + (tsize / 2.0)) *tstep) + tstep*0.5).r;
  return vec4( normalize(vec3(hL - hR , 1.0 / " << this->m_yScale << ", hD - hU)) , cHeight);
}

// ---- Main code begins ---- //
void main()
{

// Prepare variables
vec4 hmSample   = sampleHeight(in_position / " << this->m_xzScale << ");
vec4 position  = vec4(in_position.x, hmSample.w * " << this->m_yScale << ", in_position.y, 1.0);


// Write attributes
out_position   = uniform_mvMatrix * position;
out_normal     = (uniform_mvMatrix * vec4(normalize(hmSample.xyz), 0.0)).xyz;
out_texcoord   = in_texcoord;


// Write gl_* variables
gl_Position    = uniform_mvpMatrix * position;

// ---- Main code ends ---- //
}
