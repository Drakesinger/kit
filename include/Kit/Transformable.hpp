#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace kit 
{
  
  class KITAPI Transformable 
  {
    public:
      
      enum class RotationOrder : uint8_t
      {
        XYZ,
        XZY,
        YXZ,
        YZX,
        ZXY,
        ZYX
      };
      
      Transformable();
    
      Transformable * getParent();
      
      void attachTo(Transformable * parent);
      
      glm::mat4 getWorldRotationMatrix();
      glm::mat4 getLocalRotationMatrix();
     
      glm::mat4 getWorldTransformMatrix();
      glm::mat4 getLocalTransformMatrix();

      void      setPosition(glm::vec3 const & pos);
      glm::vec3 const & getLocalPosition();
      glm::vec3 getWorldPosition();
      void      translate(glm::vec3 const & offset);
      
      glm::vec3 getLocalEuler(); // euler
      glm::vec3 getWorldEuler(); // euler
      void      setEuler(glm::vec3 const & euler_angles, RotationOrder = RotationOrder::ZXY);

      glm::quat const & getLocalRotation(); // quat
      glm::quat getWorldRotation();
      void      setRotation(glm::quat const & quat);
      void      rotateX(float const & degrees);
      void      rotateY(float const & degrees);
      void      rotateZ(float const & degrees);
      
      void      setDirection(glm::vec3 const &);

      void      setScale(glm::vec3 const &);
      glm::vec3 const & getScale();
      void      scale(glm::vec3 const &);

      glm::vec3 getLocalForward();
      glm::vec3 getWorldForward();
      glm::vec3 getLocalRight();
      glm::vec3 getWorldRight();
      glm::vec3 getLocalUp();
      glm::vec3 getWorldUp();
      //void SetDirection(glm::vec3 direction_vector);
      

    private:
      Transformable * m_parent = nullptr;
      
      glm::mat4 m_transformMatrix       = glm::mat4(1.0);
      bool      m_transformMatrixDirty  = true;
      
      glm::quat  m_rotation = glm::quat();
      glm::vec3  m_position = glm::vec3(0.0, 0.0, 0.0);
      glm::vec3  m_scale    = glm::vec3(1.0, 1.0, 1.0);
  };
  
}
