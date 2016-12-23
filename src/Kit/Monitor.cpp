#include "Kit/Monitor.hpp"

#include <GLFW/glfw3.h>

std::vector<kit::Monitor*> kit::Monitor::m_monitors = std::vector<kit::Monitor*>();
uint32_t kit::Monitor::m_instanceCount = 0;

glm::uvec2 kit::VideoMode::Resolution()
{
  return glm::uvec2(m_width, m_height);
}

kit::Monitor::Monitor(GLFWmonitor * glfwhandle)
{
  m_glfwHandle = glfwhandle;
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
    m_monitors.push_back(new kit::Monitor(monitors[i]));
  }
}

GLFWmonitor * kit::Monitor::getGLFWHandle()
{
  return m_glfwHandle;
}

bool kit::Monitor::isConnected()
{
  return m_connected;
}

glm::ivec2 kit::Monitor::getPhysicalSize()
{
  glm::ivec2 returner;
  glfwGetMonitorPhysicalSize(m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

glm::ivec2 kit::Monitor::getPosition()
{
  glm::ivec2 returner;
  glfwGetMonitorPos(m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

std::string kit::Monitor::getName()
{
  return std::string(glfwGetMonitorName(m_glfwHandle));
}

std::vector<kit::VideoMode> kit::Monitor::getVideoModes()
{
  int count = 0;
  const GLFWvidmode * modes = glfwGetVideoModes(m_glfwHandle, &count);
  std::vector<kit::VideoMode> returner;

  for(int i = 0; i < count; i++)
  {
    kit::VideoMode adder;
    adder.m_width = modes[i].width;
    adder.m_height = modes[i].height;
    adder.m_redBits = modes[i].redBits;
    adder.m_greenBits = modes[i].greenBits;
    adder.m_blueBits = modes[i].blueBits;
    adder.m_refreshRate = modes[i].refreshRate;
    returner.push_back(adder);
  }

  return returner;
}

kit::VideoMode kit::Monitor::getVideoMode()
{
  const GLFWvidmode * mode = glfwGetVideoMode(m_glfwHandle);
  kit::VideoMode returner;
  returner.m_width = mode->width;
  returner.m_height = mode->height;
  returner.m_redBits = mode->redBits;
  returner.m_greenBits = mode->greenBits;
  returner.m_blueBits = mode->blueBits;
  returner.m_refreshRate = mode->refreshRate;
  return returner;
}

std::vector<kit::Monitor*> kit::Monitor::getMonitors()
{
  if(kit::Monitor::m_monitors.size() < 1)
  {
    kit::Monitor::fillMonitors();
  }
  
  if(kit::Monitor::m_monitors.size() < 1)
  {
    std::cout << "Warning: GLFW couldn't find any monitors!" << std::endl;
  }
  
  return m_monitors;
}

kit::Monitor * kit::Monitor::getPrimaryMonitor()
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
    return getMonitors()[0];
  }
  else
  {
    kit::Monitor * finder = kit::Monitor::findMonitor(primary);
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
    kit::Monitor* monitor = kit::Monitor::findMonitor(mon);
    if(monitor == nullptr)
    {
      kit::Monitor::m_monitors.push_back(new kit::Monitor(mon));
    }
    else
    {
     monitor->m_connected = true; 
    }
  }
  else
  {
    for(auto i = kit::Monitor::m_monitors.begin(); i != kit::Monitor::m_monitors.end(); i++)
    {
      if(mon == (*i)->getGLFWHandle())
      {
        (*i)->m_connected = false;
      }
    }
  }
}

kit::Monitor * kit::Monitor::findMonitor(GLFWmonitor * mon)
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
