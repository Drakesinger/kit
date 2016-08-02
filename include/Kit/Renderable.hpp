#ifndef KIT_RENDERABLE_HPP
#define KIT_RENDERABLE_HPP

#include "Kit/Transformable.hpp"

#include <memory>

namespace  kit
{
  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;

  class Renderer;
  typedef std::shared_ptr<Renderer> RendererPtr;

  class KITAPI Renderable : public kit::Transformable
  {
  public:
    typedef std::shared_ptr<Renderable> Ptr;
    Renderable();
    virtual ~Renderable();
    virtual void renderDeferred(RendererPtr);
    virtual void renderForward(RendererPtr);
    virtual void renderShadows(glm::mat4 viewmatrix, glm::mat4 projectionmatrix);
    virtual void renderGeometry();
    
    virtual bool isShadowCaster();
    virtual void setShadowCaster(bool s);
    
    virtual bool isSkinned();
    virtual std::vector<glm::mat4> getSkin();
    
    virtual int32_t getRenderPriority(); // Lower values are rendered first

    virtual bool requestAccumulationCopy();

  private:
    bool m_shadowCaster;
  };
}

#endif