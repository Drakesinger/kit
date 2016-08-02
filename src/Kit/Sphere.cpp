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
  kit::GL::bindVertexArray(this->m_glVertexArray);

  // Upload indices
  kit::GL::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glVertexIndices);
  KIT_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW));

  // Upload vertices 
  kit::GL::bindBuffer(GL_ARRAY_BUFFER, this->m_glVertexBuffer);
  KIT_GL(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW));

  // Total size
  uint32_t attributeSize = (sizeof(GLfloat)* 5);

  // Positions
  KIT_GL(glEnableVertexAttribArray(0));
  KIT_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0));

  // Texture coordinates
  KIT_GL(glEnableVertexAttribArray(1));
  KIT_GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*)(sizeof(GLfloat)* 3)));

  // Normals
  KIT_GL(glEnableVertexAttribArray(2));
  KIT_GL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0));
}

kit::Sphere::~Sphere()
{
  this->releaseBuffers();
}

void kit::Sphere::renderGeometry()
{
  
  kit::GL::bindVertexArray(this->m_glVertexArray);
  KIT_GL(glDrawElements(GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0));
}

kit::Sphere::Ptr kit::Sphere::create(uint32_t rings, uint32_t sectors)
{
  return std::make_shared<kit::Sphere>(rings, sectors);
}


void kit::Sphere::allocateBuffers()
{
  
  KIT_GL(glGenVertexArrays(1, &this->m_glVertexArray));
  KIT_GL(glGenBuffers(1, &this->m_glVertexIndices));
  KIT_GL(glGenBuffers(1, &this->m_glVertexBuffer));
}

void kit::Sphere::releaseBuffers()
{
  

  glDeleteBuffers(1, &this->m_glVertexIndices);
  glDeleteBuffers(1, &this->m_glVertexBuffer);
  glDeleteVertexArrays(1, &this->m_glVertexArray);
  glGetError();
}