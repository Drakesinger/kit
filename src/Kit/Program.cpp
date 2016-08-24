#include "Kit/Program.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Exception.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Cubemap.hpp"
#include "Kit/Shader.hpp"

#include <sstream>
#include <glm/gtc/type_ptr.hpp>

std::map <kit::Shader::Type, std::string> typeToShort { 
  { kit::Shader::Type::Vertex, "v:" }, 
  { kit::Shader::Type::Fragment, "f:" }, 
  { kit::Shader::Type::Geometry, "g:" }, 
  { kit::Shader::Type::TessControl, "c:" }, 
  { kit::Shader::Type::TessEvaluation, "e:" },
  { kit::Shader::Type::Compute, "x:" }
};


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

void kit::Program::addShaders(kit::Program::Ptr program, kit::Shader::Type type, std::vector<std::string> const & sources, std::vector<kit::Shader::Ptr> & outShaders, char const * dataDirectory)
{
  static const std::string dataDir = dataDirectory + std::string("/shaders/");
  
  for(auto & currSource : sources)
  {
    program->m_fileIdentifier += typeToShort[type] + currSource + std::string(";");
    kit::Shader::Ptr currShader = kit::Shader::create(type);
    currShader->sourceFromFile(dataDir + currSource);
    currShader->compile();
    outShaders.push_back(currShader);
    program->attachShader(currShader);
  }
}

kit::Program::Ptr kit::Program::load(SourceList c, char const * dataDirectory)
{
  kit::Program::Ptr returner = kit::Program::create();
  std::vector<kit::Shader::Ptr> shaders;
  returner->m_fileIdentifier = "";

  kit::Program::addShaders(returner, kit::Shader::Type::Compute, c, shaders, dataDirectory);

  returner->link();

  // Detach shaders 
  for(auto & currShader : shaders)
  {
    returner->detachShader(currShader);
  }

  return returner;
}

kit::Program::Ptr kit::Program::load(SourceList v, SourceList f, char const * dataDirectory)
{
  kit::Program::Ptr returner = kit::Program::create();
  std::vector<kit::Shader::Ptr> shaders;
  returner->m_fileIdentifier = "";

  kit::Program::addShaders(returner, kit::Shader::Type::Vertex, v, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::Fragment, f, shaders, dataDirectory);

  returner->link();

  // Detach shaders 
  for(auto & currShader : shaders)
  {
    returner->detachShader(currShader);
  }

  return returner;
}

kit::Program::Ptr kit::Program::load(SourceList v, SourceList g, SourceList f, char const * dataDirectory)
{
  kit::Program::Ptr returner = kit::Program::create();
  std::vector<kit::Shader::Ptr> shaders;
  returner->m_fileIdentifier = "";

  kit::Program::addShaders(returner, kit::Shader::Type::Vertex, v, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::Geometry, g, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::Fragment, f, shaders, dataDirectory);

  returner->link();

  // Detach shaders 
  for(auto & currShader : shaders)
  {
    returner->detachShader(currShader);
  }

  return returner;
}

kit::Program::Ptr kit::Program::load(SourceList v, SourceList tc, SourceList te, SourceList f, char const * dataDirectory)
{
  kit::Program::Ptr returner = kit::Program::create();
  std::vector<kit::Shader::Ptr> shaders;
  returner->m_fileIdentifier = "";

  kit::Program::addShaders(returner, kit::Shader::Type::Vertex, v, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::TessControl, tc, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::TessEvaluation, te, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::Fragment, f, shaders, dataDirectory);

  returner->link();

  // Detach shaders 
  for(auto & currShader : shaders)
  {
    returner->detachShader(currShader);
  }

  return returner;
}

kit::Program::Ptr kit::Program::load(SourceList v, SourceList tc, SourceList te, SourceList g, SourceList f, char const * dataDirectory)
{
  kit::Program::Ptr returner = kit::Program::create();
  std::vector<kit::Shader::Ptr> shaders;
  returner->m_fileIdentifier = "";

  kit::Program::addShaders(returner, kit::Shader::Type::Vertex, v, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::TessControl, tc, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::TessEvaluation, te, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::Geometry, g, shaders, dataDirectory);
  kit::Program::addShaders(returner, kit::Shader::Type::Fragment, f, shaders, dataDirectory);

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
  glLinkProgram(this->m_glHandle);  

  // Retrieve the link status
  int32_t status;
  glGetProgramiv(this->m_glHandle, GL_LINK_STATUS, &status);

  if(!status)
  {
    int32_t blen = 0;
    GLsizei slen = 0;
    glGetProgramiv(this->m_glHandle, GL_INFO_LOG_LENGTH , &blen);

    if(blen > 1)
    {
      GLchar * compiler_log = new GLchar[blen];
      glGetProgramInfoLog(this->m_glHandle, blen, &slen, compiler_log);

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
  glUseProgram(this->m_glHandle);
  this->prepareTextures();
}

void kit::Program::useFixed()
{
  glUseProgram(0);
}

uint32_t kit::Program::getHandle()
{
  return this->m_glHandle;
}

void kit::Program::setUniformTexture(const std::string & name, kit::Texture::WPtr texture )
{
  glUseProgram(this->m_glHandle);
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
  glUseProgram(this->m_glHandle);
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
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if (loc != (uint32_t)-1)
  {
    glUniformMatrix3fv(loc, 1, GL_FALSE, &matrix[0][0]);
  }
}

void kit::Program::setUniformMat4(const std::string & name, const glm::mat4 & matrix)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glUniformMatrix4fv(loc, 1, GL_FALSE, &matrix[0][0]);
  }
}

void kit::Program::setUniformMat4v(const std::string & name, const std::vector<glm::mat4> & matrices)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glUniformMatrix4fv(loc, (uint32_t)matrices.size(), GL_FALSE, glm::value_ptr(matrices[0]));
  }
}

void kit::Program::setUniform3f(const std::string & name, const glm::vec3 & vec)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glUniform3fv(loc, 1, &vec[0]);
  }
}

void kit::Program::setUniform3fv(const std::string & name, const std::vector<glm::vec3> & v)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if (loc != (uint32_t) - 1)
  {
    glUniform3fv(loc, (uint32_t)v.size(), &v[0][0]);
  }
}

void kit::Program::setUniform1f(const std::string & name, float val)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glUniform1f(loc, val);
  }
}

void kit::Program::setUniform1d(const std::string & name, double val)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glUniform1f(loc, float(val));
  }
}

void kit::Program::setUniform1i(const std::string & name, int i)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glUniform1i(loc, i);
  }
}

void kit::Program::setUniform1ui(const std::string & name, uint32_t i)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if (loc != (uint32_t) - 1)
  {
    glUniform1ui(loc, i);
  }
}

void kit::Program::setUniform4f(const std::string & name, const glm::vec4 & vec)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glUniform4fv(loc, 1, &vec[0]);
  }
}

void kit::Program::setUniform2f(const std::string & name, const glm::vec2 & vec)
{
  glUseProgram(this->m_glHandle);
  uint32_t loc = this->getUniformLocation(name);

  if(loc != (uint32_t) - 1)
  {
    glm::vec2 v(vec.x, vec.y);
    glUniform2fv(loc, 1, &v[0]);
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
    glUseProgram(this->m_glHandle);
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
      glUniform1i(it.first, i);
#ifndef KIT_SHITTY_INTEL
      glBindTextureUnit(i, t->getHandle());
#else
      glActiveTexture(GL_TEXTURE0 + i);
      t->bind();
#endif 
      i++;
    }
  }
  
  for(auto & it : this->m_cubemaps)
  {
    kit::Cubemap::Ptr t = it.second.lock();
    if(t)
    {
      glUniform1i(it.first, i);
#ifndef KIT_SHITTY_INTEL
      glBindTextureUnit(i, t->getHandle());
#else 
      glActiveTexture(GL_TEXTURE0 + i);
      t->bind();
#endif
      i++;
    }
  }
}

int32_t kit::Program::getMaxTextureUnits()
{
  int32_t returner;
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &returner);
  return returner;
}

void kit::Program::attachShader(kit::Shader::Ptr s)
{
  glAttachShader(this->m_glHandle, s->getHandle());
  this->m_fileIdentifier += "dynamic;";
}

void kit::Program::detachShader(kit::Shader::Ptr s)
{
  glDetachShader(this->m_glHandle, s->getHandle());
}
