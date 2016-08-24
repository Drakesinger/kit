#ifndef KIT_RENDERER_HPP
#define KIT_RENDERER_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

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
    void loadCCLookupTable(const std::string& name);
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
    
    void setSRGBEnabled(bool const & v);
    bool getSRGBEnabled();
    
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
    static uint32_t             m_instanceCount;
    float                       m_internalResolution = 1.0;
    glm::uvec2                  m_resolution = glm::uvec2(0, 0);
    kit::QuadPtr                m_screenQuad = nullptr;
    
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
    kit::PixelBufferPtr       m_geometryBuffer = nullptr;       // Geometry-pass goes here                        Linear
    kit::PixelBufferPtr       m_accumulationBuffer = nullptr;   // Light passes, forward pass goes here           Linear
    kit::DoubleBufferPtr      m_compositionBuffer = nullptr;    // HDR->LDR pass, post effects goes here          Linear
    
    kit::PixelBufferPtr       m_accumulationCopy = nullptr; // Holds a copy of the depth buffer for forward renderables who need it

    // Light rendering
    kit::TexturePtr           m_integratedBRDF = nullptr;
    kit::ProgramPtr           m_programEmissive = nullptr;
    kit::ProgramPtr           m_programIBL = nullptr;
    kit::ProgramPtr           m_programDirectional = nullptr;
    kit::ProgramPtr           m_programDirectionalNS = nullptr;
    kit::ProgramPtr           m_programSpot = nullptr;
    kit::ProgramPtr           m_programSpotNS = nullptr;
    kit::ProgramPtr           m_programPoint = nullptr;
    kit::ProgramPtr           m_programPointNS = nullptr;
    kit::SpherePtr            m_pointGeometry = nullptr;

    // Render payload (renderables, lights, camera)
    kit::CameraPtr            m_activeCamera = nullptr;
    std::vector<Payload::Ptr> m_payload;
    kit::SkyboxPtr            m_skybox = nullptr;

    // Shadow stuff
    bool                        m_shadowsEnabled = true;
    
    // Bloom stuff
    bool                        m_bloomEnabled = true;
    BloomQuality                m_bloomQuality = BloomQuality::High;
    float                       m_bloomDirtMultiplier = 3.0f;
    float                       m_bloomTresholdBias = 0.0f;
    kit::ProgramPtr             m_bloomBrightProgram = nullptr;
    kit::ProgramPtr             m_bloomBlurProgram = nullptr;
    kit::TexturePtr             m_bloomDirtTexture = nullptr;
    kit::DoubleBufferPtr        m_bloomBrightBuffer = nullptr;
    kit::DoubleBufferPtr        m_bloomBlurBuffer2 = nullptr;
    kit::DoubleBufferPtr        m_bloomBlurBuffer4 = nullptr;
    kit::DoubleBufferPtr        m_bloomBlurBuffer8 = nullptr;
    kit::DoubleBufferPtr        m_bloomBlurBuffer16 = nullptr;
    kit::DoubleBufferPtr        m_bloomBlurBuffer32 = nullptr;
    uint32_t                    m_bloomBlurLevel2 = 1;
    uint32_t                    m_bloomBlurLevel4 = 2;
    uint32_t                    m_bloomBlurLevel8 = 4;
    uint32_t                    m_bloomBlurLevel16 = 8;
    uint32_t                    m_bloomBlurLevel32 = 16;
    
    // HDR stuff + bloom application
    kit::ProgramPtr             m_hdrTonemap = nullptr;
    kit::ProgramPtr             m_hdrTonemapBloomHigh = nullptr;
    kit::ProgramPtr             m_hdrTonemapBloomLow = nullptr;
    kit::ProgramPtr             m_hdrTonemapBloomHighDirt = nullptr;
    kit::ProgramPtr             m_hdrTonemapBloomLowDirt = nullptr;

    // Scene fringe stuff
    bool                        m_fringeEnabled = true;
    kit::ProgramPtr             m_fringeProgram = nullptr;
    float                       m_fringeExponential = 1.5f;
    float                       m_fringeScale = 0.01f;
    
    // FXAA stuff
    bool                        m_fxaaEnabled = true;
    kit::ProgramPtr             m_fxaaProgram = nullptr;
    
    // Color correction stuff
    bool                          m_ccEnabled = false;
    kit::ProgramPtr               m_ccProgram = nullptr;
    kit::TexturePtr               m_ccLookupTable = nullptr;

    // sRGB conversion ???
    kit::ProgramPtr               m_srgbProgram = nullptr;
    bool                          m_srgbEnabled = true;

    // Debug information (GPU metrics, fps etc)
    bool                        m_metricsEnabled = false;
    kit::TextPtr                m_metrics = nullptr;
    kit::GLTimerPtr             m_metricsTimer = nullptr;
    kit::Timer                  m_metricsFPSTimer;
    uint32_t                    m_framesCount = 0;
    uint32_t                    m_metricsFPS = 0;
    uint32_t                    m_metricsFPSCalibrated = 0;

  };
  
}

#endif