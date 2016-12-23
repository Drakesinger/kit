#include "Kit/Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>

kit::Camera::Camera(float fov, float aspect_ratio, glm::vec2 cliprange)
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
  return glm::inverse(this->getTransformMatrix());
}

void kit::Camera::setClipRange(glm::vec2 cliprange)
{
  this->m_clipRange = cliprange;
  this->rebuildProjectionMatrix();
}

glm::vec2 kit::Camera::getClipRange()
{
  return this->m_clipRange;
}

void kit::Camera::setFov(float fov)
{
  this->m_fov = fov;
  this->rebuildProjectionMatrix();
}

float kit::Camera::getFov()
{
  return this->m_fov;
}

void kit::Camera::setAspectRatio(float aspect)
{
  this->m_aspect = aspect;
  this->rebuildProjectionMatrix();
}

float kit::Camera::getAspectRatio()
{
  return this->m_aspect;
}

void kit::Camera::setWhitepoint(float w)
{
  this->m_whitepoint = w;
}

void kit::Camera::setExposure(float e)
{
  this->m_exposure = e;
}

float kit::Camera::getWhitepoint()
{
  return this->m_whitepoint;
}

float kit::Camera::getExposure()
{
  return this->m_exposure;
}

void kit::Camera::rebuildProjectionMatrix()
{
  this->m_projectionMatrix = glm::perspective(glm::radians(this->m_fov), this->m_aspect, this->m_clipRange.x, this->m_clipRange.y);
}

