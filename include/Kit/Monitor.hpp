#ifndef KIT_MONITOR_HEADER
#define KIT_MONITOR_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/OpenGL.hpp"

#include <vector>
#include <string>
#include <memory>

struct GLFWmonitor;

namespace kit
{

  struct KITAPI VideoMode
  {
    int m_width;
    int m_Height;
    int m_RedBits;
    int m_GreenBits;
    int m_BlueBits;
    int m_RefreshRate;

    glm::uvec2 Resolution();
  };

  class KITAPI Monitor
  {
    public:

      typedef std::shared_ptr<Monitor> Ptr;

      ~Monitor();

      GLFWmonitor * getGLFWHandle();
      bool        isConnected();

      glm::ivec2 getPhysicalSize(); // In millimeters
      glm::ivec2 getPosition(); // ? glfwGetMonitorPos, probably relative to workspace
      std::string           getName();

      std::vector<kit::VideoMode>  getVideoModes();
      kit::VideoMode               getVideoMode();

      static std::vector<kit::Monitor::Ptr> getConnectedMonitors();
      static kit::Monitor::Ptr              getPrimaryMonitor();
      Monitor(GLFWmonitor*);

    private:

      kit::GLFWSingleton m_glfwSingleton;

      static void fillMonitors();
      
      static uint32_t m_instanceCount;
      static void __monfunc(GLFWmonitor * mon, int event);
      static kit::Monitor::Ptr findMonitor(GLFWmonitor * mon);
      static std::vector<kit::Monitor::Ptr> m_monitors;
      
      GLFWmonitor  * m_glfwHandle;
      bool           m_connected;
  };

}

#endif // KIT_MONITOR_HEADER
