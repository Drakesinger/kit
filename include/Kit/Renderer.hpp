#ifndef KIT_RENDERER_HPP
#define KIT_RENDERER_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"
#include "Kit/Timer.hpp"

#include <memory>
#include <vector>

namespace kit 
{
  class Renderable;
  typedef std::shared_ptr<Renderable> RenderablePtr;

  class Light;
  typedef std::shared_ptr<Light> LightPtr;

  class Skybox;
  typedef std::shared_ptr<Skybox> SkyboxPtr;

  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;

  class Text;
  typedef std::shared_ptr<Text> TextPtr;

  class PixelBuffer;
  typedef std::shared_ptr<PixelBuffer> PixelBufferPtr;

  class DoubleBuffer;
  typedef std::shared_ptr<DoubleBuffer> DoubleBufferPtr;

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class Sphere;
  typedef std::shared_ptr<Sphere> SpherePtr;

  class GLTimer;
  typedef std::shared_ptr<GLTimer> GLTimerPtr;

  class Quad;
  typedef std::shared_ptr<Quad> QuadPtr;

  class KITAPI Renderer : public std::enable_shared_from_this<Renderer>
  {
  public:

    typedef std::shared_ptr<kit::Renderer> Ptr;

    enum KITAPI  BloomQuality
    {
      Low,
      High
    };
    
    class KITAPI Payload
    {
    public:
      typedef std::shared_ptr<kit::Renderer::Payload> Ptr;

      static Ptr create();

      void assertSorted();

      std::vector<kit::RenderablePtr> & getRenderables();
      void addRenderable(kit::RenderablePtr renderable);
      void removeRenderable(kit::RenderablePtr renderable);
      
      std::vector<kit::LightPtr> & getLights();
      
      void addLight(kit::LightPtr lightptr);
      void removeLight(kit::LightPtr lightptr);
      
    private:
      std::vector<kit::LightPtr> m_lights;
      std::vector<kit::RenderablePtr> m_renderables;
      bool m_isSorted;
    };

    void setSkybox(kit::SkyboxPtr skybox);
    
    kit::SkyboxPtr getSkybox();

    kit::CameraPtr getActiveCamera();

    /// Sets the camera to use when rendering payload
    void setActiveCamera(kit::CameraPtr camera);

    /// Renders the payload and composes a fully rendered frame, the result is retreivable using getBuffer()
    void renderFrame();

    /// Gets a pointer to a texture containing the rendered payload
    kit::TexturePtr getBuffer();

    kit::PixelBufferPtr getAccumulationCopy();

    /// Sets the relative-to-framebuffer resolution for the geometry buffer, light buffer and composition buffer
    void setInternalResolution(float size);
    float const & getInternalResolution();

    /// Enables or disables shadows
    void setShadows(bool enabled);
    bool const & getShadows();

    /// Sets the active cameras exposure
    void setExposure(float exposure);
    float getExposure();

    /// Sets the active cameras whitepoint
    void setWhitepoint(float whitepoint);
    float getWhitepoint();

    /// Enables or disables FXAA
    void setFXAA(bool enabled);
    bool const & getFXAA();

    /// Enables or disables HDR bloom 
    void setBloom(bool enabled);
    bool const & getBloom();

    /// Enables or disables color correction
    void setColorCorrection(bool enabled);
    bool const & getColorCorrection();

    /// Sets the lookup table for color correction
    void setCCLookupTable(kit::TexturePtr lut);
    void loadCCLookupTable(std::string name);
    kit::TexturePtr getCCLookupTable();

    /// Sets bloom quality 
    void setBloomQuality(BloomQuality q);
    BloomQuality const & getBloomQuality();

    /// Sets bloom treshold bias - tweaks the treshold value for the brightpass (whitepoint+bias)
    void setBloomTresholdBias(float t);
    float const & getBloomTresholdBias();

    /// Specifies the different bloom blur levels
    void setBloomBlurLevels(uint32_t b2, uint32_t b4, uint32_t b8, uint32_t b16, uint32_t b32);
    
    /// Set bloom dirt mask
    void setBloomDirtMask(kit::TexturePtr m);
    kit::TexturePtr getBloomDirtMask();

    /// Set bloom dirt mask multiplier
    void setBloomDirtMaskMultiplier(float m);
    float const & getBloomDirtMaskMultiplier();

    /// Enables or disables scene fringe
    void setSceneFringe(bool enabled);
    bool const & getSceneFringe();

    /// Sets scene fringe exponential, how much it spreads out with its radius
    void setSceneFringeExponential(float e);
    float const & getSceneFringeExponential();

    /// Sets scene fringe scale
    void setSceneFringeScale(float s);
    float  const & getSceneFringeScale();

    /// Adds a payload to renderer
    void registerPayload(kit::Renderer::Payload::Ptr payload);

    /// Removes payload from the renderer
    void unregisterPayload(kit::Renderer::Payload::Ptr payload);

    /// Set resolution
    void setResolution(glm::uvec2 resolution);
    glm::uvec2 const & getResolution();
    
    /// Enables/disables GPU metrics
    void setGPUMetrics(bool enabled);
    bool const & getGPUMetrics();

    static Ptr create(glm::uvec2 resolution);
    
    kit::TextPtr getMetricsText();

    Renderer(glm::uvec2 resolution);
    ~Renderer();
    
    kit::PixelBufferPtr getGeometryBuffer();
    
  private:
    /// Called on every resize
    void onResize();
    
    void geometryPass();
    void shadowPass();
    void lightPass();
    void forwardPass();
    void hdrPass();
    void postFXPass();
    
    void renderFrameWithMetrics();
    void renderFrameWithoutMetrics();

    void updateBuffers();
    void renderLight(kit::LightPtr);
    
    static void allocateShared();
    static void releaseShared();
    
    // General
    kit::GL                     m_glSingleton;
    static uint32_t          m_instanceCount;
    float                       m_internalResolution;
    glm::uvec2                m_resolution;
    kit::QuadPtr                m_screenQuad;
    
    // Renderbuffers
    /*
     *  Geometry Buffer Layout
     * 
     *  A = Albedo
     *  R = Roughness
     *  E = Emissive Color / 10.0
     *  M = Metalness
     *  O = Occlusion
     *  C = Cavity
     *  N = Normal
     *  D = Depth
     *  P = Position
     * 
     * Encode/Decode normals
     * 
          vec2 encodeNormals (vec3 n)
          {
              float p = sqrt(n.z*8+8);
              return vec2(n.xy/p + 0.5);
          }
          
          vec3 decodeNormals (vec2 enc)
          {
              vec2 fenc = enc*4-2;
              float f = dot(fenc,fenc);
              float g = sqrt(1-f/4);
              vec3 n;
              n.xy = fenc*g;
              n.z = 1-f/2;
              return n;
          }
     *
     *  Layout:
     * 
     *  #   Format    Components
     *                r     g     b     a
     *  0   RGBA8     A.r   A.g   A.b   R
     *  1   RGBA16F   E.r   E.g   E.b   M
     *  2   RGBA16F   N.x   N.y   N.z   O
     *  D   Depth24   D
     */
    kit::PixelBufferPtr       m_geometryBuffer;       // Geometry-pass goes here                        Linear
    kit::PixelBufferPtr       m_accumulationBuffer;   // Light passes, forward pass goes here           Linear
    kit::DoubleBufferPtr      m_compositionBuffer;    // HDR->LDR pass, post effects goes here          Linear
    
    kit::PixelBufferPtr       m_accumulationCopy; // Holds a copy of the depth buffer for forward renderables who need it

    // Light rendering
    kit::TexturePtr           m_integratedBRDF;
    kit::ProgramPtr           m_programEmissive;
    kit::ProgramPtr           m_programIBL;
    kit::ProgramPtr           m_programDirectional;
    kit::ProgramPtr           m_programDirectionalNS;
    kit::ProgramPtr           m_programSpot;
    kit::ProgramPtr           m_programSpotNS;
    kit::ProgramPtr           m_programPoint;
    kit::ProgramPtr           m_programPointNS;
    kit::SpherePtr            m_pointGeometry;

    // Render payload (renderables, lights, camera)
    kit::CameraPtr            m_activeCamera;
    std::vector<Payload::Ptr>   m_payload;
    kit::SkyboxPtr            m_skybox;

    // Shadow stuff
    bool                        m_shadowsEnabled;
    
    // Bloom stuff
    bool                        m_bloomEnabled;
    BloomQuality                m_bloomQuality;
    float                       m_bloomDirtMultiplier;
    float                       m_bloomTresholdBias;
    kit::ProgramPtr           m_bloomBrightProgram;
    kit::ProgramPtr           m_bloomBlurProgram;
    kit::TexturePtr           m_bloomDirtTexture;
    kit::DoubleBufferPtr      m_bloomBrightBuffer;
    kit::DoubleBufferPtr      m_bloomBlurBuffer2;
    kit::DoubleBufferPtr      m_bloomBlurBuffer4;
    kit::DoubleBufferPtr      m_bloomBlurBuffer8;
    kit::DoubleBufferPtr      m_bloomBlurBuffer16;
    kit::DoubleBufferPtr      m_bloomBlurBuffer32;
    uint32_t                 m_bloomBlurLevel2;
    uint32_t                 m_bloomBlurLevel4;
    uint32_t                 m_bloomBlurLevel8;
    uint32_t                 m_bloomBlurLevel16;
    uint32_t                 m_bloomBlurLevel32;
    
    // HDR stuff + bloom application
    kit::ProgramPtr           m_hdrTonemap;
    kit::ProgramPtr           m_hdrTonemapBloomHigh;
    kit::ProgramPtr           m_hdrTonemapBloomLow;
    kit::ProgramPtr           m_hdrTonemapBloomHighDirt;
    kit::ProgramPtr           m_hdrTonemapBloomLowDirt;

    // Scene fringe stuff
    bool                        m_fringeEnabled;
    kit::ProgramPtr           m_fringeProgram;
    float                       m_fringeExponential;
    float                       m_fringeScale;
    
    // FXAA stuff
    bool                        m_fxaaEnabled;
    kit::ProgramPtr           m_fxaaProgram;
    
    // Color correction stuff
    bool                          m_ccEnabled;
    kit::ProgramPtr               m_ccProgram;
    kit::TexturePtr               m_ccLookupTable;

    // sRGB conversion ???
    kit::ProgramPtr               m_srgbProgram;

    // Debug information (GPU metrics, fps etc)
    bool                        m_metricsEnabled;
    kit::TextPtr              m_metrics;
    kit::GLTimerPtr           m_metricsTimer;
    kit::Timer                m_metricsFPSTimer;
    uint32_t               m_framesCount;
    uint32_t               m_metricsFPS;
    uint32_t               m_metricsFPSCalibrated;

  };
  
}

#endif