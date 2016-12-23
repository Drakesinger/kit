#include "Kit/Submesh.hpp"
#include "Kit/IncOpenGL.hpp"

std::map<std::string, kit::Submesh*> kit::Submesh::m_cache = std::map<std::string, kit::Submesh*>();

kit::Submesh::Submesh(const std::string&filename)
{
  allocateBuffers();
  m_indexCount = 0;

  loadGeometry(filename);
}

kit::Submesh::~Submesh()
{
  releaseBuffers();
}

void kit::Submesh::renderGeometry()
{
  glBindVertexArray(m_glVertexArray);
  glDrawElements( GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

void kit::Submesh::renderGeometryInstanced(uint32_t numInstances)
{
  glBindVertexArray(m_glVertexArray);
  glDrawElementsInstanced( GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, (void*)0, numInstances);
}

kit::Submesh* kit::Submesh::load(const std::string& name, kit::DataSource source)
{
  std::string full = kit::getDataDirectory(source) + std::string ("geometry/") + name;

  auto finder = kit::Submesh::m_cache.find(full);
  if(finder != kit::Submesh::m_cache.end())
  {
    return finder->second;
  }
  
  kit::Submesh::m_cache[full] = new kit::Submesh(full);
  
  return kit::Submesh::m_cache[full];
}

void kit::Submesh::flushCache()
{
  for(auto & t : m_cache)
    delete t.second;
  
  m_cache.clear();
}

void kit::Submesh::allocateBuffers()
{
  glGenVertexArrays(1, &m_glVertexArray);
  glGenBuffers(1, &m_glVertexIndices);
  glGenBuffers(1, &m_glVertexBuffer);
}

void kit::Submesh::releaseBuffers()
{
  
  glDeleteBuffers(1, &m_glVertexIndices);
  glDeleteBuffers(1, &m_glVertexBuffer);
  glDeleteVertexArrays(1, &m_glVertexArray);
}

void kit::Submesh::loadGeometry(const std::string&filename)
{
  
  kit::Geometry data;
  if(!data.load(filename))
  {
    KIT_THROW("Failed to load submesh data from file");
  }
  
  m_indexCount = (uint32_t)data.m_indices.size();
  
  glBindVertexArray(m_glVertexArray);
  
  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.m_indices.size() * sizeof(uint32_t), &data.m_indices[0], GL_STATIC_DRAW);
  
  // Upload vertices 
  glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, data.m_vertices.size() * 19 * sizeof(float) , &data.m_vertices[0], GL_STATIC_DRAW);
  
  // Total size
  uint32_t attributeSize = (sizeof(float) * 15) + (sizeof(int32_t) * 4);
  
  // Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);
  
  // Texture coordinates
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 3) );

  // Normals
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 5) );
  
  // Tangents
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 8) );
  
  // Bone ID's
  glEnableVertexAttribArray(4);
  glVertexAttribIPointer(4, 4, GL_INT, attributeSize, (void*) (sizeof(float) * 11)  );
  
  // Bone weights
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, attributeSize, (void*)((sizeof(float) * 11) + (sizeof(int32_t) * 4)) );
}
