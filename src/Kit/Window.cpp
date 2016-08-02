#include "Kit/Window.hpp"
#include "Kit/Monitor.hpp"
#include "Kit/Exception.hpp"

#include "Kit/Submesh.hpp"
#include "Kit/Texture.hpp"

#include <iostream>

#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif



uint32_t kit::Window::m_instanceCount = 0;
std::vector<kit::Window*> kit::Window::m_windows = std::vector<kit::Window*>();

kit::Window::Args::Args()
{
  this->sharedWindow = nullptr;
  this->mode = kit::Window::Mode::Windowed;
  kit::Monitor::Ptr m = kit::Monitor::getPrimaryMonitor();
  kit::VideoMode vm = m->getVideoMode();
  this->fullscreenMonitor = m;

  this->resolution = glm::uvec2(vm.m_width, vm.m_Height);
  this->resizable = false;
  this->title = "New window";
}

kit::Window::Args::Args(std::string title, kit::Window::Mode mode, glm::uvec2 res, kit::Monitor::Ptr fullmon, kit::Window::Ptr share, bool resi)
{
  this->mode = mode;
  this->resolution = res;
  this->sharedWindow = share;
  this->fullscreenMonitor = fullmon;
  this->resizable = resi;
  this->title = title;
}

kit::Window::Window(kit::Window::Args const & args)
{
  
  this->m_glfwHandle = nullptr;
  GLFWwindow * shared = nullptr;

  this->m_isFocused = true;
  this->m_isMinimized = false;
  
  kit::Window::m_instanceCount++;
  
  if(args.sharedWindow != nullptr)
  {
    shared = args.sharedWindow->getGLFWHandle();
  }
  
  kit::Monitor::Ptr fullmon = args.fullscreenMonitor;
  GLFWmonitor* glfwfullmon = fullmon->getGLFWHandle();

  //kit::Window::prepareGLFWHints(GLFW_SRGB_CAPABLE, GL_TRUE);
  kit::Window::prepareGLFWHints(GLFW_SAMPLES, 4);
  kit::Window::prepareGLFWHints(GLFW_CONTEXT_VERSION_MAJOR, 4);
  kit::Window::prepareGLFWHints(GLFW_CONTEXT_VERSION_MINOR, 1);
  //kit::Window::prepareGLFWHints(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  kit::Window::prepareGLFWHints(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  switch(args.mode)
  {
    case kit::Window::Mode::Windowed:
      (args.resizable ? void()  : kit::Window::prepareGLFWHints(GLFW_RESIZABLE, GL_FALSE));
      this->m_glfwHandle = glfwCreateWindow(args.resolution.x, args.resolution.y, args.title.c_str(), nullptr, shared);
      break;

    case kit::Window::Mode::Fullscreen:
      this->m_glfwHandle = glfwCreateWindow(args.resolution.x, args.resolution.y, args.title.c_str(), glfwfullmon, shared);
      break;

    case kit::Window::Mode::Borderless:
      kit::Window::prepareGLFWHints(GLFW_DECORATED, GL_FALSE);
      kit::Window::prepareGLFWHints(GLFW_RESIZABLE, GL_FALSE);
      this->m_glfwHandle = glfwCreateWindow(args.resolution.x, args.resolution.y, args.title.c_str(), nullptr, shared);
      break;

    default:
      KIT_THROW("Invalid window mode");
      break;
  }
  kit::Window::restoreGLFWHints();
  if(!this->m_glfwHandle)
  {
    KIT_THROW("Failed to create GLFW window");
  }
  kit::Window::m_windows.push_back(this);
  
  glfwSetWindowPosCallback(this->m_glfwHandle, kit::Window::__winfunc_position);
  glfwSetWindowSizeCallback(this->m_glfwHandle, kit::Window::__winfunc_size);
  glfwSetWindowCloseCallback(this->m_glfwHandle, kit::Window::__winfunc_close);
  //glfwSetWindowRefreshCallback(this->m_GlfwHandle, kit::Window::__winfunc_refresh);
  glfwSetWindowFocusCallback(this->m_glfwHandle, kit::Window::__winfunc_focus);
  glfwSetWindowIconifyCallback(this->m_glfwHandle, kit::Window::__winfunc_minimize);
  glfwSetFramebufferSizeCallback(this->m_glfwHandle, kit::Window::__winfunc_framebuffersize);
  glfwSetMouseButtonCallback(this->m_glfwHandle, kit::Window::__infunc_mousebutton);
  glfwSetCursorPosCallback(this->m_glfwHandle, kit::Window::__infunc_cursorpos);
  glfwSetCursorEnterCallback(this->m_glfwHandle, kit::Window::__infunc_cursorenter);
  glfwSetScrollCallback(this->m_glfwHandle, kit::Window::__infunc_scroll);
  glfwSetKeyCallback(this->m_glfwHandle, kit::Window::__infunc_key);
  glfwSetCharCallback(this->m_glfwHandle, kit::Window::__infunc_char);

  this->activateContext();
  kit::initializeGL3W();
  //kit::GL::enable(GL_FRAMEBUFFER_SRGB);
  kit::GL::enable(GL_MULTISAMPLE);
  KIT_GL(glViewport(0, 0, this->getFramebufferSize().x , this->getFramebufferSize().y));

  
  float aniso;
  KIT_GL(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso));
  std::cout << "Max anisotropic level: " << aniso << std::endl;
  
  glfwSwapInterval(1);
}

kit::Window::~Window()
{
  kit::Window::m_instanceCount--;

  if (kit::Window::m_instanceCount == 0)
  {
    kit::Submesh::flushCache();
    kit::Texture::flushCache();
  }

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
  glfwDestroyWindow(this->m_glfwHandle);
}

void kit::Window::setVSync(bool b)
{
  glfwSwapInterval((b?1:0));
}

kit::Window::Ptr kit::Window::create(std::string title, kit::Window::Mode mode, glm::uvec2 resolution)
{
  kit::Window::Ptr returner;
  kit::Window::Args args(title, mode, resolution);
  return kit::Window::create(args);
}

kit::Window::Ptr kit::Window::create(kit::Window::Args const & args)
{
  kit::Window::Ptr returner =  std::make_shared<kit::Window>(args);
  return returner;
}


void kit::Window::activateContext()
{
  glfwMakeContextCurrent(this->m_glfwHandle);
}

void kit::Window::display()
{
  glfwSwapBuffers(this->m_glfwHandle);
}

void kit::Window::clear(glm::vec4 c)
{
  this->bind();
  KIT_GL(glClearColor(c.x, c.y, c.z, c.w));
  KIT_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  KIT_GL(glViewport(0, 0, this->getFramebufferSize().x, this->getFramebufferSize().y));
}

void kit::Window::bind()
{
  kit::GL::bindFramebuffer(GL_FRAMEBUFFER, 0);
  KIT_GL(glViewport(0, 0, this->getFramebufferSize().x, this->getFramebufferSize().y));
}

GLFWwindow * kit::Window::getGLFWHandle()
{
  return this->m_glfwHandle;
}

int  kit::Window::getGLFWAttribute(int attrib)
{
  return glfwGetWindowAttrib(this->m_glfwHandle, attrib);
}

void kit::Window::prepareGLFWHints(int target, int hint, bool RestoreBeforePreparing)
{
  if(RestoreBeforePreparing)
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
  glfwSetWindowShouldClose(this->m_glfwHandle, true);
}

bool kit::Window::isOpen()
{
  return (glfwWindowShouldClose(this->m_glfwHandle) == 0 ? true : false);
}

void kit::Window::setTitle(std::string newtitle)
{
  glfwSetWindowTitle(this->m_glfwHandle, newtitle.c_str());
}

glm::ivec2 kit::Window::getPosition()
{
  glm::ivec2 returner;
  glfwGetWindowPos(this->m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

void kit::Window::setPosition(glm::ivec2 newpos)
{
  glfwSetWindowPos(this->m_glfwHandle, newpos.x, newpos.y);
}

glm::ivec2 kit::Window::getSize()
{
  glm::ivec2 returner;
  glfwGetWindowSize(this->m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

void kit::Window::setSize(glm::ivec2 newsize)
{
  glfwSetWindowSize(this->m_glfwHandle, newsize.x, newsize.y);
}

glm::ivec2 kit::Window::getFramebufferSize()
{
  glm::ivec2 returner;
  glfwGetFramebufferSize(this->m_glfwHandle, &returner.x, &returner.y);
  return returner;
}

void kit::Window::maximize()
{
#ifdef _WIN32
  SendMessage(glfwGetWin32Window(this->m_glfwHandle), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
#endif
}

void kit::Window::minimize()
{
  glfwIconifyWindow(this->m_glfwHandle);
}

void kit::Window::restore()
{
  glfwRestoreWindow(this->m_glfwHandle);
}

void kit::Window::show()
{
  glfwShowWindow(this->m_glfwHandle);
}

void kit::Window::hide()
{
  glfwHideWindow(this->m_glfwHandle);
}

kit::Monitor * kit::Window::getFullscreenMonitor()
{
  KIT_THROW("Not implemented.");
  return nullptr;
}


void kit::Window::addEvent(kit::WindowEvent e)
{
  this->m_eventList.push(e);
}

bool kit::Window::fetchEvent(kit::WindowEvent & e)
{
  if(!this->m_eventsDistributed)
  {
    glfwPollEvents(); 
    this->m_eventsDistributed = true;
  }

  if(m_eventList.size() < 1)
  {
    this->m_eventsDistributed = false;
    return false;
  }

  e = m_eventList.front();
  m_eventList.pop();
  
  return true;

}

bool kit::Window::isKeyDown(kit::Key k)
{
  return (glfwGetKey(this->m_glfwHandle, k) == GLFW_PRESS) ? true : false;
}

bool kit::Window::isMouseButtonDown(kit::MouseButton m)
{
  return (glfwGetMouseButton(this->m_glfwHandle, m) == GLFW_PRESS) ? true : false;
}

glm::vec2 kit::Window::getMousePosition()
{
  glm::vec2 returner;
  double x, y;
  glfwGetCursorPos(this->m_glfwHandle, &x, &y);
  returner.x = (float)x;
  returner.y = (float)y;
  return returner;
}

void kit::Window::setMousePosition(glm::vec2 newpos)
{
  glfwSetCursorPos(this->m_glfwHandle, newpos.x, newpos.y);
}

void kit::Window::mouseCursorVisible(bool b)
{
  glfwSetInputMode(this->m_glfwHandle, GLFW_CURSOR, ((b) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN));
}

void kit::Window::setMouseVirtual(bool v)
{
  this->m_virtualMouse = v;

  if(v)
  {
    glfwSetInputMode(this->m_glfwHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  else
  {
    glfwSetInputMode(this->m_glfwHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

void kit::Window::__winfunc_position(GLFWwindow* wnd, int newx, int newy){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = kit::WindowEvent::Type::Moved;
	e.moved.newPosition.x = newx;
	e.moved.newPosition.y = newy;
	w->addEvent(e);
}

void kit::Window::__winfunc_size(GLFWwindow* wnd, int newwidth, int newheight){
  if (newwidth == 0 || newheight == 0)
  {
    return;
  }

	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = kit::WindowEvent::Type::Resized;
	e.resized.newSize.x = newwidth;
	e.resized.newSize.y = newheight;
	w->addEvent(e);
}

void kit::Window::__winfunc_close(GLFWwindow* wnd){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = kit::WindowEvent::Type::Closing;
	w->addEvent(e);
}

void kit::Window::__winfunc_refresh(GLFWwindow* wnd){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = kit::WindowEvent::Type::Invalidated;
	w->addEvent(e);
}

void kit::Window::__winfunc_focus(GLFWwindow* wnd, int glbool){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = kit::WindowEvent::Type::FocusChanged;
	e.hasFocus = (glbool == GL_TRUE) ? true : false;
        w->m_isFocused = e.hasFocus;
	w->addEvent(e);
}

void kit::Window::__winfunc_minimize(GLFWwindow* wnd, int glbool){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = (glbool == GL_TRUE) ? kit::WindowEvent::Type::Minimized : kit::WindowEvent::Type::Restored;
        w->m_isMinimized = (glbool == GL_TRUE);
	w->addEvent(e);
}

void kit::Window::__winfunc_framebuffersize(GLFWwindow* wnd, int newwidth, int newheight){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = kit::WindowEvent::Type::FramebufferResized;
	e.resized.newSize.x = newwidth;
	e.resized.newSize.y = newheight;
	w->addEvent(e);
  w->activateContext();
  glViewport(0, 0, w->getFramebufferSize().x, w->getFramebufferSize().y);
}


void kit::Window::__infunc_mousebutton(GLFWwindow* wnd, int button, int action, int mods){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
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

void kit::Window::__infunc_cursorpos(GLFWwindow* wnd, double newx, double newy){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = kit::WindowEvent::Type::MouseMoved;
	e.mouse.newPosition.x = (float)newx;
	e.mouse.newPosition.y = (float)newy;
	w->addEvent(e);
}

void kit::Window::__infunc_cursorenter(GLFWwindow* wnd, int glbool){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = (glbool == GL_TRUE) ? kit::WindowEvent::Type::MouseEntered : kit::WindowEvent::Type::MouseLeft;
	w->addEvent(e);
}

void kit::Window::__infunc_scroll(GLFWwindow* wnd, double xoffset, double yoffset){
	kit::Window* w = kit::Window::kitWFromGLFW(wnd);
	kit::WindowEvent e;
	//e.m_Window = w;
	e.type = kit::WindowEvent::Type::MouseScrolled;
	e.mouse.scrollOffset.x = (float)xoffset;
	e.mouse.scrollOffset.y = (float)yoffset;
	w->addEvent(e);
}

void kit::Window::__infunc_key(GLFWwindow* wnd, int key, int scancode, int action, int mods){
  kit::Window* w = kit::Window::kitWFromGLFW(wnd);
  kit::WindowEvent e;
  //e.Window = w;

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

void kit::Window::__infunc_char(GLFWwindow* wnd, unsigned int codepoint){
  kit::Window* w = kit::Window::kitWFromGLFW(wnd);
  kit::WindowEvent e;
  //e.m_Window = w;
  e.type = kit::WindowEvent::Type::TextEntered;
  e.keyboard.unicode = codepoint;

  w->addEvent(e);
}

kit::Window* kit::Window::kitWFromGLFW(GLFWwindow* ptr){

  for(auto finder = kit::Window::m_windows.begin(); finder != kit::Window::m_windows.end(); ++finder){
    if((*finder)->getGLFWHandle() == ptr){
      return (*finder);
    }
  }
  KIT_THROW("No such window registered.");
  return nullptr; // Compiler will nag if we dont return
}

bool kit::Window::isFocused()
{
  return this->m_isFocused;
}

bool kit::Window::isMinimized()
{
  return this->m_isMinimized;
}
