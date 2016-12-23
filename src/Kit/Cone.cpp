#include "Kit/Cone.hpp"

#include "Kit/IncOpenGL.hpp"

#include <glm/gtc/constants.hpp>
#include <vector>

kit::Cone::Cone(float radius, float depth, uint32_t sectors)
{
  this->allocateBuffers();
  this->m_indexCount = 0;
  
  std::vector<float> vertices;
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
  glBindVertexArray(this->m_glVertexArray);

  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

  // Upload vertices 
  glBindBuffer(GL_ARRAY_BUFFER, this->m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

  // Total size
  uint32_t attributeSize = (sizeof(float)* 3);

  // Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);
}

kit::Cone::~Cone()
{
  this->releaseBuffers();
}

void kit::Cone::renderGeometry()
{
  glBindVertexArray(this->m_glVertexArray);
  glDrawElements(GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

void kit::Cone::allocateBuffers()
{
  glGenVertexArrays(1, &this->m_glVertexArray);
  glGenBuffers(1, &this->m_glVertexIndices);
  glGenBuffers(1, &this->m_glVertexBuffer);
}

void kit::Cone::releaseBuffers()
{
  glDeleteBuffers(1, &this->m_glVertexIndices);
  glDeleteBuffers(1, &this->m_glVertexBuffer);
  glDeleteVertexArrays(1, &this->m_glVertexArray);
}
