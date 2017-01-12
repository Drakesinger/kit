#include "Kit/Renderable.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Texture.hpp"

void kit::Renderable::renderDeferred(kit::Renderer * renderer)
{

}

void kit::Renderable::renderForward(kit::Renderer * renderer)
{

}

void kit::Renderable::renderShadows(glm::mat4 const & viewMatrix, glm::mat4 const & projectionMatrix)
{

}

void kit::Renderable::renderGeometry()
{

}

void kit::Renderable::renderReflection(Renderer *, glm::mat4 const & viewMatrix, glm::mat4 const & projectionMatrix)
{
  
}

bool kit::Renderable::isShadowCaster()
{
  return m_shadowCaster;
}

void kit::Renderable::setShadowCaster(bool s)
{
  m_shadowCaster = s;
}

kit::Renderable::Renderable()
{
  m_shadowCaster = true;
}

kit::Renderable::~Renderable()
{

}

bool kit::Renderable::isSkinned()
{
  return false;
}

std::vector<glm::mat4> kit::Renderable::getSkin()
{
  return std::vector<glm::mat4>();
}

int32_t kit::Renderable::getRenderPriority()
{
  return 0;
}

bool kit::Renderable::requestAccumulationCopy()
{
  return false;
}

bool kit::Renderable::requestPositionBuffer()
{
  return false;
}

