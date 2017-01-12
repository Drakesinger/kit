#pragma once 

#include "Kit/Transformable.hpp"

namespace  kit
{
  class Camera;
  
  class Texture;
  
  class Renderer;
  
  class KITAPI Renderable : public kit::Transformable
  {
  public:
    
    Renderable();
    virtual ~Renderable();
    virtual void renderDeferred(Renderer *);
    virtual void renderForward(Renderer *);
    virtual void renderShadows(glm::mat4 const & viewMatrix, glm::mat4 const & projectionMatrix);
    virtual void renderGeometry();
    virtual void renderReflection(Renderer *, glm::mat4 const & viewMatrix, glm::mat4 const & projectionMatrix);
    
    virtual bool isShadowCaster();
    virtual void setShadowCaster(bool s);
    
    virtual bool isSkinned();
    virtual std::vector<glm::mat4> getSkin();
    
    virtual int32_t getRenderPriority(); // Lower values are rendered first

    virtual bool requestAccumulationCopy();
    virtual bool requestPositionBuffer();

  private:
    bool m_shadowCaster;
  };
}
