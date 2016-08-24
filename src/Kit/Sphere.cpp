#include "Kit/Sphere.hpp"

#include <glm/gtc/constants.hpp>
#include <vector>

kit::Sphere::Sphere(uint32_t rings, uint32_t sectors)
{
  
  this->allocateBuffers();
  this->m_indexCount = 0;
  
  // Generate sphere geometry on the cpu
  float const R = 1.0f / float(rings - 1);
  float const S = 1.0f / float(sectors - 1);

  std::vector<GLfloat> vertices;
  std::vector<uint32_t> indices;

  // First generate per-vertex data
  for (uint32_t r = 0; r < rings; r++)
  {
    for (uint32_t s = 0; s < sectors; s++)
    {
      float const x = cos(2.0f * glm::pi<float>() * float(s) * S) * sin(glm::pi<float>() * float(r) * R);
      float const z = sin(-glm::half_pi<float>() + glm::pi<float>() * float(r) * R);
      float const y = sin(2.0f * glm::pi<float>() * float(s) * S) * sin(glm::pi<float>() * float(r) * R);
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);

      vertices.push_back(s*S);
      vertices.push_back(r*R);
    }
  }

  // Then generate index data
  for (uint32_t r = 0; r < rings-1; r++)
  {
    for (uint32_t s = 0; s < sectors-1; s++)
    {
      indices.push_back(r * sectors + s);
      indices.push_back(r * sectors + (s + 1));
      indices.push_back((r + 1) * sectors + (s + 1));

      
      indices.push_back(r * sectors + s);
      indices.push_back((r + 1) * sectors + (s + 1));
      indices.push_back((r + 1) * sectors + s);
    }
  }

  this->m_indexCount = (uint32_t)indices.size();

  // Upload sphere geometry to the gpu
  glBindVertexArray(this->m_glVertexArray);

  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

  // Upload vertices 
  glBindBuffer(GL_ARRAY_BUFFER, this->m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

  // Total size
  uint32_t attributeSize = (sizeof(GLfloat)* 5);

  // Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);

  // Texture coordinates
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*)(sizeof(GLfloat)* 3));

  // Normals
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);
}

kit::Sphere::~Sphere()
{
  this->releaseBuffers();
}

void kit::Sphere::renderGeometry()
{
  
  glBindVertexArray(this->m_glVertexArray);
  glDrawElements(GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

kit::Sphere::Ptr kit::Sphere::create(uint32_t rings, uint32_t sectors)
{
  return std::make_shared<kit::Sphere>(rings, sectors);
}


void kit::Sphere::allocateBuffers()
{
  
  glGenVertexArrays(1, &this->m_glVertexArray);
  glGenBuffers(1, &this->m_glVertexIndices);
  glGenBuffers(1, &this->m_glVertexBuffer);
}

void kit::Sphere::releaseBuffers()
{
  

  glDeleteBuffers(1, &this->m_glVertexIndices);
  glDeleteBuffers(1, &this->m_glVertexBuffer);
  glDeleteVertexArrays(1, &this->m_glVertexArray);
  glGetError();
}