#ifndef KIT_OPENGL_HEADER
#define KIT_OPENGL_HEADER

#include "Kit/Types.hpp"

namespace kit{

  void KITAPI glCheckError(const char* file, unsigned int line, const char* method, const char* op);
  void KITAPI glfwError(int error, const char* desc);
  void KITAPI initializeGL3W();

  class GLFWSingleton
  {
  public:
    GLFWSingleton();
    ~GLFWSingleton();

    static uint32_t const & getInstanceCount();

  private:

    void allocateShared();
    void releaseShared();

    static uint32_t m_instanceCount;
  };
}

#ifdef KIT_DEBUG
  #ifdef _WIN32
    #define KIT_GL(x) ((x), kit::glCheckError(__FILE__, __LINE__, __FUNCTION__, #x))
  #elif __unix
    #define  KIT_GL(expr) do { expr; kit::glCheckError(__FILE__, __LINE__, __PRETTY_FUNCTION__, #expr); } while (false)
  #endif
#else
	#define KIT_GL(x) (x)
#endif

#endif
