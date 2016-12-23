#include "Kit/Window.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Monitor.hpp"
#include "Kit/Exception.hpp"
#include "Kit/Submesh.hpp"
#include "Kit/Texture.hpp"

#include <iostream>
#include <GLFW/glfw3.h>

// For maximize functionality, currently only supported under Windows
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

uint32_t kit::Window::m_instanceCount = 0;
std::vector<kit::Window*> kit::Window::m_windows = std::vector<kit::Window*>();

kit::GLFWSingleton kit::Window::m_glfwSingleton = kit::GLFWSingleton();

kit::Window::Args::Args()
{
  // Defaults to using the resolution of the primary monitor
  
  sharedWindow = nullptr;
  mode = kit::Window::Mode::Windowed;
  fullscreenMonitor = nullptr;
  resolution = glm::uvec2(0, 0);
  resizable = false;
  title = "New window";
}

kit::Window::Args::Args(const std::string & t, kit::Window::Mode m, glm::uvec2 r, kit::Monitor * f, kit::Window * s, bool rz)
{
  mode = m;
  resolution = r;
  sharedWindow = s;
  fullscreenMonitor = f;
  resizable = rz;
  title = t;
}

kit::Window::Window(kit::Window::Args const & windowArgs)
{
  kit::Window::m_instanceCount++;
  
  m_glfwHandle = nullptr;
  m_isFocused = true;
  m_isMinimized = false;
  m_virtualMouse = false;
  m_eventsDistributed = false;

  // Get the GLFW handle from the window to share resources with
  GLFWwindow * glfwSharedWindow = nullptr;  
  if(windowArgs.sharedWindow != nullptr)
  {
    glfwSharedWindow = windowArgs.sharedWindow->getGLFWHandle();
  }
  
  // Get the GLFW handle for the fullscreen monitor to use
  auto autoMon = kit::Monitor::getPrimaryMonitor();
  GLFWmonitor* glfwFullscreenMonitor = autoMon->getGLFWHandle();
  if(windowArgs.fullscreenMonitor)
  {
      GLFWmonitor* glfwFullscreenMonitor = windowArgs.fullscreenMonitor->getGLFWHandle();
  }
  
  glm::uvec2 effResolution = windowArgs.resolution;
  if(effResolution.x < 1 || effResolution.y < 1)
  {
    effResolution = glm::uvec2(autoMon->getVideoMode().m_width, autoMon->getVideoMode().m_height);
  }

  // Set OpenGL context hints.
  kit::Window::prepareGLFWHints(GLFW_CLIENT_API, GLFW_OPENGL_API);
#ifndef KIT_ALLOW_EGL
  std::cout << "Forcing native context API" << std::endl;
  kit::Window::prepareGLFWHints(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
#else
  std::cout << "Allowing EGL contexts" << std::endl;
#endif
#ifndef KIT_SHITTY_INTEL
  kit::Window::prepareGLFWHints(GLFW_CONTEXT_VERSION_MAJOR, 4);
  kit::Window::prepareGLFWHints(GLFW_CONTEXT_VERSION_MINOR, 5);
#else 
  kit::Window::prepareGLFWHints(GLFW_CONTEXT_VERSION_MAJOR, 4);
  kit::Window::prepareGLFWHints(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif 
  kit::Window::prepareGLFWHints(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Set window-specific hints and create window according to our window-arguments
  switch(windowArgs.mode)
  {
    case kit::Window::Mode::Windowed:
      if(!windowArgs.resizable)
      {
        kit::Window::prepareGLFWHints(GLFW_RESIZABLE, GL_FALSE);
      }
      m_glfwHandle = glfwCreateWindow(effResolution.x, effResolution.y, windowArgs.title.c_str(), nullptr, glfwSharedWindow);
      break;

    case kit::Window::Mode::Fullscreen:
      m_glfwHandle = glfwCreateWindow(effResolution.x, effResolution.y, windowArgs.title.c_str(), glfwFullscreenMonitor, glfwSharedWindow);
      break;

    case kit::Window::Mode::Borderless:
      kit::Window::prepareGLFWHints(GLFW_DECORATED, GL_FALSE);
      kit::Window::prepareGLFWHints(GLFW_RESIZABLE, GL_FALSE);
      m_glfwHandle = glfwCreateWindow(effResolution.x, effResolution.y, windowArgs.title.c_str(), nullptr, glfwSharedWindow);
      break;

    default:
      KIT_THROW("Invalid window mode");
      break;
  }
  
  // Reset the GLFW hints after creation
  kit::Window::restoreGLFWHints();
  
  // Assert that we have a GLFW window
  if(!m_glfwHandle)
  {
    KIT_THROW("Failed to create GLFW window");
  }
  
  // Register the window to the static list of windows, to keep track of events/callbacks
  kit::Window::m_windows.push_back(this);
  
  // Register GLFW callbacks for this window
  glfwSetWindowPosCallback(m_glfwHandle, kit::Window::__winfunc_position);
  glfwSetWindowSizeCallback(m_glfwHandle, kit::Window::__winfunc_size);
  glfwSetWindowCloseCallback(m_glfwHandle, kit::Window::__winfunc_close);
  glfwSetWindowFocusCallback(m_glfwHandle, kit::Window::__winfunc_focus);
  glfwSetWindowIconifyCallback(m_glfwHandle, kit::Window::__winfunc_minimize);
  glfwSetFramebufferSizeCallback(m_glfwHandle, kit::Window::__winfunc_framebuffersize);
  glfwSetMouseButtonCallback(m_glfwHandle, kit::Window::__infunc_mousebutton);
  glfwSetCursorPosCallback(m_glfwHandle, kit::Window::__infunc_cursorpos);
  glfwSetCursorEnterCallback(m_glfwHandle, kit::Window::__infunc_cursorenter);
  glfwSetScrollCallback(m_glfwHandle, kit::Window::__infunc_scroll);
  glfwSetKeyCallback(m_glfwHandle, kit::Window::__infunc_key);
  glfwSetCharCallback(m_glfwHandle, kit::Window::__infunc_char);

  // Activate the current windows context
  activateContext();
  
  // Enable V-sync
  glfwSwapInterval(1);
  
  // Make sure GL3W is initialized, and set the viewport
  kit::initializeGL3W();
  glViewport(0, 0, getFramebufferSize().x , getFramebufferSize().y);
}

kit::Window::~Window()
{
  kit::Window::m_instanceCount--;

  // Unregister window from static list
  for(auto i = kit::Window::m_windows.begin(); i != kit::Window::m_windows.end();)
  {
    if(*i == this)
    {
      i = kit::Window::m_windows.erase(i);
    }
    else
    {
      ++i; 
    }
  }
  
  // Destroy the GLFW instance
  glfwDestroyWindow(m_glfwHandle);
}

void kit::Window::setVSync(bool enabled)
{
  activateContext();
  glfwSwapInterval((enabled ? 1 : 0));
}

kit::Window::Window(const std::string & title, kit::Window::Mode mode, glm::uvec2 resolution) : kit::Window(kit::Window::Args(title, mode, resolution))
{

}

void kit::Window::activateContext()
{
  if(m_glfwHandle == nullptr)
  {
    return;
  }
  
  glfwMakeContextCurrent(m_glfwHandle);
}

void kit::Window::display()
{
  glfwSwapBuffers(m_glfwHandle);
}

void kit::Window::clear(glm::vec4 clearColor)
{
  bind();
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, getFramebufferSize().x, getFramebufferSize().y);
}

void kit::Window::bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, getFramebufferSize().x, getFramebufferSize().y);
}

GLFWwindow * kit::Window::getGLFWHandle()
{
  return m_glfwHandle;
}

int  kit::Window::getGLFWAttribute(int attribute)
{
  return glfwGetWindowAttrib(m_glfwHandle, attribute);
}

void kit::Window::prepareGLFWHints(int target, int hint, bool restoreBeforePreparing)
{
  if(restoreBeforePreparing)
  {
    kit::Window::restoreGLFWHints();
  }

  glfwWindowHint(target, hint);
}

void kit::Window::restoreGLFWHints()
{
  glfwDefaultWindowHints();
}

void kit::Window::close()
{
  glfwSetWindowShouldClose(m_glfwHandle, true);
}

bool kit::Window::isOpen()
{
  return (glfwWindowShouldClose(m_glfwHandle) == 0);
}

void kit::Window::setTitle(const std::string&newTitle)
{
  glfwSetWindowTitle(m_glfwHandle, newTitle.c_str());
}

glm::ivec2 kit::Window::getPosition()
{
  glm::ivec2 returner;
  glfwGetWindowPos(m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

void kit::Window::setPosition(glm::ivec2 newPosition)
{
  glfwSetWindowPos(m_glfwHandle, newPosition.x, newPosition.y);
}

glm::ivec2 kit::Window::getSize()
{
  glm::ivec2 returner;
  glfwGetWindowSize(m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

void kit::Window::setSize(glm::ivec2 newSize)
{
  glfwSetWindowSize(m_glfwHandle, newSize.x, newSize.y);
}

glm::ivec2 kit::Window::getFramebufferSize()
{
  glm::ivec2 returner;
  glfwGetFramebufferSize(m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

void kit::Window::maximize()
{
#ifdef _WIN32
  SendMessage(glfwGetWin32Window(m_glfwHandle), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
#else
  KIT_ERR("Warning: Unsupported OS");
#endif
}

void kit::Window::minimize()
{
  glfwIconifyWindow(m_glfwHandle);
}

void kit::Window::restore()
{
  glfwRestoreWindow(m_glfwHandle);
}

void kit::Window::show()
{
  glfwShowWindow(m_glfwHandle);
}

void kit::Window::hide()
{
  glfwHideWindow(m_glfwHandle);
}

void kit::Window::addEvent(kit::WindowEvent e)
{
  m_eventList.push(e);
}

bool kit::Window::fetchEvent(kit::WindowEvent & e)
{
  // If events hasn't been distributed yet, distribute them
  if(!m_eventsDistributed)
  {
    glfwPollEvents(); 
    m_eventsDistributed = true;
  }

  // If every event have been fetched, set distribution flag to false and return false
  if(m_eventList.size() < 1)
  {
    m_eventsDistributed = false;
    return false;
  }

  // Set e to next event and pop the current
  e = m_eventList.front();
  m_eventList.pop();
  
  return true;
}

bool kit::Window::isKeyDown(kit::Key k)
{
  return (glfwGetKey(m_glfwHandle, k) == GLFW_PRESS) ? true : false;
}

bool kit::Window::isMouseButtonDown(kit::MouseButton m)
{
  return (glfwGetMouseButton(m_glfwHandle, m) == GLFW_PRESS) ? true : false;
}

glm::vec2 kit::Window::getMousePosition()
{
  glm::vec2 returner;
  double x, y;
  glfwGetCursorPos(m_glfwHandle, &x, &y);
  returner.x = (float)x;
  returner.y = (float)y;
  return returner;
}

void kit::Window::setMousePosition(glm::vec2 newPosition)
{
  glfwSetCursorPos(m_glfwHandle, newPosition.x, newPosition.y);
}

void kit::Window::mouseCursorVisible(bool visible)
{
  glfwSetInputMode(m_glfwHandle, GLFW_CURSOR, (visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN));
}

void kit::Window::setMouseVirtual(bool isVirtual)
{
  m_virtualMouse = isVirtual;

  if(isVirtual)
  {
    glfwSetInputMode(m_glfwHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  else
  {
    glfwSetInputMode(m_glfwHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

void kit::Window::__winfunc_position(GLFWwindow* window, int newx, int newy)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = kit::WindowEvent::Type::Moved;
  e.moved.newPosition.x = newx;
  e.moved.newPosition.y = newy;
  w->addEvent(e);
}

void kit::Window::__winfunc_size(GLFWwindow* window, int newWidth, int newHeight)
{
  // Ignore event if size is 0 on any axis. This happens on some platforms on certain window events.
  if (newWidth == 0 || newHeight == 0)
  {
    return;
  }

  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  //e.m_Window = w;
  e.type = kit::WindowEvent::Type::Resized;
  e.resized.newSize.x = newWidth;
  e.resized.newSize.y = newHeight;
  w->addEvent(e);
}

void kit::Window::__winfunc_close(GLFWwindow* window)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = kit::WindowEvent::Type::Closing;
  w->addEvent(e);
}

void kit::Window::__winfunc_focus(GLFWwindow* window, int glbool)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = kit::WindowEvent::Type::FocusChanged;
  e.hasFocus = (glbool == GL_TRUE);
  w->m_isFocused = e.hasFocus;
  w->addEvent(e);
}

void kit::Window::__winfunc_minimize(GLFWwindow* window, int glbool)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = (glbool == GL_TRUE) ? kit::WindowEvent::Type::Minimized : kit::WindowEvent::Type::Restored;
  w->addEvent(e);
}

void kit::Window::__winfunc_framebuffersize(GLFWwindow* window, int newWidth, int newHeight)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = kit::WindowEvent::Type::FramebufferResized;
  e.resized.newSize.x = newWidth;
  e.resized.newSize.y = newHeight;
  w->addEvent(e);
  w->activateContext();
  glViewport(0, 0, w->getFramebufferSize().x, w->getFramebufferSize().y);
}


void kit::Window::__infunc_mousebutton(GLFWwindow* window, int button, int action, int mods)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;

  switch(action){
    case GLFW_PRESS:
      e.type = kit::WindowEvent::Type::MouseButtonPressed;
      break;
      
    case GLFW_RELEASE:
      e.type = kit::WindowEvent::Type::MouseButtonReleased;
      break;
      
    default:
      break;
  }
  
  e.mouse.button = (kit::MouseButton)button;
  e.mouse.modifiers = mods;
  w->addEvent(e);
}

void kit::Window::__infunc_cursorpos(GLFWwindow* window, double newx, double newy)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = kit::WindowEvent::Type::MouseMoved;
  e.mouse.newPosition.x = (float)newx;
  e.mouse.newPosition.y = (float)newy;
  w->addEvent(e);
}

void kit::Window::__infunc_cursorenter(GLFWwindow* window, int glbool)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = (glbool == GL_TRUE) ? kit::WindowEvent::Type::MouseEntered : kit::WindowEvent::Type::MouseLeft;
  w->addEvent(e);
}

void kit::Window::__infunc_scroll(GLFWwindow* window, double xoffset, double yoffset)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = kit::WindowEvent::Type::MouseScrolled;
  e.mouse.scrollOffset.x = (float)xoffset;
  e.mouse.scrollOffset.y = (float)yoffset;
  w->addEvent(e);
}

void kit::Window::__infunc_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;

  switch(action){
    case GLFW_PRESS:
      e.type = kit::WindowEvent::Type::KeyPressed;
      break;
      
    case GLFW_RELEASE:
      e.type = kit::WindowEvent::Type::KeyReleased;
      break;
      
    case GLFW_REPEAT:
      e.type = kit::WindowEvent::Type::KeyRepeated;
      break;
  }

  e.keyboard.key = (kit::Key)key;
  e.keyboard.modifiers = mods;
  e.keyboard.scancode = scancode;

  w->addEvent(e);
}

void kit::Window::__infunc_char(GLFWwindow* window, unsigned int codepoint)
{
  kit::Window* w = kit::Window::kitWFromGLFW(window);
  kit::WindowEvent e;
  e.type = kit::WindowEvent::Type::TextEntered;
  e.keyboard.unicode = codepoint;

  w->addEvent(e);
}

kit::Window* kit::Window::kitWFromGLFW(GLFWwindow* ptr)
{
  for(auto currWindow : kit::Window::m_windows)
  {
    if(currWindow->getGLFWHandle() == ptr){
      return currWindow;
    }
  }
  
  KIT_THROW("No such window registered.");
  return nullptr; // Compiler will nag if we dont return
}

bool kit::Window::isFocused()
{
  return m_isFocused;
}

bool kit::Window::isMinimized()
{
  return m_isMinimized;
}
