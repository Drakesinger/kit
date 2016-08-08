#ifndef KIT_MATERIAL_HPP
#define KIT_MATERIAL_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"

#include <glm/gtx/transform.hpp>
#include <memory>

namespace kit 
{
  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class PixelBuffer;
  typedef std::shared_ptr<PixelBuffer> PixelBufferPtr;

  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

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
      
      typedef std::shared_ptr<Material> Ptr;
      ~Material();
      
      static kit::Material::Ptr create();
      static kit::Material::Ptr load(const std::string& filename, bool reload = false);
      bool save(const std::string& filename);

      static void clearCache();
      static std::map<std::string, kit::Material::Ptr> getCacheList();

      std::string getName();
      
      void use(kit::CameraPtr camera, const glm::mat4 & modelMatrix, const std::vector<glm::mat4> & skintransform);
      
      const glm::vec3 & getAlbedo();
      void setAlbedo(glm::vec3 albedo);
      
      kit::TexturePtr getAlbedoMap();
      void setAlbedoMap(kit::TexturePtr albedoMap);
      
      const float & getNormalStrength();
      void setNormalStrength(float strength);
      
      kit::TexturePtr getNormalMap();
      void setNormalMap(kit::TexturePtr normalMap);
      
      const float & getRoughness();
      void setRoughness(float roughness);
      
      const glm::vec2 & getRoughnessInput();
      void setRoughnessInput(glm::vec2 input);

      const glm::vec2 & getRoughnessOutput();
      void setRoughnessOutput(glm::vec2 output);

      kit::TexturePtr getRoughnessMap();
      void setRoughnessMap(kit::TexturePtr roughnessMap);
      
      const float & getMetalness();
      void setMetalness(float metalness);
      
      const glm::vec2 & getMetalnessInput();
      void setMetalnessInput(glm::vec2 input);

      const glm::vec2 & getMetalnessOutput();
      void setMetalnessOutput(glm::vec2 output);

      kit::TexturePtr getMetalnessMap();
      void setMetalnessMap(kit::TexturePtr metalnessMap);
      
      kit::TexturePtr getOcclusionMap();
      void setOcclusionMap(kit::TexturePtr occlusionMap);

      bool getOcclusionInverted();
      void setOcclusionInverted(bool i);

      const float & getOcclusionGamma();
      void setOcclusionGamma(float gamma);

      const glm::vec2 & getOcclusionInput();
      void setOcclusionInput(glm::vec2 input);

      const glm::vec2 & getOcclusionOutput();
      void setOcclusionOutput(glm::vec2 output);

      void setDynamicAR(bool);
      void setDynamicNM(bool);
      void setDynamicEO(bool);

      glm::vec3 const & getEmissiveColor();
      void setEmissiveColor(glm::vec3);

      float const & getEmissiveStrength();
      void setEmissiveStrength(float);

      kit::TexturePtr getEmissiveMap();
      void setEmissiveMap(kit::TexturePtr emissiveMap);

      bool const & getDepthRead();
      bool const & getDepthWrite();

      void setDepthRead(bool enabled);
      void setDepthWrite(bool enabled);
      void setDoubleSided(bool enabled);
      bool const & getDoubleSided();

      void setCastShadows(bool enabled);
      bool const & getCastShadows();

      void setOpacityMask(kit::TexturePtr mask);
      kit::TexturePtr getOpacityMask();

      void setOpacity(float opacity);
      float const & getOpacity();

      kit::TexturePtr getARCache();
      kit::TexturePtr getNMCache();
      kit::TexturePtr getEOCache();
      kit::TexturePtr getNDCache();
      
      void setUvScale(float v);
      void setDepthMask(kit::TexturePtr);

      kit::TexturePtr getDepthMask();
      float getUvScale();

      void assertCache();
      ProgramFlags      getFlags(bool skinned);
      Material();
    private:

      void renderARCache();
      void renderNMCache();
      void renderEOCache();
      void renderNDCache();

      void updateUniforms();
      kit::GL m_glSingleton;
      static kit::ProgramPtr getProgram(ProgramFlags);
      
      static std::map<std::string, kit::Material::Ptr> m_cache;
      static void allocateShared();
      static void releaseShared();
      static kit::ProgramPtr m_cacheProgram; // Program to re-render our caches for NM and AR and EO
      static uint32_t       m_instanceCount;
      static std::map<ProgramFlags, kit::ProgramPtr> m_programCache;
            
      bool m_depthWrite;
      bool m_depthRead;

      bool m_doubleSided;

      std::string m_filename;

      kit::PixelBufferPtr m_arCache; // Baked cache of Albedo+Roughness
      bool                m_arDirty;
      
      kit::PixelBufferPtr m_nmCache; // Baked cache of Normal+Metalness
      bool                m_nmDirty;

      kit::PixelBufferPtr m_ndCache; // Baked cache of Normal+Depth
      bool                m_ndDirty;

      kit::PixelBufferPtr m_eoCache; // Baked cache of Emissive+Occlusion
      bool                m_eoDirty;
      
      bool              m_dynamicAR;
      bool              m_dynamicNM;
      bool              m_dynamicEO;

      bool              m_castShadows;

      glm::vec3        m_albedo;
      kit::TexturePtr   m_albedoMap;

      float             m_opacity;
      kit::TexturePtr   m_opacityMask;

      BlendMode         m_blendMode;

      kit::TexturePtr   m_occlusionMap;
      float             m_occlusionGamma;
      glm::vec2        m_occlusionInput;
      glm::vec2        m_occlusionOutput;

      glm::vec3        m_emissiveColor;
      float             m_emissiveStrength;
      kit::TexturePtr   m_emissiveMap;

      float             m_normalStrength;
      kit::TexturePtr   m_normalMap;

      float             m_roughness;
      kit::TexturePtr   m_roughnessMap;
      glm::vec2        m_roughnessInput;
      glm::vec2        m_roughnessOutput;

      float             m_metalness;
      kit::TexturePtr   m_metalnessMap;
      glm::vec2        m_metalnessInput;
      glm::vec2        m_metalnessOutput;
      
      kit::ProgramPtr   m_program;
      kit::ProgramPtr   m_skinnedProgram;
      bool              m_dirty;


      // SPECIFICS
      float             m_spec_uvScale;
      kit::TexturePtr   m_spec_depthMask;
  };
}

#endif
