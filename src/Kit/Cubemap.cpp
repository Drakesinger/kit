#include "Kit/Cubemap.hpp"

#include "Kit/Exception.hpp"
#include "Kit/IncOpenGL.hpp"

#include "Kit/stb/stb_image.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sstream>

kit::Cubemap::Cubemap()
{
  glGenTextures(1, &this->m_glHandle);
  this->m_resolution = glm::uvec2(0,0);
}

kit::Cubemap::Cubemap(uint32_t handle){
  this->m_glHandle = handle;
}

kit::Cubemap::~Cubemap()
{
  glDeleteTextures(1, &this->m_glHandle);
}

kit::Cubemap * kit::Cubemap::loadRadianceMap(const std::string& name)
{

  kit::Cubemap * returner = new kit::Cubemap();
  std::stringstream namer;
  unsigned char* bufferdata;
  int x, y, n;

  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  
  std::string datadir = "./data/env/";
  
  returner->bind();
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 5);

  for(unsigned int i = 0; i < 6; i++)
  {
    
    stbi_set_flip_vertically_on_load(0);
    // Z positive for Mip level 'i'
    namer.str(std::string());
    namer << datadir.c_str() << name.c_str() << "/rad_posz_" << i << ".tga";
    bufferdata = stbi_load(namer.str().c_str(),&x, &y, &n, 4);
    KIT_ASSERT(bufferdata != NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, i, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
    stbi_image_free(bufferdata);
    
    // Z negative for Mip level 'i'
    namer.str(std::string());
    namer << datadir.c_str() << name.c_str() << "/rad_negz_" << i << ".tga";
    bufferdata = stbi_load(namer.str().c_str(),&x, &y, &n, 4);
    KIT_ASSERT(bufferdata != NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, i, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
    stbi_image_free(bufferdata);
    
    // X positive for Mip level 'i'
    namer.str(std::string());
    namer << datadir.c_str() << name.c_str() << "/rad_posx_" << i << ".tga";
    bufferdata = stbi_load(namer.str().c_str(),&x, &y, &n, 4);
    KIT_ASSERT(bufferdata != NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
    stbi_image_free(bufferdata);
    
    // X negative for Mip level 'i'
    namer.str(std::string());
    namer << datadir.c_str() << name.c_str() << "/rad_negx_" << i << ".tga";
    bufferdata = stbi_load(namer.str().c_str(),&x, &y, &n, 4);
    KIT_ASSERT(bufferdata != NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, i, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
    stbi_image_free(bufferdata);
    
    // Y positive for Mip level 'i'
    namer.str(std::string());
    namer << datadir.c_str() << name.c_str() << "/rad_posy_" << i << ".tga";
    bufferdata = stbi_load(namer.str().c_str(),&x, &y, &n, 4);
    KIT_ASSERT(bufferdata != NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, i, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
    stbi_image_free(bufferdata);
    
    // Y negative for Mip level 'i'
    namer.str(std::string());
    namer << datadir.c_str() << name.c_str() << "/rad_negy_" << i << ".tga";
    //std::cout << "WOOT " << stbi_failure_reason()  << " LOL " << namer.str().c_str() << std::endl;
    bufferdata = stbi_load(namer.str().c_str(),&x, &y, &n, 4);
    KIT_ASSERT(bufferdata != NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, i, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
    stbi_image_free(bufferdata);
  }

  //returner->SetAnisotropicLevel(0.0);
  returner->setFilteringMode(kit::Cubemap::Trilinear);
  returner->setEdgeSamplingMode(kit::Cubemap::Clamp);
  returner->m_resolution = glm::uvec2(x, y);
  kit::Cubemap::unbind();
  return returner;
}

kit::Cubemap * kit::Cubemap::loadIrradianceMap(const std::string& name)
{
  std::cout << "Loading irradiance map " << name << std::endl;
  kit::Cubemap * returner = new kit::Cubemap();
  unsigned char* bufferdata;
  int x, y, n;

  // Create filenames
  std::string f_zpos = std::string("./data/env/") + name + std::string("/irr_posz.tga");
  std::string f_zneg = std::string("./data/env/") + name + std::string("/irr_negz.tga");
  std::string f_xpos = std::string("./data/env/") + name + std::string("/irr_posx.tga");
  std::string f_xneg = std::string("./data/env/") + name + std::string("/irr_negx.tga");
  std::string f_ypos = std::string("./data/env/") + name + std::string("/irr_posy.tga");
  std::string f_yneg = std::string("./data/env/") + name + std::string("/irr_negy.tga");

  returner->bind();

  stbi_set_flip_vertically_on_load(0);
  // Load files
  bufferdata = stbi_load(f_zpos.c_str(),&x, &y, &n, 4);
  //std::cout << "WOOT " << stbi_failure_reason()  << " LOL " << f_zpos.c_str() << std::endl;
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_zneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_xpos.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_xneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_ypos.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_yneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 5);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  returner->setFilteringMode(kit::Cubemap::Trilinear);
  returner->setEdgeSamplingMode(kit::Cubemap::Clamp);
  returner->m_resolution = glm::uvec2(x, y);
  kit::Cubemap::unbind();
  return returner;
}

kit::Cubemap * kit::Cubemap::loadSkybox(const std::string& name)
{
  std::cout << "Loading skybox " << name << std::endl;
  kit::Cubemap * returner = new kit::Cubemap();
  unsigned char* bufferdata;
  int x, y, n;

  GLenum format = GL_SRGB8_ALPHA8;
  //GLenum storage = GL_FLOAT;

  // Create filenames
  std::string f_zpos = std::string("./data/env/") + name + std::string("/sb_posz.tga");
  std::string f_zneg = std::string("./data/env/") + name + std::string("/sb_negz.tga");
  std::string f_xpos = std::string("./data/env/") + name + std::string("/sb_posx.tga");
  std::string f_xneg = std::string("./data/env/") + name + std::string("/sb_negx.tga");
  std::string f_ypos = std::string("./data/env/") + name + std::string("/sb_posy.tga");
  std::string f_yneg = std::string("./data/env/") + name + std::string("/sb_negy.tga");

  returner->bind();
  stbi_set_flip_vertically_on_load(0);
  // Load files
  bufferdata = stbi_load(f_zpos.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_zneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_xpos.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_xneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_ypos.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  bufferdata = stbi_load(f_yneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  
  returner->setFilteringMode(kit::Cubemap::Trilinear);
  returner->setEdgeSamplingMode(kit::Cubemap::Clamp);
  kit::Cubemap::unbind();
  returner->m_resolution = glm::uvec2(x, y);
  return returner;
}

kit::Cubemap * kit::Cubemap::load(const std::string& zpos, const std::string& zneg, const std::string& xpos, const std::string& xneg, const std::string& ypos, const std::string& yneg)
{
  kit::Cubemap * returner = new kit::Cubemap();

  // Load our images
  unsigned char* bufferdata;
  int x, y, n;
  returner->bind();
  
  stbi_set_flip_vertically_on_load(0);
  
  // Z Positive
  bufferdata = stbi_load(zpos.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  // Z Negative
  bufferdata = stbi_load(zneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  // X Positive
  bufferdata = stbi_load(xpos.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  // X Negative
  bufferdata = stbi_load(xneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  // Y Positive
  bufferdata = stbi_load(ypos.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  // Y Negative
  bufferdata = stbi_load(yneg.c_str(),&x, &y, &n, 4);
  KIT_ASSERT(bufferdata != NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  stbi_image_free(bufferdata);
  
  returner->setFilteringMode(kit::Cubemap::None);
  returner->setEdgeSamplingMode(kit::Cubemap::Clamp);
  returner->m_resolution = glm::uvec2(x, y);
  kit::Cubemap::unbind();
  return returner;
}

kit::Cubemap * kit::Cubemap::createDepthmap(glm::uvec2 resolution)
{
  
  kit::Cubemap * returner = new kit::Cubemap();

  returner->bind();
  
  returner->m_resolution = resolution;
  
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT32F, returner->m_resolution.x, returner->m_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT32F, returner->m_resolution.x, returner->m_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT32F, returner->m_resolution.x, returner->m_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT32F, returner->m_resolution.x, returner->m_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT32F, returner->m_resolution.x, returner->m_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT32F, returner->m_resolution.x, returner->m_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  
  returner->setFilteringMode(kit::Cubemap::None);
  returner->setEdgeSamplingMode(kit::Cubemap::Clamp);
  kit::Cubemap::unbind();
  return returner;
}

void kit::Cubemap::bind(){
  glBindTexture(GL_TEXTURE_CUBE_MAP, this->m_glHandle);
}

void kit::Cubemap::unbind()
{
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

uint32_t kit::Cubemap::getHandle(){
  return this->m_glHandle;
}

kit::Cubemap::FilteringMode kit::Cubemap::getFilteringMode()
{
  return this->m_filteringMode;
}

void kit::Cubemap::setFilteringMode(kit::Cubemap::FilteringMode mode)
{
  this->m_filteringMode = mode;
  this->bind();
  
  switch(this->m_filteringMode)
  {
    case None:
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      break;
      
    case Bilinear:
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      break;
      
    case Trilinear:
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      break;
  }
  
  kit::Cubemap::unbind();
}

kit::Cubemap::EdgeSamplingMode kit::Cubemap::getEdgeSamplingMode()
{
  return this->m_edgeSamplingMode;
}

void kit::Cubemap::setEdgeSamplingMode(kit::Cubemap::EdgeSamplingMode mode)
{
  this->m_edgeSamplingMode = mode;
  this->bind();
  
  switch(this->m_edgeSamplingMode)
  {
    case Repeat:
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
      break;
      
    case RepeatMirrored:
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
      break;
      
    case Clamp:
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      break;
#ifdef __unix__
    case ClampMirrored:
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_MIRROR_CLAMP_TO_EDGE);
      break;
#elif _WIN32
    case ClampMirrored:
      KIT_THROW("ClampMirrored not supported on windows platforms.");
      break;
#endif
  }
  
  kit::Cubemap::unbind();
}
