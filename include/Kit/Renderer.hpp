#pragma once 

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include "Kit/Timer.hpp"

#include <vector>
#include <queue>

namespace kit 
{
  class Renderable;
  

  class Light;
  

  class Skybox;
  

  class Camera;
  

  class Texture;
  

  class Text;
  

  class PixelBuffer;
  

  class DoubleBuffer;
  

  class Program;
  

  class Sphere;
  

  class GLTimer;
  

  class Quad;
  

  class KITAPI RenderPayload
  {
  public:
    
    RenderPayload();

    std::vector<kit::Renderable*> & getRenderables();
    void addRenderable(kit::Renderable* renderable);
    void removeRenderable(kit::Renderable* renderable);

    std::vector<kit::Light*> & getLights();

    void addLight(kit::Light* lightptr);
    void removeLight(kit::Light* lightptr);

  private:
    std::vector<kit::Light*> m_lights;
    std::vector<kit::Renderable*> m_renderables;
  };

  class KITAPI Renderer
  {
  public:

    

    enum KITAPI  BloomQuality
    {
      Low,
      High
    };

    void setSkybox(kit::Skybox * skybox);
    
    kit::Skybox * getSkybox();

    kit::Camera * getActiveCamera();

    /// Sets the camera to use when rendering payload
    void setActiveCamera(kit::Camera * camera);

    /// Renders the payload and composes a fully rendered frame, the result is retreivable using getBuffer()
    void renderFrame();
    
    kit::Light * findIBLLight();

    /// Gets a pointer to a texture containing the rendered payload
    kit::Texture * getBuffer();

    kit::PixelBuffer * getAccumulationCopy();
    
    kit::PixelBuffer * getPositionBuffer();

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
    void setCCLookupTable(kit::Texture * lut);
    void loadCCLookupTable(const std::string& name);
    kit::Texture * getCCLookupTable();

    /// Sets bloom quality 
    void setBloomQuality(BloomQuality q);
    BloomQuality const & getBloomQuality();

    /// Sets bloom treshold bias - tweaks the treshold value for the brightpass (whitepoint+bias)
    void setBloomTresholdBias(float t);
    float const & getBloomTresholdBias();

    /// Specifies the different bloom blur levels
    void setBloomBlurLevels(uint32_t b2, uint32_t b4, uint32_t b8, uint32_t b16, uint32_t b32);
    
    /// Set bloom dirt mask
    void setBloomDirtMask(kit::Texture * m);
    kit::Texture * getBloomDirtMask();

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
    void registerPayload(kit::RenderPayload * payload);

    /// Removes payload from the renderer
    void unregisterPayload(kit::RenderPayload * payload);

    /// Set resolution
    void setResolution(glm::uvec2 resolution);
    glm::uvec2 const & getResolution();
    
    /// Enables/disables GPU metrics
    void setGPUMetrics(bool enabled);
    bool const & getGPUMetrics();
    
    kit::Text * getMetricsText();

    Renderer(glm::uvec2 resolution);
    ~Renderer();
    
    kit::PixelBuffer * getGeometryBuffer();
    
    void setSRGBEnabled(bool const & v);
    bool getSRGBEnabled();
    
    kit::Texture * getIntegratedBRDF();
    
    void updatePositionBuffer();
    void updateAccumulationCopy();
    
    void renderReflections(float planarHeight = 0.0f);
    kit::Texture * getReflectionMap();
    
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
    void renderLight(kit::Light *);
    
    static void allocateShared();
    static void releaseShared();
    
    // General
    static uint32_t             m_instanceCount;
    float                       m_internalResolution = 1.0;
    glm::uvec2                  m_resolution = glm::uvec2(0, 0);
    kit::Quad *                m_screenQuad = nullptr;
    
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
    kit::PixelBuffer *       m_geometryBuffer = nullptr;       // Geometry-pass goes here                        Linear
    kit::PixelBuffer *       m_accumulationBuffer = nullptr;   // Light passes, forward pass goes here           Linear
    kit::DoubleBuffer *      m_compositionBuffer = nullptr;    // HDR->LDR pass, post effects goes here          Linear
    
    kit::PixelBuffer *       m_accumulationCopy = nullptr; // Holds a copy of the accumulation buffer for forward renderables who need it
    kit::PixelBuffer *       m_positionBuffer = nullptr;  // Recreates worldpositions from the depth buffer, for forward renderables who need it
    kit::Program     *       m_programPosition = nullptr;
    
    kit::PixelBuffer *       m_reflectionBuffer = nullptr;
    
    
    // Light rendering
    kit::Texture *           m_integratedBRDF = nullptr;
    kit::Program *           m_programEmissive = nullptr;
    kit::Program *           m_programIBL = nullptr;
    kit::Program *           m_programDirectional = nullptr;
    kit::Program *           m_programDirectionalNS = nullptr;
    kit::Program *           m_programSpot = nullptr;
    kit::Program *           m_programSpotNS = nullptr;
    kit::Program *           m_programPoint = nullptr;
    kit::Program *           m_programPointNS = nullptr;
    kit::Sphere *            m_pointGeometry = nullptr;

    // Render payload (renderables, lights, camera)
    kit::Camera *            m_activeCamera = nullptr;
    std::vector<RenderPayload*> m_payload;
    kit::Skybox *           m_skybox = nullptr;

    // Shadow stuff
    bool                        m_shadowsEnabled = true;
    
    // Bloom stuff
    bool                        m_bloomEnabled = true;
    BloomQuality                m_bloomQuality = BloomQuality::High;
    float                       m_bloomDirtMultiplier = 3.0f;
    float                       m_bloomTresholdBias = 0.0f;
    kit::Program *             m_bloomBrightProgram = nullptr;
    kit::Program *             m_bloomBlurProgram = nullptr;
    kit::Texture *             m_bloomDirtTexture = nullptr;
    kit::DoubleBuffer *        m_bloomBrightBuffer = nullptr;
    kit::DoubleBuffer *        m_bloomBlurBuffer2 = nullptr;
    kit::DoubleBuffer *        m_bloomBlurBuffer4 = nullptr;
    kit::DoubleBuffer *        m_bloomBlurBuffer8 = nullptr;
    kit::DoubleBuffer *        m_bloomBlurBuffer16 = nullptr;
    kit::DoubleBuffer *        m_bloomBlurBuffer32 = nullptr;
    uint32_t                    m_bloomBlurLevel2 = 1;
    uint32_t                    m_bloomBlurLevel4 = 2;
    uint32_t                    m_bloomBlurLevel8 = 4;
    uint32_t                    m_bloomBlurLevel16 = 8;
    uint32_t                    m_bloomBlurLevel32 = 16;
    
    // HDR stuff + bloom application
    kit::Program *             m_hdrTonemap = nullptr;
    kit::Program *             m_hdrTonemapBloomHigh = nullptr;
    kit::Program *             m_hdrTonemapBloomLow = nullptr;
    kit::Program *             m_hdrTonemapBloomHighDirt = nullptr;
    kit::Program *             m_hdrTonemapBloomLowDirt = nullptr;

    // Scene fringe stuff
    bool                        m_fringeEnabled = true;
    kit::Program *             m_fringeProgram = nullptr;
    float                       m_fringeExponential = 1.5f;
    float                       m_fringeScale = 0.01f;
    
    // FXAA stuff
    bool                        m_fxaaEnabled = true;
    kit::Program *             m_fxaaProgram = nullptr;
    
    // Color correction stuff
    bool                          m_ccEnabled = false;
    kit::Program *               m_ccProgram = nullptr;
    kit::Texture *               m_ccLookupTable = nullptr;

    // sRGB conversion ???
    kit::Program *               m_srgbProgram = nullptr;
    bool                          m_srgbEnabled = true;

    // Debug information (GPU metrics, fps etc)
    bool                        m_metricsEnabled = false;
    kit::Text *                m_metrics = nullptr;
    kit::GLTimer *             m_metricsTimer = nullptr;
    kit::Timer                  m_metricsFPSTimer;
    uint32_t                    m_framesCount = 0;
    uint32_t                    m_metricsFPS = 0;
    uint32_t                    m_metricsFPSCalibrated = 0;

  };
  
}
