#include "Kit/Renderable.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Texture.hpp"

void kit::Renderable::renderDeferred(kit::RendererPtr renderer)
{

}

void kit::Renderable::renderForward(kit::RendererPtr renderer)
{

}

void kit::Renderable::renderShadows(glm::mat4 v, glm::mat4 p)
{

}

void kit::Renderable::renderGeometry()
{

}

bool kit::Renderable::isShadowCaster()
{
  return this->m_shadowCaster;
}

void kit::Renderable::setShadowCaster(bool s)
{
  this->m_shadowCaster = s;
}

kit::Renderable::Renderable()
{
  this->m_shadowCaster = true;
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