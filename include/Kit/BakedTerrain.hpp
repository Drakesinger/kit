#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include "Kit/Renderable.hpp"


namespace kit 
{
  class Texture;
  

  class Material;
  

  class Program;
  

  class Camera;
  

  class Renderer;
  

  class KITAPI BakedTerrain : public kit::Renderable
  {
    public:

      struct Vertex
      {
        float m_height = 0.0f;
        glm::vec3 m_normal;
      };

      struct LayerInfo
      {
        bool             used = false;
        kit::Texture *   arCache = nullptr;
        kit::Texture *   ndCache = nullptr;

        float            uvScale = 1.0f;
      };

      

      BakedTerrain(const std::string& name);
      ~BakedTerrain();

      void renderDeferred(kit::Renderer * camera) override;
      void renderGeometry() override;

      void renderShadows(glm::mat4 const & viewMatrix, glm::mat4 const & projectionMatrix) override;
      
      kit::Texture *  getArCache();
      kit::Texture *  getNxCache();
      kit::Texture *  getMaterialMask0();
      kit::Texture *  getMaterialMask1();

      const glm::uvec2 & getSize();

      const float & getXzScale();

      Vertex const & getVertexAt(uint32_t x, uint32_t y);
      float sampleHeight(float x, float z);
      glm::vec3 sampleNormal(float x, float z);

      bool checkCollision(glm::vec3 point);
      
      void setDetailDistance(float const & meters);
      
      virtual int32_t getRenderPriority() override;

    private:
      void                  updateGpuProgram();   //< Compiles a new program for the GPU

      bool                  m_valid = false;              //< True if loaded
      uint32_t           m_indexCount = 0;         //< Index count
      uint32_t                m_glVertexArray = 0;      //< VAO
      uint32_t                m_glVertexIndices = 0;    //< VBO for elements/indices
      uint32_t                m_glVertexBuffer = 0;     //< VBO for vertex data

      kit::Program *        m_program = nullptr;            //< GPU program

      kit::Texture *        m_arCache = nullptr;            //< Cached albedo+roughness values for the whole terrain, low-LOD
      kit::Texture *        m_nxCache = nullptr;            //< Cached normal values for the whole terrain, low-LOD (Empty value!)
      kit::Texture *        m_materialMask[2] = {nullptr, nullptr};    //< Cached material contribution values for the whole terrain. One component per layer: 0.r, 0.g, 0.b, 0.a, 1.r, 1.g, 1.b, 1.a

      LayerInfo             m_layerInfo[8];       //< Layer info
      uint8_t            m_numLayers = 0;          //< How many layers currently in use
      glm::uvec2            m_size;               //< Size of terrain
      float                 m_xzScale = 1.0f;
      float                 m_yScale = 1.0f;

      std::vector<Vertex>     m_heightData;
  };

}
