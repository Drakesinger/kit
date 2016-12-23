#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/Renderable.hpp"

namespace kit
{
  class Program;
  
  class Camera;
  
  class Texture;
  
  class Renderer;

  class KITAPI GridFloor : public kit::Renderable
  {
  public:
    

    GridFloor();
    ~GridFloor();

    void renderForward(kit::Renderer * renderer) override;
    void renderGeometry() override;
    
    bool isShadowCaster() override;
    
  private:


    static uint32_t m_instanceCount;
    
    static void allocateShared();
    static void releaseShared();

    static uint32_t m_glVertexArray;
    static uint32_t m_glVertexIndices;
    static uint32_t m_glVertexBuffer;

    static kit::Program * m_program;
    
    static uint32_t m_indexCount;
  };
}
