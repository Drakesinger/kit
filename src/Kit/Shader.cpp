#include "Kit/Shader.hpp"
#include "Kit/IncOpenGL.hpp"
#include "Kit/Exception.hpp"

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

std::map <kit::Shader::Type, std::string> typeToName { 
  { kit::Shader::Type::Vertex, "vertex" }, 
  { kit::Shader::Type::Fragment, "fragment" }, 
  { kit::Shader::Type::Geometry, "geometry" }, 
  { kit::Shader::Type::TessControl, "tesscontrol" }, 
  { kit::Shader::Type::TessEvaluation, "tessevaluation" },
  { kit::Shader::Type::Compute, "compute" }
};

kit::Shader::Shader(kit::Shader::Type type)
{
  this->m_glHandle = glCreateShader((uint32_t)type);
  this->m_type = type;
  this->m_source = "";
  KIT_ASSERT(this->m_glHandle);
}

kit::Shader::~Shader()
{
  glDeleteShader(this->m_glHandle);
}

bool kit::Shader::sourceFromFile(std::string const & filename)
{
  std::string line;
  std::string source = "";

  std::cout << "Loading " << typeToName[this->m_type] << " shader from file \"" << filename.c_str() << "\"" << std::endl;

  std::ifstream handle(filename);

  if(!handle.is_open())
  {
    KIT_ERR("Could not open file");
    return false;
  }

  while(std::getline(handle,line))
  {
    source.append(line);
    source.append("\n");
  }

  this->m_source = source;

  const GLchar * src = (const GLchar *)this->m_source.c_str();
  glShaderSource(this->m_glHandle, 1, &src, 0);

  return true;
}

void kit::Shader::sourceFromString(std::string const & s)
{
  this->m_source = s;

  const GLchar *source = (const GLchar *)this->m_source.c_str();
  glShaderSource(this->m_glHandle, 1, &source, 0);
}

void kit::Shader::clearSource()
{
  this->m_source = "";

  std::string emptys = "";
  const GLchar *source = (const GLchar *)emptys.c_str();
  glShaderSource(this->m_glHandle, 1, &source, 0);
}

bool kit::Shader::compile()
{
  // Attempt to compile the shader
  glCompileShader(this->m_glHandle);

  // Retrieve the compilation status
  int32_t status;
  glGetShaderiv(this->m_glHandle, GL_COMPILE_STATUS, &status);

  // If compilation failed, dump source and return false
  if (!status)
  {
    int32_t blen = 0;
    GLsizei slen = 0;
    glGetShaderiv(this->m_glHandle, GL_INFO_LOG_LENGTH , &blen);
    if (blen > 1)
    {
      GLchar* compiler_log = new GLchar[blen];
      glGetShaderInfoLog(this->m_glHandle, blen, &slen, compiler_log);

      std::stringstream ss;
      ss << typeToName[this->m_type] << "-shader compilation failed: " << compiler_log;
      KIT_ERR(ss.str());
      delete[] compiler_log;
      
      std::cout << "Dumping source..." << std::endl;
      std::cout << "------------------------" << std::endl;
      
      std::stringstream sourcestream(this->m_source);
      unsigned int linecount = 1;
      std::string currline;
      while (std::getline(sourcestream, currline))
      {
        std::cout << linecount << ":\t" << currline << std::endl;
        linecount++;
      }

      std::cout << "------------------------" << std::endl;
    }

    return false;
  }

  return true;
}

uint32_t kit::Shader::getHandle()
{
  return this->m_glHandle;
}
