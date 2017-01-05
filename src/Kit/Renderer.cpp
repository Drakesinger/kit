#include "Kit/Renderer.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Light.hpp"
#include "Kit/Cubemap.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Renderable.hpp"
#include "Kit/Skybox.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Text.hpp"
#include "Kit/PixelBuffer.hpp"
#include "Kit/DoubleBuffer.hpp"
#include "Kit/Program.hpp"
#include "Kit/Sphere.hpp"
#include "Kit/Font.hpp"
#include "Kit/GLTimer.hpp"
#include "Kit/Quad.hpp"
#include "Kit/Cone.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <glm/gtx/transform.hpp>

uint32_t kit::Renderer::m_instanceCount = 0;

std::vector<kit::Light *> & kit::RenderPayload::getLights()
{
  return m_lights;
}

std::vector<kit::Renderable *> & kit::RenderPayload::getRenderables()
{
  return m_renderables;
}

void kit::RenderPayload::addRenderable(kit::Renderable * renderable)
{
  if(renderable == nullptr)
  {
    KIT_ERR("Warning: got null renderable pointer");
    return;
  }
  
  for(auto & currRenderable : m_renderables)
  {
    if(currRenderable == renderable)
    {
      return;
    }
  }
  
  m_renderables.push_back(renderable);
  m_isSorted = false;
}

void kit::RenderPayload::removeRenderable(kit::Renderable * renderable)
{
  if(renderable == nullptr)
  {
    KIT_ERR("Warning: got null renderable pointer");
    return;
  }
  
  m_renderables.erase(std::remove(m_renderables.begin(), m_renderables.end(), renderable), m_renderables.end());
}

void kit::RenderPayload::addLight(kit::Light * lightptr)
{
  if(lightptr == nullptr)
  {
    KIT_ERR("Warning: got null light pointer");
    return;
  }
  
  for(auto & currLight : m_lights)
  {
    if(currLight == lightptr)
    {
      return;
    }
  }
  
  m_lights.push_back(lightptr);
}

void kit::RenderPayload::removeLight(kit::Light * lightptr)
{
  if(lightptr == nullptr)
  {
    KIT_ERR("Warning: got null light pointer");
    return;
  }
  
  m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), lightptr), m_lights.end());
}

kit::Renderer::Renderer(glm::uvec2 resolution)
{
  kit::Renderer::m_instanceCount++;
  if(kit::Renderer::m_instanceCount == 1)
  {
    kit::Renderer::allocateShared();
  }
  
  m_resolution = resolution;
  m_internalResolution = 1.0f;
  m_activeCamera = nullptr;
  m_skybox = nullptr;
  m_metricsEnabled = false;
  
  glm::uvec2 effectiveResolution(uint32_t(float(resolution.x) * m_internalResolution), uint32_t(float(resolution.y) * m_internalResolution));
  
  // create a screen quad to render fullscreen stuff
  m_screenQuad = new kit::Quad();
  
  // Setup light programs
  m_programEmissive = new kit::Program({"screenquad.vert"}, {"lighting/emissive-light.frag"}, kit::DataSource::Static);
  m_programIBL = new kit::Program({"lighting/directional-light.vert"}, {"normals.glsl", "lighting/ibl-light.frag"}, kit::DataSource::Static);
  m_integratedBRDF = new kit::Texture(kit::getDataDirectory(kit::DataSource::Static) + "/textures/brdf.tga", Texture::RGBA8);
  m_integratedBRDF->setEdgeSamplingMode(Texture::EdgeSamplingMode::ClampToEdge);
  m_integratedBRDF->setMinFilteringMode(Texture::FilteringMode::LinearMipmapLinear);
  m_integratedBRDF->setMagFilteringMode(Texture::FilteringMode::Linear);
  m_integratedBRDF->generateMipmap();
  
  m_programDirectional = new kit::Program({"lighting/directional-light.vert"}, {"lighting/cooktorrance.glsl", "normals.glsl", "lighting/directional-light.frag"}, kit::DataSource::Static);
  m_programDirectionalNS = new kit::Program({"lighting/directional-light.vert"}, {"lighting/cooktorrance.glsl", "normals.glsl", "lighting/directional-light-ns.frag"}, kit::DataSource::Static);
  
  m_programSpot = new kit::Program({"lighting/spot-light.vert"}, {"lighting/attenuation.glsl", "lighting/spotattenuation.glsl", "normals.glsl", "lighting/cooktorrance.glsl", "lighting/spot-light.frag"}, kit::DataSource::Static);
  m_programSpotNS = new kit::Program({"lighting/spot-light.vert"}, {"lighting/attenuation.glsl", "lighting/spotattenuation.glsl", "normals.glsl", "lighting/cooktorrance.glsl", "lighting/spot-light-ns.frag"}, kit::DataSource::Static);
  
  m_programPoint = new kit::Program({"lighting/point-light.vert"}, {"lighting/attenuation.glsl", "lighting/spotattenuation.glsl",  "normals.glsl", "lighting/cooktorrance.glsl", "lighting/point-light.frag"}, kit::DataSource::Static);
  m_programPointNS = new kit::Program({"lighting/point-light.vert"}, {"lighting/attenuation.glsl", "lighting/spotattenuation.glsl",  "normals.glsl", "lighting/cooktorrance.glsl", "lighting/point-light-ns.frag"}, kit::DataSource::Static);
  m_pointGeometry = new kit::Sphere(32, 24);
 
  m_programIBL->setUniformTexture("uniform_brdf", m_integratedBRDF);
  
  // Setup buffers
  updateBuffers();

  // --- Setup HDR bloom
  
  // Setup default variables
  m_bloomEnabled      = true;
  m_bloomQuality      = High;
  m_bloomTresholdBias = 0.0f;
  m_bloomBlurLevel2   = 1;
  m_bloomBlurLevel4   = 2;
  m_bloomBlurLevel8   = 4;
  m_bloomBlurLevel16  = 8;
  m_bloomBlurLevel32  = 16;
  m_bloomDirtMultiplier = 3.0;
  m_bloomDirtTexture = nullptr;
    
  // Load and set programs
  m_hdrTonemap         = new kit::Program({"screenquad.vert"}, {"hdr-tonemap.frag"}, kit::DataSource::Static);
  m_hdrTonemapBloomHigh    = new kit::Program({"screenquad.vert"}, {"hdr-tonemapBloom.frag"}, kit::DataSource::Static);
  m_hdrTonemapBloomHighDirt= new kit::Program({"screenquad.vert"}, {"hdr-tonemapBloomDirt.frag"}, kit::DataSource::Static);
  m_hdrTonemapBloomLow    = new kit::Program({"screenquad.vert"}, {"hdr-tonemapBloomLow.frag"}, kit::DataSource::Static);
  m_hdrTonemapBloomLowDirt= new kit::Program({"screenquad.vert"}, {"hdr-tonemapBloomLowDirt.frag"}, kit::DataSource::Static);
  m_bloomBrightProgram = new kit::Program({"screenquad.vert"}, {"hdr-brightpass.frag"}, kit::DataSource::Static);
  m_bloomBlurProgram   = new kit::Program({"screenquad.vert"}, {"hdr-blur.frag"}, kit::DataSource::Static);
  
  m_bloomBlurProgram->setUniform1f("uniform_radius", 1.0f);
  m_bloomBrightProgram->setUniform1f("uniform_exposure", 1.0f);
  m_bloomBrightProgram->setUniform1f("uniform_whitepoint", 1.0f);
  m_bloomBrightProgram->setUniform1f("uniform_tresholdBias", m_bloomTresholdBias);
  m_hdrTonemap->setUniform1f("uniform_exposure", 1.0f);
  m_hdrTonemap->setUniform1f("uniform_whitepoint", 1.0f);
  m_hdrTonemapBloomHigh->setUniform1f("uniform_exposure", 1.0f);
  m_hdrTonemapBloomHigh->setUniform1f("uniform_whitepoint", 1.0f);
  m_hdrTonemapBloomHighDirt->setUniform1f("uniform_exposure", 1.0f);
  m_hdrTonemapBloomHighDirt->setUniform1f("uniform_whitepoint", 1.0f);
  m_hdrTonemapBloomHighDirt->setUniform1f("uniform_bloomDirtMultiplier", m_bloomDirtMultiplier);
  m_hdrTonemapBloomHighDirt->setUniformTexture("uniform_bloomDirtTexture", m_bloomDirtTexture);
  m_hdrTonemapBloomLow->setUniform1f("uniform_exposure", 1.0f);
  m_hdrTonemapBloomLow->setUniform1f("uniform_whitepoint", 1.0f);
  m_hdrTonemapBloomLowDirt->setUniform1f("uniform_exposure", 1.0f);
  m_hdrTonemapBloomLowDirt->setUniform1f("uniform_whitepoint", 1.0f);
  m_hdrTonemapBloomLowDirt->setUniform1f("uniform_bloomDirtMultiplier", m_bloomDirtMultiplier);
  m_hdrTonemapBloomLowDirt->setUniformTexture("uniform_bloomDirtTexture", m_bloomDirtTexture);
  
  // --- Setup FXAA
  m_fxaaEnabled = true;
  m_fxaaProgram = new kit::Program({"screenquad.vert"}, {"fxaa.frag"}, kit::DataSource::Static);
  
  // --- Setup color correction
  m_ccEnabled = false;
  m_ccProgram = new kit::Program({ "screenquad.vert" }, { "cc.frag" }, kit::DataSource::Static);
  m_ccLookupTable = nullptr; // kit::Texture::create3DFromFile("./data/luts/test.tga", kit::Texture::RGB8);

  // --- Setup sRGB correction
  m_srgbProgram = new kit::Program({ "screenquad.vert" }, { "srgb.frag" }, kit::DataSource::Static);
  m_srgbEnabled = true;

  // --- Setup shadows
  m_shadowsEnabled = true;

  // --- Setup scene fringe
  m_fringeEnabled = false;
  m_fringeExponential = 1.5f;
  m_fringeScale = 0.01f;
  m_fringeProgram = new kit::Program({"screenquad.vert"}, {"fringe.frag"}, kit::DataSource::Static);
  m_fringeProgram->setUniform1f("uniform_exponential", m_fringeExponential);
  m_fringeProgram->setUniform1f("uniform_scale",       m_fringeScale);
  
  // Setup debug metrics
  m_metrics = new kit::Text(kit::Font::getSystemFont(), 16.0f, L"", glm::vec2(4.0f, 4.0f));
  m_metrics->setAlignment(Text::Left, Text::Top);
  m_metricsTimer = new kit::GLTimer();
  m_framesCount = 0;
  m_metricsFPS = 0;
  m_metricsFPSCalibrated = 0;
  m_metricsEnabled = true;
}

kit::Renderer::~Renderer()
{
    if(m_screenQuad) delete m_screenQuad;
    if(m_geometryBuffer) delete m_geometryBuffer;    
    if(m_accumulationBuffer) delete m_accumulationBuffer;
    if(m_compositionBuffer) delete m_compositionBuffer; 
    if(m_accumulationCopy) delete m_accumulationCopy;
    if(m_integratedBRDF) delete m_integratedBRDF;
    if(m_programEmissive) delete m_programEmissive;
    if(m_programIBL) delete m_programIBL;
    if(m_programDirectional) delete m_programDirectional;
    if(m_programDirectionalNS) delete m_programDirectionalNS;
    if(m_programSpot) delete m_programSpot;
    if(m_programSpotNS) delete m_programSpotNS;
    if(m_programPoint) delete m_programPoint;
    if(m_programPointNS) delete m_programPointNS;
    if(m_pointGeometry) delete m_pointGeometry;
    if(m_bloomBrightProgram) delete m_bloomBrightProgram;
    if(m_bloomBlurProgram) delete m_bloomBlurProgram;
    if(m_bloomBrightBuffer) delete m_bloomBrightBuffer;
    if(m_bloomBlurBuffer2) delete m_bloomBlurBuffer2;
    if(m_bloomBlurBuffer4) delete m_bloomBlurBuffer4;
    if(m_bloomBlurBuffer8) delete m_bloomBlurBuffer8;
    if(m_bloomBlurBuffer16) delete m_bloomBlurBuffer16;
    if(m_bloomBlurBuffer32) delete m_bloomBlurBuffer32;
    if(m_hdrTonemap) delete m_hdrTonemap;
    if(m_hdrTonemapBloomHigh) delete m_hdrTonemapBloomHigh;
    if(m_hdrTonemapBloomLow) delete m_hdrTonemapBloomLow;
    if(m_hdrTonemapBloomHighDirt) delete m_hdrTonemapBloomHighDirt;
    if(m_hdrTonemapBloomLowDirt) delete m_hdrTonemapBloomLowDirt;
    if(m_fringeProgram) delete m_fringeProgram;
    if(m_fxaaProgram) delete m_fxaaProgram;
    if(m_ccProgram) delete m_ccProgram;
    if(m_srgbProgram) delete m_srgbProgram;
    if(m_metrics) delete m_metrics;
    if(m_metricsTimer) delete m_metricsTimer;
  
  kit::Renderer::m_instanceCount--;
  if(kit::Renderer::m_instanceCount == 0)
  {
    kit::Renderer::releaseShared();
  }
}

void kit::Renderer::allocateShared()
{

}

void kit::Renderer::releaseShared()
{

}

void kit::Renderer::renderLight(kit::Light * currLight)
{
  kit::Camera * c = m_activeCamera;
  glm::mat4 m = currLight->getTransformMatrix();
  glm::mat4 v = c->getViewMatrix();
  glm::mat4 p = c->getProjectionMatrix();
  glm::mat4 mv = v * m;
  glm::mat4 mvp = p * v * m;

  //D
  glm::vec2 clip = c->getClipRange();
  float znear = clip.x;
  float zfar = clip.y;
  float px = (-zfar * znear ) / ( zfar - znear );//D
  float py = zfar / ( zfar - znear ) ;//D
  
  if(currLight->getType() == kit::Light::Directional)
  {
    kit::Program * currProgram;
    if(currLight->isShadowMapped() && m_shadowsEnabled)
    {
      currProgram = m_programDirectional;
      currProgram->setUniformTexture("uniform_shadowmap", currLight->getShadowBuffer()->getDepthAttachment());
      currProgram->setUniformMat4("uniform_lightViewProjMatrix", currLight->getDirectionalProjectionMatrix() * currLight->getDirectionalViewMatrix() * currLight->getDirectionalModelMatrix(c->getPosition(), c->getForward()));
    }
    else
    {
      currProgram = m_programDirectionalNS;
    }
    
    currProgram->setUniform3f("uniform_lightColor", currLight->getColor());
    currProgram->setUniform3f("uniform_lightDir", normalize(glm::vec3(v * glm::vec4(currLight->getForward(), 0.0f))));

    currProgram->setUniform2f("uniform_projConst", glm::vec2(px, py));
    
    currProgram->setUniformMat4("uniform_invProjMatrix", glm::inverse(p));
    
    currProgram->use(); 
    
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_screenQuad->render(currProgram);
  }
  else if (currLight->getType() == kit::Light::Spot)
  {
    kit::Program * currProgram;
    if(currLight->isShadowMapped() && m_shadowsEnabled)
    {
      currProgram = m_programSpot;
      currProgram->setUniformTexture("uniform_shadowmap", currLight->getShadowBuffer()->getDepthAttachment());
      currProgram->setUniformMat4("uniform_lightViewProjMatrix", currLight->getSpotProjectionMatrix() * currLight->getSpotViewMatrix());
    }
    else
    {
      currProgram = m_programSpotNS;
    }
    
    currProgram->setUniform3f("uniform_lightColor", currLight->getColor());
    currProgram->setUniform3f("uniform_lightPosition", glm::vec3(v * glm::vec4(currLight->getPosition(), 1.0f)));
    currProgram->setUniform3f("uniform_lightDirection", glm::vec3(v * glm::vec4(currLight->getForward(), 0.0f)));
    currProgram->setUniform4f("uniform_lightFalloff", currLight->getAttenuation());
    currProgram->setUniform2f("uniform_coneAngle", glm::vec2(glm::cos(glm::radians(currLight->getConeAngle().x) * 0.5f), glm::cos(glm::radians(currLight->getConeAngle().y) * 0.5f)));
    currProgram->setUniformMat4("uniform_MVPMatrix", mvp);
    currProgram->setUniformMat4("uniform_MVMatrix", mv);
    currProgram->setUniform2f("uniform_projConst", glm::vec2(px, py));
    
    currProgram->use();
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    currLight->getSpotGeometry()->renderGeometry();
    glDisable(GL_CULL_FACE);
  }
  else if(currLight->getType() == kit::Light::Point)
  {
    kit::Program * currProgram;
    //if(currLight->m_shadowMapped)TODO
    //{
    //  currProgram = m_programPoint;
    //  currProgram->setUniformCubemap("uniform_shadowmap", currLight->getShadowBuffer()->getCubemap());
    //  currProgram->setUniformMat4("uniform_invViewMatrix", glm::inverse(kit::Light::m_cameraCache->getTransformMatrix()));
    //}
    //else
    //{
      currProgram = m_programPointNS;
    //}
    
    currProgram->setUniform3f("uniform_lightColor", currLight->getColor());
    currProgram->setUniformMat4("uniform_MVPMatrix", mvp);
    //currProgram->setUniform1f("uniform_lightRadius", currLight->getRadius());
    currProgram->setUniform3f("uniform_lightPosition", glm::vec3(v * glm::vec4(currLight->getPosition(), 1.0f)));    
    currProgram->setUniform4f("uniform_lightFalloff", currLight->getAttenuation());
    
    currProgram->setUniformMat4("uniform_MVMatrix", mv);
    currProgram->setUniform2f("uniform_projConst", glm::vec2(px, py));

    currProgram->use();
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    m_pointGeometry->renderGeometry();
    glDisable(GL_CULL_FACE);
  }
  else if(currLight->getType() == kit::Light::IBL)
  {

    m_programIBL->setUniform3f("uniform_lightColor", currLight->getColor());
    m_programIBL->setUniformCubemap("uniform_lightIrradiance", currLight->getIrradianceMap());
    m_programIBL->setUniformCubemap("uniform_lightRadiance", currLight->getRadianceMap());
    //m_programIBL->setUniformCubemap("uniform_lightReflection", currLight->getReflectionMap());
    m_programIBL->setUniform2f("uniform_projConst", glm::vec2(px, py));
    
    m_programIBL->setUniformMat4("uniform_invProjMatrix", glm::inverse(p));
    
    m_programIBL->use();
    
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_screenQuad->render(m_programIBL);
    
  }
}

void kit::Renderer::setSkybox(kit::Skybox * skybox)
{
  m_skybox = skybox;
}

kit::RenderPayload::RenderPayload()
{
}

void kit::RenderPayload::assertSorted()
{
  if (!m_isSorted)
  {
    // Sort payload by render priority
    std::sort(m_renderables.begin(), m_renderables.end(), [](kit::Renderable * const & a, kit::Renderable * const & b) {return a->getRenderPriority() < b->getRenderPriority(); });
    m_isSorted = true;
  }
}

void kit::Renderer::registerPayload(kit::RenderPayload * payload)
{
  if(payload == nullptr)
  {
    KIT_ERR("Warning: got null payload pointer");
    return;
  }
  
  for(auto & currPayload : m_payload)
  {
    if(payload == currPayload)
    {
      KIT_ERR("Warning: payload was added twice, ignoring");
      return;
    }
  }
  
  m_payload.push_back(payload);
}

void kit::Renderer::unregisterPayload(kit::RenderPayload * payload)
{
  if(payload == nullptr)
  {
    KIT_ERR("Warning: got null payload pointer");
    return;
  }
  
  std::cout << "Removing payload " << payload << std::endl;
  
  if(m_payload.size() == 0)
  {
    std::cout << "Warning: tried to remove payload when no payload existed" << std::endl;
    return;
  }
  
  m_payload.erase(std::remove(m_payload.begin(), m_payload.end(), payload), m_payload.end());
}

kit::Texture * kit::Renderer::getBuffer()
{
  return m_compositionBuffer->getFrontBuffer()->getColorAttachment(0);
}

void kit::Renderer::onResize()
{
  updateBuffers();

  if (m_activeCamera)
  {
    m_activeCamera->setAspectRatio((float)m_resolution.x / (float)m_resolution.y);
  }
}

void kit::Renderer::renderFrame()
{
  if (m_metricsEnabled)
  {
    renderFrameWithMetrics();
  }
  else
  {
    renderFrameWithoutMetrics();
  }
}

void kit::Renderer::renderFrameWithoutMetrics()
{
  if (m_activeCamera == nullptr)
  {
    return;
  }

  // render geometry pass
  geometryPass();

  // render shadow pass
  shadowPass();

  // render light pass
  lightPass();

  // render forward pass
  forwardPass();

  // render HDR pass
  hdrPass();

  // render postfx pass
  postFXPass();

  kit::PixelBuffer::unbind();
  kit::Program::useFixed();
  
  m_framesCount++;
  double milli = (double)m_metricsFPSTimer.timeSinceStart().asMilliseconds();
  if (milli >= 1000.0)
  {
    double fps = m_framesCount;
    double fpsCalibrated = fps * (1000.0 / milli);
    m_metricsFPS = (uint32_t)fps;
    m_metricsFPSCalibrated = (uint32_t)fpsCalibrated;
    m_framesCount = 0;
    
    std::wstringstream s;
    s << std::setprecision(3) << std::fixed << std::right;
    s << L"Timed FPS:     " << std::setw(7) << m_metricsFPS << " fps" << std::endl;
    s << L"Calibrated FPS:    " << std::setw(7) << m_metricsFPSCalibrated << " fps" << std::endl;
    
    m_metrics->setText(s.str());
    
    m_metricsFPSTimer.restart();
  }


  
}

void kit::Renderer::renderFrameWithMetrics()
{
  if(m_activeCamera == nullptr)
  {
    return;
  }
  
  uint64_t geometryPassTime, shadowPassTime, lightPassTime, forwardPassTime, hdrPassTime, postFxPassTime;
  
  // render geometry pass
  m_metricsTimer->start();
  geometryPass();
  geometryPassTime = m_metricsTimer->end();

  // render shadow pass
  m_metricsTimer->start();
  shadowPass();
  shadowPassTime = m_metricsTimer->end();
  
  // render light pass
  m_metricsTimer->start();
  lightPass();
  lightPassTime = m_metricsTimer->end();
  
  // render forward pass
  m_metricsTimer->start();
  forwardPass();
  forwardPassTime = m_metricsTimer->end();

  // render HDR pass
  m_metricsTimer->start();
  hdrPass();
  hdrPassTime = m_metricsTimer->end();
  
  // render postfx pass
  m_metricsTimer->start();
  postFXPass();
  postFxPassTime = m_metricsTimer->end();

  uint64_t totalTime = geometryPassTime + lightPassTime + forwardPassTime + shadowPassTime + hdrPassTime + postFxPassTime;
  
  m_framesCount++;
  double milli = (double)m_metricsFPSTimer.timeSinceStart().asMilliseconds();
  if (milli >= 1000.0)
  {
    double fps = m_framesCount;
    double fpsCalibrated = fps * (1000.0 / milli);
    m_metricsFPS = (uint32_t)fps;
    m_metricsFPSCalibrated = (uint32_t)fpsCalibrated;
    m_framesCount = 0;
    m_metricsFPSTimer.restart();
  }

  std::wstringstream s;
  s << std::setprecision(3) << std::fixed << std::right;
  s << L"Rendertime:    " << std::setw(7) << (((double)totalTime        /1000.0)/1000.0) << " ms" << std::endl;
  s << L"Potential FPS: " << std::setw(7) << 1000.0/(((double)totalTime / 1000.0) / 1000.0) << " fps" << std::endl;
  s << L"Timed FPS:     " << std::setw(7) << m_metricsFPS << " fps" << std::endl;
  s << L"Actual FPS:    " << std::setw(7) << m_metricsFPSCalibrated << " fps" << std::endl;
  s << std::endl;
  s << L"-Passes--------" << std::endl;
  s << L"--Geometry:    " << std::setw(7) << (((double)geometryPassTime /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--Shadow:      " << std::setw(7) << (((double)shadowPassTime   /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--Light:       " << std::setw(7) << (((double)lightPassTime    /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--Forward:     " << std::setw(7) << (((double)forwardPassTime  /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--HDR:         " << std::setw(7) << (((double)hdrPassTime      /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--Composition: " << std::setw(7) << (((double)postFxPassTime   /1000.0)/1000.0) << " ms" << std::endl;
  
  m_metrics->setText(s.str());
 
  kit::PixelBuffer::unbind();
  kit::Program::useFixed();
}

void kit::Renderer::geometryPass()
{
  glDisable(GL_BLEND);

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  // Clear and bind the geometry buffer
  m_geometryBuffer->clear({ glm::vec4(0.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 0.0) }, 1.0f);

  // For each payload ...
  for (auto & currPayload : m_payload)
  {
    currPayload->assertSorted();

    // For each renderable in current payload ...
    for (auto & currRenderable : currPayload->getRenderables())
    {
      // Render the renderable in deferred mode
      currRenderable->renderDeferred(this);
    }
  }
}

void kit::Renderer::shadowPass()
{
  if (!m_shadowsEnabled) return;

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  // For each payload ...
  // (note that all payloads are isolated from each other in terms of shadows)
  for (kit::RenderPayload * & currPayload : m_payload)
  {
    // For each light in current payload ...
    for (kit::Light * & currLight : currPayload->getLights())
    {
      // Ignore light if its not shadowmapped
      if (!currLight->isShadowMapped())
      {
        continue;
      }

      // If light is a spotlight..
      if (currLight->getType() == Light::Spot)
      {
        // Setup spotshadow program and buffers
        currLight->getShadowBuffer()->clearDepth(1.0f);

        // For each renderable in current payload ...
        for (kit::Renderable * & currRenderable : currPayload->getRenderables())
        {
          // Ignore renderable if its not a shadowcaster
          if (!currRenderable->isShadowCaster())
          {
            continue;
          }

          // Render the renderable to the shadowmap
          currRenderable->renderShadows(currLight->getSpotViewMatrix(), currLight->getSpotProjectionMatrix());
        }
      }

      // If light is directional
      if (currLight->getType() == Light::Directional)
      {
        // Setup directional program and buffers
        currLight->getShadowBuffer()->clearDepth(1.0f);

        // For each renderable in current payload ...
        for (kit::Renderable * & currRenderable : currPayload->getRenderables())
        {
          // Ignore renderable if its not a shadowcaster
          if (!currRenderable->isShadowCaster())
          {
            continue;
          }

          // Render the renderable to the shadowmap
          currRenderable->renderShadows(currLight->getDirectionalViewMatrix() * currLight->getDirectionalModelMatrix(m_activeCamera->getPosition(), m_activeCamera->getForward()), currLight->getDirectionalProjectionMatrix());
        }
      }

      // Point light shadows are not implemented yet
      //else if (currLight->getType() == Light::Point)
      //{
      //  
      //}
    }
  }
}

void kit::Renderer::lightPass()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  //glDisable(GL_CULL_FACE);
  
  // Clear and bind the light accumulation buffer
  glm::mat4 invViewMatrix;

  m_accumulationBuffer->clear({ glm::vec4(0.0, 0.0, 0.0, 1.0) });
  invViewMatrix = glm::inverse(m_activeCamera->getViewMatrix());

  // Update our light programs
  m_programIBL->setUniformMat4("uniform_invViewMatrix", invViewMatrix);
  m_programSpot->setUniformMat4("uniform_invViewMatrix", invViewMatrix);
  m_programDirectional->setUniformMat4("uniform_invViewMatrix", invViewMatrix);

  // For each payload ...
  for (auto & currPayload : m_payload)
  {
    // For each light in current payload ...
    for (auto & currLight : currPayload->getLights())
    {
      // Render the light
      renderLight(currLight);
    }
  }

  // Render emissive light
  m_screenQuad->render(m_programEmissive);
}

void kit::Renderer::forwardPass()
{
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Bind the accumulation buffer
  m_accumulationBuffer->bind();

  // If we have a skybox
  if (m_skybox)
  {
    glDisable(GL_CULL_FACE);
    // Render the skybox
    m_skybox->render(this);
  }

  // For each payload ...
  for (auto & currPayload : m_payload)
  {
    currPayload->assertSorted();

    // For each renderable in current payload ...
    for (auto & currRenderable : currPayload->getRenderables())
    {
      if (currRenderable->requestAccumulationCopy())
      {
       // std::cout << "Handle " << m_accumulationBuffer->getHandle() << ": " << m_accumulationBuffer->getResolution().x << "x" << m_accumulationBuffer->getResolution().y << " to " << m_depthCopyBuffer->getHandle() << ": " << m_depthCopyBuffer->getResolution().x << "x" << m_depthCopyBuffer->getResolution().y << std::endl;
        glBlitNamedFramebuffer(
          m_accumulationBuffer->getHandle(), 
          m_accumulationCopy->getHandle(), 
          0, 0, m_accumulationBuffer->getResolution().x, m_accumulationBuffer->getResolution().y,
          0, 0, m_accumulationCopy->getResolution().x, m_accumulationCopy->getResolution().y,
          GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST
        );
      }

      // Render current renderable in forward mode
      currRenderable->renderForward(this);
    }
  }
}

void kit::Renderer::hdrPass()
{
  
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);


  // If we have bloom enabled
  if (m_bloomEnabled) {
    // Render brightpass
    //m_bloomBrightBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
    m_bloomBrightBuffer->getBackBuffer()->bind();
    m_bloomBrightProgram->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
    m_bloomBrightProgram->setUniform1f("uniform_exposure", m_activeCamera->getExposure());
    m_bloomBrightProgram->setUniformTexture("uniform_sourceTexture", m_accumulationBuffer->getColorAttachment(0));
    m_screenQuad->render(m_bloomBrightProgram);
    m_bloomBrightBuffer->flip();

    // If bloom quality is high
    if (m_bloomQuality == High)
    {
      // Blit the bright buffer to the first bloom blur buffer
      m_bloomBlurBuffer2->blitFrom(m_bloomBrightBuffer);
      m_bloomBlurBuffer2->flip();

      // Blur the first bloom blur buffer
      for (uint32_t i = 0; i < m_bloomBlurLevel2; i++)
      {
        //m_bloomBlurBuffer2->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
        m_bloomBlurBuffer2->getBackBuffer()->bind();
        m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer2->getFrontBuffer()->getColorAttachment(0));
        m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer2->getResolution().x));
        m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
        m_screenQuad->render(m_bloomBlurProgram);
        m_bloomBlurBuffer2->flip();

        //m_bloomBlurBuffer2->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
        m_bloomBlurBuffer2->getBackBuffer()->bind(); 
        m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer2->getFrontBuffer()->getColorAttachment(0));
        m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer2->getResolution().x));
        m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
        m_screenQuad->render(m_bloomBlurProgram);
        m_bloomBlurBuffer2->flip();
      }

      // Blit the first bloom blur buffer to the second bloom blur buffer
      m_bloomBlurBuffer4->blitFrom(m_bloomBlurBuffer2);
      m_bloomBlurBuffer4->flip();

      // Blur the second bloom blur buffer
      for (uint32_t i = 0; i < m_bloomBlurLevel4; i++)
      {
        //m_bloomBlurBuffer4->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
        m_bloomBlurBuffer4->getBackBuffer()->bind();
        m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer4->getFrontBuffer()->getColorAttachment(0));
        m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer4->getResolution().x));
        m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
        m_screenQuad->render(m_bloomBlurProgram);
        m_bloomBlurBuffer4->flip();

        //m_bloomBlurBuffer4->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
        m_bloomBlurBuffer4->getBackBuffer()->bind();
        m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer4->getFrontBuffer()->getColorAttachment(0));
        m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer4->getResolution().x));
        m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
        m_screenQuad->render(m_bloomBlurProgram);
        m_bloomBlurBuffer4->flip();
      }

      // Blit the second bloom blur buffer to the third bloom blur buffer
      m_bloomBlurBuffer8->blitFrom(m_bloomBlurBuffer4);
      m_bloomBlurBuffer8->flip();
    }
    // .. Or if bloom quality is low
    else
    {
      // Blir the bright buffer directly to the third bloom blur buffer
      m_bloomBlurBuffer8->blitFrom(m_bloomBrightBuffer);
      m_bloomBlurBuffer8->flip();
    }

    // Blur the third bloom blur buffer
    for (uint32_t i = 0; i < m_bloomBlurLevel8; i++)
    {
      //m_bloomBlurBuffer8->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      m_bloomBlurBuffer8->getBackBuffer()->bind();
      m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer8->getFrontBuffer()->getColorAttachment(0));
      m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer8->getResolution().x));
      m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
      m_screenQuad->render(m_bloomBlurProgram);
      m_bloomBlurBuffer8->flip();

      //m_bloomBlurBuffer8->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      m_bloomBlurBuffer8->getBackBuffer()->bind();
      m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer8->getFrontBuffer()->getColorAttachment(0));
      m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer8->getResolution().x));
      m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
      m_screenQuad->render(m_bloomBlurProgram);
      m_bloomBlurBuffer8->flip();
    }

    // Blit the third bloom blur buffer to the fourth bloom blur buffer
    m_bloomBlurBuffer16->blitFrom(m_bloomBlurBuffer8);
    m_bloomBlurBuffer16->flip();

    // Blur the fourth bloom blur buffer
    for (uint32_t i = 0; i < m_bloomBlurLevel16; i++)
    {
      //m_bloomBlurBuffer16->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      m_bloomBlurBuffer16->getBackBuffer()->bind();
      m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer16->getFrontBuffer()->getColorAttachment(0));
      m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer16->getResolution().x));
      m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
      m_screenQuad->render(m_bloomBlurProgram);
      m_bloomBlurBuffer16->flip();

      //m_bloomBlurBuffer16->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      m_bloomBlurBuffer16->getBackBuffer()->bind();
      m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer16->getFrontBuffer()->getColorAttachment(0));
      m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer16->getResolution().x));
      m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
      m_screenQuad->render(m_bloomBlurProgram);
      m_bloomBlurBuffer16->flip();
    }

    // Blit the fourth bloom blur buffer to our fifth bloom blur buffer
    m_bloomBlurBuffer32->blitFrom(m_bloomBlurBuffer16);
    m_bloomBlurBuffer32->flip();

    // Blur the fifth bloom blur buffer
    for (uint32_t i = 0; i < m_bloomBlurLevel32; i++)
    {
      //m_bloomBlurBuffer32->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      m_bloomBlurBuffer32->getBackBuffer()->bind();
      m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer32->getFrontBuffer()->getColorAttachment(0));
      m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer32->getResolution().x));
      m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
      m_screenQuad->render(m_bloomBlurProgram);
      m_bloomBlurBuffer32->flip();

      //m_bloomBlurBuffer32->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      m_bloomBlurBuffer32->getBackBuffer()->bind();
      m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", m_bloomBlurBuffer32->getFrontBuffer()->getColorAttachment(0));
      m_bloomBlurProgram->setUniform1f("uniform_resolution", float(m_bloomBlurBuffer32->getResolution().x));
      m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
      m_screenQuad->render(m_bloomBlurProgram);
      m_bloomBlurBuffer32->flip();
    }

    // Select the right HDR tonemap program to use, based on quality and if we have a dirt texture
    kit::Program * hdrProgram = (m_bloomQuality == High ? (m_bloomDirtTexture ? m_hdrTonemapBloomHighDirt : m_hdrTonemapBloomHigh) : (m_bloomDirtTexture ? m_hdrTonemapBloomLowDirt : m_hdrTonemapBloomLow));

    hdrProgram->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
    hdrProgram->setUniform1f("uniform_exposure", m_activeCamera->getExposure());

    // Setup the tonemap program
    hdrProgram->setUniformTexture("uniform_sourceTexture", m_accumulationBuffer->getColorAttachment(0));
    if (m_bloomQuality == High)
    {
      hdrProgram->setUniformTexture("uniform_bloom2Texture", m_bloomBlurBuffer2->getFrontBuffer()->getColorAttachment(0));
      hdrProgram->setUniformTexture("uniform_bloom4Texture", m_bloomBlurBuffer4->getFrontBuffer()->getColorAttachment(0));
    }
    hdrProgram->setUniformTexture("uniform_bloom8Texture", m_bloomBlurBuffer8->getFrontBuffer()->getColorAttachment(0));
    hdrProgram->setUniformTexture("uniform_bloom16Texture", m_bloomBlurBuffer16->getFrontBuffer()->getColorAttachment(0));
    hdrProgram->setUniformTexture("uniform_bloom32Texture", m_bloomBlurBuffer32->getFrontBuffer()->getColorAttachment(0));

    // Clear our composition buffer and render our HDR tonemap program
    //m_compositionBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
    m_compositionBuffer->getBackBuffer()->bind();
    m_screenQuad->render(hdrProgram);
    m_compositionBuffer->flip();
  }
  // ... Or if bloom is not enabled
  else
  {
    // Do regular HDR tonemapping
    m_hdrTonemap->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
    m_hdrTonemap->setUniform1f("uniform_exposure", m_activeCamera->getExposure());
    m_hdrTonemap->setUniformTexture("uniform_sourceTexture", m_accumulationBuffer->getColorAttachment(0));
    //m_compositionBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
    m_compositionBuffer->getBackBuffer()->bind();
    m_screenQuad->render(m_hdrTonemap);
    m_compositionBuffer->flip();
  }

}

void kit::Renderer::postFXPass()
{
  
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  if (m_fringeEnabled)
  {
    m_compositionBuffer->getBackBuffer()->bind();
    m_fringeProgram->setUniformTexture("uniform_sourceTexture", m_compositionBuffer->getFrontBuffer()->getColorAttachment(0));
    m_screenQuad->render(m_fringeProgram);
    m_compositionBuffer->flip();
    kit::Program::useFixed();
  }

  if (m_fxaaEnabled)
  {
    m_compositionBuffer->getBackBuffer()->bind();
    m_fxaaProgram->setUniformTexture("uniform_sourceTexture", m_compositionBuffer->getFrontBuffer()->getColorAttachment(0));
    m_screenQuad->render(m_fxaaProgram);
    m_compositionBuffer->flip();
  }

  // sRGB conversion
  if(m_srgbEnabled)
  {
    m_compositionBuffer->getBackBuffer()->bind();
    m_srgbProgram->setUniformTexture("uniform_sourceTexture", m_compositionBuffer->getFrontBuffer()->getColorAttachment(0));
    m_screenQuad->render(m_srgbProgram);
    m_compositionBuffer->flip();
  }
  
  if (m_ccEnabled && m_ccLookupTable != nullptr)
  {
    m_compositionBuffer->getBackBuffer()->bind();
    m_ccProgram->setUniformTexture("uniform_sourceTexture", m_compositionBuffer->getFrontBuffer()->getColorAttachment(0));
    m_ccProgram->setUniformTexture("uniform_lut", m_ccLookupTable);
    m_screenQuad->render(m_ccProgram);
    m_compositionBuffer->flip();
  }
}

void kit::Renderer::setActiveCamera(kit::Camera * camera)
{
  m_activeCamera = camera;
  if(m_activeCamera)
  {
    m_activeCamera->setAspectRatio((float)m_resolution.x / (float)m_resolution.y);
    m_hdrTonemap->setUniform1f("uniform_exposure", m_activeCamera->getExposure());
    m_hdrTonemapBloomHigh->setUniform1f("uniform_exposure", m_activeCamera->getExposure());
    m_hdrTonemapBloomHighDirt->setUniform1f("uniform_exposure", m_activeCamera->getExposure());
    m_hdrTonemapBloomLow->setUniform1f("uniform_exposure", m_activeCamera->getExposure());
    m_hdrTonemapBloomLowDirt->setUniform1f("uniform_exposure", m_activeCamera->getExposure());
    m_bloomBrightProgram->setUniform1f("uniform_exposure", m_activeCamera->getExposure());
    m_hdrTonemap->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
    m_hdrTonemapBloomHigh->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
    m_hdrTonemapBloomHighDirt->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
    m_hdrTonemapBloomLow->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
    m_hdrTonemapBloomLowDirt->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
    m_bloomBrightProgram->setUniform1f("uniform_whitepoint", m_activeCamera->getWhitepoint());
  }
}

kit::Camera * kit::Renderer::getActiveCamera()
{
  return m_activeCamera;
}

void kit::Renderer::setCCLookupTable(kit::Texture * tex)
{
  m_ccLookupTable = tex;
}

void kit::Renderer::loadCCLookupTable(const std::string&name)
{
  m_ccLookupTable = new kit::Texture("./data/luts/" + name, Texture::RGB8, 1, Texture::Texture3D);
  m_ccLookupTable->setEdgeSamplingMode(Texture::ClampToEdge);
}

kit::Texture * kit::Renderer::getCCLookupTable()
{
  return m_ccLookupTable;
}

void kit::Renderer::setExposure(float exposure)
{
  if (!m_activeCamera)
  {
    return;
  }

  m_activeCamera->setExposure(exposure);
  m_hdrTonemap->setUniform1f("uniform_exposure", exposure);
  m_hdrTonemapBloomHigh->setUniform1f("uniform_exposure", exposure);
  m_hdrTonemapBloomHighDirt->setUniform1f("uniform_exposure", exposure);
  m_hdrTonemapBloomLow->setUniform1f("uniform_exposure", exposure);
  m_hdrTonemapBloomLowDirt->setUniform1f("uniform_exposure", exposure);
  m_bloomBrightProgram->setUniform1f("uniform_exposure", exposure);
}

float kit::Renderer::getExposure()
{
  if (m_activeCamera == nullptr) return 0.0;
  return m_activeCamera->getExposure();
}

void kit::Renderer::setWhitepoint(float whitepoint)
{
  if (!m_activeCamera)
  {
    return;
  }

  m_activeCamera->setWhitepoint(whitepoint);
  m_hdrTonemap->setUniform1f("uniform_whitepoint", whitepoint);
  m_hdrTonemapBloomHigh->setUniform1f("uniform_whitepoint", whitepoint);
  m_hdrTonemapBloomHighDirt->setUniform1f("uniform_whitepoint", whitepoint);
  m_hdrTonemapBloomLow->setUniform1f("uniform_whitepoint", whitepoint);
  m_hdrTonemapBloomLowDirt->setUniform1f("uniform_whitepoint", whitepoint);
  m_bloomBrightProgram->setUniform1f("uniform_whitepoint", whitepoint);
}

float kit::Renderer::getWhitepoint()
{
  if (m_activeCamera == nullptr) return 0.0;
  return m_activeCamera->getWhitepoint();
}

void kit::Renderer::setBloom(bool enabled)
{
  m_bloomEnabled = enabled;
}

bool const & kit::Renderer::getBloom()
{
  return m_bloomEnabled;
}

void kit::Renderer::setFXAA(bool enabled)
{
  m_fxaaEnabled = enabled;
}

bool const & kit::Renderer::getFXAA()
{
  return m_fxaaEnabled;
}

void kit::Renderer::setInternalResolution(float size)
{
  m_internalResolution = size;
  onResize();
}

float const & kit::Renderer::getInternalResolution()
{
  return m_internalResolution;
}

void kit::Renderer::setResolution(glm::uvec2 resolution)
{
  m_resolution = resolution;
  onResize();
}

glm::uvec2 const & kit::Renderer::getResolution()
{
  return m_resolution;
}

void kit::Renderer::setShadows(bool enabled)
{
  m_shadowsEnabled = enabled;
}

bool const & kit::Renderer::getShadows()
{
  return m_shadowsEnabled;
}

void kit::Renderer::setSceneFringe(bool enabled)
{
  m_fringeEnabled = enabled;
}

bool const & kit::Renderer::getSceneFringe()
{
  return m_fringeEnabled;
}

void kit::Renderer::setSceneFringeExponential(float e)
{
  m_fringeExponential = e;
  m_fringeProgram->setUniform1f("uniform_exponential", e);
}

float const & kit::Renderer::getSceneFringeExponential()
{
  return m_fringeExponential;
}

void kit::Renderer::setSceneFringeScale(float s)
{
  m_fringeScale = s;
  m_fringeProgram->setUniform1f("uniform_scale", s);
}

float const & kit::Renderer::getSceneFringeScale()
{
  return m_fringeScale;
}

void kit::Renderer::setBloomQuality(kit::Renderer::BloomQuality q)
{
  m_bloomQuality = q;
}

kit::Renderer::BloomQuality const & kit::Renderer::getBloomQuality()
{
  return m_bloomQuality;
}

void kit::Renderer::setBloomBlurLevels(uint32_t b2, uint32_t b4, uint32_t b8, uint32_t b16, uint32_t b32)
{
  m_bloomBlurLevel2 = b2;
  m_bloomBlurLevel4 = b4;
  m_bloomBlurLevel8 = b8;
  m_bloomBlurLevel16 = b16;
  m_bloomBlurLevel32 = b32;
}

void kit::Renderer::setBloomDirtMask(kit::Texture * m)
{
  m_bloomDirtTexture = m;
  m_hdrTonemapBloomHighDirt->setUniformTexture("uniform_bloomDirtTexture", m_bloomDirtTexture);
  m_hdrTonemapBloomLowDirt->setUniformTexture("uniform_bloomDirtTexture", m_bloomDirtTexture);
}

kit::Texture * kit::Renderer::getBloomDirtMask()
{
  return m_bloomDirtTexture;
}

bool const & kit::Renderer::getColorCorrection()
{
  return m_ccEnabled;
}

void kit::Renderer::setColorCorrection(bool b)
{
  m_ccEnabled = b;
}

void kit::Renderer::setBloomDirtMaskMultiplier(float m)
{
  m_bloomDirtMultiplier = m;
  m_hdrTonemapBloomHighDirt->setUniform1f("uniform_bloomDirtMultiplier", m_bloomDirtMultiplier);
  m_hdrTonemapBloomLowDirt->setUniformTexture("uniform_bloomDirtTexture", m_bloomDirtTexture);
}

float const & kit::Renderer::getBloomDirtMaskMultiplier()
{
  return m_bloomDirtMultiplier;
}

void kit::Renderer::setBloomTresholdBias(float t)
{
  m_bloomTresholdBias = t;
  m_bloomBrightProgram->setUniform1f("uniform_tresholdBias", m_bloomTresholdBias);
}

float const & kit::Renderer::getBloomTresholdBias()
{
  return m_bloomTresholdBias;
}

kit::Text * kit::Renderer::getMetricsText()
{
  return m_metrics;
}

void kit::Renderer::setGPUMetrics(bool enabled)
{
  m_metricsEnabled = enabled;
  if (!m_metricsEnabled)
  {
    m_metrics->setText(L"");
  }
}

bool const & kit::Renderer::getGPUMetrics()
{
  return m_metricsEnabled;
}

void kit::Renderer::updateBuffers()
{
  glm::uvec2 effectiveResolution(uint32_t(float(m_resolution.x) * m_internalResolution), uint32_t(float(m_resolution.y) * m_internalResolution));

  // create rendering buffers
  if(m_compositionBuffer)
    delete m_compositionBuffer;
  
  m_compositionBuffer = new kit::DoubleBuffer(
    effectiveResolution,
    {
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB16F)
    }
  );
  m_compositionBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });

  
  if(m_geometryBuffer)
    delete m_geometryBuffer;
  
  m_geometryBuffer = new kit::PixelBuffer(
    effectiveResolution,
    {
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8),
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16F),
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16F)
    },
    kit::PixelBuffer::AttachmentInfo(Texture::DepthComponent24)
    );
  m_geometryBuffer->clear({ glm::vec4(0.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 0.0) }, 1.0f);
  m_geometryBuffer->getColorAttachment(0)->setEdgeSamplingMode(kit::Texture::EdgeSamplingMode::ClampToEdge);

  if(m_accumulationBuffer)
    delete m_accumulationBuffer;
  
  m_accumulationBuffer = new kit::PixelBuffer(
    effectiveResolution,
    {
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16F)
    },
    kit::PixelBuffer::AttachmentInfo(m_geometryBuffer->getDepthAttachment())
    );
  m_accumulationBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) }, 1.0f);
  m_accumulationBuffer->getColorAttachment(0)->setEdgeSamplingMode(kit::Texture::EdgeSamplingMode::ClampToEdge);

  if(m_accumulationCopy)
    delete m_accumulationCopy;
  
  m_accumulationCopy = new kit::PixelBuffer(
    effectiveResolution,
    {
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16F)
    },
    kit::PixelBuffer::AttachmentInfo(kit::Texture::DepthComponent24)
  );
  m_accumulationCopy->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) }, 1.0f);
  m_accumulationCopy->getColorAttachment(0)->setEdgeSamplingMode(kit::Texture::EdgeSamplingMode::ClampToEdge);

  if(m_bloomBrightBuffer)
    delete m_bloomBrightBuffer;
  
  m_bloomBrightBuffer = new kit::DoubleBuffer(effectiveResolution, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  m_bloomBrightBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });

  if(m_bloomBlurBuffer2)
    delete m_bloomBlurBuffer2;
  
  glm::uvec2 bloom2Res(uint32_t(float(effectiveResolution.x) * (1.0f / 2.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 2.0f)));
  m_bloomBlurBuffer2 = new kit::DoubleBuffer(bloom2Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  m_bloomBlurBuffer2->clear({glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)});

  if(m_bloomBlurBuffer4)
    delete m_bloomBlurBuffer4;
  
  glm::uvec2 bloom4Res(uint32_t(float(effectiveResolution.x) * (1.0f / 4.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 4.0f)));
  m_bloomBlurBuffer4 = new kit::DoubleBuffer(bloom4Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  m_bloomBlurBuffer4->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });

  if(m_bloomBlurBuffer8)
    delete m_bloomBlurBuffer8;
  
  glm::uvec2 bloom8Res(uint32_t(float(effectiveResolution.x) * (1.0f / 8.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 8.0f)));
  m_bloomBlurBuffer8 = new kit::DoubleBuffer(bloom8Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  m_bloomBlurBuffer8->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });

  if(m_bloomBlurBuffer16)
    delete m_bloomBlurBuffer16;
  
  glm::uvec2 bloom16Res(uint32_t(float(effectiveResolution.x) * (1.0f / 16.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 16.0f)));
  m_bloomBlurBuffer16 = new kit::DoubleBuffer(bloom16Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  m_bloomBlurBuffer16->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });

  if(m_bloomBlurBuffer32)
    delete m_bloomBlurBuffer32;
  
  glm::uvec2 bloom32Res(uint32_t(float(effectiveResolution.x) * (1.0f / 32.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 32.0f)));
  m_bloomBlurBuffer32 = new kit::DoubleBuffer(bloom32Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  m_bloomBlurBuffer32->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });

  m_programDirectional->setUniformTexture("uniform_textureA", m_geometryBuffer->getColorAttachment(0));
  m_programDirectional->setUniformTexture("uniform_textureB", m_geometryBuffer->getColorAttachment(1));
  m_programDirectional->setUniformTexture("uniform_textureC", m_geometryBuffer->getColorAttachment(2));
  m_programDirectional->setUniformTexture("uniform_textureDepth", m_geometryBuffer->getDepthAttachment());
  m_programDirectionalNS->setUniformTexture("uniform_textureA", m_geometryBuffer->getColorAttachment(0));
  m_programDirectionalNS->setUniformTexture("uniform_textureB", m_geometryBuffer->getColorAttachment(1));
  m_programDirectionalNS->setUniformTexture("uniform_textureC", m_geometryBuffer->getColorAttachment(2));
  m_programDirectionalNS->setUniformTexture("uniform_textureDepth", m_geometryBuffer->getDepthAttachment());
  m_programSpot->setUniformTexture("uniform_textureA", m_geometryBuffer->getColorAttachment(0));
  m_programSpot->setUniformTexture("uniform_textureB", m_geometryBuffer->getColorAttachment(1));
  m_programSpot->setUniformTexture("uniform_textureC", m_geometryBuffer->getColorAttachment(2));
  m_programSpot->setUniformTexture("uniform_textureDepth", m_geometryBuffer->getDepthAttachment());
  m_programSpotNS->setUniformTexture("uniform_textureA", m_geometryBuffer->getColorAttachment(0));
  m_programSpotNS->setUniformTexture("uniform_textureB", m_geometryBuffer->getColorAttachment(1));
  m_programSpotNS->setUniformTexture("uniform_textureC", m_geometryBuffer->getColorAttachment(2));
  m_programSpotNS->setUniformTexture("uniform_textureDepth", m_geometryBuffer->getDepthAttachment());
  m_programPoint->setUniformTexture("uniform_textureA", m_geometryBuffer->getColorAttachment(0));
  m_programPoint->setUniformTexture("uniform_textureB", m_geometryBuffer->getColorAttachment(1));
  m_programPoint->setUniformTexture("uniform_textureC", m_geometryBuffer->getColorAttachment(2));
  m_programPoint->setUniformTexture("uniform_textureDepth", m_geometryBuffer->getDepthAttachment());
  m_programPointNS->setUniformTexture("uniform_textureA", m_geometryBuffer->getColorAttachment(0));
  m_programPointNS->setUniformTexture("uniform_textureB", m_geometryBuffer->getColorAttachment(1));
  m_programPointNS->setUniformTexture("uniform_textureC", m_geometryBuffer->getColorAttachment(2));
  m_programPointNS->setUniformTexture("uniform_textureDepth", m_geometryBuffer->getDepthAttachment());
  m_programIBL->setUniformTexture("uniform_textureA", m_geometryBuffer->getColorAttachment(0));
  m_programIBL->setUniformTexture("uniform_textureB", m_geometryBuffer->getColorAttachment(1));
  m_programIBL->setUniformTexture("uniform_textureC", m_geometryBuffer->getColorAttachment(2));
  m_programIBL->setUniformTexture("uniform_textureDepth", m_geometryBuffer->getDepthAttachment());
  m_programEmissive->setUniformTexture("uniform_textureB", m_geometryBuffer->getColorAttachment(1));
}

kit::Skybox * kit::Renderer::getSkybox()
{
  return m_skybox;
}

kit::PixelBuffer * kit::Renderer::getAccumulationCopy()
{
  return m_accumulationCopy;
}

kit::PixelBuffer * kit::Renderer::getGeometryBuffer()
{
  return m_geometryBuffer;
}

bool kit::Renderer::getSRGBEnabled()
{
  return m_srgbEnabled;
}

void kit::Renderer::setSRGBEnabled(const bool& v)
{
  m_srgbEnabled = v;
}
