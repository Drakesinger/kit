#ifndef KIT_GRIDFLOOR_HPP
#define KIT_GRIDFLOOR_HPP

#include "Kit/GL.hpp"
#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/Renderable.hpp"

#include <memory>

namespace kit
{
  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;

  class Renderer;
  typedef std::shared_ptr<Renderer> RendererPtr;


  class KITAPI GridFloor : public kit::Renderable
  {
  public:
    typedef std::shared_ptr<kit::GridFloor> Ptr;

    GridFloor();
    ~GridFloor();

    static kit::GridFloor::Ptr create();

    void renderForward(kit::RendererPtr renderer) override;
    void renderGeometry() override;
    
    bool isShadowCaster();
    
  private:


    static uint32_t m_instanceCount;
    
    static void allocateShared();
    static void releaseShared();

    static GLuint m_glVertexArray;
    static GLuint m_glVertexIndices;
    static GLuint m_glVertexBuffer;

    static kit::ProgramPtr m_program;
    
    static uint32_t m_indexCount;
  };
}

#endif // KIT_SPHERE_HPP