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
