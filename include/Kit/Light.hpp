#ifndef KIT_LIGHT_HPP
#define KIT_LIGHT_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/Transformable.hpp"
#include "Kit/Cubemap.hpp"
#include <memory>

namespace kit 
{
  class Cone;
  typedef std::shared_ptr<Cone> ConePtr;

  class PixelBuffer;
  typedef std::shared_ptr<PixelBuffer> PixelBufferPtr;


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
      
      typedef std::shared_ptr<Light> Ptr;
      
      static kit::Light::Ptr create(kit::Light::Type t, glm::uvec2 shadowmapsize = glm::uvec2(0, 0));
      
      Light(kit::Light::Type t, glm::uvec2 shadowmapsize = glm::uvec2(0, 0));
      ~Light();
      
      void setColor(glm::vec3 c);
      glm::vec3 getColor();
      
      kit::Light::Type const & getType();

      void setRadius(float r);
      float const & getRadius();

      void setConeAngle(float inner, float outer);
      glm::vec2 const  getConeAngle();

      void setEnvironment(std::string name);
      
      void setIrradianceMap(kit::Cubemap::Ptr irradiance);
      kit::Cubemap::Ptr getIrradianceMap();
      
      void setRadianceMap(kit::Cubemap::Ptr radiance);
      kit::Cubemap::Ptr getRadianceMap();

      void setReflectionMap(kit::Cubemap::Ptr reflection);
      kit::Cubemap::Ptr getReflectionMap();
      
      bool isShadowMapped();
      
      kit::PixelBufferPtr getShadowBuffer();
      glm::mat4 getDirectionalProjectionMatrix();
      glm::mat4 getDirectionalViewMatrix();
      glm::mat4 getDirectionalModelMatrix(glm::vec3 pos, glm::vec3 forward);
      void      setMaxShadowDistance(float);
      float     getMaxShadowDistance();
      
      glm::mat4 getSpotProjectionMatrix();
      glm::mat4 getSpotViewMatrix();
      kit::ConePtr getSpotGeometry();
      
      //kit::CubemapBuffer::Ptr getPointShadowBuffer();
      glm::mat4 getPointProjectionMatrix();
      glm::mat4 getPointViewMatrix(kit::Cubemap::Side s);
      
      glm::vec4 getAttenuation();
      
    private:

      void updateCone();

      bool                      m_shadowMapped;
      
      float                     m_maxShadowDistance;
      kit::PixelBufferPtr       m_shadowBuffer;
      
      //kit::CubemapBuffer::Ptr    m_pointShadowMap;
      kit::ConePtr              m_spotGeometry;
      kit::Light::Type          m_type;
      glm::vec3                m_color;
      float                     m_radius;           // Used by pointlights & spotlights
      float                     m_coneInner;        // Used by spotlights
      float                     m_coneOuter;        // Used by spotlights
      glm::vec4                m_attenuation;
      kit::Cubemap::Ptr           m_irradianceMap;
      kit::Cubemap::Ptr           m_radianceMap;
      kit::Cubemap::Ptr           m_reflectionMap;
  };
  
}

#endif