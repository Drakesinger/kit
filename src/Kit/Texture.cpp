#include "Kit/Texture.hpp"
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

std::map<std::string, kit::Texture::Ptr> kit::Texture::m_cachedTextures = std::map<std::string, kit::Texture::Ptr>();

kit::Texture::Texture(Type t)
{
#ifndef KIT_SHITTY_INTEL
  glCreateTextures(t, 1, &this->m_glHandle);
#else
  glGenTextures(1, &this->m_glHandle);
#endif

  this->m_arraySize = 0;  
  this->m_edgeSamplingModeS = kit::Texture::Repeat;
  this->m_edgeSamplingModeT = kit::Texture::Repeat;
  this->m_edgeSamplingModeR = kit::Texture::Repeat;
  this->m_internalFormat = RGBA8;
  this->m_magFilteringMode = kit::Texture::Linear;
  this->m_minFilteringMode = kit::Texture::LinearMipmapLinear;
  this->m_resolution = glm::uvec3(0, 0, 0);
  this->m_type = t;
  this->m_anisotropicLevel = 8.0f;
}

kit::Texture::~Texture()
{
  glDeleteTextures(1, &this->m_glHandle);
  glGetError();
}

kit::Texture::Ptr kit::Texture::create2D(glm::uvec2 resolution, kit::Texture::InternalFormat format, kit::Texture::EdgeSamplingMode edgemode, kit::Texture::FilteringMode minfilter, kit::Texture::FilteringMode magfilter)
{
  kit::Texture::Ptr returner = std::make_shared<kit::Texture>(Texture2D);
  returner->m_internalFormat    = format;
  returner->m_resolution        = glm::uvec3(resolution, 0);

#ifndef KIT_SHITTY_INTEL
  glTextureStorage2D(returner->m_glHandle, returner->calculateMipLevels(), returner->m_internalFormat, returner->m_resolution.x, returner->m_resolution.y);
#else
  returner->bind();
  glTexStorage2D(returner->m_type, returner->calculateMipLevels(), returner->m_internalFormat, returner->m_resolution.x, returner->m_resolution.y);
#endif

  returner->setEdgeSamplingMode(edgemode);

  // Stupid AMD bug https://gist.github.com/haikarainen/97959adfe4e3ca10968a
  //std::vector<GLubyte> data(returner->m_resolution.x * returner->m_resolution.y, 0);
  //glTextureSubImage2D(returner->m_glHandle, 0, 0, 0, returner->m_resolution.x, returner->m_resolution.y, GL_RED, GL_UNSIGNED_BYTE, &data[0]);
  returner->setMinFilteringMode(minfilter);
  returner->setMagFilteringMode(magfilter);

  returner->setAnisotropicLevel(8.0f);

  return returner;
}

kit::Texture::Ptr kit::Texture::create2DFromFile(const std::string&filename, kit::Texture::InternalFormat format, kit::Texture::EdgeSamplingMode edgemode, kit::Texture::FilteringMode minfilter, kit::Texture::FilteringMode magfilter)
{
  std::cout << "Loading texture from file " << filename.c_str() << std::endl;
  kit::Texture::Ptr returner = std::make_shared<kit::Texture>(Texture2D);
  returner->m_internalFormat    = format;

  // Try to load data from file
  unsigned char* bufferdata;
  int x, y, n;

  stbi_set_flip_vertically_on_load(1);
  bufferdata = stbi_load(filename.c_str(), &x, &y, &n, 4);
  if (bufferdata == nullptr)
  {
    KIT_ERR(stbi_failure_reason());
    x = 1;
    y = 1;
    n = 4;
    bufferdata = new unsigned char[4];
    bufferdata[0] = 255;
    bufferdata[1] = 0;
    bufferdata[2] = 0;
    bufferdata[3] = 255;
  }

  // Set resolution
  returner->m_resolution        = glm::uvec3(x, y, 0);

  // Specify storage and upload data to GPU
#ifndef KIT_SHITTY_INTEL
  glTextureStorage2D(returner->m_glHandle, returner->calculateMipLevels(), returner->m_internalFormat, returner->m_resolution.x, returner->m_resolution.y);
  glTextureSubImage2D(returner->m_glHandle, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
#else
  returner->bind();
  glTexStorage2D(returner->m_type, returner->calculateMipLevels(), returner->m_internalFormat, returner->m_resolution.x, returner->m_resolution.y);
  glTexSubImage2D(returner->m_type, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
#endif

  // Free loaded data
  stbi_image_free(bufferdata);

  // Set parameters
  returner->setEdgeSamplingMode(edgemode);
  returner->setMinFilteringMode(minfilter);
  returner->setMagFilteringMode(magfilter);

  returner->setAnisotropicLevel(8.0f);

  // Generate mipmap
  if(minfilter != kit::Texture::Linear  &&  minfilter != kit::Texture::Nearest)
  {
    returner->generateMipmap();
  }
  
  return returner;
}

kit::Texture::Ptr kit::Texture::create3DFromFile(const std::string&filename, kit::Texture::InternalFormat format, kit::Texture::EdgeSamplingMode edgemode, kit::Texture::FilteringMode minfilter, kit::Texture::FilteringMode magfilter)
{
  kit::Texture::Ptr returner = std::make_shared<kit::Texture>(Texture3D);
  returner->m_internalFormat = format;

  // Try to load data from file
  unsigned char* bufferdata;
  int x, y, n;

  stbi_set_flip_vertically_on_load(0);
  bufferdata = stbi_load(filename.c_str(), &x, &y, &n, 4);
  if (bufferdata == nullptr) {
    KIT_ERR(stbi_failure_reason());
    return nullptr;
  }

  if (y != x*x || y%y != 0)
  {
    KIT_ERR("Failed to load 3d texture from file, not perfectly cubical");
  }

  // Set resolution
  returner->m_resolution = glm::uvec3(x, x, x);

  // Specify storage and upload data to GPU
#ifndef KIT_SHITTY_INTEL
  glTextureStorage3D(returner->m_glHandle, 1, returner->m_internalFormat, x, x, x);
  glTextureSubImage3D(returner->m_glHandle, 0, 0, 0, 0, x, x, x, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
#else
  returner->bind();
  glTexStorage3D(returner->m_type, 1, returner->m_internalFormat, x, x, x);
  glTexSubImage3D(returner->m_type, 0, 0, 0, 0, x, x, x, GL_RGBA, GL_UNSIGNED_BYTE, bufferdata);
#endif
  // Free loaded data
  stbi_image_free(bufferdata);

  // Set parameters
  returner->setEdgeSamplingMode(edgemode);
  returner->setMinFilteringMode(minfilter);
  returner->setMagFilteringMode(magfilter);

  // Generate mipmap
  //returner->generateMipmap();

  return returner;
}

kit::Texture::Ptr kit::Texture::createShadowmap(glm::uvec2 resolution)
{
  kit::Texture::Ptr returner = kit::Texture::create2D(resolution, kit::Texture::DepthComponent24);
#ifndef KIT_SHITTY_INTEL
  glTextureParameteri(returner->m_glHandle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
#else
  returner->bind();
  glTexParameteri(returner->m_type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
#endif
  return returner;
}

kit::Texture::Ptr kit::Texture::load(const std::string&name, bool srgb)
{
  if(kit::Texture::m_cachedTextures.find(name) != kit::Texture::m_cachedTextures.end())
  {
    return kit::Texture::m_cachedTextures.at(name);
  }

  static const std::string dataDir = "./data/textures/";
  kit::Texture::Ptr returner =  kit::Texture::create2DFromFile(dataDir + name, srgb ? SRGB8Alpha8 : RGBA8, kit::Texture::Repeat, kit::Texture::LinearMipmapLinear, kit::Texture::Linear);
  returner->m_filename = name;
  returner->setAnisotropicLevel(8.0f);

  kit::Texture::m_cachedTextures[name] = returner;

  return returner;
}

std::string kit::Texture::getFilename()
{
  return this->m_filename;
}

void kit::Texture::flushCache()
{
  kit::Texture::m_cachedTextures.clear();
  kit::Texture::m_cachedTextures.clear();
}

uint32_t kit::Texture::calculateMipLevels()
{
  uint32_t maxSize = (glm::max)(this->m_resolution.x, this->m_resolution.y);
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
  glGenerateTextureMipmap(this->m_glHandle);
#else
  this->bind();
  glGenerateMipmap(this->m_type);
#endif
}

void kit::Texture::bind()
{
  glBindTexture(this->m_type, this->m_glHandle);
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
  glm::uvec3 fragPosition(uint32_t(position.x * float(this->m_resolution.x)), uint32_t(position.y * float(this->m_resolution.y)), uint32_t(position.z * float(this->m_resolution.z)));

  if (fragPosition.x >= this->m_resolution.x) fragPosition.x = this->m_resolution.x - 1;
  if (fragPosition.y >= this->m_resolution.y) fragPosition.y = this->m_resolution.y - 1;

  uint32_t numFloats = this->m_resolution.x * m_resolution.y * 4;
  size_t dataSize = numFloats * sizeof(float);
  std::vector<float> data(numFloats);

  // Download pixels from the GPU
#ifndef KIT_SHITTY_INTEL
  glGetTextureImage(this->m_glHandle, 0, GL_RGBA, GL_FLOAT, (GLsizei)dataSize, &data[0]);
#else
  this->bind();
  glGetTexImage(this->m_type, 0, GL_RGBA, GL_FLOAT, &data[0]);
#endif

  // Fill the returner
  returner.x = data[ ( (this->m_resolution.x * this->m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * this->m_resolution.x)) * 4    ];
  returner.y = data[ ( (this->m_resolution.x * this->m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * this->m_resolution.x)) * 4 + 1];
  returner.z = data[ ( (this->m_resolution.x * this->m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * this->m_resolution.x)) * 4 + 2];
  returner.w = data[ ( (this->m_resolution.x * this->m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * this->m_resolution.x)) * 4 + 3];

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
  glm::uvec3 fragPosition(uint32_t(position.x * float(this->m_resolution.x)), uint32_t(position.y * float(this->m_resolution.y)), uint32_t(position.z * float(this->m_resolution.z)));

  // Clamp again, because now we are in pixelspace, and we dont really want to be able to pick at 1.0, as that is one pixel outside. Ugly but it works.
  if (fragPosition.x >= this->m_resolution.x) fragPosition.x = this->m_resolution.x - 1;
  if (fragPosition.y >= this->m_resolution.y) fragPosition.y = this->m_resolution.y - 1;

  uint32_t numUints = this->m_resolution.x * m_resolution.y * 4;
  size_t dataSize = numUints * sizeof(uint32_t);
  std::vector<uint32_t> data(numUints);

  // Download pixels from the GPU
#ifndef KIT_SHITTY_INTEL
  glGetTextureImage(this->m_glHandle, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, (GLsizei)dataSize, &data[0]);
#else
  this->bind();
  glGetTexImage(this->m_type, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT,  &data[0]);
#endif

  // Fill the returner
  returner.x = data[((this->m_resolution.x * this->m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * this->m_resolution.x)) * 4];
  returner.y = data[((this->m_resolution.x * this->m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * this->m_resolution.x)) * 4 + 1];
  returner.z = data[((this->m_resolution.x * this->m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * this->m_resolution.x)) * 4 + 2];
  returner.w = data[((this->m_resolution.x * this->m_resolution.y) * fragPosition.z) + (fragPosition.x + (fragPosition.y * this->m_resolution.x)) * 4 + 3];

  // Return the returner
  return returner;
}

uint32_t kit::Texture::getArraySize()
{
  return this->m_arraySize;
}

glm::uvec3 kit::Texture::getResolution()
{
  return this->m_resolution;
}

kit::Texture::EdgeSamplingMode kit::Texture::getEdgeSamplingMode(EdgeSamplingAxis axis)
{
  switch(axis)
  {
    case kit::Texture::All:
      KIT_THROW("Cant get edge sampling mode from all axes, do each one individually");
      break;

    case kit::Texture::S:
      return this->m_edgeSamplingModeS;
      break;

    case kit::Texture::T:
      return this->m_edgeSamplingModeT;
      break;

    case kit::Texture::R:
      return this->m_edgeSamplingModeR;
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
      this->m_edgeSamplingModeS = mode;
      this->m_edgeSamplingModeT = mode;
      this->m_edgeSamplingModeR = mode;
#ifndef KIT_SHITTY_INTEL
      glTextureParameteri(this->m_glHandle, S, mode);
      glTextureParameteri(this->m_glHandle, T, mode);
      glTextureParameteri(this->m_glHandle, R, mode);
#else 
      this->bind();
      glTexParameteri(this->m_type, S, mode);
      glTexParameteri(this->m_type, T, mode);
      glTexParameteri(this->m_type, R, mode);
#endif
      break;

    case kit::Texture::S:
      this->m_edgeSamplingModeS = mode;
#ifndef KIT_SHITTY_INTEL
      glTextureParameteri(this->m_glHandle, axis, mode);
#else 
      this->bind();
      glTexParameteri(this->m_type, axis, mode);
#endif
      break;

    case kit::Texture::T:
      this->m_edgeSamplingModeT = mode;
#ifndef KIT_SHITTY_INTEL
      glTextureParameteri(this->m_glHandle, axis, mode);
#else 
      this->bind();
      glTexParameteri(this->m_type, axis, mode);
#endif
      break;

    case kit::Texture::R:
      this->m_edgeSamplingModeR = mode;
#ifndef KIT_SHITTY_INTEL
      glTextureParameteri(this->m_glHandle, axis, mode);
#else 
      this->bind();
      glTexParameteri(this->m_type, axis, mode);
#endif
      break;
  }
}

kit::Texture::FilteringMode kit::Texture::getMinFilteringMode()
{
  return this->m_minFilteringMode;
}

void kit::Texture::setMinFilteringMode(kit::Texture::FilteringMode mode)
{
#ifndef KIT_SHITTY_INTEL
  glTextureParameteri(this->m_glHandle, GL_TEXTURE_MIN_FILTER, mode);
#else 
  this->bind();
  glTexParameteri(this->m_type, GL_TEXTURE_MIN_FILTER, mode);
#endif
}

kit::Texture::FilteringMode kit::Texture::getMagFilteringMode()
{
  return this->m_magFilteringMode;
}

void kit::Texture::setMagFilteringMode(kit::Texture::FilteringMode mode)
{
#ifndef KIT_SHITTY_INTEL
  glTextureParameteri(this->m_glHandle, GL_TEXTURE_MAG_FILTER, mode);
#else 
  this->bind();
  glTexParameteri(this->m_type, GL_TEXTURE_MAG_FILTER, mode);
#endif
}

float kit::Texture::getAnisotropicLevel()
{
  return this->m_anisotropicLevel;
}

void kit::Texture::setAnisotropicLevel(float l)
{
#ifndef KIT_SHITTY_INTEL
  glTextureParameterf(this->m_glHandle, GL_TEXTURE_MAX_ANISOTROPY_EXT, l);
#else
  this->bind();
  glTexParameterf(this->m_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, l);
#endif
}

GLuint kit::Texture::getHandle(){
  return this->m_glHandle;
}

kit::Texture::InternalFormat kit::Texture::getInternalFormat()
{
  return this->m_internalFormat;
}

std::vector<std::string> kit::Texture::getAvailableTextures(const std::string&prefix, bool reload)
{
  std::vector<std::string> result;

  for (auto & currEntry : kit::listFilesystemEntries("./data/textures/" + prefix, true, false))
  {
    result.push_back(currEntry.filename);
  }

  return result;
}

bool kit::Texture::saveToFile(const std::string&filename)
{
  // Fetch data from GPU
  unsigned char * data = new unsigned char[(this->m_resolution.x * this->m_resolution.y) * 4];

#ifndef KIT_SHITTY_INTEL
  glGetTextureImage(this->m_glHandle, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLsizei)(((this->m_resolution.x * this->m_resolution.y) * 4) * sizeof(unsigned char)), &data[0]);
#else
  this->bind();
  glGetTexImage(this->m_type, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
#endif 

  stbi_write_set_flip_vertically_on_save(1);

  // Write data to disk
  if(stbi_write_tga(filename.c_str(), this->m_resolution.x, this->m_resolution.y, 4, (void*)data) == 0)
  {
    KIT_ERR("Failed to write image to file")
    delete[] data;
    return false;
  }

  delete[] data;
  return true;
}
