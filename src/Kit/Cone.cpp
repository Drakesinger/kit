#include "Kit/Cone.hpp"

#include <glm/gtc/constants.hpp>
#include <vector>

kit::Cone::Cone(float radius, float depth, uint32_t sectors)
{
  this->allocateBuffers();
  this->m_indexCount = 0;
  
  std::vector<GLfloat> vertices;
  std::vector<uint32_t> indices;

  // Create root (head)
  for (int i = 0; i < 3; i++)
  {
    vertices.push_back(0.0f);
  }

  // Create base
  float deltaBaseAngle = (2 * glm::pi<float>()) / sectors;
  for (uint32_t i = 0; i < sectors; i++)
  {
    float angle = float(i) * deltaBaseAngle;
    vertices.push_back(radius * cosf(angle));
    vertices.push_back(radius * sinf(angle));
    vertices.push_back(-depth);
  }

  // Create indices
  // - Head to vertices
  for (uint32_t i = 0; i < sectors; i++)
  {
    indices.push_back(0);
    indices.push_back((i % sectors) + 1);
    indices.push_back(((i + 1) % sectors) + 1);
  }

  // - Base
  for (uint32_t i = 0; i < sectors - 2; i++)
  {
    indices.push_back(1);
    indices.push_back(i+3);
    indices.push_back(i+2);
  }

  this->m_indexCount = (uint32_t)indices.size();

  // Upload cone geometry to the gpu
  kit::GL::bindVertexArray(this->m_glVertexArray);

  // Upload indices
  kit::GL::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glVertexIndices);
  KIT_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW));

  // Upload vertices 
  kit::GL::bindBuffer(GL_ARRAY_BUFFER, this->m_glVertexBuffer);
  KIT_GL(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW));

  // Total size
  uint32_t attributeSize = (sizeof(GLfloat)* 3);

  // Positions
  KIT_GL(glEnableVertexAttribArray(0));
  KIT_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0));
}

kit::Cone::~Cone()
{
  this->releaseBuffers();
}

void kit::Cone::renderGeometry()
{
  kit::GL::bindVertexArray(this->m_glVertexArray);
  KIT_GL(glDrawElements(GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0));
}

kit::Cone::Ptr kit::Cone::create(float radius, float depth, uint32_t sectors)
{
  return std::make_shared<kit::Cone>(radius, depth, sectors);
}


void kit::Cone::allocateBuffers()
{
  KIT_GL(glGenVertexArrays(1, &this->m_glVertexArray));
  KIT_GL(glGenBuffers(1, &this->m_glVertexIndices));
  KIT_GL(glGenBuffers(1, &this->m_glVertexBuffer));
}

void kit::Cone::releaseBuffers()
{
  KIT_GL(glDeleteBuffers(1, &this->m_glVertexIndices));
  KIT_GL(glDeleteBuffers(1, &this->m_glVertexBuffer));
  KIT_GL(glDeleteVertexArrays(1, &this->m_glVertexArray));
}