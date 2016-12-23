#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

namespace kit
{
  class KITAPI Frustum
  {
  public:
    

    Frustum(float fov, float ratio, glm::vec2 cliprange);
    ~Frustum();

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
