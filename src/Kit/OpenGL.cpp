#include "Kit/OpenGL.hpp"

#include "Kit/Exception.hpp"
#include "Kit/IncOpenGL.hpp"

#include <string>
#include <sstream>
#include <csignal>

#include <GLFW/glfw3.h>

uint32_t kit::GLFWSingleton::m_instanceCount = 0;

kit::GLFWSingleton::GLFWSingleton()
{
  kit::GLFWSingleton::m_instanceCount++;
  if (kit::GLFWSingleton::m_instanceCount == 1)
  {
    kit::GLFWSingleton::allocateShared();
  }
}

kit::GLFWSingleton::~GLFWSingleton()
{
  kit::GLFWSingleton::m_instanceCount--;
  if (kit::GLFWSingleton::m_instanceCount == 0)
  {
    kit::GLFWSingleton::releaseShared();
  }
}

uint32_t const & kit::GLFWSingleton::getInstanceCount()
{
  return kit::GLFWSingleton::m_instanceCount;
}

void kit::GLFWSingleton::allocateShared()
{
  static bool wasRan = false;
  if(!wasRan)
  {
    glfwSetErrorCallback(kit::glfwError);
    if (!glfwInit())
    {
      KIT_THROW("Failed to initialize GLFW3");
    }

    std::cout << "GLFW3 initialized." << std::endl;
    wasRan = true;
  }
}

void kit::GLFWSingleton::releaseShared()
{
  glfwTerminate();
  std::cout << "GLFW3 terminated." << std::endl;
}

void kit::initializeGL3W()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }

  std::cout << "Initializing OpenGL" << std::endl;

  if (gl3wInit())
  {
    KIT_ERR("Failed to initialize OpenGL");
    return;
  }

  if (!gl3wIsSupported(4, 3)) {
    KIT_ERR("OpenGL version 4.3 core is not supported");
    return;
  }

  std::cout << "OpenGL initialized -- version " << glGetString(GL_VERSION) << ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "." << std::endl;
  initialized = true;
}


void kit::glCheckError(const char* file, unsigned int line, const char* method, const char * op) {
  // Get the last error
  GLenum err = glGetError();

  if (err != GL_NO_ERROR) {
    std::string error = "UNDEFINED";

    // Decode the error code
    switch (err)
    {
      case GL_INVALID_ENUM:
        error = "GL_INVALID_ENUM";
        break;

      case GL_INVALID_VALUE:
        error = "GL_INVALID_VALUE";
        break;

      case GL_INVALID_OPERATION:
        error = "GL_INVALID_OPERATION";
        break;

      case GL_STACK_OVERFLOW:
        error = "GL_STACK_OVERFLOW";
        break;

      case GL_STACK_UNDERFLOW:
        error = "GL_STACK_UNDERFLOW";
        break;

      case GL_OUT_OF_MEMORY:
        error = "GL_OUT_OF_MEMORY";
        break;

      case GL_INVALID_FRAMEBUFFER_OPERATION:
        error = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
#ifdef __unix__
      //case GL_CONTEXT_LOST:
      //  error = "GL_CONTEXT_LOST";
      //  description = "lost context";
      //  break;
#endif
    }

    std::cout << error << " from " << op << " in " << file << ":" << line << " (inside " << method << ")" << std::endl;
    std::raise(SIGINT);
  }
}

void kit::glfwError(int error, const char* desc){
    std::stringstream err;
    err << "GLFW error " << error << ": " << desc;
    KIT_THROW(err.str());
}
