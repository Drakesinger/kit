#ifndef KIT_CAMERA_HPP
#define KIT_CAMERA_HPP
#include "Kit/Export.hpp"
#include "Kit/Transformable.hpp"

#include<memory>

namespace kit 
{
  class KITAPI Camera : public kit::Transformable
  {
    public: 
      
      typedef std::shared_ptr<Camera> Ptr;

      static kit::Camera::Ptr create(float fov, float aspect_ratio, glm::vec2 cliprange);
  
      glm::mat4 getViewMatrix();
      const glm::mat4 & getProjectionMatrix();
      
      void setFov(float fov);
      float getFov();
      
      void setAspectRatio(float aspect);
      float getAspectRatio();
      
      void setClipRange(glm::vec2 cliprange);
      glm::vec2 getClipRange();

      void setExposure(float exposure);
      void setWhitepoint(float whitepoint);

      float getExposure();
      float getWhitepoint();

      Camera(float fov, float aspect_ratio, glm::vec2 cliprange);
      ~Camera();

    protected:
      void rebuildProjectionMatrix();
      
      glm::mat4 m_projectionMatrix;
      float m_fov;
      float m_aspect;
      glm::vec2 m_clipRange;
      float                       m_exposure;
      float                       m_whitepoint;
  };
  
}

#endif