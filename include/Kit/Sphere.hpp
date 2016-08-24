#ifndef KIT_SPHERE_HPP
#define KIT_SPHERE_HPP


#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <memory>

namespace kit
{
  class KITAPI Sphere
  {
  public:
    typedef std::shared_ptr<kit::Sphere> Ptr;

    Sphere(uint32_t rings, uint32_t sectors);
    ~Sphere();

    static kit::Sphere::Ptr create(uint32_t rings, uint32_t sectors);

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

#endif // KIT_SPHERE_HPP