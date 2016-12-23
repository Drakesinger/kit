#include "Kit/Light.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/PixelBuffer.hpp"
#include "Kit/Cubemap.hpp"
#include "Kit/Cone.hpp"

#include <glm/gtx/transform.hpp>

kit::Light::Light(kit::Light::Type t, glm::uvec2 shadowmapsize) : kit::Transformable()
{

  m_type = t;
  m_shadowMapped = false;
  m_shadowBuffer = nullptr;
  m_maxShadowDistance = 40.0f;
  m_color = glm::vec3(1.0f, 1.0f, 1.0f);
  
  if (m_type == kit::Light::Point)
  {
    setRadius(1.0);
  }

  if (m_type == kit::Light::Spot)
  {
    setRadius(1.0);
    m_coneInner = 30.0f;
    m_coneOuter = 45.0f;
    updateCone();

    if (shadowmapsize != glm::uvec2(0, 0))
    {
      m_shadowBuffer = kit::PixelBuffer::createShadowBuffer(shadowmapsize);
      m_shadowMapped = true;
    }
  }
  
  if (m_type == kit::Light::Directional)
  {
    if (shadowmapsize != glm::uvec2(0, 0))
    {
      m_shadowBuffer = kit::PixelBuffer::createShadowBuffer(shadowmapsize);
      m_shadowMapped = true;
    }
  }
}

kit::Light::~Light()
{
  if(m_shadowBuffer)
    delete m_shadowBuffer;
  
  if(m_spotGeometry)
    delete m_spotGeometry;
  
  if(m_irradianceMap)
    delete m_irradianceMap;
  
  if(m_radianceMap)
    delete m_radianceMap;
}

kit::PixelBuffer * kit::Light::getShadowBuffer()
{
  return m_shadowBuffer;
}

glm::vec3 kit::Light::getColor()
{
  return m_color;
}

void kit::Light::setColor(glm::vec3 c)
{
  m_color = c;
}

float const & kit::Light::getRadius()
{
  if (m_type != kit::Light::Spot && m_type != kit::Light::Point)
  {
    KIT_THROW("Light-type is not spot/point, can't get radius");
  }

  return m_radius;
}

void kit::Light::setRadius(float r)
{
  if (m_type != kit::Light::Spot && m_type != kit::Light::Point)
  {
    KIT_ERR("Refuse to set light radius, not a spot/point light");
    return;
  }

  m_radius = r;
  if (m_radius < 0.5f)
  {
    m_radius = 0.5f;
  }
  float Range = m_radius / 2.0f;
  m_attenuation = glm::vec4(Range, 1.0f, 4.5f / Range, 75.0f / (Range*Range));

  if (m_type == kit::Light::Point)
  {
    setScale(glm::vec3(m_radius, m_radius, m_radius));
  }

  if (m_type == kit::Light::Spot)
  {
    updateCone();
  }
}

kit::Cubemap * kit::Light::getIrradianceMap()
{
  if (m_type != kit::Light::IBL)
  {
    KIT_THROW("Light-type is not IBL, can't get irradiance map");
  }

  return m_irradianceMap;
}

kit::Cubemap * kit::Light::getRadianceMap()
{
  if (m_type != kit::Light::IBL)
  {
    KIT_THROW("Light-type is not IBL, can't get radiance map");
  }

  return m_radianceMap;
}

glm::vec2 const  kit::Light::getConeAngle()
{
  if (m_type != kit::Light::Spot)
  {
    KIT_THROW("Light-type is not spot, can't get cone angle");
  }

  return glm::vec2(m_coneInner, m_coneOuter);
}

void kit::Light::setConeAngle(float inner, float outer)
{
  if (m_type != kit::Light::Spot)
  {
    KIT_ERR("Refuse to set light cone angle, not a spotlight");
    return;
  }

  m_coneInner = inner;
  m_coneOuter = outer;
  if (m_coneInner < 1.000f)
  {
    m_coneInner = 1.0f;
  }

  if (m_coneOuter < m_coneInner)
  { 
    m_coneOuter = m_coneInner+0.1f;
  }

  if (m_coneOuter > 179.0f)
  {
    m_coneOuter = 179.0f;
  }
  if (m_coneOuter > 180.0f)
  {
    m_coneOuter = 180.0f;
  }

  updateCone();
}

void kit::Light::updateCone()
{
  if (m_type != kit::Light::Spot)
  {
    KIT_ERR("Refuse to update light cone, not a spotlight");
    return;
  }

  float height = m_radius;
  float coneRadiusAngle = glm::radians(m_coneOuter) / 2.0f;
  float rad = glm::tan(coneRadiusAngle) * height;
  m_spotGeometry = new kit::Cone(rad, height, 32);
}

bool kit::Light::isShadowMapped()
{
  return m_shadowMapped;
}

const kit::Light::Type& kit::Light::getType()
{
  return m_type;
}

glm::mat4 kit::Light::getPointProjectionMatrix()
{
  return glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, m_radius);
}

glm::mat4 kit::Light::getPointViewMatrix(kit::Cubemap::Side s)
{  
  switch(s)
  {
    default:
    case Cubemap::Count:
    case Cubemap::PositiveX:
      return glm::lookAt(getPosition(), getPosition() + glm::vec3(1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0));
      break;
    case Cubemap::NegativeX:
      return glm::lookAt(getPosition(), getPosition() + glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0));
      break;
    case Cubemap::PositiveY:
      return glm::lookAt(getPosition(), getPosition() + glm::vec3(0.0,1.0,0.0), glm::vec3(0.0,0.0,1.0));
      break;
    case Cubemap::NegativeY:
      return glm::lookAt(getPosition(), getPosition() + glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0));
      break;
    case Cubemap::PositiveZ:
      return glm::lookAt(getPosition(), getPosition() + glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,-1.0,0.0));
      break;
    case Cubemap::NegativeZ:
      return glm::lookAt(getPosition(), getPosition() + glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,-1.0,0.0));
      break;
  }
  KIT_THROW("No such side");
  return glm::mat4();
}

glm::mat4 kit::Light::getSpotProjectionMatrix()
{
  return glm::perspective(glm::radians(m_coneOuter), 1.0f, 0.05f, m_radius);
}

glm::mat4 kit::Light::getSpotViewMatrix()
{
  return glm::lookAt(getPosition(), getPosition() + getForward(), glm::vec3(0, 1, 0));
}


glm::mat4 kit::Light::getDirectionalProjectionMatrix()
{
  float boxSize = m_maxShadowDistance;
  return glm::ortho(-boxSize, boxSize, -boxSize, boxSize, -boxSize, boxSize);
}

glm::mat4 kit::Light::getDirectionalViewMatrix()
{
  glm::mat4 depthViewMatrix = glm::lookAt(-getForward(), glm::vec3(0,0,0), glm::vec3(0,1,0));

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
  m_maxShadowDistance = f;
}

float kit::Light::getMaxShadowDistance()
{
  return m_maxShadowDistance;
}

kit::Cone * kit::Light::getSpotGeometry()
{
  return m_spotGeometry;
}

glm::vec4 kit::Light::getAttenuation()
{
  return m_attenuation;
}


void kit::Light::setEnvironment(const std::string&name)
{
  if (m_type != kit::Light::IBL)
  {
    KIT_ERR("Refuse to set light environment map, not an image-based light");
    return;
  }
  
  m_radianceMap = kit::Cubemap::loadRadianceMap(name);
  m_irradianceMap = kit::Cubemap::loadIrradianceMap(name);
  //m_reflectionMap = kit::Cubemap::loadSkybox(name);
  
}
