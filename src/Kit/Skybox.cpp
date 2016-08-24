#include "Kit/Skybox.hpp"
#include "Kit/Shader.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Program.hpp"
#include "Kit/Cubemap.hpp"
#include "Kit/Renderer.hpp"

static const std::vector<float> bufferdata = {
  
  // We need no more than a front and backside, as they cover every corner and not more
  
  // Front side
  -1.0, 1.0,  1.0,  // Top left
  1.0,  1.0,  1.0,  // Top right 
  1.0,  1.0,  -1.0, // Bottom right
  -1.0, 1.0,  -1.0, // Bottom left
  
  // Back side
  -1.0, -1.0, 1.0,  // Top left
  1.0,  -1.0, 1.0,  // Top right 
  1.0,  -1.0, -1.0, // Bottom right
  -1.0, -1.0, -1.0 // Bottom left
};

static const std::vector<uint32_t> indices = {
  // Front side
  0, 1, 2, 0, 2, 3,
  // Back side
  4, 5, 6, 4, 6, 7,
  //6, 5, 4, 7, 6, 4,
  // Left side
  0, 4, 7, 0, 7, 3,
  // Right side
  5, 1, 2, 5, 2, 6,
  // Top side,
  0, 1, 5, 0, 5, 4,
  // Bottom side,
  3, 2, 6, 3, 6, 7
};

static const char * vertexSource
="#version 410\n\
layout(location = 0) in vec3 in_position;\n\
layout(location = 0) out vec3 out_texcoords;\n\
uniform mat4 uniform_rotProjMatrix;\n\
uniform float uniform_farclip;\n\
void main()\n\
{\n\
  out_texcoords = in_position;\n\
  vec4 pos = vec4(in_position * uniform_farclip * 0.5, 1.0);\n\
  gl_Position = uniform_rotProjMatrix * pos;\n\
}";

static const char * pixelSource
= "#version 410\n\
layout(location = 0) in vec3 in_texcoords;\n\
layout(location = 0) out vec4 out_color;\n\
\n\
uniform samplerCube uniform_texture;\n\
uniform float uniform_strength;\n\
\n\
void main()\n\
{\n\
  vec3 tx = normalize(in_texcoords);\n\
  out_color = vec4(textureLod(uniform_texture, tx, 0.0).xyz * uniform_strength, 1.0);\n\
}";

static const char * pixelSourceNoTex
= "#version 410\n\
layout(location = 0) in vec3 in_texcoords;\n\
layout(location = 0) out vec4 out_color;\n\
\n\
uniform vec4 uniform_color;\n\
\n\
void main()\n\
{\n\
  vec3 tx = normalize(in_texcoords);\n\
  out_color = vec4(uniform_color.rgb * uniform_color.a, 1.0);\n\
}";

uint32_t kit::Skybox::m_instanceCount = 0;
kit::Program::Ptr kit::Skybox::m_program = nullptr;
kit::Program::Ptr kit::Skybox::m_programNoTexture = nullptr;
GLuint kit::Skybox::m_glVertexArray = 0;
GLuint kit::Skybox::m_glVertexIndices = 0;
GLuint kit::Skybox::m_glVertexBuffer = 0;

kit::Skybox::Skybox(kit::Cubemap::Ptr cubemap)
{
  kit::Skybox::m_instanceCount++;
  if(kit::Skybox::m_instanceCount == 1)
  {
    kit::Skybox::allocateShared();
  }
  
  this->m_strength = 1.0f;
  this->m_color = glm::vec3(2.0f/255.0f, 3.0f/255.0f, 4.0f/255.0f);
  this->m_texture = cubemap;
}

kit::Skybox::~Skybox()
{
  kit::Skybox::m_instanceCount--;
  if(kit::Skybox::m_instanceCount == 0)
  {
    kit::Skybox::releaseShared();
  }
}

void kit::Skybox::renderGeometry()
{
  glBindVertexArray(kit::Skybox::m_glVertexArray);
  // A cube has 6 faces, 2 triangles each, 3 verticces per triangle
  glDrawElements(GL_TRIANGLES, (uint32_t)indices.size(), GL_UNSIGNED_INT, (void*)0);
}

void kit::Skybox::render(kit::Renderer::Ptr renderer)
{
  glm::mat4 rotProjMatrix = renderer->getActiveCamera()->getProjectionMatrix() * glm::inverse(renderer->getActiveCamera()->getRotationMatrix());
  if (this->m_texture)
  {
    kit::Skybox::m_program->use();
    kit::Skybox::m_program->setUniformMat4("uniform_rotProjMatrix", rotProjMatrix);
    kit::Skybox::m_program->setUniform1f("uniform_farclip", renderer->getActiveCamera()->getClipRange().y);
    kit::Skybox::m_program->setUniformCubemap("uniform_texture", this->m_texture);
    kit::Skybox::m_program->setUniform1f("uniform_strength", this->m_strength);
  }
  else
  {
    kit::Skybox::m_programNoTexture->use();
    kit::Skybox::m_programNoTexture->setUniformMat4("uniform_rotProjMatrix", rotProjMatrix);
    kit::Skybox::m_programNoTexture->setUniform1f("uniform_farclip", renderer->getActiveCamera()->getClipRange().y);
    kit::Skybox::m_programNoTexture->setUniform4f("uniform_color", glm::vec4(this->m_color.x, this->m_color.y, this->m_color.z, this->m_strength));
  }

  this->renderGeometry();
}

kit::Skybox::Ptr kit::Skybox::create(kit::Cubemap::Ptr cubemap)
{
  //return kit::Skybox::Ptr(new kit::Skybox(cubemap));
  return std::make_shared<kit::Skybox>(cubemap);
}

void kit::Skybox::allocateShared()
{
  
  // Generate and bind our Vertex Array Object
  glGenVertexArrays(1, &kit::Skybox::m_glVertexArray);
  
  glBindVertexArray(kit::Skybox::m_glVertexArray);
  
  // Generate our Vertex Index Buffer Object
  glGenBuffers(1, &kit::Skybox::m_glVertexIndices);

  // Generate our Vertex Buffer Object
  glGenBuffers(1, &kit::Skybox::m_glVertexBuffer);
  
  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kit::Skybox::m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);
  
  // Upload positions
  glBindBuffer(GL_ARRAY_BUFFER, kit::Skybox::m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, bufferdata.size() * sizeof(float) , &bufferdata[0], GL_STATIC_DRAW);

  // Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);

  kit::Skybox::m_program = kit::Program::create();
  kit::Skybox::m_programNoTexture = kit::Program::create();
  
  auto pixelShader = kit::Shader::create(Shader::Type::Fragment);
  pixelShader->sourceFromString(pixelSource);
  pixelShader->compile();
  
  auto pixelShaderNoTexture = kit::Shader::create(Shader::Type::Fragment);
  pixelShaderNoTexture->sourceFromString(pixelSourceNoTex);
  pixelShaderNoTexture->compile();

  auto vertexShader = kit::Shader::create(Shader::Type::Vertex);
  vertexShader->sourceFromString(vertexSource);
  vertexShader->compile();
  
  kit::Skybox::m_program->attachShader(vertexShader);
  kit::Skybox::m_program->attachShader(pixelShader);
  kit::Skybox::m_program->link();
  kit::Skybox::m_program->detachShader(pixelShader);
  kit::Skybox::m_program->detachShader(vertexShader);

  kit::Skybox::m_programNoTexture->attachShader(vertexShader);
  kit::Skybox::m_programNoTexture->attachShader(pixelShaderNoTexture);
  kit::Skybox::m_programNoTexture->link();
  kit::Skybox::m_programNoTexture->detachShader(pixelShaderNoTexture);
  kit::Skybox::m_programNoTexture->detachShader(vertexShader);
}

void kit::Skybox::releaseShared()
{
  glDeleteBuffers(1, &kit::Skybox::m_glVertexIndices);
  glDeleteBuffers(1, &kit::Skybox::m_glVertexBuffer);
  glDeleteVertexArrays(1, &kit::Skybox::m_glVertexArray);
}

kit::Cubemap::Ptr kit::Skybox::getTexture()
{
  return this->m_texture;
}

void kit::Skybox::setTexture(kit::Cubemap::Ptr cubemaptexture)
{
  this->m_texture = cubemaptexture;
}

void kit::Skybox::setColor(glm::vec3 c)
{
  this->m_color = c;
}

void kit::Skybox::setStrength(float s)
{
  this->m_strength = s;
}

glm::vec3 kit::Skybox::getColor()
{
  return this->m_color;
}

float kit::Skybox::getStrength()
{
  return this->m_strength;
}
