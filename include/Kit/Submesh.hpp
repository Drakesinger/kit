#ifndef KIT_SUBMESH_HPP
#define KIT_SUBMESH_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <memory>

namespace kit 
{
  class KITAPI Submesh 
  {
    public:
      typedef std::shared_ptr<kit::Submesh> Ptr;
      
      ~Submesh();
      
      static kit::Submesh::Ptr load(const std::string& geometry, kit::DataSource source = kit::DataSource::Data);
      static void flushCache();
      
      void renderGeometry();
      void renderGeometryInstanced(uint32_t numInstances);
      
      Submesh(const std::string& filename);
    private:
      void loadGeometry(const std::string& filename);
      
      
      // Cache
      static std::map<std::string, kit::Submesh::Ptr> m_cache;
      
      // Individual GPU data
      void allocateBuffers();
      void releaseBuffers();
      
      uint32_t m_glVertexArray;   
      uint32_t m_glVertexIndices; 
      uint32_t m_glVertexBuffer;
      
      uint32_t m_indexCount;

  };
}

#endif // KIT_SUBMESH_HPP