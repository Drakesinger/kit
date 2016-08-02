#include "Kit/Monitor.hpp"

#include "Kit/IncOpenGL.hpp"

#include <GLFW/glfw3.h>

std::vector<kit::Monitor::Ptr> kit::Monitor::m_monitors = std::vector<kit::Monitor::Ptr>();
uint32_t kit::Monitor::m_instanceCount = 0;

glm::uvec2 kit::VideoMode::Resolution()
{
  return glm::uvec2(this->m_width, this->m_Height);
}

kit::Monitor::Monitor(GLFWmonitor * glfwhandle)
{
  this->m_glfwHandle = glfwhandle;
  kit::Monitor::m_instanceCount++;
}

kit::Monitor::~Monitor()
{
  kit::Monitor::m_instanceCount--;
}

void kit::Monitor::fillMonitors()
{
  int monCount = 0;
  GLFWmonitor** monitors = glfwGetMonitors(&monCount);
  for(int i = 0; i < monCount; i++)
  {
    kit::Monitor::m_monitors.push_back(std::make_shared<kit::Monitor>(monitors[i]));
  }
}


GLFWmonitor * kit::Monitor::getGLFWHandle()
{
  return this->m_glfwHandle;
}

bool kit::Monitor::isConnected()
{
  return this->m_connected;
}

glm::ivec2 kit::Monitor::getPhysicalSize()
{
  glm::ivec2 returner;
  glfwGetMonitorPhysicalSize(this->m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

glm::ivec2 kit::Monitor::getPosition()
{
  glm::ivec2 returner;
  glfwGetMonitorPos(this->m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

std::string kit::Monitor::getName()
{
  return std::string(glfwGetMonitorName(this->m_glfwHandle));
}

std::vector<kit::VideoMode> kit::Monitor::getVideoModes()
{
  int count = 0;
  const GLFWvidmode * modes = glfwGetVideoModes(this->m_glfwHandle, &count);
  std::vector<kit::VideoMode> returner;

  for(int i = 0; i < count; i++)
  {
    kit::VideoMode adder;
    adder.m_width = modes[i].width;
    adder.m_Height = modes[i].height;
    adder.m_RedBits = modes[i].redBits;
    adder.m_GreenBits = modes[i].greenBits;
    adder.m_BlueBits = modes[i].blueBits;
    adder.m_RefreshRate = modes[i].refreshRate;
    returner.push_back(adder);
  }

  return returner;
}

kit::VideoMode kit::Monitor::getVideoMode()
{
  const GLFWvidmode * mode = glfwGetVideoMode(this->m_glfwHandle);
  kit::VideoMode returner;
  returner.m_width = mode->width;
  returner.m_Height = mode->height;
  returner.m_RedBits = mode->redBits;
  returner.m_GreenBits = mode->greenBits;
  returner.m_BlueBits = mode->blueBits;
  returner.m_RefreshRate = mode->refreshRate;
  return returner;
}

std::vector<kit::Monitor::Ptr> kit::Monitor::getConnectedMonitors()
{
  if(kit::Monitor::m_monitors.size() < 1)
  {
    kit::Monitor::fillMonitors();
  }
  
  return kit::Monitor::m_monitors;
}

kit::Monitor::Ptr kit::Monitor::getPrimaryMonitor()
{
  if(kit::Monitor::m_monitors.size() < 1)
  {
    kit::Monitor::fillMonitors();
    std::cout << "Found " << kit::Monitor::m_monitors.size() << " monitors!" << std::endl;
  }
  GLFWmonitor* primary = glfwGetPrimaryMonitor();
  if(primary == nullptr)
  {
    std::cout << "Warning: GLFW reported primary monitor null" << std::endl;
  }
  else
  {
    kit::Monitor::Ptr finder = kit::Monitor::findMonitor(primary);
    if(finder == nullptr)
    {
      KIT_THROW("Primary monitor is connected but is not registered. This should NEVER happen.");
    }
    
    return finder;
  }
  return nullptr;
}

void kit::Monitor::__monfunc(GLFWmonitor * mon, int event)
{
  if(event == GLFW_CONNECTED)
  {
    kit::Monitor::Ptr monitor = kit::Monitor::findMonitor(mon);
    if(monitor == nullptr)
    {
      kit::Monitor::m_monitors.push_back(kit::Monitor::Ptr(new kit::Monitor(mon)));
    }
    else
    {
     monitor->m_connected = true; 
    }
  }
  else
  {
    for(auto i = kit::Monitor::m_monitors.begin(); i != kit::Monitor::m_monitors.end();)
    {
      if(mon == (*i)->getGLFWHandle())
      {
        (*i)->m_connected = false;
        i = kit::Monitor::m_monitors.erase(i);
      }
      else
      {
        i++;
      }
    }
  }
}

kit::Monitor::Ptr kit::Monitor::findMonitor(GLFWmonitor * mon)
{
  for(auto i = kit::Monitor::m_monitors.begin(); i != kit::Monitor::m_monitors.end();i++)
  {
    if(mon == (*i)->getGLFWHandle())
    {
      return *i;
    }
  }
  return nullptr;
}
