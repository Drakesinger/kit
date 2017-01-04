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
      
      static std::shared_ptr<kit::Submesh> load(const std::string& geometry);
      
      void renderGeometry();
      void renderGeometryInstanced(uint32_t numInstances);
      
      Submesh(const std::string& filename);
    private:
      void loadGeometry(const std::string& filename);
      
      // Cache
      static std::map<std::string, std::weak_ptr<kit::Submesh>> m_cache;
      
      // Individual GPU data
      void allocateBuffers();
      void releaseBuffers();
      
      uint32_t m_glVertexArray;   
      uint32_t m_glVertexIndices; 
      uint32_t m_glVertexBuffer;
      
      uint32_t m_indexCount;

  };
}
