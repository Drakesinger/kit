#ifndef KIT_CONE_HPP
#define KIT_CONE_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"


#include <memory>

namespace kit
{
  class KITAPI Cone
  {
  public:
    typedef std::shared_ptr<kit::Cone> Ptr;

    Cone(float radius, float depth, uint32_t sectors);
    ~Cone();

    static kit::Cone::Ptr create(float radius, float depth, uint32_t sectors);

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

#endif // KIT_CONE_HPP