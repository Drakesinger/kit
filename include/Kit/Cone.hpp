#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

namespace kit
{
  class KITAPI Cone
  {
  public:
    

    Cone(float radius, float depth, uint32_t sectors);
    ~Cone();

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
