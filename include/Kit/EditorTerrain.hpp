#ifndef KIT_EDITORTERRAIN_HPP
#define KIT_EDITORTERRAIN_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include "Kit/Renderable.hpp"

#include <memory>

namespace kit 
{
  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;

  class PixelBuffer;
  typedef std::shared_ptr<PixelBuffer> PixelBufferPtr;

  class DoubleBuffer;
  typedef std::shared_ptr<DoubleBuffer> DoubleBufferPtr;

  class Material;
  typedef std::shared_ptr<Material> MaterialPtr;

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class Renderer;
  typedef std::shared_ptr<Renderer> RendererPtr;

  class KITAPI EditorTerrain : public kit::Renderable
  {
    public:
      struct Triangle;

      struct Vertex
      {
        uint32_t m_id;

        glm::vec3 m_position;
        glm::vec2 m_uv;
        glm::vec3 m_normal;
        glm::vec3 m_tangent;
        glm::vec3 m_bitangent;

        std::vector<Triangle*> m_triangles;
      };

      struct Triangle
      {
        Triangle(Vertex* a, Vertex* b, Vertex* c);

        glm::vec3 m_normal;
        glm::vec3 m_tangent;
        glm::vec3 m_bitangent;

        Vertex* m_a;
        Vertex* m_b;
        Vertex* m_c;
      };

      struct LayerInfo
      {
        kit::MaterialPtr material;
      };
      
      enum PaintOperation
      {
        Add = 0,
        Subtract = 1,
        Set = 2,
        Smooth = 3
      };

      typedef std::shared_ptr<EditorTerrain> Ptr;

      EditorTerrain();
      ~EditorTerrain();

      void bake();

      void save();
      void saveAs(const std::string& name);

      static Ptr create(const std::string& name, glm::uvec2 resolution, float xzScale = 0.25, float yScale = 5.0f);
      static Ptr load(const std::string& name);

      void reset(const std::string&, glm::uvec2 resolution, float xzScale, float yScale);
      
      void renderDeferred(kit::RendererPtr renderer) override;
      void renderForward(kit::RendererPtr renderer) override;
      void renderGeometry() override;
      void renderShadows(glm::mat4 v, glm::mat4 p) override;

      void renderPickbuffer(kit::CameraPtr camera);
      
      void paintMaterialMask(uint8_t layerid, kit::TexturePtr brush, glm::vec2 positionUv, glm::vec2 sizeUv, PaintOperation op,  float str);
      void paintHeightmap(kit::TexturePtr brush, glm::vec2 positionPixels, glm::vec2 sizePixels, PaintOperation op, float str);

      void setNumLayers(uint8_t);
      uint8_t getNumLayers();
      LayerInfo & getLayerInfo(int layer);
      
      Vertex * getVertexAt(int32_t x, int32_t y);
      
      glm::vec3 sampleHeightmap(int32_t x, int32_t y); //< Samples heightmap by pixel
      glm::vec3 sampleBilinear(float x, float z);            //< Bilinearly samples heightmap. Input world (x,z). Output (world height, rotationx, rotationy)
      
      void setDecalBrush(kit::TexturePtr brush = nullptr, glm::vec2 positionUv = glm::vec2(0.f,0.f), glm::vec2 sizeUv= glm::vec2(0.f,0.f));
      
      kit::TexturePtr getARCache();
      kit::TexturePtr getNXCache();
      kit::TexturePtr getHeightmap();
      kit::PixelBufferPtr getMaterialMask();
      
      glm::uvec2 getResolution();
      glm::vec2 getWorldSize();
      float getXZScale();
      
      void setName(const std::string&);
      
      void                            bakeCPUHeight();      //< Bakes CPU height-data from GPU heightmap
      void                            bakeCPUNormals();     //< Bakes CPU normals and tangents from CPU vertex data

      void                            updateGpuProgram();   //< Compiles a new program for the GPU
      void                            bakeARNXCache();      //< Renders deferred ARNX cache into m_arnxCache
      
      void invalidateMaterials();
      
    private:
      void generateCache();
      
      std::string                     m_name;
      bool                            m_valid;              //< False if terrain has been invalidated

      LayerInfo                       m_layerInfo[8];       //< Layer info
      uint8_t                      m_numLayers;          //< How many layers currently in use

      float                           m_yScale;
      float                           m_xzScale;
      glm::uvec2                      m_resolution;

      // CPU 
      std::vector<Vertex*>            m_vertices;
      std::vector<Triangle*>          m_triangles;
      std::map<Vertex*, uint32_t>  m_indexCache;

      // GPU data
      uint32_t                     m_indexCount;         //< Index count
      uint32_t                          m_glVertexArray;      //< VAO
      uint32_t                          m_glVertexIndices;    //< VBO for elements/indices
      uint32_t                          m_glVertexBuffer;     //< VBO for vertex data

      kit::DoubleBufferPtr            m_materialMask;       //< Material mask, paintable
      kit::ProgramPtr                 m_materialPaintProgram;            //< GPU program
      
      kit::DoubleBufferPtr            m_heightmap;          //< Heightmap, paintable
      kit::ProgramPtr                 m_heightPaintProgram;            //< GPU program

      kit::ProgramPtr                 m_program;            //< GPU program
      kit::ProgramPtr                 m_pickProgram;            //< GPU program
      kit::ProgramPtr                 m_shadowProgram;

      kit::ProgramPtr                 m_bakeProgramArnx;    //< GPU program for baking arnx
      kit::PixelBufferPtr             m_arnxCache;          //< Albedo+roughness and normal+x values for the whole terrain, low-LOD

      glm::vec2                      m_decalBrushPosition;
      glm::vec2                      m_decalBrushSize;
      kit::TexturePtr                 m_decalBrush;
      kit::ProgramPtr                 m_decalProgram;
      
      kit::ProgramPtr                 m_wireProgram;
  };

}

#endif