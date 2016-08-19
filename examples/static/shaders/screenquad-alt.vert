#version 410 core

layout(location = 0) out vec2 out_texCoords;

const vec2 quadVertices[4] = vec2[4]( vec2( -1.0, -1.0), vec2( 1.0, -1.0), vec2( -1.0, 1.0), vec2( 1.0, 1.0));
const vec2 quadCoords[4]   = vec2[4]( vec2( 0.0, 0.0),   vec2( 1.0, 0.0),   vec2( 0.0, 1.0), vec2( 1.0, 1.0));

void main()
{
  gl_Position = vec4(quadVertices[gl_VertexID], 0.0, 1.0);
  out_texCoords = vec2(quadCoords[gl_VertexID]);
}