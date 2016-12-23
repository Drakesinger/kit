#include "Kit/Material.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Exception.hpp"
#include "Kit/Shader.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Program.hpp"
#include "Kit/PixelBuffer.hpp"
#include "Kit/Camera.hpp"
#include <Kit/Quad.hpp>

#include <glm/gtx/transform.hpp>
#include <sstream>
#include <fstream>

uint32_t kit::Material::m_instanceCount = 0;
kit::Program * kit::Material::m_cacheProgram = nullptr;
std::map<std::string, kit::Material*> kit::Material::m_cache = std::map<std::string, kit::Material*>();
std::map<kit::Material::ProgramFlags, kit::Program*> kit::Material::m_programCache = std::map<kit::Material::ProgramFlags, kit::Program*>();

static const char * glslVersion = "#version 430 core\n";

static const char * glslCacheVertex
="layout (location = 0) in vec3 in_position;\n\
  layout (location = 1) in vec2 in_uv;\n\
  layout (location = 0) out vec2 out_uv;\n\
  \n\
  uniform vec2 uniform_position = vec2(0.0, 0.0);\n\
  uniform vec2 uniform_size = vec2(1.0, 1.0);\n\
  \n\
  void main()\n\
  {\n\
    vec2 finalpos = uniform_position + (in_position.xy * uniform_size);\n\
    gl_Position = vec4(finalpos, in_position.z, 1.0);\n\
    gl_Position.xy *= 2.0;\n\
    gl_Position.y = 1.0 - gl_Position.y;\n\
    gl_Position.x -= 1.0;\n\
    out_uv = in_uv;\n\
  }\n";

static const char * glslCacheUtils
= "float applyGamma(float inval, float gamma)\n\
{\n\
  return pow(inval, 1.0 / gamma);\n\
}\n\
\n\
vec3 applyGamma(vec3 inval, float gamma)\n\
{\n\
  return pow(inval, vec3(1.0 / gamma));\n\
}\n\
\n\
float levelsInputRange(float inval, float rmin, float rmax)\n\
{\n\
  return min(max(inval - rmin, 0.0) / (rmax - rmin), 1.0);\n\
}\n\
\n\
vec3 levelsInputRange(vec3 inval, float rmin, float rmax)\n\
{\n\
  return min(max(inval - vec3(rmin), 0.0) / (vec3(rmax) - vec3(rmin)), 1.0);\n\
}\n\
\n\
float levelsInput(float inval, float rmin, float gamma, float rmax)\n\
{\n\
  return applyGamma(levelsInputRange(inval, rmin, rmax), gamma);\n\
}\n\
\n\
vec3 levelsInput(vec3 inval, float rmin, float gamma, float rmax)\n\
{\n\
  return applyGamma(levelsInputRange(inval, rmin, rmax), gamma);\n\
}\n\
\n\
float levelsOutput(float inval, float rmin, float rmax)\n\
{\n\
  return mix(rmin, rmax, inval);\n\
}\n\
\n\
vec3 levelsOutput(vec3 inval, float rmin, float rmax)\n\
{\n\
  return mix(vec3(rmin), vec3(rmax), inval);\n\
}\n\
\n\
float levels(float inval, float inputmin, float gamma, float inputmax, float outputmin, float outputmax)\n\
{\n\
  return levelsOutput(levelsInput(inval, inputmin, gamma, inputmax), outputmin, outputmax);\n\
}\n\
\n\
vec3 levels(vec3 inval, float inputmin, float gamma, float inputmax, float outputmin, float outputmax)\n\
{\n\
  return levelsOutput(levelsInput(inval, inputmin, gamma, inputmax), outputmin, outputmax);\n\
}\n";

static const char * glslCachePixel
="layout(location = 0) in vec2 in_texCoords;\n\
\n\
uniform sampler2D uniform_mapA;\n\
uniform vec3      uniform_defaultA;\n\
uniform sampler2D uniform_mapB;\n\
uniform float     uniform_defaultB;\n\
\n\
uniform int uniform_usemapA;\n\
uniform int uniform_usemapB;\n\
\n\
out vec4 out_color;\n\
\n\
void main()\n\
{\n\
  vec3 valA = uniform_defaultA;\n\
  float valB = uniform_defaultB;\n\
  \n\
  if(uniform_usemapA == 1)\n\
  {\n\
    valA = texture(uniform_mapA, in_texCoords).rgb;\n\
  }\n\
  \n\
  if(uniform_usemapB == 1)\n\
  {\n\
    valB = texture(uniform_mapB, in_texCoords).r;\n\
  }\n\
  \n\
  out_color = vec4(valA, valB);\n\
}\n";

void kit::Material::clearCache(std::string entry)
{
  if (entry == "")
  {
    for(auto & t : m_cache)
    {
      delete t.second;
    }
    
    m_cache.clear();
  }
  else
  {
    auto t = m_cache.find(entry);
    if(t != m_cache.end())
    {
      delete t->second;
      m_cache.erase(entry);
    }
  }
}

std::map<std::string, kit::Material*> kit::Material::getCacheList()
{
  return m_cache;
}

bool kit::Material::save(const std::string&outfilename)
{
  std::ofstream fhandle(std::string("./data/materials/") + outfilename);
  if (!fhandle)
  {
    KIT_ERR("Failed to save material");
    return false;
  }

  fhandle << "# Albedo color properties" << std::endl;
  if (m_albedoMap)
  {
    fhandle << "albedomap \"" << m_albedoMap->getFilename() << "\"" << std::endl;
  }
  fhandle << "albedo " << m_albedo.x << " " << m_albedo.y << " " << m_albedo.z << std::endl;
  fhandle << std::endl;

  fhandle << "# Occlusion properties" << std::endl;
  if (m_occlusionMap)
  {
    fhandle << "occlusionmap \"" << m_occlusionMap->getFilename() << "\"" << std::endl;
  }
  fhandle << std::endl;

  fhandle << "# Normal map properties" << std::endl;
  if (m_normalMap)
  {
    fhandle << "normalmap \"" << m_normalMap->getFilename() << "\"" << std::endl;
  }
  fhandle << std::endl;

  fhandle << "# Emissive properties" << std::endl;
  if (m_emissiveMap)
  {
    fhandle << "emissivemap \"" << m_emissiveMap->getFilename() << "\"" << std::endl;
  }
  fhandle << "emissivestrength " << m_emissiveStrength << std::endl;
  fhandle << "emissivecolor " << m_emissiveColor.x << " " << m_emissiveColor.y << " " << m_emissiveColor.z << std::endl;
  fhandle << std::endl;

  fhandle << "# Roughness properties" << std::endl;
  if (m_roughnessMap)
  {
    fhandle << "roughnessmap \"" << m_roughnessMap->getFilename() << "\"" << std::endl;
  }
  fhandle << "roughness " << m_roughness << std::endl;
  fhandle << std::endl;

  fhandle << "# Metalness properties" << std::endl;
  if (m_metalnessMap)
  {
    fhandle << "metalnessmap \"" << m_metalnessMap->getFilename() << "\"" << std::endl;
  }
  fhandle << "metalness " << m_metalness << std::endl;
  fhandle << std::endl;

  fhandle << "# Other properties" << std::endl;
  fhandle << "doublesided " << (m_doubleSided ? "true" : "false") << std::endl;
  fhandle << "depthwrite " << (m_depthWrite ? "true" : "false") << std::endl;
  fhandle << "depthread " << (m_depthRead ? "true" : "false") << std::endl;
  fhandle << "castshadows " << (m_castShadows ? "true" : "false") << std::endl;
  fhandle << "opacity " << m_opacity << std::endl;
  if (m_opacityMask)
  {
    fhandle << "opacitymask " << m_opacityMask->getFilename() << std::endl;
  }
  fhandle << "blendmode " << (m_blendMode == None ? "none" : m_blendMode == Add ? "add" : "alpha");

  fhandle << std::endl;

  fhandle.close();
  return true;
}

kit::Material * kit::Material::load(const std::string&filename, bool use_cache)
{
  kit::Material * currMaterial = nullptr;
  
  if(use_cache)
  {
    auto finder = kit::Material::m_cache.find(filename);
    bool existsInCache = (finder != kit::Material::m_cache.end());

    if(existsInCache)
    {
      return finder->second;
    }
  }
  
  currMaterial = new kit::Material();
  currMaterial->m_filename = filename;
  
  if(use_cache)
  {
    m_cache[filename] = currMaterial;
  }
  
  std::ifstream fhandle(std::string("./data/materials/") + filename);
  if(!fhandle)
  {
    KIT_ERR("Failed to load material");
    return currMaterial;
  }
  
  std::string currline = "";
  while(std::getline(fhandle, currline))
  {
    std::vector<std::string> currtokens = kit::splitString(currline);
    
    if(currtokens.size() != 0)
    {
      KIT_ASSERT(currtokens.size() >= 2 /* Invalid material parameter length */);
      
      std::string identifier = currtokens[0];
      if(identifier == std::string("albedo"))
      {
        KIT_ASSERT(currtokens.size() == 4 /* Color needs 3 float values */);
        currMaterial->m_albedo.x = (float)std::atof(currtokens[1].c_str());
		    currMaterial->m_albedo.y = (float)std::atof(currtokens[2].c_str());
		    currMaterial->m_albedo.z = (float)std::atof(currtokens[3].c_str());
      }
      else if(identifier == std::string("albedomap"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* Colormap needs 1 string value (no spaces!) */);
        currMaterial->m_albedoMap = kit::Texture::load(currtokens[1].c_str(), true);
      }
      else if (identifier == std::string("occlusionmap"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* occlusionmap needs 1 string value (no spaces!) */);
        currMaterial->m_occlusionMap = kit::Texture::load(currtokens[1].c_str(), false);
      }
      else if(identifier == std::string("emissivecolor"))
      {
        KIT_ASSERT(currtokens.size() == 4 /* Emissive color needs 3 float values */);
        currMaterial->m_emissiveColor.x = (float)std::atof(currtokens[1].c_str());
        currMaterial->m_emissiveColor.y = (float)std::atof(currtokens[2].c_str());
        currMaterial->m_emissiveColor.z = (float)std::atof(currtokens[3].c_str());
      }
      else if(identifier == std::string("emissivestrength"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* Emissive strength needs 1 float values */);
        currMaterial->m_emissiveStrength = (float)std::atof(currtokens[1].c_str());
      }
      else if (identifier == std::string("emissivemap"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* emissivemap needs 1 string value (no spaces!) */);
        currMaterial->m_emissiveMap = kit::Texture::load(currtokens[1].c_str(), true);
      }
      else if(identifier == std::string("normalmap"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* Normalmap needs 1 string value (no spaces!) */);
        currMaterial->m_normalMap = kit::Texture::load(currtokens[1].c_str(), false);
      }
      else if(identifier == std::string("roughness"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* Roughness needs 1 float value */);
        currMaterial->m_roughness = (float)std::atof(currtokens[1].c_str());
      }
      else if(identifier == std::string("roughnessmap"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* Roughnessmap needs 1 string value (no spaces!) */);
        currMaterial->m_roughnessMap = kit::Texture::load(currtokens[1].c_str(), false);
      }
      else if(identifier == std::string("metalness"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* Metalness needs 1 float value */);
        currMaterial->m_metalness = (float)std::atof(currtokens[1].c_str());
      }
      else if(identifier == std::string("metalnessmap"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* Metalnessmap needs 1 string value (no spaces!) */);
        currMaterial->m_metalnessMap = kit::Texture::load(currtokens[1].c_str(), false);
      }
      else if (identifier == std::string("doublesided"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* doublesided needs 1 bool value (no spaces!) */);
        currMaterial->m_doubleSided = (currtokens[1] == std::string("true"));
      }
      else if (identifier == std::string("depthwrite"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* depthwrite needs 1 bool value (no spaces!) */);
        currMaterial->m_depthWrite = (currtokens[1] == std::string("true"));
      }
      else if (identifier == std::string("depthread"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* depthread needs 1 bool value (no spaces!) */);
        currMaterial->m_depthRead = (currtokens[1] == std::string("true"));
      }
      else if (identifier == std::string("castshadows"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* castshadows needs 1 bool value (no spaces!) */);
        currMaterial->m_castShadows = (currtokens[1] == std::string("true"));
      }
      else if (identifier == std::string("opacity"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* opacity needs 1 float value */);
        currMaterial->m_opacity = (float)std::atof(currtokens[1].c_str());
      }
      else if (identifier == std::string("opacitymask"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* opacitymask needs 1 string value (no spaces!) */);
        currMaterial->m_opacityMask = kit::Texture::load(currtokens[1].c_str(), false);
      }
      else if (identifier == std::string("blendmode"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* blendmode needs 1 string value (no spaces!) */);
        currMaterial->m_blendMode = (currtokens[1] == "none" ? None : currtokens[1] == "add" ? Add : Alpha);
      }
      else if (identifier == std::string("spec_uvscale"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* spec_uvscale needs 1 float value */);
        currMaterial->m_spec_uvScale = (float)std::atof(currtokens[1].c_str());
      }
      else if (identifier == std::string("spec_depthmask"))
      {
        KIT_ASSERT(currtokens.size() == 2 /* spec_depthmask needs 1 string value (no spaces!) */);
        currMaterial->m_spec_depthMask = kit::Texture::load(currtokens[1].c_str(), false);
      }
      else
      {
        KIT_ERR(std::string("Warning: Unknown material parameter ") + identifier);
      }
    }
  }
  fhandle.close();

  currMaterial->assertCache();
  

  return currMaterial;
}

kit::Material::Material()
{
  kit::Material::m_instanceCount++;
  if(kit::Material::m_instanceCount == 1)
  {
    kit::Material::allocateShared();
  }
  
  m_arCache = new kit::PixelBuffer(glm::uvec2(128, 128), {kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8, Texture::Repeat, Texture::LinearMipmapLinear, Texture::Linear)});
  m_nmCache = new kit::PixelBuffer(glm::uvec2(128, 128), {kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8, Texture::Repeat, Texture::LinearMipmapLinear, Texture::Linear)});
  m_ndCache = new kit::PixelBuffer(glm::uvec2(128, 128), {kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8, Texture::Repeat, Texture::LinearMipmapLinear, Texture::Linear) });
  m_eoCache = new kit::PixelBuffer(glm::uvec2(128, 128), {kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8, Texture::Repeat, Texture::LinearMipmapLinear, Texture::Linear)});

}

kit::Material::~Material()
{
  if(m_arCache) delete m_arCache;
  if(m_nmCache) delete m_nmCache;
  if(m_ndCache) delete m_ndCache;
  if(m_eoCache) delete m_eoCache;
  if(m_albedoMap) delete m_albedoMap;
  if(m_opacityMask) delete m_opacityMask;
  if(m_occlusionMap) delete m_occlusionMap;
  if(m_emissiveMap) delete m_emissiveMap;
  if(m_normalMap) delete m_normalMap;
  if(m_roughnessMap) delete m_roughnessMap;
  if(m_metalnessMap) delete m_metalnessMap;
  if(m_spec_depthMask) delete m_spec_depthMask;
  
  kit::Material::m_instanceCount--;
  if(kit::Material::m_instanceCount == 0)
  {
    kit::Material::releaseShared();
  }
}

void kit::Material::allocateShared()
{
  m_cacheProgram = new kit::Program();

  auto vertexShader = new kit::Shader(Shader::Type::Vertex);
  std::stringstream vss;
  vss << glslVersion << glslCacheVertex;
  vertexShader->sourceFromString(vss.str());
  vertexShader->compile();
  
  auto pixelShader = new kit::Shader(Shader::Type::Fragment);
  std::stringstream pss;
  pss << glslVersion << glslCacheUtils << glslCachePixel;
  pixelShader->sourceFromString(pss.str());
  pixelShader->compile();

  kit::Material::m_cacheProgram->attachShader(vertexShader);
  kit::Material::m_cacheProgram->attachShader(pixelShader);
  kit::Material::m_cacheProgram->link();
  kit::Material::m_cacheProgram->detachShader(pixelShader);
  kit::Material::m_cacheProgram->detachShader(vertexShader);
  
  delete vertexShader;
  delete pixelShader;
}

void kit::Material::releaseShared()
{
  delete m_cacheProgram;
  for(auto & t : m_programCache)
  {
    if(t.second) delete t.second;
  }
}

void kit::Material::renderARCache()
{
  glm::uvec2 mapsize(2048, 2048);
  m_arCache = new kit::PixelBuffer(mapsize, {kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8, Texture::Repeat, Texture::LinearMipmapLinear, Texture::Linear)});
  m_arCache->bind();
  kit::Material::m_cacheProgram->use();
  
  kit::Material::m_cacheProgram->setUniform1f("uniform_defaultB", m_roughness);

  if(m_roughnessMap != nullptr)
  {
    kit::Material::m_cacheProgram->setUniformTexture("uniform_mapB", m_roughnessMap);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapB", 1);
  }
  else
  {
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapB", 0);
  }
  
  if(m_albedoMap != nullptr)
  {
    kit::Material::m_cacheProgram->setUniformTexture("uniform_mapA", m_albedoMap);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapA", 1);
  }
  else
  {
    kit::Material::m_cacheProgram->setUniform3f("uniform_defaultA", m_albedo);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapA", 0);
  }

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  
  kit::Quad::renderGeometry();
  
  kit::PixelBuffer::unbind();
  m_arCache->getColorAttachment(0)->generateMipmap();

  m_arDirty = false;
}

void kit::Material::renderNMCache()
{
  glm::uvec2 mapsize(2048, 2048);
  m_nmCache = new kit::PixelBuffer(mapsize, {kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8, Texture::Repeat, Texture::LinearMipmapLinear, Texture::Linear)});
  m_nmCache->bind();
  kit::Material::m_cacheProgram->use();
  
  kit::Material::m_cacheProgram->setUniform1f("uniform_defaultB", m_metalness);

  if(m_metalnessMap != nullptr)
  {
    kit::Material::m_cacheProgram->setUniformTexture("uniform_mapB", m_metalnessMap);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapB", 1);
  }
  else
  {
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapB", 0);
  }
  
  if(m_normalMap != nullptr)
  {
    kit::Material::m_cacheProgram->setUniformTexture("uniform_mapA", m_normalMap);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapA", 1);
  }
  else
  {
    kit::Material::m_cacheProgram->setUniform3f("uniform_defaultA", glm::vec3(0.5, 0.5, 1.0));
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapA", 0);
  }

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  kit::Quad::renderGeometry();
  
  kit::PixelBuffer::unbind();
  m_nmCache->getColorAttachment(0)->generateMipmap();

  m_nmDirty = false;
}

void kit::Material::renderNDCache()
{
  glm::uvec2 mapsize(2048, 2048);
  m_ndCache = new kit::PixelBuffer(mapsize, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8, Texture::Repeat, Texture::LinearMipmapLinear, Texture::Linear) });
  m_ndCache->bind();
  kit::Material::m_cacheProgram->use();

  if (m_spec_depthMask != nullptr)
  {
    kit::Material::m_cacheProgram->setUniformTexture("uniform_mapB", m_spec_depthMask);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapB", 1);
  }
  else
  {
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapB", 0);
    kit::Material::m_cacheProgram->setUniform1f("uniform_defaultB", 1.0f);
  }

  if (m_normalMap != nullptr)
  {
    kit::Material::m_cacheProgram->setUniformTexture("uniform_mapA", m_normalMap);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapA", 1);
  }
  else
  {
    kit::Material::m_cacheProgram->setUniform3f("uniform_defaultA", glm::vec3(0.5, 0.5, 1.0));
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapA", 0);
  }

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  
  kit::Quad::renderGeometry();

  kit::PixelBuffer::unbind();
  m_ndCache->getColorAttachment(0)->generateMipmap();

  m_ndDirty = false;
}

void kit::Material::renderEOCache()
{
  glm::uvec2 mapsize(2048, 2048);
  m_eoCache = new kit::PixelBuffer(mapsize, {kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA8, Texture::Repeat, Texture::LinearMipmapLinear, Texture::Linear)});
  m_eoCache->bind();
  kit::Material::m_cacheProgram->use();
  
  if(m_occlusionMap != nullptr)
  {
    kit::Material::m_cacheProgram->setUniform1f("uniform_defaultB", 1.0f);
    kit::Material::m_cacheProgram->setUniformTexture("uniform_mapB", m_occlusionMap);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapB", 1);
  }
  else
  {
    kit::Material::m_cacheProgram->setUniform1f("uniform_defaultB", 1.0f);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapB", 0);
  }
  
  if(m_emissiveMap != nullptr)
  {
    kit::Material::m_cacheProgram->setUniformTexture("uniform_mapA", m_emissiveMap);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapA", 1);
  }
  else
  {
    kit::Material::m_cacheProgram->setUniform3f("uniform_defaultA", m_emissiveColor);
    kit::Material::m_cacheProgram->setUniform1i("uniform_usemapA", 0);
  }

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  kit::Quad::renderGeometry();
  
  kit::PixelBuffer::unbind();
  m_eoCache->getColorAttachment(0)->generateMipmap();

  m_eoDirty = false;
}

kit::Material::ProgramFlags kit::Material::getFlags(bool skinned, bool instanced)
{
  kit::Material::ProgramFlags flags;
  flags.m_skinned = skinned;
  flags.m_instanced = instanced;
  flags.m_albedoMap = (m_albedoMap != nullptr);
  flags.m_roughnessMap = (m_roughnessMap != nullptr);
  flags.m_dynamicAR = m_dynamicAR;
  flags.m_normalMap = (m_normalMap != nullptr);
  flags.m_metalnessMap = (m_metalnessMap != nullptr);
  flags.m_dynamicNM = m_dynamicNM;
  flags.m_emissiveMap = (m_emissiveMap != nullptr);
  flags.m_occlusionMap = (m_occlusionMap != nullptr);
  flags.m_dynamicEO = m_dynamicEO;
  flags.m_forward = (m_opacity < 1.0f || m_blendMode != kit::Material::None);
  flags.m_opacityMask = (m_opacityMask != nullptr);
  return flags;
}

kit::Program * kit::Material::getProgram(kit::Material::ProgramFlags flags)
{
  auto finder = kit::Material::m_programCache.find(flags);
  if(finder != kit::Material::m_programCache.end())
  {
    return finder->second;
  }
  
  std::cout
    << "Compiling a new material-program with flags "
    << (flags.m_skinned ? "S" : "-")
    << (flags.m_instanced ? "I" : "-")
    << (flags.m_forward ? "F" : "-")
    << (flags.m_opacityMask ? "P" : "-")
    << (flags.m_dynamicAR ? "D" : "-")
    << (flags.m_albedoMap ? "A" : "-")
    << (flags.m_roughnessMap ? "R" : "-")
    << (flags.m_dynamicNM ? "D" : "-")
    << (flags.m_normalMap ? "N" : "-")
    << (flags.m_metalnessMap ? "M" : "-")
    << (flags.m_dynamicEO ? "D" : "-")
    << (flags.m_emissiveMap ? "E" : "-")
    << (flags.m_occlusionMap ? "O" : "-")
    << std::endl;
 
  bool needUvs = (flags.m_albedoMap || flags.m_roughnessMap || flags.m_normalMap || flags.m_metalnessMap || flags.m_emissiveMap || flags.m_occlusionMap || flags.m_opacityMask);
  
  // Generate vertex source
  std::stringstream vertexsource;
  {
    // Header
    vertexsource << glslVersion << std::endl;
    vertexsource << std::endl;
    
    // In attributes
    vertexsource << "layout (location = 0) in vec3 in_position;" << std::endl;
    vertexsource << "layout (location = 1) in vec2 in_texcoord;" << std::endl;
    vertexsource << "layout (location = 2) in vec3 in_normal;" << std::endl;
    
    if (flags.m_normalMap)
    {
      vertexsource << "layout (location = 3) in vec3 in_tangent;" << std::endl;
    }
    
    if(flags.m_skinned)
    {
      vertexsource << "layout (location = 4) in ivec4 in_boneids;" << std::endl;
      vertexsource << "layout (location = 5) in vec4  in_boneweights;" << std::endl;
    }
    
    vertexsource << std::endl;
    
    // Out attributes
    vertexsource << "layout (location = 0) out vec2 out_texcoord;" << std::endl;
    vertexsource << "layout (location = 1) out vec3 out_normal;" << std::endl;
    
    if (flags.m_normalMap)
    {
      vertexsource << "layout (location = 2) out vec3 out_tangent;" << std::endl;
      vertexsource << "layout (location = 3) out vec3 out_bitangent;" << std::endl;
    }
    
    //vertexsource << "layout (location = 4) out vec4 out_vertexPos;" << std::endl;
    
    vertexsource << std::endl;
    
    // Uniforms
    vertexsource << "uniform mat4 uniform_mvpMatrix;" << std::endl;
    //vertexsource << "uniform mat4 uniform_mvMatrix;" << std::endl;
    vertexsource << "uniform mat3 uniform_normalMatrix;" << std::endl;
    
    if(flags.m_skinned)
    {
      vertexsource << "uniform mat4 uniform_bones[128];" << std::endl;
    }
    
    if(flags.m_instanced)
    {
      vertexsource << "uniform mat4 uniform_instanceTransform[128];" << std::endl;
    }
    
    vertexsource << std::endl;
    
    // Main code
    vertexsource << "void main()" << std::endl;
    vertexsource << "{" << std::endl;
    if(flags.m_skinned)
    {
      vertexsource << "  mat4 boneTransform = uniform_bones[in_boneids[0]] * in_boneweights[0]; " << std::endl;
      vertexsource << "  boneTransform += uniform_bones[in_boneids[1]] * in_boneweights[1];" << std::endl;
      vertexsource << "  boneTransform += uniform_bones[in_boneids[2]] * in_boneweights[2];" << std::endl;
      vertexsource << "  boneTransform += uniform_bones[in_boneids[3]] * in_boneweights[3];" << std::endl;
      vertexsource << std::endl;
      vertexsource << "  vec4 position = boneTransform * vec4(in_position, 1.0);" << std::endl;
      vertexsource << "  vec3 normal = (boneTransform * vec4(normalize(in_normal), 0.0)).xyz;" << std::endl;
    }
    else
    {
      vertexsource << "  vec4 position = vec4(in_position, 1.0);" << std::endl;
      vertexsource << "  vec3 normal = normalize(in_normal);" << std::endl;
    }
    vertexsource << std::endl;
    
    if(flags.m_instanced)
    {
      vertexsource << "  gl_Position = uniform_mvpMatrix * uniform_instanceTransform[gl_InstanceID] * position;" << std::endl;
      vertexsource << "  out_normal = uniform_normalMatrix * mat3(uniform_instanceTransform[gl_InstanceID]) * normal;" << std::endl;
    }
    else
    {
      vertexsource << "  gl_Position = uniform_mvpMatrix * position;" << std::endl;
      vertexsource << "  out_normal = uniform_normalMatrix * normal;" << std::endl;
    }
    
    if(needUvs)
    {
      vertexsource << "  out_texcoord = in_texcoord;" << std::endl;
    }
    
    if (flags.m_normalMap)
    {
      if(flags.m_skinned)
      {        
        vertexsource << "  out_tangent = normalize(uniform_normalMatrix * (boneTransform * vec4(normalize(in_tangent), 0.0)).xyz);" << std::endl;
      }
      else
      {
        vertexsource << "  out_tangent = normalize(uniform_normalMatrix * normalize(in_tangent));" << std::endl;
      }
      vertexsource << "  out_bitangent = cross(out_normal, out_tangent);" << std::endl;
    }
    
    vertexsource << "}" << std::endl;
  }

  // Generate pixel source
  std::stringstream pixelsource;
  {
    // Header
    pixelsource << glslVersion << std::endl;
    pixelsource << std::endl;
    
    // In attributes
    pixelsource << "layout (location = 0) in vec2 in_texCoords;" << std::endl;
    pixelsource << "layout (location = 1) in vec3 in_normal;" << std::endl;
    
    if (flags.m_normalMap)
    {
      pixelsource << "layout (location = 2) in vec3 in_tangent;" << std::endl;
      pixelsource << "layout (location = 3) in vec3 in_bitangent;" << std::endl;
    }
        
    pixelsource << std::endl;
    
    // Out attributes
    if (flags.m_forward)
    {
      pixelsource << "layout (location = 0) out vec4 out_color;" << std::endl;
    }
    else
    {
      pixelsource << "layout (location = 0) out vec4 out_A;" << std::endl;
      pixelsource << "layout (location = 1) out vec4 out_B;" << std::endl;
      pixelsource << "layout (location = 2) out vec4 out_C;" << std::endl;
    }

    // Uniforms
    pixelsource << "uniform vec3 uniform_albedo;" << std::endl;

    if(flags.m_albedoMap || flags.m_roughnessMap)
    {
      if (flags.m_dynamicAR)
      {
        if (flags.m_albedoMap) pixelsource << "uniform sampler2D uniform_albedoMap;" << std::endl;
        if (flags.m_roughnessMap) pixelsource << "uniform sampler2D uniform_roughnessMap;" << std::endl;
      }
      else
      {
        pixelsource << "uniform sampler2D uniform_ARMap;" << std::endl;
      }
    }

    if(!flags.m_roughnessMap)
    {
      pixelsource << "uniform float uniform_roughness;" << std::endl;
    }

    if(flags.m_normalMap || flags.m_metalnessMap)
    {
      if (flags.m_dynamicNM)
      {
        if (flags.m_normalMap) pixelsource << "uniform sampler2D uniform_normalMap;" << std::endl;
        if (flags.m_metalnessMap) pixelsource << "uniform sampler2D uniform_metalnessMap;" << std::endl;
      }
      else
      {
        pixelsource << "uniform sampler2D uniform_NMMap;" << std::endl;
      }
    }

    if(!flags.m_metalnessMap)
    {
      pixelsource << "uniform float uniform_metalness;" << std::endl;
    }

    if (flags.m_emissiveMap || flags.m_occlusionMap)
    {
      if (flags.m_dynamicEO)
      {
        if (flags.m_emissiveMap) pixelsource << "uniform sampler2D uniform_emissiveMap;" << std::endl;
        if (flags.m_occlusionMap) pixelsource << "uniform sampler2D uniform_occlusionMap;" << std::endl;
      }
      else
      {
        pixelsource << "uniform sampler2D uniform_EOMap;" << std::endl;
      }
    }
    pixelsource << "uniform vec3 uniform_emissiveColor;" << std::endl;
    pixelsource << "uniform float uniform_emissiveStrength;" << std::endl;

    if (flags.m_forward)
    {
      pixelsource << "uniform float uniform_opacity;" << std::endl;
    }

    if (flags.m_opacityMask)
    {
      pixelsource << "uniform sampler2D uniform_opacityMask;" << std::endl;
    }

    pixelsource << std::endl;
    
    if (!flags.m_forward)
    {
      pixelsource << "vec2 encodeNormal (vec3 n)" << std::endl;
      pixelsource << "{" << std::endl;
      pixelsource << "  float p = sqrt(n.z*8+8);" << std::endl;
      pixelsource << "  return vec2(n.xy/p + 0.5);" << std::endl;
      pixelsource << "}" << std::endl;
    }

    if (flags.m_forward)
    {
      pixelsource << "float beckmannDistribution(float x, float roughness)" << std::endl;
      pixelsource << "{" << std::endl;
      pixelsource << "  float NdotH = max(x, 0.0001);" << std::endl;
      pixelsource << "  float cos2Alpha = NdotH * NdotH;" << std::endl;
      pixelsource << "  float tan2Alpha = (cos2Alpha - 1.0) / cos2Alpha;" << std::endl;
      pixelsource << "  float roughness2 = roughness * roughness;" << std::endl;
      pixelsource << "  float denom = 3.141592653589793 * roughness2 * cos2Alpha * cos2Alpha;" << std::endl;
      pixelsource << "  return exp(tan2Alpha / roughness2) / denom;" << std::endl;
      pixelsource << "}" << std::endl;
      pixelsource << "" << std::endl;
      pixelsource << "vec3 cookTorranceSpecular(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness, vec3 F0)" << std::endl;
      pixelsource << "{" << std::endl;
      pixelsource << "  //Half angle vector" << std::endl;
      pixelsource << "  vec3 H = normalize(lightDirection + viewDirection);" << std::endl;
      pixelsource << "" << std::endl;
      pixelsource << "  float VdotN = max(dot(viewDirection, surfaceNormal), 0.0001);" << std::endl;
      pixelsource << "  float LdotN = max(dot(lightDirection, surfaceNormal), 0.0001);" << std::endl;
      pixelsource << "" << std::endl;
      pixelsource << "  //Geometric term" << std::endl;
      pixelsource << "  float NdotH = max(dot(surfaceNormal, H), 0.0001);" << std::endl;
      pixelsource << "  float VdotH = max(dot(viewDirection, H), 0.0001);" << std::endl;
      pixelsource << "  float LdotH = max(dot(lightDirection, H), 0.0001);" << std::endl;
      pixelsource << "  float G1 = (2.0 * NdotH * VdotN) / VdotH;" << std::endl;
      pixelsource << "  float G2 = (2.0 * NdotH * LdotN) / LdotH;" << std::endl;
      pixelsource << "  float G = min(1.0, min(G1, G2));" << std::endl;
      pixelsource << "" << std::endl;
      pixelsource << "  //Distribution term" << std::endl;
      pixelsource << "  float D = beckmannDistribution(NdotH, roughness);" << std::endl;
      pixelsource << "" << std::endl;
      pixelsource << "  //Fresnel term" << std::endl;
      pixelsource << "  vec3 F = (F0 + (1.0 - F0)) * pow(1.0 - VdotN, 5.0);" << std::endl;
      pixelsource << "" << std::endl;
      pixelsource << "  //Multiply terms and done" << std::endl;
      pixelsource << "  return  F * G * D / max(3.14159265 * VdotN, 0.0001);" << std::endl;
      pixelsource << "}" << std::endl;
    }

    pixelsource << std::endl;
    
    pixelsource << "void main()" << std::endl;
    pixelsource << "{" << std::endl;
    
    // AR
    if (!flags.m_dynamicAR && (flags.m_albedoMap || flags.m_roughnessMap))
    {
      pixelsource << "  vec4 ARData = texture(uniform_ARMap, in_texCoords).rgba;" << std::endl;
    }

    // Albedo
    if(flags.m_albedoMap)
    {
      if (flags.m_dynamicAR)
      {
        pixelsource << "  vec4 albedoData = texture(uniform_albedoMap, in_texCoords).rgba;" << std::endl;
        pixelsource << "  vec3 in_albedo = albedoData.rgb * uniform_albedo;" << std::endl;
      }
      else
      {
        pixelsource << "  vec3 in_albedo = ARData.rgb * uniform_albedo;" << std::endl;
      }
    }
    else
    {
      pixelsource << "  vec3  in_albedo     = uniform_albedo;" << std::endl;
    }

    // Roughness
    if (flags.m_roughnessMap)
    {
      if (flags.m_dynamicAR)
      {
        pixelsource << "  float in_roughness = texture(uniform_roughnessMap, in_texCoords).r;" << std::endl;
      }
      else
      {
        pixelsource << "  float in_roughness = ARData.a;" << std::endl;
      }
    }
    else
    {
      pixelsource << "  float in_roughness  = uniform_roughness;" << std::endl;
    }

    // EO
    if (!flags.m_dynamicEO && (flags.m_emissiveMap || flags.m_occlusionMap))
    {
      pixelsource << "  vec4 EOData = texture(uniform_EOMap, in_texCoords).rgba;" << std::endl;
    }
    
    // Emissive
    pixelsource << "  float in_emissiveStrength = uniform_emissiveStrength;" << std::endl;
    if (flags.m_emissiveMap)
    {
      if (flags.m_dynamicEO)
      {
        pixelsource << "  vec3 in_emissiveColor = texture(uniform_emissiveMap, in_texCoords).rgb * uniform_emissiveColor;" << std::endl;
      }
      else
      {
        pixelsource << "  vec3 in_emissiveColor = EOData.rgb * uniform_emissiveColor;" << std::endl;
      }
    }
    else
    {
      pixelsource << "  vec3 in_emissiveColor = uniform_emissiveColor;" << std::endl;
    }

    // Occlusion
    if (flags.m_occlusionMap)
    {
      if (flags.m_dynamicEO)
      {
        pixelsource << "  float in_occlusion = texture(uniform_occlusionMap, in_texCoords).r;" << std::endl;
      }
      else
      {
        pixelsource << "  float in_occlusion = EOData.a;" << std::endl;
      }
    }
    else
    {
      pixelsource << "  float in_occlusion = 1.0;" << std::endl;
    }
    
    // NM
    if (!flags.m_dynamicNM && (flags.m_normalMap || flags.m_metalnessMap))
    {
      pixelsource << "  vec4 NMData = texture(uniform_NMMap, in_texCoords).rgba;" << std::endl;
    }

    // Normal
    if (flags.m_normalMap)
    {
      pixelsource << "  mat3 normalRotation= (mat3(in_tangent, in_bitangent, in_normal));" << std::endl;

      if (flags.m_dynamicNM)
      {
        pixelsource << "  vec3 normalTex     = ((texture(uniform_normalMap, in_texCoords).rgb - 0.5)*2.0);" << std::endl;
      }
      else
      {
        pixelsource << "  vec3 normalTex     = ((NMData.rgb - 0.5)*2.0);" << std::endl;
      }
      pixelsource << "  normalTex          = normalize(normalTex);" << std::endl;
      pixelsource << "  vec3 normal        = normalize(normalRotation * normalTex);" << std::endl;
    }
    else
    {
      pixelsource << "  vec3 normal = normalize(in_normal);" << std::endl;
    }

    pixelsource << "  if(!gl_FrontFacing) normal = -normal;" << std::endl;

    // Metalness
    if (flags.m_metalnessMap)
    {
      if (flags.m_dynamicNM)
      {
        pixelsource << "  float in_metalness = texture(uniform_metalnessMap, in_texCoords).a;" << std::endl;
      }
      else
      {
        pixelsource << "  float in_metalness = NMData.a;" << std::endl;
      }
    }
    else
    {
      pixelsource << "  float in_metalness = uniform_metalness;" << std::endl;
    }

    // Opacity
    if (flags.m_forward)
    {
      if (flags.m_opacityMask)
      {
        pixelsource << "  float in_opacity = uniform_opacity * texture(uniform_opacityMask, in_texCoords).a;" << std::endl;
      }
      else
      {
        pixelsource << "  float in_opacity = uniform_opacity;" << std::endl;
      }
    }
    else
    {
      if (flags.m_opacityMask)
      {
        pixelsource << "  float in_opacity = texture(uniform_opacityMask, in_texCoords).a;" << std::endl;
        pixelsource << "  if(in_opacity < 0.5) discard;" << std::endl;
      }
    }

    if (!flags.m_forward)
    {
      // Write albedo + roughness
      pixelsource << "  out_A.xyz = in_albedo;" << std::endl;
      pixelsource << "  out_A.w = in_roughness;" << std::endl;

      // Write  emissive + metalness
      pixelsource << "  out_B.xyz = in_emissiveColor * in_emissiveStrength;" << std::endl;
      pixelsource << "  out_B.w = in_metalness;" << std::endl;

      // Write normal + emissive strength + occlusion
      pixelsource << "  out_C.xyz = normal;" << std::endl;
      //pixelsource << "  out_C.z = in_emissiveStrength;" << std::endl;
      pixelsource << "  out_C.w = in_occlusion;" << std::endl;
    }
    else
    {
      //pixelsource << "  if(in_opacity < 0.9) discard;" << std::endl;

      pixelsource << "  out_color.rgb = in_albedo;" << std::endl;
      pixelsource << "  out_color.rgb += in_emissiveColor * in_emissiveStrength;" << std::endl;
      pixelsource << "  out_color.a =  in_opacity;" << std::endl;
      pixelsource << "  out_color.rgb *= in_opacity;" << std::endl;
    }

    pixelsource << "}" << std::endl;
  }
  
  auto vertexShader = new kit::Shader(Shader::Type::Vertex);
  vertexShader->sourceFromString(vertexsource.str());
  vertexShader->compile();
  
  auto pixelShader = new kit::Shader(Shader::Type::Fragment);
  pixelShader->sourceFromString(pixelsource.str());
  pixelShader->compile();
  
  kit::Program * returner = new kit::Program();
  returner->attachShader(vertexShader);
  returner->attachShader(pixelShader);
  returner->link();
  returner->detachShader(vertexShader);
  returner->detachShader(pixelShader);
  
  delete vertexShader;
  delete pixelShader;

  kit::Material::m_programCache[flags] = returner;
  
  return returner;  

}

void kit::Material::setDynamicEO(bool eo)
{
  if (eo != m_dynamicEO)
  {
    m_dirty = true;
  }
  m_dynamicEO = eo;
}

void kit::Material::assertCache()
{
  if(m_arDirty && !m_dynamicAR)
  {
    renderARCache();
  }
  
  if(m_nmDirty && !m_dynamicNM)
  {
    renderNMCache();
  }
  
  if (m_eoDirty && !m_dynamicEO)
  {
    renderEOCache();
  }

  if (m_ndDirty)
  {
    renderNDCache();
  }

  if(m_dirty)
  {
    kit::Material::ProgramFlags flags = getFlags(false, false);
    kit::Material::ProgramFlags sflags = getFlags(true, false);
    kit::Material::ProgramFlags iflags = getFlags(false, true);
    kit::Material::ProgramFlags siflags = getFlags(true, true);
  
    m_program = kit::Material::getProgram(flags);
    m_sProgram = kit::Material::getProgram(sflags);
    m_iProgram = kit::Material::getProgram(iflags);
    m_siProgram = kit::Material::getProgram(siflags);
    
    m_dirty = false;
  }
}

void kit::Material::use(kit::Camera * cam, const glm::mat4 & modelMatrix, const std::vector<glm::mat4> & skinTransform, const std::vector<glm::mat4> & instanceTransform)
{
  assertCache();
  kit::Material::ProgramFlags flags = getFlags(skinTransform.size() > 0, instanceTransform.size() > 0);

  kit::Program * currProgram = nullptr;
  
  if(flags.m_skinned && flags.m_instanced) currProgram = m_siProgram;
  if(flags.m_skinned && !flags.m_instanced) currProgram = m_sProgram;
  if(!flags.m_skinned && flags.m_instanced) currProgram = m_iProgram;
  if(!flags.m_skinned && !flags.m_instanced) currProgram = m_program;

  currProgram->use();
  currProgram->setUniform3f("uniform_albedo", m_albedo);
  
  if(flags.m_albedoMap || flags.m_roughnessMap)
  {
    if (flags.m_dynamicAR)
    {
      if (flags.m_albedoMap) currProgram->setUniformTexture("uniform_albedoMap", m_albedoMap);
      if (flags.m_roughnessMap) currProgram->setUniformTexture("uniform_roughnessMap", m_roughnessMap);
    }
    else
    {
      currProgram->setUniformTexture("uniform_ARMap", m_arCache->getColorAttachment(0));
    }
  }

  if (!flags.m_roughnessMap)
  {
    currProgram->setUniform1f("uniform_roughness", m_roughness);
  }
  
  currProgram->setUniform3f("uniform_emissiveColor", m_emissiveColor);
  currProgram->setUniform1f("uniform_emissiveStrength", m_emissiveStrength);
  
  if (flags.m_emissiveMap || flags.m_occlusionMap)
  {
    if (flags.m_dynamicEO)
    {
      if(flags.m_occlusionMap) currProgram->setUniformTexture("uniform_occlusionMap", m_occlusionMap);
      if(flags.m_emissiveMap) currProgram->setUniformTexture("uniform_emissiveMap", m_emissiveMap);
    }
    else
    {
      currProgram->setUniformTexture("uniform_EOMap", m_eoCache->getColorAttachment(0));
    }
  }

  if(flags.m_normalMap || flags.m_metalnessMap)
  {
    if (flags.m_dynamicNM)
    {
      if (flags.m_normalMap) currProgram->setUniformTexture("uniform_normalMap", m_normalMap);
      if (flags.m_metalnessMap) currProgram->setUniformTexture("uniform_metalnessMap", m_metalnessMap);
    }
    else
    {
      currProgram->setUniformTexture("uniform_NMMap", m_nmCache->getColorAttachment(0));
    }

  }

  if(!flags.m_metalnessMap)
  {
    currProgram->setUniform1f("uniform_metalness", m_metalness);
  }
  
  if (flags.m_forward)
  {
    currProgram->setUniform1f("uniform_opacity", m_opacity);
  }

  if (flags.m_opacityMask)
  {
    currProgram->setUniformTexture("uniform_opacityMask", m_opacityMask);
  }

  if(flags.m_skinned)
  {
    currProgram->setUniformMat4v("uniform_bones", skinTransform);
  }
  
  if(flags.m_instanced)
  {
    currProgram->setUniformMat4v("uniform_instanceTransform", instanceTransform);
  }
  
  glm::mat4 viewMatrix = cam->getViewMatrix();
  glm::mat4 projectionMatrix = cam->getProjectionMatrix();
  glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
  glm::mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
  
  glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix)));
  //glm::mat4 normalMatrix = modelViewMatrix;

  //currProgram->setUniformMat4("uniform_mvMatrix", modelViewMatrix);
  currProgram->setUniformMat4("uniform_mvpMatrix", modelViewProjectionMatrix);
  currProgram->setUniformMat3("uniform_normalMatrix", normalMatrix);

  if (m_depthRead)
  {
    glEnable(GL_DEPTH_TEST);
  }
  else
  {
    glDisable(GL_DEPTH_TEST);
  }
  glDepthMask( m_depthWrite ? GL_TRUE : GL_FALSE );

  if (m_doubleSided)
  {
    glDisable(GL_CULL_FACE);
  }
  else
  {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
  }

  if (m_blendMode == None)
  {
    glDisable(GL_BLEND);
  }
  else
  {
    glEnable(GL_BLEND);
    if (m_blendMode == Add)
    {
      glBlendFunc(GL_ONE, GL_ONE);
    }
    else
    {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
  }
}

const bool & kit::Material::getCastShadows()
{
  return m_castShadows;
}

void kit::Material::setCastShadows(bool b)
{
  m_castShadows = b;
}

const glm::vec3 & kit::Material::getAlbedo()
{
  return m_albedo;
}

void kit::Material::setAlbedo(glm::vec3 albedo)
{
  if (m_albedo != albedo)
  {
    m_albedo = albedo;
    m_arDirty = true;
  }
}

kit::Texture * kit::Material::getAlbedoMap()
{
  return m_albedoMap;
}

void kit::Material::setAlbedoMap(kit::Texture * albedoMap)
{ 
  if (m_albedoMap != albedoMap)
  {
    m_dirty = true;
  }
  m_albedoMap = albedoMap;
  m_arDirty = true;
}

kit::Texture * kit::Material::getOcclusionMap()
{
  return m_occlusionMap;
}

void kit::Material::setOcclusionMap(kit::Texture * c)
{
  if (c != m_occlusionMap)
  {
    m_dirty = true;
  }
  m_occlusionMap = c;
  m_eoDirty = true;
}

kit::Texture * kit::Material::getNormalMap()
{
  return m_normalMap;
}

void kit::Material::setNormalMap(kit::Texture * normalMap)
{
  if (normalMap != m_normalMap)
  {
    m_dirty = true;
  }
  m_normalMap = normalMap;
  m_nmDirty = true;
  m_ndDirty = true;
}

const float & kit::Material::getRoughness()
{
  return m_roughness;
}

void kit::Material::setRoughness(float roughness)
{
  if (m_roughness != roughness)
  {
    m_roughness = roughness;
    m_arDirty = true;
  }
}

kit::Texture * kit::Material::getRoughnessMap()
{
  return m_roughnessMap;
}

void kit::Material::setRoughnessMap(kit::Texture * roughnessMap)
{
  if (roughnessMap != m_roughnessMap)
  {
    m_dirty = true;
  }
  m_roughnessMap = roughnessMap;
  m_arDirty = true;
}

const float & kit::Material::getMetalness()
{
  return m_metalness;
}

void kit::Material::setMetalness(float metalness)
{
  if (metalness != m_metalness)
  {
    m_metalness = metalness;
    m_nmDirty = true;
  }
}

kit::Texture * kit::Material::getMetalnessMap()
{
  return m_metalnessMap;
}

void kit::Material::setMetalnessMap(kit::Texture * metalnessMap)
{ 
  if (m_metalnessMap != metalnessMap)
  {
    m_dirty = true;
  }
  m_metalnessMap = metalnessMap;
  m_nmDirty = true;
}

kit::Texture * kit::Material::getARCache()
{
  return m_arCache->getColorAttachment(0);
}

kit::Texture * kit::Material::getNMCache()
{
  return m_nmCache->getColorAttachment(0);
}

kit::Texture * kit::Material::getNDCache()
{
  return m_ndCache->getColorAttachment(0);
}

kit::Material::ProgramFlags::ProgramFlags()
{
  m_skinned = false;
  m_instanced = false;
  m_dynamicAR = false;
  m_albedoMap = false;
  m_roughnessMap = false;
  m_dynamicNM = false;
  m_normalMap = false;
  m_metalnessMap = false;
  m_dynamicEO = false;
  m_emissiveMap = false;
  m_occlusionMap = false;
}

glm::vec3 const & kit::Material::getEmissiveColor()
{
  return m_emissiveColor;
}

void kit::Material::setEmissiveColor(glm::vec3 c)
{
  if (c != m_emissiveColor)
  {
    m_emissiveColor = c;
    m_eoDirty = true;
  }
}

float const & kit::Material::getEmissiveStrength()
{
  return m_emissiveStrength;
}

void kit::Material::setEmissiveStrength(float v)
{
  if (v != m_emissiveStrength)
  {
    m_emissiveStrength = v;
    m_eoDirty = true;
  }
}

kit::Texture * kit::Material::getEmissiveMap()
{
  return m_emissiveMap;
}

void kit::Material::setEmissiveMap(kit::Texture * em)
{
  if (em != m_emissiveMap)
  {
    m_dirty = true;
  }

  m_emissiveMap = em;
  m_eoDirty = true;
}

bool const & kit::Material::getDepthRead()
{
  return m_depthRead;
}

void kit::Material::setDepthRead(bool enabled)
{
  m_depthRead = enabled;
}

bool const & kit::Material::getDepthWrite()
{
  return m_depthWrite;
}

void kit::Material::setDepthWrite(bool enabled)
{
  m_depthWrite = enabled;
}

float const & kit::Material::getOpacity()
{
  return m_opacity;
}

void kit::Material::setOpacity(float v)
{
  if (v != m_opacity)
  {
    m_opacity = v;
    m_dirty = true;
  }
}

kit::Texture * kit::Material::getOpacityMask()
{
  return m_opacityMask;
}

void kit::Material::setDoubleSided(bool e)
{
  m_doubleSided = e;
}

bool const & kit::Material::getDoubleSided()
{
  return m_doubleSided;
}

std::string kit::Material::getName()
{
  return m_filename;
}

void kit::Material::setDepthMask(kit::Texture * m)
{
  if (m_spec_depthMask != m)
  {
    m_spec_depthMask = m;
    m_ndDirty = true;
  }
}

kit::Texture * kit::Material::getDepthMask()
{
  return m_spec_depthMask;
}

void kit::Material::setUvScale(float uv)
{
  m_spec_uvScale = uv;
}

float kit::Material::getUvScale()
{
  return m_spec_uvScale;
}
