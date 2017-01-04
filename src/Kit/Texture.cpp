#include "Kit/Texture.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Exception.hpp"
#include "Kit/Types.hpp"

#include "Kit/stb/stb_image.h"
#include "Kit/stb/stb_image_write.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <random>
#include <array>

std::unordered_map<std::string, std::weak_ptr<kit::Texture>> kit::Texture::m_cache = std::unordered_map<std::string, std::weak_ptr<kit::Texture>>();

kit::Texture::Texture(Type t)
{
#ifndef KIT_SHITTY_INTEL
  glCreateTextures(t, 1, &m_glHandle);
#else
  glGenTextures(1, &m_glHandle);
#endif

  m_type = t;
}

kit::Texture::~Texture()
{
  std::cout << "Removing texture \"" << m_filename << "\"" << std::endl;
  glDeleteTextures(1, &m_glHandle);
  glGetError();
}

kit::Texture::Texture(glm::uvec2 resolution, kit::Texture::InternalFormat format, uint8_t levels) : kit::Texture(Type::Texture2D)
{
  m_internalFormat    = format;
  m_resolution        = glm::uvec3(resolution, 0);

  uint8_t mipLevels = levels > 0 ? levels : calculateMipLevels();
  
#ifndef KIT_SHITTY_INTEL
  glTextureStorage2D(m_glHandle, mipLevels, m_internalFormat, m_resolution.x, m_resolution.y);
#else
  bind();
  glTexStorage2D(m_type, mipLevels, m_internalFormat, m_resolution.x, m_resolution.y);
#endif

  setEdgeSamplingMode(EdgeSamplingMode::Repeat);
  setMinFilteringMode(m_minFilteringMode);
  setMagFilteringMode(m_magFilteringMode);

  setAnisotropicLevel(1.0f);
}

kit::Texture::Texture(const std::string & filename, kit::Texture::InternalFormat format, uint8_t levels, Type t) : kit::Texture(t)
{
  std::cout << "Loading texture from file \"" << filename.c_str() << "\"" << std::endl;
  m_filename = filename;
  if(t == Type::Texture2D)
  {
    m_internalFormat    = format;

    // Try to load data from file
    unsigned char* bufferdata;
    int x, y, n;

    stbi_set_flip_vertically_on_load(1);
    bufferdata = stbi_load(filename.c_str(), &x, &y, &n, 4);
    if (bufferdata == nullptr)
    {
      KIT_THROW(stbi_failure_reason());
    }

    // Set resolution
    m_resolution        = glm::uvec3(x, y, 0);

    uint8_t mipLevels = levels > 0 ? levels : calculateMipLevels();
    
    // Specify storage and upload data to GPU
  #ifndef KIT_SHITTY_INTEL
    glTextureStorage2D(m_glHandle, mipLevels, m_internalFormat, m_resolution.x, m_resolution.y);
    glTextureSubImage2D(m_glHandle, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  #else
    bind();
    glTexStorage2D(m_type, mipLevels, m_internalFormat, m_resolution.x, m_resolution.y);
    glTexSubImage2D(m_type, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  #endif

    // Free loaded data
    stbi_image_free(bufferdata);

    // Set parameters
    setEdgeSamplingMode(EdgeSamplingMode::Repeat);
    setMinFilteringMode(m_minFilteringMode);
    setMagFilteringMode(m_magFilteringMode);

    setAnisotropicLevel(1.0f);
  }
  if(t == Type::Texture3D)
  {
    m_internalFormat = format;

    // Try to load data from file
    unsigned char* bufferdata;
    int x, y, n;

    stbi_set_flip_vertically_on_load(0);
    bufferdata = stbi_load(filename.c_str(), &x, &y, &n, 4);
    if (bufferdata == nullptr) {
      KIT_THROW(stbi_failure_reason());
    }

    if (y != x*x || y%y != 0)
    {
      KIT_THROW("Failed to load 3d texture from file, not perfectly cubical");
    }

    // Set resolution
    m_resolution = glm::uvec3(x, x, x);

    // Specify storage and upload data to GPU
  #ifndef KIT_SHITTY_INTEL
    glTextureStorage3D(m_glHandle, 1, m_internalFormat, x, x, x);
    glTextureSubImage3D(m_glHandle, 0, 0, 0, 0, x, x, x, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  #else
    returner->bind();
    glTexStorage3D(returner->m_type, 1, m_internalFormat, x, x, x);
    glTexSubImage3D(returner->m_type, 0, 0, 0, 0, x, x, x, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
  #endif
    // Free loaded data
    stbi_image_free(bufferdata);

    setEdgeSamplingMode(EdgeSamplingMode::Repeat);
    setMinFilteringMode(m_minFilteringMode);
    setMagFilteringMode(m_magFilteringMode);

    setAnisotropicLevel(1.0f);
  }
}

kit::Texture * kit::Texture::createShadowmap(glm::uvec2 resolution)
{
  kit::Texture * returner = new kit::Texture(resolution, kit::Texture::DepthComponent24, 1);
#ifndef KIT_SHITTY_INTEL
  glTextureParameteri(returner->getHandle(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
#else
  returner->bind();
  glTexParameteri(returner->getType(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
#endif
  return returner;
}

std::shared_ptr<kit::Texture> kit::Texture::load(const std::string & name, bool sRGB)
{
  std::string key = name + (sRGB ? ".sRGB" : ".linear");
  InternalFormat format = sRGB ? SRGB8Alpha8 : RGBA8;
  std::string path = kit::getDataDirectory() + "textures/" + name;
  
  auto & entry = m_cache[key];
  auto sharedEntry = entry.lock();
  
  if(!sharedEntry)
  {
    entry = sharedEntry = std::make_shared<kit::Texture>(path, format);
    sharedEntry->setMinFilteringMode(FilteringMode::LinearMipmapLinear);
    sharedEntry->setMagFilteringMode(FilteringMode::Linear);
    sharedEntry->setAnisotropicLevel(4.0f);
    sharedEntry->generateMipmap();
  }

  return sharedEntry;
}

std::string kit::Texture::getFilename()
{
  return m_filename;
}

uint32_t kit::Texture::calculateMipLevels()
{
  uint32_t maxSize = (glm::max)(m_resolution.x, m_resolution.y);
  uint32_t miplevels = 1;
  while(maxSize > 1)
  {
    maxSize /= 2;
    miplevels++;
  }

  return (miplevels > 6 ? 6 : miplevels);
}

void kit::Texture::generateMipmap()
{
#ifndef KIT_SHITTY_INTEL
  glGenerateTextureMipmap(m_glHandle);
#else
  bind();
  glGenerateMipmap(m_type);
#endif
}

void kit::Texture::bind()
{
  glBindTexture(m_type, m_glHandle);
}

void kit::Texture::unbind(kit::Texture::Type t)
{
  glBindTexture(t, 0);
}

glm::vec4 kit::Texture::getPixelFloat(glm::vec3 position)
{
  glm::vec4 returner(0.0, 0.0, 0.0, 0.0);

  // Clamp position
  if (position.x > 1.0f) position.x = 1.0f;
  if (position.x < 0.0f) position.x = 0.0f;
  if (position.y > 1.0f) position.y = 1.0f;
  if (position.y < 0.0f) position.x = 0.0f;
  if (position.z > 1.0f) position.z = 1.0f;
  if (position.z < 0.0f) position.x = 0.0f;

  // Flip Y
  position.y = 1.0f - position.y;

  // Get a fragment position
  glm::uvec3 fragPosition(uint32_t(position.x * float(m_resolution.x)), uint32_t(position.y * float(m_resolution.y)), uint32_t(position.z * float(m_resolution.z)));

  if (fragPosition.x >= m_resolution.x) fragPosition.x = m_resolution.x - 1;
  if (fragPosition.y >= m_resolution.y) fragPosition.y = m_resolution.y - 1;

  uint32_t numFloats = m_resolution.x * m_resolution.y * 4;
  size_t dataSize = numFloats * sizeof(float);
  std::vector<float> data(numFloats);

  // Download pixels from the GPU
#ifndef KIT_SHITTY_INTEL
  glGetTextureImage(m_glHandle, 0, GL_RGBA, GL_FLOAT, (GLsizei)dataSize, &data[0]);
#else
  bind();
  glGetTexImage(m_type, 0, GL_RGBA, GL_FLOAT, &data[0]);
#endif

  // Fill the returner
  returner.x = data[ ( (m_resolution.x * m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * m_resolution.x)) * 4    ];
  returner.y = data[ ( (m_resolution.x * m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * m_resolution.x)) * 4 + 1];
  returner.z = data[ ( (m_resolution.x * m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * m_resolution.x)) * 4 + 2];
  returner.w = data[ ( (m_resolution.x * m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * m_resolution.x)) * 4 + 3];

  // Return the returner
  return returner;
}

glm::uvec4 kit::Texture::getPixelUint(glm::vec3 position)
{
  glm::uvec4 returner(0, 0, 0, 0);

  // Clamp position
  if (position.x > 1.0f) position.x = 1.0f;
  if (position.x < 0.0f) position.x = 0.0f;
  if (position.y > 1.0f) position.y = 1.0f;
  if (position.y < 0.0f) position.x = 0.0f;
  if (position.z > 1.0f) position.z = 1.0f;
  if (position.z < 0.0f) position.x = 0.0f;

  // Flip Y
  position.y = 1.0f - position.y;

  // Get a fragment position
  glm::uvec3 fragPosition(uint32_t(position.x * float(m_resolution.x)), uint32_t(position.y * float(m_resolution.y)), uint32_t(position.z * float(m_resolution.z)));

  // Clamp again, because now we are in pixelspace, and we dont really want to be able to pick at 1.0, as that is one pixel outside. Ugly but it works.
  if (fragPosition.x >= m_resolution.x) fragPosition.x = m_resolution.x - 1;
  if (fragPosition.y >= m_resolution.y) fragPosition.y = m_resolution.y - 1;

  uint32_t numUints = m_resolution.x * m_resolution.y * 4;
  size_t dataSize = numUints * sizeof(uint32_t);
  std::vector<uint32_t> data(numUints);

  // Download pixels from the GPU
#ifndef KIT_SHITTY_INTEL
  glGetTextureImage(m_glHandle, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, (GLsizei)dataSize, &data[0]);
#else
  bind();
  glGetTexImage(m_type, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT,  &data[0]);
#endif

  // Fill the returner
  returner.x = data[((m_resolution.x * m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * m_resolution.x)) * 4];
  returner.y = data[((m_resolution.x * m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * m_resolution.x)) * 4 + 1];
  returner.z = data[((m_resolution.x * m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * m_resolution.x)) * 4 + 2];
  returner.w = data[((m_resolution.x * m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * m_resolution.x)) * 4 + 3];

  // Return the returner
  return returner;
}

uint32_t kit::Texture::getArraySize()
{
  return m_arraySize;
}

glm::uvec3 kit::Texture::getResolution()
{
  return m_resolution;
}

kit::Texture::EdgeSamplingMode kit::Texture::getEdgeSamplingMode(EdgeSamplingAxis axis)
{
  switch(axis)
  {
    case kit::Texture::All:
      KIT_THROW("Cant get edge sampling mode from all axes, do each one individually");
      break;

    case kit::Texture::S:
      return m_edgeSamplingModeS;
      break;

    case kit::Texture::T:
      return m_edgeSamplingModeT;
      break;

    case kit::Texture::R:
      return m_edgeSamplingModeR;
      break;
  }

  KIT_ERR("Warning: Invalid parameter passed as axis");
  return kit::Texture::ClampToEdge;
}

void kit::Texture::setEdgeSamplingMode(kit::Texture::EdgeSamplingMode mode, kit::Texture::EdgeSamplingAxis axis)
{
  switch(axis)
  {
    case kit::Texture::All:
      m_edgeSamplingModeS = mode;
      m_edgeSamplingModeT = mode;
      m_edgeSamplingModeR = mode;
#ifndef KIT_SHITTY_INTEL
      glTextureParameteri(m_glHandle, S, mode);
      glTextureParameteri(m_glHandle, T, mode);
      glTextureParameteri(m_glHandle, R, mode);
#else 
      bind();
      glTexParameteri(m_type, S, mode);
      glTexParameteri(m_type, T, mode);
      glTexParameteri(m_type, R, mode);
#endif
      break;

    case kit::Texture::S:
      m_edgeSamplingModeS = mode;
#ifndef KIT_SHITTY_INTEL
      glTextureParameteri(m_glHandle, axis, mode);
#else 
      bind();
      glTexParameteri(m_type, axis, mode);
#endif
      break;

    case kit::Texture::T:
      m_edgeSamplingModeT = mode;
#ifndef KIT_SHITTY_INTEL
      glTextureParameteri(m_glHandle, axis, mode);
#else 
      bind();
      glTexParameteri(m_type, axis, mode);
#endif
      break;

    case kit::Texture::R:
      m_edgeSamplingModeR = mode;
#ifndef KIT_SHITTY_INTEL
      glTextureParameteri(m_glHandle, axis, mode);
#else 
      bind();
      glTexParameteri(m_type, axis, mode);
#endif
      break;
  }
}

kit::Texture::FilteringMode kit::Texture::getMinFilteringMode()
{
  return m_minFilteringMode;
}

void kit::Texture::setMinFilteringMode(kit::Texture::FilteringMode mode)
{
  m_minFilteringMode = mode;
  
#ifndef KIT_SHITTY_INTEL
  glTextureParameteri(m_glHandle, GL_TEXTURE_MIN_FILTER, m_minFilteringMode);
#else 
  bind();
  glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, m_minFilteringMode);
#endif
}

kit::Texture::FilteringMode kit::Texture::getMagFilteringMode()
{
  return m_magFilteringMode;
}

void kit::Texture::setMagFilteringMode(kit::Texture::FilteringMode mode)
{
  m_magFilteringMode = mode;
  
#ifndef KIT_SHITTY_INTEL
  glTextureParameteri(m_glHandle, GL_TEXTURE_MAG_FILTER, m_magFilteringMode);
#else 
  bind();
  glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, m_magFilteringMode);
#endif
}

float kit::Texture::getAnisotropicLevel()
{
  return m_anisotropicLevel;
}

void kit::Texture::setAnisotropicLevel(float l)
{
  m_anisotropicLevel = l;
#ifndef KIT_SHITTY_INTEL
  glTextureParameterf(m_glHandle, GL_TEXTURE_MAX_ANISOTROPY_EXT, l);
#else
  bind();
  glTexParameterf(m_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, l);
#endif
}

uint32_t kit::Texture::getHandle()
{
  return m_glHandle;
}

kit::Texture::InternalFormat kit::Texture::getInternalFormat()
{
  return m_internalFormat;
}

bool kit::Texture::saveToFile(const std::string&filename)
{
  // Fetch data from GPU
  unsigned char * data = new unsigned char[(m_resolution.x * m_resolution.y) * 4];

#ifndef KIT_SHITTY_INTEL
  glGetTextureImage(m_glHandle, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLsizei)(((m_resolution.x * m_resolution.y) * 4) * sizeof(unsigned char)), &data[0]);
#else
  bind();
  glGetTexImage(m_type, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
#endif 

  stbi_write_set_flip_vertically_on_save(1);

  // Write data to disk
  if(stbi_write_tga(filename.c_str(), m_resolution.x, m_resolution.y, 4, (void*)data) == 0)
  {
    KIT_ERR("Failed to write image to file")
    delete[] data;
    return false;
  }

  delete[] data;
  return true;
}
