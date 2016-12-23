#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <memory>

namespace kit 
{
  class KITAPI Submesh 
  {
    public:
      
      
      ~Submesh();
      
      static kit::Submesh * load(const std::string& geometry, kit::DataSource source = kit::DataSource::Data);
      static void flushCache();
      
      void renderGeometry();
      void renderGeometryInstanced(uint32_t numInstances);
      
      Submesh(const std::string& filename);
    private:
      void loadGeometry(const std::string& filename);
      
      
      // Cache
      static std::map<std::string, kit::Submesh*> m_cache;
      
      // Individual GPU data
      void allocateBuffers();
      void releaseBuffers();
      
      uint32_t m_glVertexArray;   
      uint32_t m_glVertexIndices; 
      uint32_t m_glVertexBuffer;
      
      uint32_t m_indexCount;

  };
}
