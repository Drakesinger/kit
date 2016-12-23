#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <glm/gtx/transform.hpp>

namespace kit 
{
  class Texture;
  

  class Program;
  

  class PixelBuffer;
  

  class Camera;
  

  class KITAPI Material
  {
    public:
   
      enum BlendMode
      {
        None = 0,
        Alpha,
        Add
      };

      struct ProgramFlags 
      {
        ProgramFlags();
        
        bool operator<(const ProgramFlags& b) const {
          return
            std::tie(
              this->m_skinned,
              this->m_instanced,
              this->m_forward,
              this->m_opacityMask,
              this->m_dynamicAR,
              this->m_albedoMap,
              this->m_roughnessMap,
              this->m_dynamicNM,
              this->m_normalMap,
              this->m_metalnessMap,
              this->m_dynamicEO,
              this->m_emissiveMap,
              this->m_occlusionMap
              )
            < std::tie(
              b.m_skinned,
              b.m_instanced,
              b.m_forward,
              b.m_opacityMask,
              b.m_dynamicAR,
              b.m_albedoMap,
              b.m_roughnessMap,
              b.m_dynamicNM,
              b.m_normalMap,
              b.m_metalnessMap,
              b.m_dynamicEO,
              b.m_emissiveMap,
              b.m_occlusionMap
              );
        }
        
        bool m_skinned;
        bool m_instanced;

        bool m_forward;
        bool m_opacityMask;

        bool m_dynamicAR;
        bool m_albedoMap;
        bool m_roughnessMap;
        
        bool m_dynamicNM;
        bool m_normalMap;
        bool m_metalnessMap;

        bool m_dynamicEO;
        bool m_emissiveMap;
        bool m_occlusionMap;
      };
      
      
      ~Material();
      Material();
      
      static kit::Material * load(const std::string& filename, bool use_cache = true);
      
      bool save(const std::string& filename);

      static void clearCache(std::string entry = "");

      static std::map<std::string, kit::Material*> getCacheList();

      std::string getName();
      
      void use(kit::Camera * camera, const glm::mat4 & modelMatrix, const std::vector<glm::mat4> & skinTransform, const std::vector<glm::mat4> & instanceTransform);
      
      const glm::vec3 & getAlbedo();
      void setAlbedo(glm::vec3 albedo);
      
      kit::Texture * getAlbedoMap();
      void setAlbedoMap(kit::Texture * albedoMap);
      
      kit::Texture * getNormalMap();
      void setNormalMap(kit::Texture * normalMap);
      
      const float & getRoughness();
      void setRoughness(float roughness);

      kit::Texture * getRoughnessMap();
      void setRoughnessMap(kit::Texture * roughnessMap);
      
      const float & getMetalness();
      void setMetalness(float metalness);

      kit::Texture * getMetalnessMap();
      void setMetalnessMap(kit::Texture * metalnessMap);
      
      kit::Texture * getOcclusionMap();
      void setOcclusionMap(kit::Texture * occlusionMap);

      void setDynamicAR(bool);
      void setDynamicNM(bool);
      void setDynamicEO(bool);

      glm::vec3 const & getEmissiveColor();
      void setEmissiveColor(glm::vec3);

      float const & getEmissiveStrength();
      void setEmissiveStrength(float);

      kit::Texture * getEmissiveMap();
      void setEmissiveMap(kit::Texture * emissiveMap);

      bool const & getDepthRead();
      bool const & getDepthWrite();

      void setDepthRead(bool enabled);
      void setDepthWrite(bool enabled);
      void setDoubleSided(bool enabled);
      bool const & getDoubleSided();

      void setCastShadows(bool enabled);
      bool const & getCastShadows();

      void setOpacityMask(kit::Texture * mask);
      kit::Texture * getOpacityMask();

      void setOpacity(float opacity);
      float const & getOpacity();

      kit::Texture * getARCache();
      kit::Texture * getNMCache();
      kit::Texture * getEOCache();
      kit::Texture * getNDCache();
      
      void setUvScale(float v);
      void setDepthMask(kit::Texture *);

      kit::Texture * getDepthMask();
      float getUvScale();

      void assertCache();
      ProgramFlags  getFlags(bool skinned, bool instanced);
    private:

      void renderARCache();
      void renderNMCache();
      void renderEOCache();
      void renderNDCache();

      void updateUniforms();
      static kit::Program * getProgram(ProgramFlags);
      
      static std::map<std::string, kit::Material*> m_cache;
      static void allocateShared();
      static void releaseShared();
      static kit::Program * m_cacheProgram; // Program to re-render our caches for NM and AR and EO
      static uint32_t       m_instanceCount;
      static std::map<ProgramFlags, kit::Program *> m_programCache;
      
      std::string m_filename;

      kit::PixelBuffer * m_arCache = nullptr; // Baked cache of Albedo+Roughness
      bool                m_arDirty = true;
      
      kit::PixelBuffer * m_nmCache = nullptr; // Baked cache of Normal+Metalness
      bool                m_nmDirty = true;

      kit::PixelBuffer * m_ndCache = nullptr; // Baked cache of Normal+Depth (Used for terrains)
      bool                m_ndDirty = true;

      kit::PixelBuffer * m_eoCache = nullptr; // Baked cache of Emissive+Occlusion
      bool                m_eoDirty = true;
      
      bool              m_dynamicAR = false;
      bool              m_dynamicNM = false;
      bool              m_dynamicEO = false;

      bool m_castShadows = true;
      bool m_depthWrite = true;
      bool m_depthRead = true;
      bool m_doubleSided = false;

      glm::vec3        m_albedo = glm::vec3(1.0f, 1.0f, 1.0f);
      kit::Texture *   m_albedoMap = nullptr;

      float             m_opacity = 1.0f;
      kit::Texture *   m_opacityMask = nullptr;

      BlendMode         m_blendMode = BlendMode::None;

      kit::Texture *   m_occlusionMap = nullptr;

      glm::vec3        m_emissiveColor = glm::vec3(1.0f, 1.0f, 1.0f);
      float            m_emissiveStrength = 0.0f;
      kit::Texture *   m_emissiveMap = nullptr;

      kit::Texture *   m_normalMap = nullptr;

      float            m_roughness = 0.0f;
      kit::Texture *   m_roughnessMap = nullptr;

      float             m_metalness = 0.0f;
      kit::Texture *   m_metalnessMap = nullptr;
      
      kit::Program *   m_program = nullptr;
      kit::Program *   m_sProgram = nullptr;
      kit::Program *   m_iProgram = nullptr;
      kit::Program *   m_siProgram = nullptr;
      bool             m_dirty = true;

      // SPECIFICS
      float            m_spec_uvScale = 1.0f;
      kit::Texture *   m_spec_depthMask = nullptr;
  };
}

