#ifndef KIT_BAKEDTERRAIN_HPP
#define KIT_BAKEDTERRAIN_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"
#include "Kit/Renderable.hpp"

#include <memory>

namespace kit 
{
  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;

  class Material;
  typedef std::shared_ptr<Material> MaterialPtr;

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class Renderer;
  typedef std::shared_ptr<Renderer> RendererPtr;

  class KITAPI BakedTerrain : public kit::Renderable
  {
    public:

      struct Vertex
      {
        float m_height;
        glm::vec3 m_normal;
      };

      struct LayerInfo
      {
        bool             used;
        kit::TexturePtr  arCache;
        kit::TexturePtr  ndCache;

        float            uvScale;
      };

      typedef std::shared_ptr<BakedTerrain> Ptr;

      BakedTerrain();
      ~BakedTerrain();

      static Ptr load(const std::string& name);

      void renderDeferred(kit::RendererPtr camera) override;
      void renderGeometry() override;

      void renderShadows(glm::mat4 viewmatrix, glm::mat4 projectionmatrix) override;
      
      kit::TexturePtr getArCache();
      kit::TexturePtr getNxCache();
      kit::TexturePtr getMaterialMask0();
      kit::TexturePtr getMaterialMask1();

      const glm::uvec2 & getSize();

      const float & getXzScale();

      Vertex const & getVertexAt(uint32_t x, uint32_t y);
      float sampleHeight(float x, float z);
      glm::vec3 sampleNormal(float x, float z);

      bool checkCollision(glm::vec3 point);
      
      virtual int32_t getRenderPriority() override;

    private:
      kit::GL m_glSingleton;
      void                  updateGpuProgram();   //< Compiles a new program for the GPU

      bool                  m_valid;              //< True if loaded
      uint32_t           m_indexCount;         //< Index count
      GLuint                m_glVertexArray;      //< VAO
      GLuint                m_glVertexIndices;    //< VBO for elements/indices
      GLuint                m_glVertexBuffer;     //< VBO for vertex data

      kit::ProgramPtr       m_program;            //< GPU program

      kit::TexturePtr       m_arCache;            //< Cached albedo+roughness values for the whole terrain, low-LOD
      kit::TexturePtr       m_nxCache;            //< Cached normal values for the whole terrain, low-LOD (Empty value!)
      kit::TexturePtr       m_materialMask[2];    //< Cached material contribution values for the whole terrain. One component per layer: 0.r, 0.g, 0.b, 0.a, 1.r, 1.g, 1.b, 1.a

      LayerInfo             m_layerInfo[8];       //< Layer info
      uint8_t            m_numLayers;          //< How many layers currently in use
      glm::uvec2            m_size;               //< Size of terrain
      float                 m_xzScale;
      float                 m_yScale;

      std::vector<Vertex>     m_heightData;
  };

}

#endif