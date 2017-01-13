#include "Kit/Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>

kit::Camera::Camera(float const & fov, float const & aspect_ratio, glm::vec2 const & cliprange)
{
  this->m_fov = fov;
  this->m_aspect = aspect_ratio;
  this->m_clipRange = cliprange;
  this->m_projectionMatrix = glm::mat4(1.0);
  this->m_exposure = 1.0f;
  this->m_whitepoint = 1.0f;
  this->rebuildProjectionMatrix();
}

kit::Camera::~Camera()
{

}

const glm::mat4 & kit::Camera::getProjectionMatrix()
{
  return this->m_projectionMatrix;
}

glm::mat4 kit::Camera::getViewMatrix()
{
  return glm::inverse(this->getWorldTransformMatrix());
}

void kit::Camera::setClipRange(glm::vec2 const & cliprange)
{
  this->m_clipRange = cliprange;
  this->rebuildProjectionMatrix();
}

glm::vec2 const & kit::Camera::getClipRange()
{
  return this->m_clipRange;
}

void kit::Camera::setFov(float const & fov)
{
  this->m_fov = fov;
  this->rebuildProjectionMatrix();
}

float const & kit::Camera::getFov()
{
  return this->m_fov;
}

void kit::Camera::setAspectRatio(float const & aspect)
{
  this->m_aspect = aspect;
  this->rebuildProjectionMatrix();
}

float const & kit::Camera::getAspectRatio()
{
  return this->m_aspect;
}

void kit::Camera::setWhitepoint(float const & w)
{
  this->m_whitepoint = w;
}

void kit::Camera::setExposure(float const & e)
{
  this->m_exposure = e;
}

float const & kit::Camera::getWhitepoint()
{
  return this->m_whitepoint;
}

float const & kit::Camera::getExposure()
{
  return this->m_exposure;
}

void kit::Camera::rebuildProjectionMatrix()
{
  this->m_projectionMatrix = glm::perspective(glm::radians(this->m_fov), this->m_aspect, this->m_clipRange.x, this->m_clipRange.y);
}

