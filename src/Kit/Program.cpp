#include "Kit/Program.hpp"
#include "Kit/Exception.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Cubemap.hpp"
#include "Kit/Shader.hpp"

#include <sstream>
#include <glm/gtc/type_ptr.hpp>


kit::Program::Program()
{
  this->m_fileIdentifier = "";
  this->m_glHandle = glCreateProgram();
  KIT_ASSERT(this->m_glHandle);
}

kit::Program::~Program()
{
  glDeleteProgram(this->m_glHandle);
  glGetError();
}

kit::Program::Ptr kit::Program::create()
{
  return std::make_shared<kit::Program>();
}

kit::Program::Ptr kit::Program::load(std::vector< std::string > const &  vertexSources, std::vector< std::string > const &  geometrySources, std::vector< std::string > const &  fragSources)
{
  kit::Program::Ptr returner = kit::Program::create();

  std::string dataDir("./data/shaders/");

  std::vector<kit::Shader::Ptr> shaders;
  returner->m_fileIdentifier = "";

  // Compile vertex shaders
  for(auto & currVertSource : vertexSources)
  {
    returner->m_fileIdentifier += std::string("v:") + currVertSource + std::string(";");
    kit::Shader::Ptr currVertShader = kit::Shader::create(Shader::Type::Vertex);
    currVertShader->sourceFromFile(dataDir + currVertSource);
    currVertShader->compile();
    shaders.push_back(currVertShader);
    returner->attachShader(currVertShader);
  }

  // Compile geometry shaders
  for(auto & currGeometrySource : geometrySources)
  {
    returner->m_fileIdentifier += std::string("g:") + currGeometrySource + std::string(";");
    kit::Shader::Ptr currShader = kit::Shader::create(Shader::Type::Geometry);
    currShader->sourceFromFile(dataDir + currGeometrySource);
    currShader->compile();
    shaders.push_back(currShader);
    returner->attachShader(currShader);
  }
  
  // Compile pixel shaders
  for(auto & currPixelSource : fragSources)
  {
    returner->m_fileIdentifier += std::string("f:") + currPixelSource + std::string(";");
    kit::Shader::Ptr currPixelShader = kit::Shader::create(Shader::Type::Fragment);
    currPixelShader->sourceFromFile(dataDir + currPixelSource);
    currPixelShader->compile();
    shaders.push_back(currPixelShader);
    returner->attachShader(currPixelShader);
  }

  // Link program
  returner->link();

  // Detach shaders 
  for(auto & currShader : shaders)
  {
    returner->detachShader(currShader);
  }

  return returner;
}

bool kit::Program::link()
{
  // Attempt to link the program
  KIT_GL(glLinkProgram(this->m_glHandle));  

  // Retrieve the link status
  GLint status;
  KIT_GL(glGetProgramiv(this->m_glHandle, GL_LINK_STATUS, &status));

  if(!status)
  {
    GLint blen = 0;
    GLsizei slen = 0;
    KIT_GL(glGetProgramiv(this->m_glHandle, GL_INFO_LOG_LENGTH , &blen));

    if(blen > 1)
    {
      GLchar * compiler_log = new GLchar[blen];
      KIT_GL(glGetProgramInfoLog(this->m_glHandle, blen, &slen, compiler_log));

      std::stringstream ss;
      ss << "Program linkage failed: " << compiler_log;
      KIT_ERR(ss.str());
      delete[] compiler_log;
    }

    return false;
  }

  return true;
}

void kit::Program::use()
{
  kit::GL::useProgram(this->m_glHandle);
  this->prepareTextures();
}

void kit::Program::useFixed()
{
  kit::GL::useProgram(0);
}

GLuint kit::Program::getHandle()
{
  return this->m_glHandle;
}

void kit::Program::setUniformTexture(const std::string & name, kit::Texture::WPtr texture )
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    auto it = this->m_textures.find(loc);

    if(it != this->m_textures.end())
    {
      it->second = texture;
    }
    else
    {
      static const int32_t max = this->getMaxTextureUnits();

      if(this->m_textures.size() + this->m_cubemaps.size() > (uint32_t)max)
      {
        KIT_ERR("Could not set uniform, max texture units reached.");
        return;
      }

      this->m_textures[loc] = texture;
    }
    
    this->prepareTextures();
  }
}

void kit::Program::setUniformCubemap(const std::string & name, kit::Cubemap::WPtr texture )
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    auto it = this->m_cubemaps.find(loc);

    if(it != this->m_cubemaps.end())
    {
      it->second = texture;
    }
    else
    {
      static const int32_t max = this->getMaxTextureUnits();

      if(this->m_textures.size() + this->m_cubemaps.size() > (uint32_t)max)
      {
        KIT_ERR("Could not set uniform, max texture units reached.");
        return;
      }

      this->m_cubemaps[loc] = texture;
    }
    
    this->prepareTextures();
  }
}

void kit::Program::setUniformMat3(const std::string & name, const glm::mat3 & matrix)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if (loc != (uint32_t)-1)
  {
    KIT_GL(glUniformMatrix3fv(loc, 1, GL_FALSE, &matrix[0][0]));
  }
}

void kit::Program::setUniformMat4(const std::string & name, const glm::mat4 & matrix)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    KIT_GL(glUniformMatrix4fv(loc, 1, GL_FALSE, &matrix[0][0]));
  }
}

void kit::Program::setUniformMat4v(const std::string & name, const std::vector<glm::mat4> & matrices)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    KIT_GL(glUniformMatrix4fv(loc, (uint32_t)matrices.size(), GL_FALSE, glm::value_ptr(matrices[0])));
  }
}

void kit::Program::setUniform3f(const std::string & name, const glm::vec3 & vec)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    KIT_GL(glUniform3fv(loc, 1, &vec[0]));
  }
}

void kit::Program::setUniform3fv(const std::string & name, const std::vector<glm::vec3> & v)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if (loc != (uint32_t) - 1)
  {
    KIT_GL(glUniform3fv(loc, (uint32_t)v.size(), &v[0][0]));
  }
}

void kit::Program::setUniform1f(const std::string & name, float val)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    KIT_GL(glUniform1f(loc, val));
  }
}

void kit::Program::setUniform1d(const std::string & name, double val)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    KIT_GL(glUniform1f(loc, float(val)));
  }
}

void kit::Program::setUniform1i(const std::string & name, int i)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    KIT_GL(glUniform1i(loc, i));
  }
}

void kit::Program::setUniform1ui(const std::string & name, uint32_t i)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if (loc != (uint32_t) - 1)
  {
    KIT_GL(glUniform1ui(loc, i));
  }
}

void kit::Program::setUniform4f(const std::string & name, const glm::vec4 & vec)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    KIT_GL(glUniform4fv(loc, 1, &vec[0]));
  }
}

void kit::Program::setUniform2f(const std::string & name, const glm::vec2 & vec)
{
  kit::GL::useProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glm::vec2 v(vec.x, vec.y);
    KIT_GL(glUniform2fv(loc, 1, &v[0]));
  }
}

uint32_t kit::Program::getUniformLocation(const std::string & name)
{
  auto it = this->m_locationCache.find(name.c_str());

  if(it != this->m_locationCache.end())
  {
    return it->second;
  }
  else
  {
    kit::GL::useProgram(this->m_glHandle);
    int loc = glGetUniformLocation(this->m_glHandle, name.c_str());

    if(loc == -1)
    {
      std::stringstream e;
      e << "No such uniform \"" << name.c_str() << "\" in shader (file identifer string: (\"" << this->m_fileIdentifier << "\")";
      KIT_ERR(e.str());
      return -1;
    }

    this->m_locationCache.insert(std::make_pair(name, loc));
    return loc;
  }
}

 void kit::Program::prepareTextures()
{
  uint32_t i = 1;

  for(auto & it : this->m_textures)
  {
    kit::Texture::Ptr t = it.second.lock();
    if(t)
    {
      KIT_GL(glUniform1i(it.first, i));
      KIT_GL(glBindTextureUnit(i, t->getHandle()));
      i++;
    }
  }
  
  for(auto & it : this->m_cubemaps)
  {
    kit::Cubemap::Ptr t = it.second.lock();
    if(t)
    {
      KIT_GL(glUniform1i(it.first, i));
      KIT_GL(glBindTextureUnit(i, t->getHandle()));
      i++;
    }
  }
}

int32_t kit::Program::getMaxTextureUnits()
{
  int32_t returner;
  KIT_GL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &returner));
  return returner;
}

void kit::Program::attachShader(kit::ShaderPtr s)
{
  kit::GL::attachShader(this->m_glHandle, s->getHandle());
  this->m_fileIdentifier += "dynamic;";
}

void kit::Program::detachShader(kit::ShaderPtr s)
{
  kit::GL::detachShader(this->m_glHandle, s->getHandle());
}
