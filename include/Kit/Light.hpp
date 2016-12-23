#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/Transformable.hpp"
#include "Kit/Cubemap.hpp"

namespace kit 
{
  class Cone;
  
  class PixelBuffer;
  
  class KITAPI Light : public kit::Transformable
  {
    public:
      
      enum Type 
      {
        Directional,
        Spot,
        Point,
        IBL
      };
      
      Light(kit::Light::Type t, glm::uvec2 shadowmapsize = glm::uvec2(0, 0));
      ~Light();
      
      void setColor(glm::vec3 c);
      glm::vec3 getColor();
      
      kit::Light::Type const & getType();

      void setRadius(float r);
      float const & getRadius();

      void setConeAngle(float inner, float outer);
      glm::vec2 const  getConeAngle();

      void setEnvironment(const std::string& name);
      
      kit::Cubemap* getIrradianceMap();
      kit::Cubemap* getRadianceMap();

      bool isShadowMapped();
      
      kit::PixelBuffer* getShadowBuffer();
      glm::mat4 getDirectionalProjectionMatrix();
      glm::mat4 getDirectionalViewMatrix();
      glm::mat4 getDirectionalModelMatrix(glm::vec3 pos, glm::vec3 forward);
      void      setMaxShadowDistance(float);
      float     getMaxShadowDistance();
      
      glm::mat4 getSpotProjectionMatrix();
      glm::mat4 getSpotViewMatrix();
      kit::Cone* getSpotGeometry();
      
      //kit::CubemapBuffer::Ptr getPointShadowBuffer();
      glm::mat4 getPointProjectionMatrix();
      glm::mat4 getPointViewMatrix(kit::Cubemap::Side s);
      
      glm::vec4 getAttenuation();
      
    private:

      void updateCone();

      bool                      m_shadowMapped;
      
      float                     m_maxShadowDistance;
      kit::PixelBuffer*       m_shadowBuffer = nullptr;
      
      //kit::CubemapBuffer::Ptr    m_pointShadowMap;
      kit::Cone*              m_spotGeometry = nullptr;
      kit::Light::Type          m_type;
      glm::vec3                m_color;
      float                     m_radius;           // Used by pointlights & spotlights
      float                     m_coneInner;        // Used by spotlights
      float                     m_coneOuter;        // Used by spotlights
      glm::vec4                m_attenuation;
      kit::Cubemap*           m_irradianceMap = nullptr;
      kit::Cubemap*           m_radianceMap = nullptr;
  };
  
}
