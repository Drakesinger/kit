#include "Kit/Quad.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Shader.hpp"
#include "Kit/Program.hpp"

unsigned int kit::Quad::m_instanceCount = 0;
uint32_t kit::Quad::m_glVertexIndices = 0;
uint32_t kit::Quad::m_glVertexArray = 0;
uint32_t kit::Quad::m_glVertexBuffer = 0;
kit::Program * kit::Quad::m_program = nullptr;
kit::Program * kit::Quad::m_programTextured = nullptr;


static const std::vector<GLushort> indexData { 0, 3, 2, 0, 2, 1 };

static const std::vector<float> vertexData {
  0.0,  0.0,  0.0,  0.0,  1.0, // Bottomleft
  1.0,  0.0,  0.0,  1.0,  1.0, // Bottomright
  1.0,  1.0,  0.0,  1.0,  0.0, // Topright
  0.0,  1.0,  0.0,  0.0,  0.0  // Topleft
};

static const char * vertexSource
="#version 410 core\n\
  \n\
  layout (location = 0) in vec3 in_position;\n\
  layout (location = 1) in vec2 in_uv;\n\
  layout (location = 2) out vec2 out_uv;\n\
  \n\
  uniform vec2 uniform_position;\n\
  uniform vec2 uniform_size;\n\
  \n\
  void main()\n\
  {\n\
    vec2 finalpos = uniform_position + (in_position.xy * uniform_size);\n\
    gl_Position = vec4(finalpos, in_position.z, 1.0);\n\
    gl_Position.xy *= 2.0;\n\
    gl_Position.y = 1.0 - gl_Position.y;\n\
    gl_Position.x -= 1.0;\n\
    out_uv = in_uv;\n\
    //out_uv.y = 1.0 - out_uv.y;\n\
  }\n";

static const char * pixelSource
="#version 410 core\n\
  \n\
  uniform vec4 uniform_color;\n\
  \n\
  out vec4 out_color;\n\
  \n\
  void main()\n\
  {\n\
    out_color = uniform_color;\n\
  }\n";
  
static const char * pixelSourceTextured
="#version 410 core\n\
  \n\
  layout (location = 2) in vec2 in_uv;\n\
  \n\
  uniform vec4 uniform_color;\n\
  uniform sampler2D uniform_texture;\n\
  uniform vec2 uniform_texSubOffset;\n\
  uniform vec2 uniform_texSubSize;\n\
  \n\
  out vec4 out_color;\n\
  \n\
  void main()\n\
  {\n\
    out_color = texture(uniform_texture, uniform_texSubOffset + (in_uv * uniform_texSubSize)) * uniform_color;\n\
  }\n";

kit::Quad::Quad(glm::vec2 position, glm::vec2 size, glm::vec4  color, kit::Texture * texture)
{
  
  kit::Quad::m_instanceCount += 1;
  if(kit::Quad::m_instanceCount == 1)
  {
    allocateShared();
  }
  
  m_position = position;
  m_size = size;
  m_color = color;
  m_texture = texture;
  m_texSubSize = glm::vec2(1.0f, 1.0f);
  m_texSubOffset = glm::vec2(0.0f, 0.0f);
  
  m_blending = true;
}

kit::Quad::~Quad()
{
  kit::Quad::m_instanceCount -= 1;
  if(kit::Quad::m_instanceCount < 1)
  {
    releaseShared();
  }
}

void kit::Quad::prepareProgram(kit::Program * customprogram)
{
  
  if(customprogram != nullptr)
  {
    customprogram->setUniform2f("uniform_size", m_size);
    customprogram->setUniform2f("uniform_position", m_position);
    //customprogram->setUniform2f("uniform_texSubOffset", m_texSubOffset);
    //customprogram->setUniform2f("uniform_texSubSize", m_texSubSize);
    customprogram->use();
  }
  else if(!m_texture)
  {
      if (m_blending)
      {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      else
      {
        glDisable(GL_BLEND);
      }
      glDisable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);

      kit::Quad::m_program->setUniform4f("uniform_color", kit::srgbDec(m_color));
      kit::Quad::m_program->setUniform2f("uniform_size", m_size);
      kit::Quad::m_program->setUniform2f("uniform_position", m_position);
      kit::Quad::m_program->use();
  }
  else
  {
    if (m_blending)
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
      glDisable(GL_BLEND);
    }
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    kit::Quad::m_programTextured->setUniformTexture("uniform_texture", m_texture);
    kit::Quad::m_programTextured->setUniform2f("uniform_texSubOffset", m_texSubOffset);
    kit::Quad::m_programTextured->setUniform2f("uniform_texSubSize", m_texSubSize);
    kit::Quad::m_programTextured->setUniform4f("uniform_color", kit::srgbDec(m_color));
    kit::Quad::m_programTextured->setUniform2f("uniform_size", m_size);
    kit::Quad::m_programTextured->setUniform2f("uniform_position", m_position);
    kit::Quad::m_programTextured->use();
  }
}

void kit::Quad::renderGeometry()
{
  
  glBindVertexArray(kit::Quad::m_glVertexArray);
  glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
}

void kit::Quad::renderAltGeometry()
{
  
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void kit::Quad::render(kit::Program * customprogram)
{
  prepareProgram(customprogram);
  kit::Quad::renderGeometry();
}

glm::vec2 const & kit::Quad::getPosition()
{
  return m_position;
}

glm::vec2 const & kit::Quad::getSize()
{
  return m_size;
}

void kit::Quad::setPosition(glm::vec2 position)
{
  m_position = position;
}

void kit::Quad::setSize(glm::vec2 size)
{
  m_size = size;
}

void kit::Quad::setColor(glm::vec4 color)
{
  m_color = (color);
}

void kit::Quad::setTexture(kit::Texture * texture)
{
  m_texture = texture;
}

void kit::Quad::allocateShared()
{
  
  // Generate and bind our Vertex Array Object
  glGenVertexArrays(1, &kit::Quad::m_glVertexArray);
  glBindVertexArray(kit::Quad::m_glVertexArray);

  // Generate our Vertex Index Buffer Object
  glGenBuffers(1, &kit::Quad::m_glVertexIndices);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kit::Quad::m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLushort), &indexData[0], GL_STATIC_DRAW);

  // Generate our Vertex Buffer Object
  glGenBuffers(1, &kit::Quad::m_glVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, kit::Quad::m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) (sizeof(float) * 3));
  
  // allocate our programs!
  auto vertexShader = new kit::Shader(Shader::Type::Vertex);
  vertexShader->sourceFromString(vertexSource);
  vertexShader->compile();

  auto pixelShader = new kit::Shader(Shader::Type::Fragment);
  pixelShader->sourceFromString(pixelSource);
  pixelShader->compile();
  
  auto pixelShaderTextured = new kit::Shader(Shader::Type::Fragment);
  pixelShaderTextured->sourceFromString(pixelSourceTextured);
  pixelShaderTextured->compile();

  m_program = new kit::Program();
  m_program->attachShader(vertexShader);
  m_program->attachShader(pixelShader);
  m_program->link();
  m_program->detachShader(vertexShader);
  m_program->detachShader(pixelShader);
  
  m_programTextured = new kit::Program();
  m_programTextured->attachShader(vertexShader);
  m_programTextured->attachShader(pixelShaderTextured);
  m_programTextured->link();
  m_programTextured->detachShader(vertexShader);
  m_programTextured->detachShader(pixelShaderTextured);
  
  delete vertexShader;
  delete pixelShader;
  delete pixelShaderTextured;
}

void kit::Quad::releaseShared()
{
  
  delete kit::Quad::m_programTextured;
  delete kit::Quad::m_program;
  
  glDeleteBuffers(1, &kit::Quad::m_glVertexIndices);
  glDeleteBuffers(1, &kit::Quad::m_glVertexBuffer);
  glDeleteVertexArrays(1, &kit::Quad::m_glVertexArray);
}

const float & kit::Quad::getDepth()
{
  return m_depth;
}

void kit::Quad::setDepth(float d)
{
  m_depth = d;
}

void kit::Quad::setBlending(bool b)
{
  m_blending = b;
}

void kit::Quad::setTextureSubRect(glm::vec2 position, glm::vec2 size)
{
  m_texSubOffset = position;
  m_texSubSize = size;
}
