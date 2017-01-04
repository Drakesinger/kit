#include "Kit/Transformable.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stack>

kit::Transformable::Transformable()
{
  m_transformMatrix = glm::mat4(1.0);
  m_transformMatrixDirty = true;
  m_position = glm::vec3(0.0, 0.0, 0.0);
  m_rotation = glm::quat();
  m_scale = glm::vec3(1.0, 1.0, 1.0);
}

void kit::Transformable::attachTo(kit::Transformable * parent)
{
  m_parent = parent;
}

kit::Transformable * kit::Transformable::getParent() 
{
  return m_parent;
}


glm::mat4 kit::Transformable::getTransformMatrix()
{
  if(m_transformMatrixDirty)
  {   
    glm::mat4 translationMatrix = glm::translate(m_position);
    glm::mat4 rotationMatrix = glm::toMat4(getRotation());
    glm::mat4 scaleMatrix = glm::scale(m_scale);
    
    m_transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
    m_transformMatrixDirty = false;
  }
  
  if (m_parent != nullptr)
  {
    return m_transformMatrix * m_parent->getTransformMatrix();
  }

  return m_transformMatrix;
}

glm::mat4 kit::Transformable::getRotationMatrix()
{
  return glm::toMat4(getRotation());
}

void kit::Transformable::setPosition(glm::vec3 pos)
{
  m_position = pos;
  m_transformMatrixDirty = true;
}

glm::vec3 kit::Transformable::getPosition()
{ 
  return m_position;
}

void kit::Transformable::translate(glm::vec3 offset)
{
  m_position += offset;
  m_transformMatrixDirty = true;
}

glm::vec3 kit::Transformable::getEuler()
{
  auto e = glm::eulerAngles(getRotation());
  return glm::degrees(e);
}

void kit::Transformable::setEuler(glm::vec3 r, kit::Transformable::RotationOrder rotationOrder)
{
  m_rotation = glm::quat();

  switch(rotationOrder)
  {
    case kit::Transformable::RotationOrder::XYZ:
      m_rotation = glm::rotate(m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      break;

    case kit::Transformable::RotationOrder::XZY:
      m_rotation = glm::rotate(m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      break;

    case kit::Transformable::RotationOrder::YXZ:
      m_rotation = glm::rotate(m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      break;

    case kit::Transformable::RotationOrder::YZX:
      m_rotation = glm::rotate(m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      break;

    default:
    case kit::Transformable::RotationOrder::ZXY:
      m_rotation = glm::rotate(m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      break;

    case kit::Transformable::RotationOrder::ZYX:
      m_rotation = glm::rotate(m_rotation, glm::radians(r.z), glm::vec3(0.0f, 0.0f, 1.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.y), glm::vec3(0.0f, 1.0f, 0.0f));
      m_rotation = glm::rotate(m_rotation, glm::radians(r.x), glm::vec3(1.0f, 0.0f, 0.0f));
      break;
  }

  
  m_transformMatrixDirty = true;
}

glm::quat kit::Transformable::getRotation()
{
  return m_rotation;
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
    m_rotation = glm::rotate(glm::quat(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  }
  else
  {
    glm::vec3 xv = glm::cross(glm::vec3(0.0, 1.0, 0.0), adjustVec);
    xv = glm::normalize(xv);
    
    glm::vec3 yv = glm::cross(adjustVec, xv);
    yv = glm::normalize(yv);
    
    glm::mat3 mat(xv, yv, adjustVec);
    
    m_rotation = glm::quat_cast(mat);
    
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
  
  m_transformMatrixDirty = true;
}

void kit::Transformable::setRotation(glm::quat quat)
{
  m_rotation = quat;
  m_transformMatrixDirty = true;
}

void kit::Transformable::rotateX(float degrees)
{
  m_rotation = glm::rotate(getRotation(), glm::radians(degrees), glm::vec3(1.0, 0.0, 0.0));
  m_transformMatrixDirty = true;
}

void kit::Transformable::rotateY(float degrees)
{
  m_rotation = glm::rotate(getRotation(), glm::radians(degrees), glm::vec3(0.0, 1.0, 0.0));
  m_transformMatrixDirty = true;
}

void kit::Transformable::rotateZ(float degrees)
{
  m_rotation = glm::rotate(getRotation(), glm::radians(degrees), glm::vec3(0.0, 0.0, 1.0));
  m_transformMatrixDirty = true;
}


glm::vec3 kit::Transformable::getScale()
{
  return m_scale;
}

void kit::Transformable::setScale(glm::vec3 s)
{
  m_scale = s;
  m_transformMatrixDirty = true;
}

void kit::Transformable::scale(glm::vec3 s)
{
  m_scale += s;
  m_transformMatrixDirty = true;
}

glm::vec3 kit::Transformable::getForward()
{
  return getRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 kit::Transformable::getRight()
{
  return getRotation() * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 kit::Transformable::getUp()
{
  return getRotation() * glm::vec3(0.0f, 1.0f, 0.0f);
}
