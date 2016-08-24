#include "Kit/Submesh.hpp"

std::map<std::string, kit::Submesh::Ptr> kit::Submesh::m_cache = std::map<std::string, kit::Submesh::Ptr>();

kit::Submesh::Submesh(const std::string&filename)
{
  this->allocateBuffers();
  this->m_indexCount = 0;
  
  std::string fullpath = std::string("./data/geometry/") + filename;
  this->loadGeometry(fullpath);
}

kit::Submesh::~Submesh()
{
  this->releaseBuffers();
}

void kit::Submesh::renderGeometry()
{
  glBindVertexArray(this->m_glVertexArray);
  glDrawElements( GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

void kit::Submesh::renderGeometryInstanced(uint32_t numInstances)
{
  glBindVertexArray(this->m_glVertexArray);
  glDrawElementsInstanced( GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0, numInstances);
}

kit::Submesh::Ptr kit::Submesh::load(const std::string& name)
{
  auto finder = kit::Submesh::m_cache.find(name);
  if(finder != kit::Submesh::m_cache.end())
  {
    return finder->second;
  }
  
  kit::Submesh::m_cache[name] = std::make_shared<kit::Submesh>(name);
  
  return kit::Submesh::m_cache[name];
}

void kit::Submesh::flushCache()
{
  kit::Submesh::m_cache.clear();
}

void kit::Submesh::allocateBuffers()
{
  
  glGenVertexArrays(1, &this->m_glVertexArray);
  glGenBuffers(1, &this->m_glVertexIndices);
  glGenBuffers(1, &this->m_glVertexBuffer);
}

void kit::Submesh::releaseBuffers()
{
  
  glDeleteBuffers(1, &this->m_glVertexIndices);
  glDeleteBuffers(1, &this->m_glVertexBuffer);
  glDeleteVertexArrays(1, &this->m_glVertexArray);
}

void kit::Submesh::loadGeometry(const std::string&filename)
{
  
  kit::Geometry data;
  if(!data.load(filename))
  {
    KIT_THROW("Failed to load submesh data from file");
  }
  
  this->m_indexCount = (uint32_t)data.m_indices.size();
  
  glBindVertexArray(this->m_glVertexArray);
  
  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.m_indices.size() * sizeof(uint32_t), &data.m_indices[0], GL_STATIC_DRAW);
  
  // Upload vertices 
  glBindBuffer(GL_ARRAY_BUFFER, this->m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, data.m_vertices.size() * 19 * sizeof(float) , &data.m_vertices[0], GL_STATIC_DRAW);
  
  // Total size
  uint32_t attributeSize = (sizeof(GLfloat) * 15) + (sizeof(GLint) * 4);
  
  // Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);
  
  // Texture coordinates
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(GLfloat) * 3) );

  // Normals
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(GLfloat) * 5) );
  
  // Tangents
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(GLfloat) * 8) );
  
  // Bone ID's
  glEnableVertexAttribArray(4);
  glVertexAttribIPointer(4, 4, GL_INT, attributeSize, (void*) (sizeof(GLfloat) * 11)  );
  
  // Bone weights
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, attributeSize, (void*)((sizeof(GLfloat) * 11) + (sizeof(GLint) * 4)) );
}
