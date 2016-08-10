#include "Kit/Renderer.hpp"

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

std::vector<kit::Light::Ptr> & kit::Renderer::Payload::getLights()
{
  return this->m_lights;
}

std::vector<kit::Renderable::Ptr> & kit::Renderer::Payload::getRenderables()
{
  return this->m_renderables;
}

void kit::Renderer::Payload::addRenderable(kit::Renderable::Ptr renderable)
{
  if(renderable == nullptr)
  {
    KIT_ERR("Warning: got null renderable pointer");
    return;
  }
  
  for(auto & currRenderable : this->m_renderables)
  {
    if(currRenderable == renderable)
    {
      return;
    }
  }
  
  this->m_renderables.push_back(renderable);
  this->m_isSorted = false;
}

void kit::Renderer::Payload::removeRenderable(kit::Renderable::Ptr renderable)
{
  if(renderable == nullptr)
  {
    KIT_ERR("Warning: got null renderable pointer");
    return;
  }
  
  this->m_renderables.erase(std::remove(this->m_renderables.begin(), this->m_renderables.end(), renderable), this->m_renderables.end());
}

void kit::Renderer::Payload::addLight(kit::Light::Ptr lightptr)
{
  if(lightptr == nullptr)
  {
    KIT_ERR("Warning: got null light pointer");
    return;
  }
  
  for(auto & currLight : this->m_lights)
  {
    if(currLight == lightptr)
    {
      return;
    }
  }
  
  this->m_lights.push_back(lightptr);
}

void kit::Renderer::Payload::removeLight(kit::Light::Ptr lightptr)
{
  if(lightptr == nullptr)
  {
    KIT_ERR("Warning: got null light pointer");
    return;
  }
  
  this->m_lights.erase(std::remove(this->m_lights.begin(), this->m_lights.end(), lightptr), this->m_lights.end());
}

kit::Renderer::Renderer(glm::uvec2 resolution)
{
  kit::Renderer::m_instanceCount++;
  if(kit::Renderer::m_instanceCount == 1)
  {
    kit::Renderer::allocateShared();
  }
  
  this->m_resolution = resolution;
  this->m_internalResolution = 1.0f;
  this->m_activeCamera = nullptr;
  this->m_skybox = nullptr;
  this->m_metricsEnabled = false;
  
  glm::uvec2 effectiveResolution(uint32_t(float(resolution.x) * this->m_internalResolution), uint32_t(float(resolution.y) * this->m_internalResolution));
  
  // create a screen quad to render fullscreen stuff
  this->m_screenQuad = kit::Quad::create();
  
  // Setup light programs
  this->m_programEmissive = kit::Program::load({"screenquad.vert"}, {"lighting/emissive-light.frag"});
  this->m_programIBL = kit::Program::load({"lighting/directional-light.vert"}, {"normals.glsl", "lighting/ibl-light.frag"});
  this->m_integratedBRDF = kit::Texture::load("brdf.tga", false);
  this->m_integratedBRDF->setEdgeSamplingMode(Texture::ClampToEdge);
  
  this->m_programDirectional = kit::Program::load({"lighting/directional-light.vert"}, {"lighting/cooktorrance.glsl", "normals.glsl", "lighting/directional-light.frag"});
  this->m_programDirectionalNS = kit::Program::load({"lighting/directional-light.vert"}, {"lighting/cooktorrance.glsl", "normals.glsl", "lighting/directional-light-ns.frag"});
  
  this->m_programSpot = kit::Program::load({"lighting/spot-light.vert"}, {"lighting/attenuation.glsl", "lighting/spotattenuation.glsl", "normals.glsl", "lighting/cooktorrance.glsl", "lighting/spot-light.frag"});
  this->m_programSpotNS = kit::Program::load({"lighting/spot-light.vert"}, {"lighting/attenuation.glsl", "lighting/spotattenuation.glsl", "normals.glsl", "lighting/cooktorrance.glsl", "lighting/spot-light-ns.frag"});
  
  this->m_programPoint = kit::Program::load({"lighting/point-light.vert"}, {"lighting/attenuation.glsl", "lighting/spotattenuation.glsl",  "normals.glsl", "lighting/cooktorrance.glsl", "lighting/point-light.frag"});
  this->m_programPointNS = kit::Program::load({"lighting/point-light.vert"}, {"lighting/attenuation.glsl", "lighting/spotattenuation.glsl",  "normals.glsl", "lighting/cooktorrance.glsl", "lighting/point-light-ns.frag"});
  this->m_pointGeometry = kit::Sphere::create(32, 24);
 
  this->m_programIBL->setUniformTexture("uniform_brdf", this->m_integratedBRDF);
  
  // Setup buffers
  this->updateBuffers();

  // --- Setup HDR bloom
  
  // Setup default variables
  this->m_bloomEnabled      = true;
  this->m_bloomQuality      = High;
  this->m_bloomTresholdBias = 0.0f;
  this->m_bloomBlurLevel2   = 1;
  this->m_bloomBlurLevel4   = 2;
  this->m_bloomBlurLevel8   = 4;
  this->m_bloomBlurLevel16  = 8;
  this->m_bloomBlurLevel32  = 16;
  this->m_bloomDirtMultiplier = 3.0;
  this->m_bloomDirtTexture = nullptr;
    
  // Load and set programs
  this->m_hdrTonemap         = kit::Program::load({"screenquad.vert"}, {"hdr-tonemap.frag"});
  this->m_hdrTonemapBloomHigh    = kit::Program::load({"screenquad.vert"}, {"hdr-tonemapBloom.frag"});
  this->m_hdrTonemapBloomHighDirt= kit::Program::load({"screenquad.vert"}, {"hdr-tonemapBloomDirt.frag"});
  this->m_hdrTonemapBloomLow    = kit::Program::load({"screenquad.vert"}, {"hdr-tonemapBloomLow.frag"});
  this->m_hdrTonemapBloomLowDirt= kit::Program::load({"screenquad.vert"}, {"hdr-tonemapBloomLowDirt.frag"});
  this->m_bloomBrightProgram = kit::Program::load({"screenquad.vert"}, {"hdr-brightpass.frag"});
  this->m_bloomBlurProgram   = kit::Program::load({"screenquad.vert"}, {"hdr-blur.frag"});
  
  this->m_bloomBlurProgram->setUniform1f("uniform_radius", 1.0f);
  this->m_bloomBrightProgram->setUniform1f("uniform_exposure", 1.0f);
  this->m_bloomBrightProgram->setUniform1f("uniform_whitepoint", 1.0f);
  this->m_bloomBrightProgram->setUniform1f("uniform_tresholdBias", this->m_bloomTresholdBias);
  this->m_hdrTonemap->setUniform1f("uniform_exposure", 1.0f);
  this->m_hdrTonemap->setUniform1f("uniform_whitepoint", 1.0f);
  this->m_hdrTonemapBloomHigh->setUniform1f("uniform_exposure", 1.0f);
  this->m_hdrTonemapBloomHigh->setUniform1f("uniform_whitepoint", 1.0f);
  this->m_hdrTonemapBloomHighDirt->setUniform1f("uniform_exposure", 1.0f);
  this->m_hdrTonemapBloomHighDirt->setUniform1f("uniform_whitepoint", 1.0f);
  this->m_hdrTonemapBloomHighDirt->setUniform1f("uniform_bloomDirtMultiplier", this->m_bloomDirtMultiplier);
  this->m_hdrTonemapBloomHighDirt->setUniformTexture("uniform_bloomDirtTexture", this->m_bloomDirtTexture);
  this->m_hdrTonemapBloomLow->setUniform1f("uniform_exposure", 1.0f);
  this->m_hdrTonemapBloomLow->setUniform1f("uniform_whitepoint", 1.0f);
  this->m_hdrTonemapBloomLowDirt->setUniform1f("uniform_exposure", 1.0f);
  this->m_hdrTonemapBloomLowDirt->setUniform1f("uniform_whitepoint", 1.0f);
  this->m_hdrTonemapBloomLowDirt->setUniform1f("uniform_bloomDirtMultiplier", this->m_bloomDirtMultiplier);
  this->m_hdrTonemapBloomLowDirt->setUniformTexture("uniform_bloomDirtTexture", this->m_bloomDirtTexture);
  
  // --- Setup FXAA
  this->m_fxaaEnabled = true;
  this->m_fxaaProgram = kit::Program::load({"screenquad.vert"}, {"fxaa.frag"});
  
  // --- Setup color correction
  this->m_ccEnabled = false;
  this->m_ccProgram = kit::Program::load({ "screenquad.vert" }, { "cc.frag" });
  this->m_ccLookupTable = nullptr; // kit::Texture::create3DFromFile("./data/luts/test.tga", kit::Texture::RGB8);

  // --- Setup sRGB correction
  this->m_srgbProgram = kit::Program::load({ "screenquad.vert" }, { "srgb.frag" });
  this->m_srgbEnabled = true;

  // --- Setup shadows
  this->m_shadowsEnabled = true;

  // --- Setup scene fringe
  this->m_fringeEnabled = false;
  this->m_fringeExponential = 1.5f;
  this->m_fringeScale = 0.01f;
  this->m_fringeProgram = kit::Program::load({"screenquad.vert"}, {"fringe.frag"});
  this->m_fringeProgram->setUniform1f("uniform_exponential", this->m_fringeExponential);
  this->m_fringeProgram->setUniform1f("uniform_scale",       this->m_fringeScale);
  
  // Setup debug metrics
  this->m_metrics = kit::Text::create(kit::Font::getSystemFont(), 16.0f, L"", glm::vec2(4.0f, 4.0f));
  this->m_metrics->setAlignment(Text::Left, Text::Top);
  this->m_metricsTimer = kit::GLTimer::create();
  this->m_framesCount = 0;
  this->m_metricsFPS = 0;
  this->m_metricsFPSCalibrated = 0;
  this->m_metricsEnabled = true;
}

kit::Renderer::~Renderer()
{
  kit::Renderer::m_instanceCount--;
  if(kit::Renderer::m_instanceCount == 0)
  {
    kit::Renderer::releaseShared();
  }
}

kit::Renderer::Ptr kit::Renderer::create(glm::uvec2 resolution)
{
  return std::make_shared<kit::Renderer>(resolution);
}

void kit::Renderer::allocateShared()
{

}

void kit::Renderer::releaseShared()
{

}

void kit::Renderer::renderLight(kit::Light::Ptr currLight)
{
  kit::Camera::Ptr c = this->m_activeCamera;
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
    kit::Program::Ptr currProgram;
    if(currLight->isShadowMapped() && this->m_shadowsEnabled)
    {
      currProgram = this->m_programDirectional;
      currProgram->setUniformTexture("uniform_shadowmap", currLight->getShadowBuffer()->getDepthAttachment());
      currProgram->setUniformMat4("uniform_lightViewProjMatrix", currLight->getDirectionalProjectionMatrix() * currLight->getDirectionalViewMatrix() * currLight->getDirectionalModelMatrix(c->getPosition(), c->getForward()));
    }
    else
    {
      currProgram = this->m_programDirectionalNS;
    }
    
    currProgram->use(); 
    currProgram->setUniform3f("uniform_lightColor", currLight->getColor());
    currProgram->setUniform3f("uniform_lightDir", normalize(glm::vec3(v * glm::vec4(currLight->getForward(), 0.0f))));

    currProgram->setUniform2f("uniform_projConst", glm::vec2(px, py));
    
    currProgram->setUniformMat4("uniform_invProjMatrix", glm::inverse(p));
    
    //KIT_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    this->m_screenQuad->render(currProgram);
  }
  else if (currLight->getType() == kit::Light::Spot)
  {
    kit::Program::Ptr currProgram;
    if(currLight->isShadowMapped() && this->m_shadowsEnabled)
    {
      currProgram = this->m_programSpot;
      currProgram->setUniformTexture("uniform_shadowmap", currLight->getShadowBuffer()->getDepthAttachment());
      currProgram->setUniformMat4("uniform_lightViewProjMatrix", currLight->getSpotProjectionMatrix() * currLight->getSpotViewMatrix());
    }
    else
    {
      currProgram = this->m_programSpotNS;
    }
    
    currProgram->use();
    currProgram->setUniform3f("uniform_lightColor", currLight->getColor());
    currProgram->setUniform3f("uniform_lightPosition", glm::vec3(v * glm::vec4(currLight->getPosition(), 1.0f)));
    currProgram->setUniform3f("uniform_lightDirection", glm::vec3(v * glm::vec4(currLight->getForward(), 0.0f)));
    currProgram->setUniform4f("uniform_lightFalloff", currLight->getAttenuation());
    currProgram->setUniform2f("uniform_coneAngle", glm::vec2(glm::cos(glm::radians(currLight->getConeAngle().x) * 0.5f), glm::cos(glm::radians(currLight->getConeAngle().y) * 0.5f)));
    currProgram->setUniformMat4("uniform_MVPMatrix", mvp);
    currProgram->setUniformMat4("uniform_MVMatrix", mv);
    currProgram->setUniform2f("uniform_projConst", glm::vec2(px, py));
    
    kit::GL::enable(GL_CULL_FACE);
    kit::GL::cullFace(GL_FRONT);
    currLight->getSpotGeometry()->renderGeometry();
    kit::GL::disable(GL_CULL_FACE);
  }
  else if(currLight->getType() == kit::Light::Point)
  {
    kit::Program::Ptr currProgram;
    //if(currLight->m_shadowMapped)TODO
    //{
    //  currProgram = this->m_programPoint;
    //  currProgram->setUniformCubemap("uniform_shadowmap", currLight->getShadowBuffer()->getCubemap());
    //  currProgram->setUniformMat4("uniform_invViewMatrix", glm::inverse(kit::Light::m_cameraCache->getTransformMatrix()));
    //}
    //else
    //{
      currProgram = this->m_programPointNS;
    //}
    
    currProgram->use();
    currProgram->setUniform3f("uniform_lightColor", currLight->getColor());
    currProgram->setUniformMat4("uniform_MVPMatrix", mvp);
    //currProgram->setUniform1f("uniform_lightRadius", currLight->getRadius());
    currProgram->setUniform3f("uniform_lightPosition", glm::vec3(v * glm::vec4(currLight->getPosition(), 1.0f)));    
    currProgram->setUniform4f("uniform_lightFalloff", currLight->getAttenuation());
    
    currProgram->setUniformMat4("uniform_MVMatrix", mv);
    currProgram->setUniform2f("uniform_projConst", glm::vec2(px, py));
    
    kit::GL::enable(GL_CULL_FACE);
    kit::GL::cullFace(GL_FRONT);
    this->m_pointGeometry->renderGeometry();
    kit::GL::disable(GL_CULL_FACE);
  }
  else if(currLight->getType() == kit::Light::IBL)
  {
    this->m_programIBL->use();
    this->m_programIBL->setUniform3f("uniform_lightColor", currLight->getColor());
    this->m_programIBL->setUniformCubemap("uniform_lightIrradiance", currLight->getIrradianceMap());
    this->m_programIBL->setUniformCubemap("uniform_lightRadiance", currLight->getRadianceMap());
    //this->m_programIBL->setUniformCubemap("uniform_lightReflection", currLight->getReflectionMap());
    this->m_programIBL->setUniform2f("uniform_projConst", glm::vec2(px, py));
    
    this->m_programIBL->setUniformMat4("uniform_invProjMatrix", glm::inverse(p));
    
    //KIT_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    this->m_screenQuad->render(this->m_programIBL);
    
  }
}

void kit::Renderer::setSkybox(kit::Skybox::Ptr skybox)
{
  this->m_skybox = skybox;
}

kit::Renderer::Payload::Ptr kit::Renderer::Payload::create()
{
  auto returner = std::make_shared<kit::Renderer::Payload>();
  returner->m_isSorted = false;
  return returner;
}

void kit::Renderer::Payload::assertSorted()
{
  if (!this->m_isSorted)
  {
    // Sort payload by render priority
    std::sort(this->m_renderables.begin(), this->m_renderables.end(), [](kit::Renderable::Ptr const & a, kit::Renderable::Ptr const & b) {return a->getRenderPriority() < b->getRenderPriority(); });
    this->m_isSorted = true;
  }
}

void kit::Renderer::registerPayload(kit::Renderer::Payload::Ptr payload)
{
  if(payload == nullptr)
  {
    KIT_ERR("Warning: got null payload pointer");
    return;
  }
  
  for(auto & currPayload : this->m_payload)
  {
    if(payload == currPayload)
    {
      KIT_ERR("Warning: payload was added twice, ignoring");
      return;
    }
  }
  
  this->m_payload.push_back(payload);
}

void kit::Renderer::unregisterPayload(kit::Renderer::Payload::Ptr payload)
{
  if(payload == nullptr)
  {
    KIT_ERR("Warning: got null payload pointer");
    return;
  }
  
  std::cout << "Removing payload " << payload << std::endl;
  
  if(this->m_payload.size() == 0)
  {
    std::cout << "Warning: tried to remove payload when no payload existed" << std::endl;
    return;
  }
  
  this->m_payload.erase(std::remove(this->m_payload.begin(), this->m_payload.end(), payload), this->m_payload.end());
}

kit::Texture::Ptr kit::Renderer::getBuffer()
{
  return this->m_compositionBuffer->getFrontBuffer()->getColorAttachment(0);
}

void kit::Renderer::onResize()
{
  this->updateBuffers();

  if (this->m_activeCamera)
  {
    this->m_activeCamera->setAspectRatio((float)this->m_resolution.x / (float)this->m_resolution.y);
  }
}

void kit::Renderer::renderFrame()
{
  if (this->m_metricsEnabled)
  {
    this->renderFrameWithMetrics();
  }
  else
  {
    this->renderFrameWithoutMetrics();
  }
}

void kit::Renderer::renderFrameWithoutMetrics()
{
  if (this->m_activeCamera == nullptr)
  {
    return;
  }

  // render geometry pass
  this->geometryPass();

  // render shadow pass
  this->shadowPass();

  // render light pass
  this->lightPass();

  // render forward pass
  this->forwardPass();

  // render HDR pass
  this->hdrPass();

  // render postfx pass
  this->postFXPass();

  kit::PixelBuffer::unbind();
  kit::Program::useFixed();
  
  this->m_framesCount++;
  double milli = (double)this->m_metricsFPSTimer.timeSinceStart().asMilliseconds();
  if (milli >= 1000.0)
  {
    double fps = this->m_framesCount;
    double fpsCalibrated = fps * (1000.0 / milli);
    this->m_metricsFPS = (uint32_t)fps;
    this->m_metricsFPSCalibrated = (uint32_t)fpsCalibrated;
    this->m_framesCount = 0;
    
    std::wstringstream s;
    s << std::setprecision(3) << std::fixed << std::right;
    s << L"Timed FPS:     " << std::setw(7) << this->m_metricsFPS << " fps" << std::endl;
    s << L"Calibrated FPS:    " << std::setw(7) << this->m_metricsFPSCalibrated << " fps" << std::endl;
    
    this->m_metrics->setText(s.str());
    
    this->m_metricsFPSTimer.restart();
  }


  
}

void kit::Renderer::renderFrameWithMetrics()
{
  if(this->m_activeCamera == nullptr)
  {
    return;
  }
  
  uint64_t geometryPassTime, shadowPassTime, lightPassTime, forwardPassTime, hdrPassTime, postFxPassTime;
  
  // render geometry pass
  this->m_metricsTimer->start();
  this->geometryPass();
  geometryPassTime = this->m_metricsTimer->end();

  // render shadow pass
  this->m_metricsTimer->start();
  this->shadowPass();
  shadowPassTime = this->m_metricsTimer->end();
  
  // render light pass
  this->m_metricsTimer->start();
  this->lightPass();
  lightPassTime = this->m_metricsTimer->end();
  
  // render forward pass
  this->m_metricsTimer->start();
  this->forwardPass();
  forwardPassTime = this->m_metricsTimer->end();

  // render HDR pass
  this->m_metricsTimer->start();
  this->hdrPass();
  hdrPassTime = this->m_metricsTimer->end();
  
  // render postfx pass
  this->m_metricsTimer->start();
  this->postFXPass();
  postFxPassTime = this->m_metricsTimer->end();

  uint64_t totalTime = geometryPassTime + lightPassTime + forwardPassTime + shadowPassTime + hdrPassTime + postFxPassTime;
  
  this->m_framesCount++;
  double milli = (double)this->m_metricsFPSTimer.timeSinceStart().asMilliseconds();
  if (milli >= 1000.0)
  {
    double fps = this->m_framesCount;
    double fpsCalibrated = fps * (1000.0 / milli);
    this->m_metricsFPS = (uint32_t)fps;
    this->m_metricsFPSCalibrated = (uint32_t)fpsCalibrated;
    this->m_framesCount = 0;
    this->m_metricsFPSTimer.restart();
  }

  std::wstringstream s;
  s << std::setprecision(3) << std::fixed << std::right;
  s << L"Rendertime:    " << std::setw(7) << (((double)totalTime        /1000.0)/1000.0) << " ms" << std::endl;
  s << L"Potential FPS: " << std::setw(7) << 1000.0/(((double)totalTime / 1000.0) / 1000.0) << " fps" << std::endl;
  s << L"Timed FPS:     " << std::setw(7) << this->m_metricsFPS << " fps" << std::endl;
  s << L"Actual FPS:    " << std::setw(7) << this->m_metricsFPSCalibrated << " fps" << std::endl;
  s << std::endl;
  s << L"-Passes--------" << std::endl;
  s << L"--Geometry:    " << std::setw(7) << (((double)geometryPassTime /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--Shadow:      " << std::setw(7) << (((double)shadowPassTime   /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--Light:       " << std::setw(7) << (((double)lightPassTime    /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--Forward:     " << std::setw(7) << (((double)forwardPassTime  /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--HDR:         " << std::setw(7) << (((double)hdrPassTime      /1000.0)/1000.0) << " ms" << std::endl;
  s << L"--Composition: " << std::setw(7) << (((double)postFxPassTime   /1000.0)/1000.0) << " ms" << std::endl;
  
  this->m_metrics->setText(s.str());
 
  kit::PixelBuffer::unbind();
  kit::Program::useFixed();
}

void kit::Renderer::geometryPass()
{
  kit::GL::disable(GL_BLEND);

  kit::GL::depthMask(GL_TRUE);
  kit::GL::enable(GL_DEPTH_TEST);

  // Clear and bind the geometry buffer
  this->m_geometryBuffer->clear({ glm::vec4(0.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 0.0) }, 1.0f);

  // For each payload ...
  for (auto & currPayload : this->m_payload)
  {
    currPayload->assertSorted();

    // For each renderable in current payload ...
    for (auto & currRenderable : currPayload->getRenderables())
    {
      // Render the renderable in deferred mode
      currRenderable->renderDeferred(shared_from_this());
    }
  }
}

void kit::Renderer::shadowPass()
{
  if (!this->m_shadowsEnabled) return;

  kit::GL::depthMask(GL_TRUE);
  kit::GL::enable(GL_DEPTH_TEST);
  kit::GL::disable(GL_BLEND);

  // For each payload ...
  // (note that all payloads are isolated from each other in terms of shadows)
  for (kit::Renderer::Payload::Ptr & currPayload : this->m_payload)
  {
    // For each light in current payload ...
    for (kit::Light::Ptr & currLight : currPayload->getLights())
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
        for (kit::Renderable::Ptr & currRenderable : currPayload->getRenderables())
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
        for (kit::Renderable::Ptr & currRenderable : currPayload->getRenderables())
        {
          // Ignore renderable if its not a shadowcaster
          if (!currRenderable->isShadowCaster())
          {
            continue;
          }

          // Render the renderable to the shadowmap
          currRenderable->renderShadows(currLight->getDirectionalViewMatrix() * currLight->getDirectionalModelMatrix(this->m_activeCamera->getPosition(), this->m_activeCamera->getForward()), currLight->getDirectionalProjectionMatrix());
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
  kit::GL::enable(GL_BLEND);
  kit::GL::blendFunc(GL_ONE, GL_ONE);
  kit::GL::disable(GL_DEPTH_TEST);
  kit::GL::depthMask(GL_FALSE);
  //kit::GL::disable(GL_CULL_FACE);
  
  // Clear and bind the light accumulation buffer
  glm::mat4 invViewMatrix;

  this->m_accumulationBuffer->clear({ glm::vec4(0.0, 0.0, 0.0, 1.0) });
  invViewMatrix = glm::inverse(this->m_activeCamera->getViewMatrix());

  // Update our light programs
  this->m_programIBL->setUniformMat4("uniform_invViewMatrix", invViewMatrix);
  this->m_programSpot->setUniformMat4("uniform_invViewMatrix", invViewMatrix);
  this->m_programDirectional->setUniformMat4("uniform_invViewMatrix", invViewMatrix);

  /*
  // TODO fix this
  kit::PixelBuffer::Ptr gbuffer = this->m_geometryBuffer;

  if (this->m_shadowsEnabled)
  {
    this->m_programDirectional->setUniformTexture("uniform_textureA", gbuffer->getColorAttachment(0));
    this->m_programDirectional->setUniformTexture("uniform_textureB", gbuffer->getColorAttachment(1));
    this->m_programDirectional->setUniformTexture("uniform_textureC", gbuffer->getColorAttachment(2));
    this->m_programDirectional->setUniformTexture("uniform_textureDepth", gbuffer->getDepthAttachment());

    this->m_programSpot->setUniformTexture("uniform_textureA", gbuffer->getColorAttachment(0));
    this->m_programSpot->setUniformTexture("uniform_textureB", gbuffer->getColorAttachment(1));
    this->m_programSpot->setUniformTexture("uniform_textureC", gbuffer->getColorAttachment(2));
    this->m_programSpot->setUniformTexture("uniform_textureDepth", gbuffer->getDepthAttachment());
  }

  this->m_programDirectionalNS->setUniformTexture("uniform_textureA", gbuffer->getColorAttachment(0));
  this->m_programDirectionalNS->setUniformTexture("uniform_textureB", gbuffer->getColorAttachment(1));
  this->m_programDirectionalNS->setUniformTexture("uniform_textureC", gbuffer->getColorAttachment(2));
  this->m_programDirectionalNS->setUniformTexture("uniform_textureDepth", gbuffer->getDepthAttachment());
  this->m_programSpotNS->setUniformTexture("uniform_textureA", gbuffer->getColorAttachment(0));
  this->m_programSpotNS->setUniformTexture("uniform_textureB", gbuffer->getColorAttachment(1));
  this->m_programSpotNS->setUniformTexture("uniform_textureC", gbuffer->getColorAttachment(2));
  this->m_programSpotNS->setUniformTexture("uniform_textureDepth", gbuffer->getDepthAttachment());
  this->m_programPoint->setUniformTexture("uniform_textureA", gbuffer->getColorAttachment(0));
  this->m_programPoint->setUniformTexture("uniform_textureB", gbuffer->getColorAttachment(1));
  this->m_programPoint->setUniformTexture("uniform_textureC", gbuffer->getColorAttachment(2));
  this->m_programPoint->setUniformTexture("uniform_textureDepth", gbuffer->getDepthAttachment());
  this->m_programPointNS->setUniformTexture("uniform_textureA", gbuffer->getColorAttachment(0));
  this->m_programPointNS->setUniformTexture("uniform_textureB", gbuffer->getColorAttachment(1));
  this->m_programPointNS->setUniformTexture("uniform_textureC", gbuffer->getColorAttachment(2));
  this->m_programPointNS->setUniformTexture("uniform_textureDepth", gbuffer->getDepthAttachment());
  this->m_programIBL->setUniformTexture("uniform_textureA", gbuffer->getColorAttachment(0));
  this->m_programIBL->setUniformTexture("uniform_textureB", gbuffer->getColorAttachment(1));
  this->m_programIBL->setUniformTexture("uniform_textureC", gbuffer->getColorAttachment(2));
  this->m_programIBL->setUniformTexture("uniform_textureDepth", gbuffer->getDepthAttachment());
  this->m_programEmissive->setUniformTexture("uniform_textureB", gbuffer->getColorAttachment(1));
*/
  
  // For each payload ...
  for (auto & currPayload : this->m_payload)
  {
    // For each light in current payload ...
    for (auto & currLight : currPayload->getLights())
    {
      // Render the light
      this->renderLight(currLight);
    }
  }

  // Render emissive light
  this->m_screenQuad->render(this->m_programEmissive);
}

void kit::Renderer::forwardPass()
{
  kit::GL::depthMask(GL_TRUE);
  kit::GL::enable(GL_DEPTH_TEST);
  kit::GL::enable(GL_BLEND);
  kit::GL::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Bind the accumulation buffer
  this->m_accumulationBuffer->bind();

  // If we have a skybox
  if (this->m_skybox)
  {
    kit::GL::disable(GL_CULL_FACE);
    // Render the skybox
    this->m_skybox->render(shared_from_this());
  }

  // For each payload ...
  for (auto & currPayload : this->m_payload)
  {
    currPayload->assertSorted();

    // For each renderable in current payload ...
    for (auto & currRenderable : currPayload->getRenderables())
    {
      if (currRenderable->requestAccumulationCopy())
      {
       // std::cout << "Handle " << this->m_accumulationBuffer->getHandle() << ": " << this->m_accumulationBuffer->getResolution().x << "x" << this->m_accumulationBuffer->getResolution().y << " to " << this->m_depthCopyBuffer->getHandle() << ": " << this->m_depthCopyBuffer->getResolution().x << "x" << this->m_depthCopyBuffer->getResolution().y << std::endl;
        KIT_GL(glBlitNamedFramebuffer(
          this->m_accumulationBuffer->getHandle(), 
          this->m_accumulationCopy->getHandle(), 
          0, 0, this->m_accumulationBuffer->getResolution().x, this->m_accumulationBuffer->getResolution().y,
          0, 0, this->m_accumulationCopy->getResolution().x, this->m_accumulationCopy->getResolution().y,
          GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST
        ));
      }

      // Render current renderable in forward mode
      currRenderable->renderForward(shared_from_this());
    }
  }
}

void kit::Renderer::hdrPass()
{
  
  kit::GL::disable(GL_BLEND);
  kit::GL::disable(GL_DEPTH_TEST);
  kit::GL::depthMask(GL_FALSE);
  kit::GL::disable(GL_CULL_FACE);


  // If we have bloom enabled
  if (this->m_bloomEnabled) {
    // Render brightpass
    //this->m_bloomBrightBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
    this->m_bloomBrightBuffer->getBackBuffer()->bind();
    this->m_bloomBrightProgram->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
    this->m_bloomBrightProgram->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());
    this->m_bloomBrightProgram->setUniformTexture("uniform_sourceTexture", this->m_accumulationBuffer->getColorAttachment(0));
    this->m_screenQuad->render(this->m_bloomBrightProgram);
    this->m_bloomBrightBuffer->flip();

    // If bloom quality is high
    if (this->m_bloomQuality == High)
    {
      // Blit the bright buffer to the first bloom blur buffer
      this->m_bloomBlurBuffer2->blitFrom(this->m_bloomBrightBuffer);
      this->m_bloomBlurBuffer2->flip();

      // Blur the first bloom blur buffer
      for (uint32_t i = 0; i < this->m_bloomBlurLevel2; i++)
      {
        //this->m_bloomBlurBuffer2->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
        this->m_bloomBlurBuffer2->getBackBuffer()->bind();
        this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer2->getFrontBuffer()->getColorAttachment(0));
        this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer2->getResolution().x));
        this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
        this->m_screenQuad->render(this->m_bloomBlurProgram);
        this->m_bloomBlurBuffer2->flip();

        //this->m_bloomBlurBuffer2->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
        this->m_bloomBlurBuffer2->getBackBuffer()->bind(); 
        this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer2->getFrontBuffer()->getColorAttachment(0));
        this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer2->getResolution().x));
        this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
        this->m_screenQuad->render(this->m_bloomBlurProgram);
        this->m_bloomBlurBuffer2->flip();
      }

      // Blit the first bloom blur buffer to the second bloom blur buffer
      this->m_bloomBlurBuffer4->blitFrom(this->m_bloomBlurBuffer2);
      this->m_bloomBlurBuffer4->flip();

      // Blur the second bloom blur buffer
      for (uint32_t i = 0; i < this->m_bloomBlurLevel4; i++)
      {
        //this->m_bloomBlurBuffer4->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
        this->m_bloomBlurBuffer4->getBackBuffer()->bind();
        this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer4->getFrontBuffer()->getColorAttachment(0));
        this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer4->getResolution().x));
        this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
        this->m_screenQuad->render(this->m_bloomBlurProgram);
        this->m_bloomBlurBuffer4->flip();

        //this->m_bloomBlurBuffer4->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
        this->m_bloomBlurBuffer4->getBackBuffer()->bind();
        this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer4->getFrontBuffer()->getColorAttachment(0));
        this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer4->getResolution().x));
        this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
        this->m_screenQuad->render(this->m_bloomBlurProgram);
        this->m_bloomBlurBuffer4->flip();
      }

      // Blit the second bloom blur buffer to the third bloom blur buffer
      this->m_bloomBlurBuffer8->blitFrom(this->m_bloomBlurBuffer4);
      this->m_bloomBlurBuffer8->flip();
    }
    // .. Or if bloom quality is low
    else
    {
      // Blir the bright buffer directly to the third bloom blur buffer
      this->m_bloomBlurBuffer8->blitFrom(this->m_bloomBrightBuffer);
      this->m_bloomBlurBuffer8->flip();
    }

    // Blur the third bloom blur buffer
    for (uint32_t i = 0; i < this->m_bloomBlurLevel8; i++)
    {
      //this->m_bloomBlurBuffer8->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      this->m_bloomBlurBuffer8->getBackBuffer()->bind();
      this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer8->getFrontBuffer()->getColorAttachment(0));
      this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer8->getResolution().x));
      this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
      this->m_screenQuad->render(this->m_bloomBlurProgram);
      this->m_bloomBlurBuffer8->flip();

      //this->m_bloomBlurBuffer8->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      this->m_bloomBlurBuffer8->getBackBuffer()->bind();
      this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer8->getFrontBuffer()->getColorAttachment(0));
      this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer8->getResolution().x));
      this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
      this->m_screenQuad->render(this->m_bloomBlurProgram);
      this->m_bloomBlurBuffer8->flip();
    }

    // Blit the third bloom blur buffer to the fourth bloom blur buffer
    this->m_bloomBlurBuffer16->blitFrom(this->m_bloomBlurBuffer8);
    this->m_bloomBlurBuffer16->flip();

    // Blur the fourth bloom blur buffer
    for (uint32_t i = 0; i < this->m_bloomBlurLevel16; i++)
    {
      //this->m_bloomBlurBuffer16->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      this->m_bloomBlurBuffer16->getBackBuffer()->bind();
      this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer16->getFrontBuffer()->getColorAttachment(0));
      this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer16->getResolution().x));
      this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
      this->m_screenQuad->render(this->m_bloomBlurProgram);
      this->m_bloomBlurBuffer16->flip();

      //this->m_bloomBlurBuffer16->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      this->m_bloomBlurBuffer16->getBackBuffer()->bind();
      this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer16->getFrontBuffer()->getColorAttachment(0));
      this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer16->getResolution().x));
      this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
      this->m_screenQuad->render(this->m_bloomBlurProgram);
      this->m_bloomBlurBuffer16->flip();
    }

    // Blit the fourth bloom blur buffer to our fifth bloom blur buffer
    this->m_bloomBlurBuffer32->blitFrom(this->m_bloomBlurBuffer16);
    this->m_bloomBlurBuffer32->flip();

    // Blur the fifth bloom blur buffer
    for (uint32_t i = 0; i < this->m_bloomBlurLevel32; i++)
    {
      //this->m_bloomBlurBuffer32->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      this->m_bloomBlurBuffer32->getBackBuffer()->bind();
      this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer32->getFrontBuffer()->getColorAttachment(0));
      this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer32->getResolution().x));
      this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(1.0, 0.0));
      this->m_screenQuad->render(this->m_bloomBlurProgram);
      this->m_bloomBlurBuffer32->flip();

      //this->m_bloomBlurBuffer32->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
      this->m_bloomBlurBuffer32->getBackBuffer()->bind();
      this->m_bloomBlurProgram->setUniformTexture("uniform_sourceTexture", this->m_bloomBlurBuffer32->getFrontBuffer()->getColorAttachment(0));
      this->m_bloomBlurProgram->setUniform1f("uniform_resolution", float(this->m_bloomBlurBuffer32->getResolution().x));
      this->m_bloomBlurProgram->setUniform2f("uniform_dir", glm::vec2(0.0, 1.0));
      this->m_screenQuad->render(this->m_bloomBlurProgram);
      this->m_bloomBlurBuffer32->flip();
    }

    // Select the right HDR tonemap program to use, based on quality and if we have a dirt texture
    kit::Program::Ptr hdrProgram = (this->m_bloomQuality == High ? (this->m_bloomDirtTexture ? this->m_hdrTonemapBloomHighDirt : this->m_hdrTonemapBloomHigh) : (this->m_bloomDirtTexture ? this->m_hdrTonemapBloomLowDirt : this->m_hdrTonemapBloomLow));

    hdrProgram->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
    hdrProgram->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());

    // Setup the tonemap program
    hdrProgram->setUniformTexture("uniform_sourceTexture", this->m_accumulationBuffer->getColorAttachment(0));
    if (this->m_bloomQuality == High)
    {
      hdrProgram->setUniformTexture("uniform_bloom2Texture", this->m_bloomBlurBuffer2->getFrontBuffer()->getColorAttachment(0));
      hdrProgram->setUniformTexture("uniform_bloom4Texture", this->m_bloomBlurBuffer4->getFrontBuffer()->getColorAttachment(0));
    }
    hdrProgram->setUniformTexture("uniform_bloom8Texture", this->m_bloomBlurBuffer8->getFrontBuffer()->getColorAttachment(0));
    hdrProgram->setUniformTexture("uniform_bloom16Texture", this->m_bloomBlurBuffer16->getFrontBuffer()->getColorAttachment(0));
    hdrProgram->setUniformTexture("uniform_bloom32Texture", this->m_bloomBlurBuffer32->getFrontBuffer()->getColorAttachment(0));

    // Clear our composition buffer and render our HDR tonemap program
    //this->m_compositionBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
    this->m_compositionBuffer->getBackBuffer()->bind();
    this->m_screenQuad->render(hdrProgram);
    this->m_compositionBuffer->flip();
  }
  // ... Or if bloom is not enabled
  else
  {
    // Do regular HDR tonemapping
    this->m_hdrTonemap->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
    this->m_hdrTonemap->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());
    this->m_hdrTonemap->setUniformTexture("uniform_sourceTexture", this->m_accumulationBuffer->getColorAttachment(0));
    //this->m_compositionBuffer->clear({ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
    this->m_compositionBuffer->getBackBuffer()->bind();
    this->m_screenQuad->render(this->m_hdrTonemap);
    this->m_compositionBuffer->flip();
  }

}

void kit::Renderer::postFXPass()
{
  
  kit::GL::disable(GL_BLEND);
  kit::GL::disable(GL_DEPTH_TEST);
  kit::GL::depthMask(GL_FALSE);
  kit::GL::disable(GL_CULL_FACE);

  if (this->m_fringeEnabled)
  {
    this->m_compositionBuffer->getBackBuffer()->bind();
    this->m_fringeProgram->setUniformTexture("uniform_sourceTexture", this->m_compositionBuffer->getFrontBuffer()->getColorAttachment(0));
    this->m_screenQuad->render(this->m_fringeProgram);
    this->m_compositionBuffer->flip();
    kit::Program::useFixed();
  }

  if (this->m_fxaaEnabled)
  {
    this->m_compositionBuffer->getBackBuffer()->bind();
    this->m_fxaaProgram->setUniformTexture("uniform_sourceTexture", this->m_compositionBuffer->getFrontBuffer()->getColorAttachment(0));
    this->m_screenQuad->render(this->m_fxaaProgram);
    this->m_compositionBuffer->flip();
  }

  // sRGB conversion
  if(this->m_srgbEnabled)
  {
    this->m_compositionBuffer->getBackBuffer()->bind();
    this->m_srgbProgram->setUniformTexture("uniform_sourceTexture", this->m_compositionBuffer->getFrontBuffer()->getColorAttachment(0));
    this->m_screenQuad->render(this->m_srgbProgram);
    this->m_compositionBuffer->flip();
  }
  
  if (this->m_ccEnabled && this->m_ccLookupTable != nullptr)
  {
    this->m_compositionBuffer->getBackBuffer()->bind();
    this->m_ccProgram->setUniformTexture("uniform_sourceTexture", this->m_compositionBuffer->getFrontBuffer()->getColorAttachment(0));
    this->m_ccProgram->setUniformTexture("uniform_lut", this->m_ccLookupTable);
    this->m_screenQuad->render(this->m_ccProgram);
    this->m_compositionBuffer->flip();
  }
}

void kit::Renderer::setActiveCamera(kit::Camera::Ptr camera)
{
  this->m_activeCamera = camera;
  if(this->m_activeCamera)
  {
    this->m_activeCamera->setAspectRatio((float)this->m_resolution.x / (float)this->m_resolution.y);
    this->m_hdrTonemap->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());
    this->m_hdrTonemapBloomHigh->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());
    this->m_hdrTonemapBloomHighDirt->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());
    this->m_hdrTonemapBloomLow->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());
    this->m_hdrTonemapBloomLowDirt->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());
    this->m_bloomBrightProgram->setUniform1f("uniform_exposure", this->m_activeCamera->getExposure());
    this->m_hdrTonemap->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
    this->m_hdrTonemapBloomHigh->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
    this->m_hdrTonemapBloomHighDirt->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
    this->m_hdrTonemapBloomLow->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
    this->m_hdrTonemapBloomLowDirt->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
    this->m_bloomBrightProgram->setUniform1f("uniform_whitepoint", this->m_activeCamera->getWhitepoint());
  }
}

kit::Camera::Ptr kit::Renderer::getActiveCamera()
{
  return this->m_activeCamera;
}

void kit::Renderer::setCCLookupTable(kit::Texture::Ptr tex)
{
  this->m_ccLookupTable = tex;
}

void kit::Renderer::loadCCLookupTable(const std::string&name)
{
  this->m_ccLookupTable = kit::Texture::create3DFromFile("./data/luts/" + name, Texture::RGB8);
}

kit::Texture::Ptr kit::Renderer::getCCLookupTable()
{
  return this->m_ccLookupTable;
}

void kit::Renderer::setExposure(float exposure)
{
  if (!this->m_activeCamera)
  {
    return;
  }

  this->m_activeCamera->setExposure(exposure);
  this->m_hdrTonemap->setUniform1f("uniform_exposure", exposure);
  this->m_hdrTonemapBloomHigh->setUniform1f("uniform_exposure", exposure);
  this->m_hdrTonemapBloomHighDirt->setUniform1f("uniform_exposure", exposure);
  this->m_hdrTonemapBloomLow->setUniform1f("uniform_exposure", exposure);
  this->m_hdrTonemapBloomLowDirt->setUniform1f("uniform_exposure", exposure);
  this->m_bloomBrightProgram->setUniform1f("uniform_exposure", exposure);
}

float kit::Renderer::getExposure()
{
  if (this->m_activeCamera == nullptr) return 0.0;
  return this->m_activeCamera->getExposure();
}

void kit::Renderer::setWhitepoint(float whitepoint)
{
  if (!this->m_activeCamera)
  {
    return;
  }

  this->m_activeCamera->setWhitepoint(whitepoint);
  this->m_hdrTonemap->setUniform1f("uniform_whitepoint", whitepoint);
  this->m_hdrTonemapBloomHigh->setUniform1f("uniform_whitepoint", whitepoint);
  this->m_hdrTonemapBloomHighDirt->setUniform1f("uniform_whitepoint", whitepoint);
  this->m_hdrTonemapBloomLow->setUniform1f("uniform_whitepoint", whitepoint);
  this->m_hdrTonemapBloomLowDirt->setUniform1f("uniform_whitepoint", whitepoint);
  this->m_bloomBrightProgram->setUniform1f("uniform_whitepoint", whitepoint);
}

float kit::Renderer::getWhitepoint()
{
  if (this->m_activeCamera == nullptr) return 0.0;
  return this->m_activeCamera->getWhitepoint();
}

void kit::Renderer::setBloom(bool enabled)
{
  this->m_bloomEnabled = enabled;
}

bool const & kit::Renderer::getBloom()
{
  return this->m_bloomEnabled;
}

void kit::Renderer::setFXAA(bool enabled)
{
  this->m_fxaaEnabled = enabled;
}

bool const & kit::Renderer::getFXAA()
{
  return this->m_fxaaEnabled;
}

void kit::Renderer::setInternalResolution(float size)
{
  this->m_internalResolution = size;
  this->onResize();
}

float const & kit::Renderer::getInternalResolution()
{
  return this->m_internalResolution;
}

void kit::Renderer::setResolution(glm::uvec2 resolution)
{
  this->m_resolution = resolution;
  this->onResize();
}

void kit::Renderer::setShadows(bool enabled)
{
  this->m_shadowsEnabled = enabled;
}

bool const & kit::Renderer::getShadows()
{
  return this->m_shadowsEnabled;
}

void kit::Renderer::setSceneFringe(bool enabled)
{
  this->m_fringeEnabled = enabled;
}

bool const & kit::Renderer::getSceneFringe()
{
  return this->m_fringeEnabled;
}

void kit::Renderer::setSceneFringeExponential(float e)
{
  this->m_fringeExponential = e;
  this->m_fringeProgram->setUniform1f("uniform_exponential", e);
}

float const & kit::Renderer::getSceneFringeExponential()
{
  return this->m_fringeExponential;
}

void kit::Renderer::setSceneFringeScale(float s)
{
  this->m_fringeScale = s;
  this->m_fringeProgram->setUniform1f("uniform_scale", s);
}

float const & kit::Renderer::getSceneFringeScale()
{
  return this->m_fringeScale;
}

void kit::Renderer::setBloomQuality(kit::Renderer::BloomQuality q)
{
  this->m_bloomQuality = q;
}

kit::Renderer::BloomQuality const & kit::Renderer::getBloomQuality()
{
  return this->m_bloomQuality;
}

void kit::Renderer::setBloomBlurLevels(uint32_t b2, uint32_t b4, uint32_t b8, uint32_t b16, uint32_t b32)
{
  this->m_bloomBlurLevel2 = b2;
  this->m_bloomBlurLevel4 = b4;
  this->m_bloomBlurLevel8 = b8;
  this->m_bloomBlurLevel16 = b16;
  this->m_bloomBlurLevel32 = b32;
}

void kit::Renderer::setBloomDirtMask(kit::Texture::Ptr m)
{
  this->m_bloomDirtTexture = m;
  this->m_hdrTonemapBloomHighDirt->setUniformTexture("uniform_bloomDirtTexture", this->m_bloomDirtTexture);
  this->m_hdrTonemapBloomLowDirt->setUniformTexture("uniform_bloomDirtTexture", this->m_bloomDirtTexture);
}

kit::Texture::Ptr kit::Renderer::getBloomDirtMask()
{
  return this->m_bloomDirtTexture;
}

bool const & kit::Renderer::getColorCorrection()
{
  return this->m_ccEnabled;
}

void kit::Renderer::setColorCorrection(bool b)
{
  this->m_ccEnabled = b;
}

void kit::Renderer::setBloomDirtMaskMultiplier(float m)
{
  this->m_bloomDirtMultiplier = m;
  this->m_hdrTonemapBloomHighDirt->setUniform1f("uniform_bloomDirtMultiplier", this->m_bloomDirtMultiplier);
  this->m_hdrTonemapBloomLowDirt->setUniformTexture("uniform_bloomDirtTexture", this->m_bloomDirtTexture);
}

float const & kit::Renderer::getBloomDirtMaskMultiplier()
{
  return this->m_bloomDirtMultiplier;
}

void kit::Renderer::setBloomTresholdBias(float t)
{
  this->m_bloomTresholdBias = t;
  this->m_bloomBrightProgram->setUniform1f("uniform_tresholdBias", this->m_bloomTresholdBias);
}

float const & kit::Renderer::getBloomTresholdBias()
{
  return this->m_bloomTresholdBias;
}

kit::Text::Ptr kit::Renderer::getMetricsText()
{
  return this->m_metrics;
}

void kit::Renderer::setGPUMetrics(bool enabled)
{
  this->m_metricsEnabled = enabled;
  if (!this->m_metricsEnabled)
  {
    this->m_metrics->setText(L"");
  }
}

bool const & kit::Renderer::getGPUMetrics()
{
  return this->m_metricsEnabled;
}

void kit::Renderer::updateBuffers()
{
  glm::uvec2 effectiveResolution(uint32_t(float(this->m_resolution.x) * this->m_internalResolution), uint32_t(float(this->m_resolution.y) * this->m_internalResolution));

  // create rendering buffers
  this->m_compositionBuffer = kit::DoubleBuffer::create(
    effectiveResolution,
    {
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB16F)
    }
  );

  this->m_geometryBuffer = kit::PixelBuffer::create(
    effectiveResolution,
    {
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8),
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16F),
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16F)
    },
    kit::PixelBuffer::AttachmentInfo(Texture::DepthComponent24)
    );

  this->m_accumulationBuffer = kit::PixelBuffer::create(
    effectiveResolution,
    {
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16F)
    },
    kit::PixelBuffer::AttachmentInfo(this->m_geometryBuffer->getDepthAttachment())
    );

  this->m_accumulationCopy = kit::PixelBuffer::create(
    effectiveResolution,
    {
      kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16F)
    },
    kit::PixelBuffer::AttachmentInfo(kit::Texture::DepthComponent24)
  );

  this->m_bloomBrightBuffer = kit::DoubleBuffer::create(effectiveResolution, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });

  glm::uvec2 bloom2Res(uint32_t(float(effectiveResolution.x) * (1.0f / 2.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 2.0f)));
  this->m_bloomBlurBuffer2 = kit::DoubleBuffer::create(bloom2Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  glm::uvec2 bloom4Res(uint32_t(float(effectiveResolution.x) * (1.0f / 4.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 4.0f)));
  this->m_bloomBlurBuffer4 = kit::DoubleBuffer::create(bloom4Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  glm::uvec2 bloom8Res(uint32_t(float(effectiveResolution.x) * (1.0f / 8.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 8.0f)));
  this->m_bloomBlurBuffer8 = kit::DoubleBuffer::create(bloom8Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  glm::uvec2 bloom16Res(uint32_t(float(effectiveResolution.x) * (1.0f / 16.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 16.0f)));
  this->m_bloomBlurBuffer16 = kit::DoubleBuffer::create(bloom16Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });
  glm::uvec2 bloom32Res(uint32_t(float(effectiveResolution.x) * (1.0f / 32.0f)), uint32_t(float(effectiveResolution.y) * (1.0f / 32.0f)));
  this->m_bloomBlurBuffer32 = kit::DoubleBuffer::create(bloom32Res, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGB8) });

  this->m_programDirectional->setUniformTexture("uniform_textureA", this->m_geometryBuffer->getColorAttachment(0));
  this->m_programDirectional->setUniformTexture("uniform_textureB", this->m_geometryBuffer->getColorAttachment(1));
  this->m_programDirectional->setUniformTexture("uniform_textureC", this->m_geometryBuffer->getColorAttachment(2));
  this->m_programDirectional->setUniformTexture("uniform_textureDepth", this->m_geometryBuffer->getDepthAttachment());
  this->m_programDirectionalNS->setUniformTexture("uniform_textureA", this->m_geometryBuffer->getColorAttachment(0));
  this->m_programDirectionalNS->setUniformTexture("uniform_textureB", this->m_geometryBuffer->getColorAttachment(1));
  this->m_programDirectionalNS->setUniformTexture("uniform_textureC", this->m_geometryBuffer->getColorAttachment(2));
  this->m_programDirectionalNS->setUniformTexture("uniform_textureDepth", this->m_geometryBuffer->getDepthAttachment());
  this->m_programSpot->setUniformTexture("uniform_textureA", this->m_geometryBuffer->getColorAttachment(0));
  this->m_programSpot->setUniformTexture("uniform_textureB", this->m_geometryBuffer->getColorAttachment(1));
  this->m_programSpot->setUniformTexture("uniform_textureC", this->m_geometryBuffer->getColorAttachment(2));
  this->m_programSpot->setUniformTexture("uniform_textureDepth", this->m_geometryBuffer->getDepthAttachment());
  this->m_programSpotNS->setUniformTexture("uniform_textureA", this->m_geometryBuffer->getColorAttachment(0));
  this->m_programSpotNS->setUniformTexture("uniform_textureB", this->m_geometryBuffer->getColorAttachment(1));
  this->m_programSpotNS->setUniformTexture("uniform_textureC", this->m_geometryBuffer->getColorAttachment(2));
  this->m_programSpotNS->setUniformTexture("uniform_textureDepth", this->m_geometryBuffer->getDepthAttachment());
  this->m_programPoint->setUniformTexture("uniform_textureA", this->m_geometryBuffer->getColorAttachment(0));
  this->m_programPoint->setUniformTexture("uniform_textureB", this->m_geometryBuffer->getColorAttachment(1));
  this->m_programPoint->setUniformTexture("uniform_textureC", this->m_geometryBuffer->getColorAttachment(2));
  this->m_programPoint->setUniformTexture("uniform_textureDepth", this->m_geometryBuffer->getDepthAttachment());
  this->m_programPointNS->setUniformTexture("uniform_textureA", this->m_geometryBuffer->getColorAttachment(0));
  this->m_programPointNS->setUniformTexture("uniform_textureB", this->m_geometryBuffer->getColorAttachment(1));
  this->m_programPointNS->setUniformTexture("uniform_textureC", this->m_geometryBuffer->getColorAttachment(2));
  this->m_programPointNS->setUniformTexture("uniform_textureDepth", this->m_geometryBuffer->getDepthAttachment());
  this->m_programIBL->setUniformTexture("uniform_textureA", this->m_geometryBuffer->getColorAttachment(0));
  this->m_programIBL->setUniformTexture("uniform_textureB", this->m_geometryBuffer->getColorAttachment(1));
  this->m_programIBL->setUniformTexture("uniform_textureC", this->m_geometryBuffer->getColorAttachment(2));
  this->m_programIBL->setUniformTexture("uniform_textureDepth", this->m_geometryBuffer->getDepthAttachment());
  this->m_programEmissive->setUniformTexture("uniform_textureB", this->m_geometryBuffer->getColorAttachment(1));

}

kit::Skybox::Ptr kit::Renderer::getSkybox()
{
  return this->m_skybox;
}

kit::PixelBuffer::Ptr kit::Renderer::getAccumulationCopy()
{
  return this->m_accumulationCopy;
}

kit::PixelBufferPtr kit::Renderer::getGeometryBuffer()
{
  return this->m_geometryBuffer;
}

bool kit::Renderer::getSRGBEnabled()
{
  return this->m_srgbEnabled;
}

void kit::Renderer::setSRGBEnabled(const bool& v)
{
  this->m_srgbEnabled = v;
}
