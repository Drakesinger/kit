#pragma once

#include "Kit/Export.hpp"
#include "Kit/Transformable.hpp"

namespace kit 
{
  ///
  /// \brief Camera class
  ///
  class KITAPI Camera : public kit::Transformable
  {
    public: 

      ///
      /// \brief Constructor
      ///
      /// \param fov Vertical field-of-view
      /// \param aspect_ratio Aspect ratio
      /// \param cliprange Cliprange. near in x, far in y.
      ///
      Camera(float fov, float aspect_ratio, glm::vec2 cliprange);

      ///
      /// \brief Destructor
      ///
      ~Camera();

      ///
      /// \returns The view matrix (inverse camera transform)
      ///
      glm::mat4 getViewMatrix();

      ///
      /// \returns The projection matrix
      ///
      const glm::mat4 & getProjectionMatrix();

      ///
      /// \brief Set the vertical field of view
      /// \param fov The new vertical field of view
      ///
      void setFov(float fov);

      ///
      /// \returns The vertical field of view
      ///
      float getFov();

      ///
      /// \brief Set the aspect ratio
      /// \param aspect The new aspect ratio
      ///
      void setAspectRatio(float aspect);

      ///
      /// \returns The aspect ratio
      ///
      float getAspectRatio();

      ///
      /// \brief Set the cliprange
      /// \param cliprange The new cliprange. near in x, far in y.
      ///
      void setClipRange(glm::vec2 cliprange);

      ///
      /// \returns The cliprange. near in x, far in y.
      ///
      glm::vec2 getClipRange();

      ///
      /// \brief Set the exposure
      /// \param exposure The new exposure value
      ///
      void setExposure(float exposure);

      ///
      /// \returns The exposure
      ///
      float getExposure();

      ///
      /// \brief Set the whitepoint
      /// \param whitepoint The new whitepoint value
      ///
      void setWhitepoint(float whitepoint);

      ///
      /// \returns The whitepoint
      ///
      float getWhitepoint();

    protected:
      void              rebuildProjectionMatrix();

      glm::mat4         m_projectionMatrix;
      float             m_fov;
      float             m_aspect;
      glm::vec2         m_clipRange;
      float             m_exposure;
      float             m_whitepoint;
  };
}
