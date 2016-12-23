#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/OpenGL.hpp"

#include <vector>
#include <string>

struct GLFWmonitor;

namespace kit
{

  struct KITAPI VideoMode
  {
    int m_width;
    int m_height;
    int m_redBits;
    int m_greenBits;
    int m_blueBits;
    int m_refreshRate;

    glm::uvec2 Resolution();
  };

  class KITAPI Monitor
  {
    public:

      ~Monitor();

      GLFWmonitor * getGLFWHandle();
      bool        isConnected();

      glm::ivec2 getPhysicalSize(); // In millimeters
      glm::ivec2 getPosition(); // ? glfwGetMonitorPos, probably relative to workspace
      std::string           getName();

      std::vector<kit::VideoMode>  getVideoModes();
      kit::VideoMode               getVideoMode();

      static std::vector<kit::Monitor*> getMonitors();
      static kit::Monitor*              getPrimaryMonitor();
      Monitor(GLFWmonitor*);

    private:

      kit::GLFWSingleton m_glfwSingleton;

      static void fillMonitors();
      
      static uint32_t m_instanceCount;
      static void __monfunc(GLFWmonitor * mon, int event);
      static kit::Monitor* findMonitor(GLFWmonitor * mon);
      static std::vector<kit::Monitor*> m_monitors;
      
      GLFWmonitor  * m_glfwHandle;
      bool           m_connected;
  };

}
