#ifndef KIT_FRUSTUM_HPP
#define KIT_FRUSTUM_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"


#include <memory>

namespace kit
{
  class KITAPI Frustum
  {
  public:
    typedef std::shared_ptr<kit::Frustum> Ptr;

    Frustum(float fov, float ratio, glm::vec2 cliprange);
    ~Frustum();

    static kit::Frustum::Ptr create(float fov, float ratio, glm::vec2 cliprange);

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

#endif // KIT_FRUSTUM_HPP