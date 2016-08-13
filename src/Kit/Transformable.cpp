#include "Kit/Transformable.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stack>

kit::Transformable::Transformable()
{
  this->m_transformMatrix = glm::mat4(1.0);
  this->m_transformMatrixDirty = true;
  this->m_position = glm::vec3(0.0, 0.0, 0.0);
  this->m_rotation = glm::quat();
  this->m_scale = glm::vec3(1.0, 1.0, 1.0);
}

void kit::Transformable::attachTo(kit::Transformable::Ptr parent)
{
  this->m_parent = parent;
}

kit::Transformable::Ptr kit::Transformable::getParent() 
{
  return this->m_parent.lock();
}


glm::mat4 kit::Transformable::getTransformMatrix()
{
  if(this->m_transformMatrixDirty)
  {   
    glm::mat4 translationMatrix = glm::translate(this->m_position);
    glm::mat4 rotationMatrix = glm::toMat4(this->getRotation());
    glm::mat4 scaleMatrix = glm::scale(this->m_scale);
    
    this->m_transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
    this->m_transformMatrixDirty = false;
  }
  
  if (this->m_parent.lock() != nullptr)
  {
    return this->m_parent.lock()->getTransformMatrix() * this->m_transformMatrix;
  }

  return this->m_transformMatrix;
}

glm::mat4 kit::Transformable::getRotationMatrix()
{
  return glm::toMat4(this->getRotation());
}

void kit::Transformable::setPosition(glm::vec3 pos)
{
  this->m_position = pos;
  this->m_transformMatrixDirty = true;
}

glm::vec3 kit::Transformable::getPosition()
{ 
  return this->m_position;
}

void kit::Transformable::translate(glm::vec3 offset)
{
  this->m_position += offset;
  this->m_transformMatrixDirty = true;
}

glm::vec3 kit::Transformable::getEuler()
{
  auto e = glm::eulerAngles(this->getRotation());
  return glm::degrees(e);
}

void kit::Transformable::setEuler(glm::vec3 r, kit::Transformable::RotationOrder rotationOrder)
{
  this->m_rotation = glm::quat();

  switch(rotationOrder)
  {
    case kit::Transformable::RotationOrder::XYZ:
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      break;

    case kit::Transformable::RotationOrder::XZY:
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      break;

    case kit::Transformable::RotationOrder::YXZ:
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      break;

    case kit::Transformable::RotationOrder::YZX:
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      break;

    default:
    case kit::Transformable::RotationOrder::ZXY:
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      break;

    case kit::Transformable::RotationOrder::ZYX:
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      this->m_rotation = glm::rotate(this->m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      break;
  }

  
  this->m_transformMatrixDirty = true;
}

glm::quat kit::Transformable::getRotation()
{
  return this->m_rotation;
}

void kit::Transformable::setDirection(glm::vec3 d)
{
  if(d == glm::vec3(0.0, 0.0, 0.0))
  {
    return;
  }
  
  glm::vec3 adjustVec = -d;
  adjustVec = glm::normalize(adjustVec);
  
  if(adjustVec == glm::vec3(0.0, 1.0, 0.0))
  {
    this->m_rotation = glm::rotate(glm::quat(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  }
  else
  {
    glm::vec3 xv = glm::cross(glm::vec3(0.0, 1.0, 0.0), adjustVec);
    xv = glm::normalize(xv);
    
    glm::vec3 yv = glm::cross(adjustVec, xv);
    yv = glm::normalize(yv);
    
    glm::mat3 mat(xv, yv, adjustVec);
    
    this->m_rotation = glm::quat_cast(mat);
    
    /*column1.x = xaxis.x;
    column1.y = yaxis.x;
    column1.z = direction.x;

    column2.x = xaxis.y;
    column2.y = yaxis.y;
    column2.z = direction.y;

    column3.x = xaxis.z;
    column3.y = yaxis.z;
    column3.z = direction.z;*/
    
  }
  
  this->m_transformMatrixDirty = true;
}

void kit::Transformable::setRotation(glm::quat quat)
{
  this->m_rotation = quat;
  this->m_transformMatrixDirty = true;
}

void kit::Transformable::rotateX(float degrees)
{
  this->m_rotation = glm::rotate(this->getRotation(), glm::radians(degrees), glm::vec3(1.0, 0.0, 0.0));
  this->m_transformMatrixDirty = true;
}

void kit::Transformable::rotateY(float degrees)
{
  this->m_rotation = glm::rotate(this->getRotation(), glm::radians(degrees), glm::vec3(0.0, 1.0, 0.0));
  this->m_transformMatrixDirty = true;
}

void kit::Transformable::rotateZ(float degrees)
{
  this->m_rotation = glm::rotate(this->getRotation(), glm::radians(degrees), glm::vec3(0.0, 0.0, 1.0));
  this->m_transformMatrixDirty = true;
}


glm::vec3 kit::Transformable::getScale()
{
  return this->m_scale;
}

void kit::Transformable::setScale(glm::vec3 s)
{
  this->m_scale = s;
  this->m_transformMatrixDirty = true;
}

void kit::Transformable::scale(glm::vec3 s)
{
  this->m_scale += s;
  this->m_transformMatrixDirty = true;
}

glm::vec3 kit::Transformable::getForward()
{
  return this->getRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 kit::Transformable::getRight()
{
  return this->getRotation() * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 kit::Transformable::getUp()
{
  return this->getRotation() * glm::vec3(0.0f, 1.0f, 0.0f);
}