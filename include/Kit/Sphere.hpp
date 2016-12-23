#pragma once


#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <memory>

namespace kit
{
  class KITAPI Sphere
  {
  public:
    

    Sphere(uint32_t rings, uint32_t sectors);
    ~Sphere();
    
    void renderGeometry();

  private:

    // Individual GPU data
    void allocateBuffers();
    void releaseBuffers();

    uint32_t m_glVertexArray;
    uint32_t m_glVertexIndices;
    uint32_t m_glVertexBuffer;

    uint32_t m_indexCount;

  };
}
