#include "Kit/Frustum.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <glm/glm.hpp>

kit::Frustum::Frustum(float fov, float ratio, glm::vec2 cliprange)
{
  
  this->allocateBuffers();
  this->m_indexCount = 0;

  float Hnear = 2 * glm::tan(glm::radians(fov) / 2) * cliprange.x;
  float Wnear = Hnear * ratio;

  float Hfar = 2 * glm::tan(glm::radians(fov) / 2) * cliprange.y;
  float Wfar = Hfar * ratio;

  glm::vec3 p(0.0, 0.0, 0.0);
  glm::vec3 d(0.0, 0.0, 1.0);
  glm::vec3 up(0.0, 1.0, 0.0);
  glm::vec3 right(1.0, 0.0, 0.0);
  
  glm::vec3 fc = p + d * cliprange.y;

  glm::vec3 ftl = fc + (up * Hfar/2.0f) - (right * Wfar/2.0f);
  glm::vec3 ftr = fc + (up * Hfar/2.0f) + (right * Wfar/2.0f);
  glm::vec3 fbl = fc - (up * Hfar/2.0f) - (right * Wfar/2.0f);
  glm::vec3 fbr = fc - (up * Hfar/2.0f) + (right * Wfar/2.0f);

  glm::vec3 nc = p + d * cliprange.x;

  glm::vec3 ntl = nc + (up * Hnear/2.0f) - (right * Wnear/2.0f);
  glm::vec3 ntr = nc + (up * Hnear/2.0f) + (right * Wnear/2.0f);
  glm::vec3 nbl = nc - (up * Hnear/2.0f) - (right * Wnear/2.0f);
  glm::vec3 nbr = nc - (up * Hnear/2.0f) + (right * Wnear/2.0f);
  
  
  std::vector<GLfloat> vertices;
  std::vector<uint32_t> indices;

  // Near plane
  vertices.push_back(ntl.x); // 0
  vertices.push_back(ntl.y);
  vertices.push_back(ntl.z);
  
  vertices.push_back(ntr.x); // 1
  vertices.push_back(ntr.y);
  vertices.push_back(ntr.z);

  vertices.push_back(nbr.x); // 2
  vertices.push_back(nbr.y);
  vertices.push_back(nbr.z);
  
  vertices.push_back(nbl.x); // 3
  vertices.push_back(nbl.y);
  vertices.push_back(nbl.z);

  // Far plane
  vertices.push_back(ftl.x); // 4
  vertices.push_back(ftl.y);
  vertices.push_back(ftl.z);
  
  vertices.push_back(ftr.x); // 5
  vertices.push_back(ftr.y);
  vertices.push_back(ftr.z);

  vertices.push_back(fbr.x); // 6
  vertices.push_back(fbr.y);
  vertices.push_back(fbr.z);
  
  vertices.push_back(fbl.x); // 7
  vertices.push_back(fbl.y);
  vertices.push_back(fbl.z);
  
  // Indices
  // Top plane
  indices.push_back(0);
  indices.push_back(4);
  indices.push_back(5);
  
  indices.push_back(0);
  indices.push_back(5);
  indices.push_back(1);
  
  // Bottom plane
  indices.push_back(2);
  indices.push_back(6);
  indices.push_back(7);
  
  indices.push_back(2);
  indices.push_back(7);
  indices.push_back(3);
  
  // Left plane
  indices.push_back(3);
  indices.push_back(7);
  indices.push_back(4);
  
  indices.push_back(3);
  indices.push_back(4);
  indices.push_back(0);
  
  // Right plane
  indices.push_back(1);
  indices.push_back(5);
  indices.push_back(6);
  
  indices.push_back(1);
  indices.push_back(6);
  indices.push_back(2);
  
  // Near plane
  indices.push_back(3);
  indices.push_back(0);
  indices.push_back(1);
  
  indices.push_back(3);
  indices.push_back(1);
  indices.push_back(2);
  
  // Far plane
  indices.push_back(6);
  indices.push_back(5);
  indices.push_back(4);
  
  indices.push_back(6);
  indices.push_back(4);
  indices.push_back(7);

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

kit::Frustum::~Frustum()
{
  this->releaseBuffers();
}

void kit::Frustum::renderGeometry()
{
  
  kit::GL::bindVertexArray(this->m_glVertexArray);
  KIT_GL(glDrawElements(GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0));
}

kit::Frustum::Ptr kit::Frustum::create(float fov, float ratio, glm::vec2 cliprange)
{
  return std::make_shared<kit::Frustum>(fov, ratio, cliprange);
}


void kit::Frustum::allocateBuffers()
{
  
  KIT_GL(glGenVertexArrays(1, &this->m_glVertexArray));
  KIT_GL(glGenBuffers(1, &this->m_glVertexIndices));
  KIT_GL(glGenBuffers(1, &this->m_glVertexBuffer));
}

void kit::Frustum::releaseBuffers()
{
  
  KIT_GL(glDeleteBuffers(1, &this->m_glVertexIndices));
  KIT_GL(glDeleteBuffers(1, &this->m_glVertexBuffer));
  KIT_GL(glDeleteVertexArrays(1, &this->m_glVertexArray));
}