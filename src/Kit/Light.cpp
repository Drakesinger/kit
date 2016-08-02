#include "Kit/Light.hpp"

#include "Kit/PixelBuffer.hpp"
#include "Kit/Cubemap.hpp"
#include "Kit/Cone.hpp"

#include <glm/gtx/transform.hpp>

kit::Light::Light(kit::Light::Type t, glm::uvec2 shadowmapsize) : kit::Transformable()
{

  this->m_type = t;
  this->m_shadowMapped = false;
  this->m_shadowBuffer = nullptr;
  this->m_maxShadowDistance = 40.0f;
  this->m_color = glm::vec3(1.0f, 1.0f, 1.0f);
  
  if (this->m_type == kit::Light::Point)
  {
    this->setRadius(1.0);
  }

  if (this->m_type == kit::Light::Spot)
  {
    this->setRadius(1.0);
    this->m_coneInner = 30.0f;
    this->m_coneOuter = 45.0f;
    this->updateCone();

    if (shadowmapsize != glm::uvec2(0, 0))
    {
      this->m_shadowBuffer = kit::PixelBuffer::createShadowBuffer(shadowmapsize);
      this->m_shadowMapped = true;
    }
  }
  
  if (this->m_type == kit::Light::Directional)
  {
    if (shadowmapsize != glm::uvec2(0, 0))
    {
      this->m_shadowBuffer = kit::PixelBuffer::createShadowBuffer(shadowmapsize);
      this->m_shadowMapped = true;
    }
  }
}

kit::Light::~Light()
{
  
}

kit::Light::Ptr kit::Light::create(kit::Light::Type t, glm::uvec2 shadowmapsize)
{
  //return kit::Light::Ptr(new kit::Light(t));
  return std::make_shared<kit::Light>(t, shadowmapsize);
}

kit::PixelBuffer::Ptr kit::Light::getShadowBuffer()
{
  return this->m_shadowBuffer;
}

glm::vec3 kit::Light::getColor()
{
  return this->m_color;
}

void kit::Light::setColor(glm::vec3 c)
{
  this->m_color = c;
}

float const & kit::Light::getRadius()
{
  if (this->m_type != kit::Light::Spot && this->m_type != kit::Light::Point)
  {
    KIT_THROW("Light-type is not spot/point, can't get radius");
  }

  return this->m_radius;
}

void kit::Light::setRadius(float r)
{
  if (this->m_type != kit::Light::Spot && this->m_type != kit::Light::Point)
  {
    KIT_ERR("Refuse to set light radius, not a spot/point light");
    return;
  }

  this->m_radius = r;
  if (this->m_radius < 0.5f)
  {
    this->m_radius = 0.5f;
  }
  float Range = this->m_radius / 2.0f;
  this->m_attenuation = glm::vec4(Range, 1.0f, 4.5f / Range, 75.0f / (Range*Range));

  if (this->m_type == kit::Light::Point)
  {
    this->setScale(glm::vec3(this->m_radius, this->m_radius, this->m_radius));
  }

  if (this->m_type == kit::Light::Spot)
  {
    this->updateCone();
  }
}

kit::Cubemap::Ptr kit::Light::getIrradianceMap()
{
  if (this->m_type != kit::Light::IBL)
  {
    KIT_THROW("Light-type is not IBL, can't get irradiance map");
  }

  return this->m_irradianceMap;
}

void kit::Light::setIrradianceMap(kit::Cubemap::Ptr c)
{
  if (this->m_type != kit::Light::IBL)
  {
    KIT_ERR("Refuse to set light irradiance map, not an image-based light");
    return;
  }

 this->m_irradianceMap = c; 
}

kit::Cubemap::Ptr kit::Light::getRadianceMap()
{
  if (this->m_type != kit::Light::IBL)
  {
    KIT_THROW("Light-type is not IBL, can't get radiance map");
  }

  return this->m_radianceMap;
}

void kit::Light::setRadianceMap(kit::Cubemap::Ptr radiance)
{
  if (this->m_type != kit::Light::IBL)
  {
    KIT_ERR("Refuse to set light radiance map, not an image-based light");
    return;
  }

  this->m_radianceMap = radiance;
}

kit::Cubemap::Ptr kit::Light::getReflectionMap()
{
  if (this->m_type != kit::Light::IBL)
  {
    KIT_THROW("Light-type is not IBL, can't get radiance map");
  }

  return this->m_reflectionMap;
}

void kit::Light::setReflectionMap(kit::Cubemap::Ptr r)
{
  if (this->m_type != kit::Light::IBL)
  {
    KIT_ERR("Refuse to set light radiance map, not an image-based light");
    return;
  }

  this->m_reflectionMap = r;
}

glm::vec2 const  kit::Light::getConeAngle()
{
  if (this->m_type != kit::Light::Spot)
  {
    KIT_THROW("Light-type is not spot, can't get cone angle");
  }

  return glm::vec2(this->m_coneInner, this->m_coneOuter);
}

void kit::Light::setConeAngle(float inner, float outer)
{
  if (this->m_type != kit::Light::Spot)
  {
    KIT_ERR("Refuse to set light cone angle, not a spotlight");
    return;
  }

  this->m_coneInner = inner;
  this->m_coneOuter = outer;
  if (this->m_coneInner < 1.000f)
  {
    this->m_coneInner = 1.0f;
  }

  if (this->m_coneOuter < this->m_coneInner)
  { 
    this->m_coneOuter = this->m_coneInner+0.1f;
  }

  if (this->m_coneOuter > 179.0f)
  {
    this->m_coneOuter = 179.0f;
  }
  if (this->m_coneOuter > 180.0f)
  {
    this->m_coneOuter = 180.0f;
  }

  this->updateCone();
}

void kit::Light::updateCone()
{
  if (this->m_type != kit::Light::Spot)
  {
    KIT_ERR("Refuse to update light cone, not a spotlight");
    return;
  }

  float height = this->m_radius;
  float coneRadiusAngle = glm::radians(this->m_coneOuter) / 2.0f;
  float rad = glm::tan(coneRadiusAngle) * height;
  this->m_spotGeometry = kit::Cone::create(rad, height, 32);
}

bool kit::Light::isShadowMapped()
{
  return this->m_shadowMapped;
}

const kit::Light::Type& kit::Light::getType()
{
  return this->m_type;
}

glm::mat4 kit::Light::getPointProjectionMatrix()
{
  return glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, this->m_radius);
}

glm::mat4 kit::Light::getPointViewMatrix(kit::Cubemap::Side s)
{  
  switch(s)
  {

    case Cubemap::PositiveX:
      return glm::lookAt(this->getPosition(), this->getPosition() + glm::vec3(1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0));
      break;
    case Cubemap::NegativeX:
      return glm::lookAt(this->getPosition(), this->getPosition() + glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0));
      break;
    case Cubemap::PositiveY:
      return glm::lookAt(this->getPosition(), this->getPosition() + glm::vec3(0.0,1.0,0.0), glm::vec3(0.0,0.0,1.0));
      break;
    case Cubemap::NegativeY:
      return glm::lookAt(this->getPosition(), this->getPosition() + glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0));
      break;
    case Cubemap::PositiveZ:
      return glm::lookAt(this->getPosition(), this->getPosition() + glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,-1.0,0.0));
      break;
    case Cubemap::NegativeZ:
      return glm::lookAt(this->getPosition(), this->getPosition() + glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,-1.0,0.0));
      break;
  }
  KIT_THROW("No such side");
  return glm::mat4();
}

glm::mat4 kit::Light::getSpotProjectionMatrix()
{
  return glm::perspective(glm::radians(this->m_coneOuter), 1.0f, 0.05f, this->m_radius);
}

glm::mat4 kit::Light::getSpotViewMatrix()
{
  return glm::lookAt(this->getPosition(), this->getPosition() + this->getForward(), glm::vec3(0, 1, 0));
}


glm::mat4 kit::Light::getDirectionalProjectionMatrix()
{
  float boxSize = this->m_maxShadowDistance;
  return glm::ortho(-boxSize, boxSize, -boxSize, boxSize, -boxSize, boxSize);
}

glm::mat4 kit::Light::getDirectionalViewMatrix()
{
  glm::mat4 depthViewMatrix = glm::lookAt(-this->getForward(), glm::vec3(0,0,0), glm::vec3(0,1,0));

  return depthViewMatrix;
}

glm::mat4 kit::Light::getDirectionalModelMatrix(glm::vec3 cameraPosition, glm::vec3 cameraForward)
{
  glm::mat4 depthModelMatrix = glm::translate(-cameraPosition);

  return depthModelMatrix;
}

//currLight->getDirectionalProjectionMatrix() * currLight->getDirectionalViewMatrix(-c->getPosition()


/*
Matrix viewProjectionMatrix = viewMatrix * projectionMatrix;
Vector3 ViewTarget = ...; // A point in front of the viewpoint camera.
Vector3 ShadowTarget = viewProjectionMatrix * ViewTarget;

ShadowTarget.XY = Round(ShadowTarget.XY / ShadowTexture.Size) * ShadowTexture.Size;
Matrix ShadowMatrix = BaseShadowMatrix * TranslationMatrix(ShadowTarget);
*/

void kit::Light::setMaxShadowDistance(float f)
{
  if(f < 0.001f)
  {
    f = 0.001f;
  }
  this->m_maxShadowDistance = f;
}

float kit::Light::getMaxShadowDistance()
{
  return this->m_maxShadowDistance;
}

kit::Cone::Ptr kit::Light::getSpotGeometry()
{
  return this->m_spotGeometry;
}

glm::vec4 kit::Light::getAttenuation()
{
  return this->m_attenuation;
}


void kit::Light::setEnvironment(std::string name)
{
  if (this->m_type != kit::Light::IBL)
  {
    KIT_ERR("Refuse to set light environment map, not an image-based light");
    return;
  }
  
  this->m_radianceMap = kit::Cubemap::loadRadianceMap(name);
  this->m_irradianceMap = kit::Cubemap::loadIrradianceMap(name);
  this->m_reflectionMap = kit::Cubemap::loadSkybox(name);
  
}
