#ifndef KIT_TRANSFORMABLE_HPP
#define KIT_TRANSFORMABLE_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

namespace kit 
{
  
  class KITAPI Transformable 
  {
    public:
      typedef std::shared_ptr<Transformable> Ptr;
      typedef std::weak_ptr<Transformable> WPtr;

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
    
      Transformable::Ptr getParent();
      
      void attachTo(Transformable::Ptr parent);
      
      glm::mat4 getRotationMatrix();
      glm::mat4 getTransformMatrix();

      void      setPosition(glm::vec3 pos);
      glm::vec3 getPosition();
      void      translate(glm::vec3 offset);
      
      glm::vec3 getEuler(); // euler
      void      setEuler(glm::vec3 euler_angles, RotationOrder = RotationOrder::ZXY);

      glm::quat getRotation(); // quat
      void      setRotation(glm::quat quat);
      void      rotateX(float degrees);
      void      rotateY(float degrees);
      void      rotateZ(float degrees);
      
      void      setDirection(glm::vec3);

      void      setScale(glm::vec3);
      glm::vec3 getScale();
      void      scale(glm::vec3);

      glm::vec3 getForward();
      glm::vec3 getRight();
      glm::vec3 getUp();
      //void SetDirection(glm::vec3 direction_vector);
      

    private:
      Transformable::WPtr m_parent;
      
      glm::mat4 m_transformMatrix;
      bool m_transformMatrixDirty;
      
      glm::quat  m_rotation;
      glm::vec3  m_position;
      glm::vec3  m_scale;
  };
  
}

#endif // KIT_TRANSFORMABLE_HPP
