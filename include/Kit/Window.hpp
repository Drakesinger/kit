#ifndef KIT_WINDOW_HEADER
#define KIT_WINDOW_HEADER

#include "Kit/WindowEvent.hpp"
#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"
#include "Kit/Input.hpp"
#include "Kit/Monitor.hpp"

#include <queue>
#include <string>
#include <memory>

struct GLFWwindow;

namespace kit
{

  class Monitor;

  class KITAPI Window
  {
    public:

      typedef std::shared_ptr<Window> Ptr;
      typedef std::weak_ptr<Window> WPtr;
      
      enum KITAPI Mode
      {
        Windowed = 0,
        Fullscreen = 1,
        Borderless = 2
      };

      struct KITAPI Args
      {
          Args();
          Args(std::string title, kit::Window::Mode mode, glm::uvec2 res, kit::Monitor::Ptr fullscreenmonitor = kit::Monitor::getPrimaryMonitor() , kit::Window::Ptr share = nullptr, bool resizable = false);

          std::string        title;
          kit::Monitor::Ptr   fullscreenMonitor;
          kit::Window::Mode  mode;
          glm::uvec2       resolution;
          kit::Window::Ptr   sharedWindow;
          bool               resizable;

      private:
        kit::GLFWSingleton m_glfwSingleton;
      };

      ~Window();

      void    activateContext();

      void    display();
      void    clear(glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));

      GLFWwindow * getGLFWHandle();
      int   getGLFWAttribute(int attrib);
      static void prepareGLFWHints(int target, int hint, bool RestoreBeforePreparing = false);
      static void     restoreGLFWHints();

      static kit::Window::Ptr create(std::string title, kit::Window::Mode mode, glm::uvec2 resolution);
      static kit::Window::Ptr create(kit::Window::Args const & args);

      void bind();

      void close();
      bool isOpen();

      void setTitle(std::string newtitle);

      glm::ivec2 getPosition();
      void setPosition(glm::ivec2 newpos);

      glm::ivec2 getSize();
      void setSize(glm::ivec2 newsize);

      glm::ivec2 getFramebufferSize();

      void maximize();
      void minimize();
      void restore();

      void show();
      void hide();

      static void setVSync(bool);

      kit::Monitor * getFullscreenMonitor(); // Returns nullptr if windowed

      void addEvent(kit::WindowEvent e);
      bool fetchEvent(kit::WindowEvent & e);

      bool isFocused();
      bool isMinimized();
      
      bool isKeyDown(kit::Key k);
      bool isMouseButtonDown(kit::MouseButton m);
      glm::vec2 getMousePosition();
      void setMousePosition(glm::vec2);
      void mouseCursorVisible(bool);

      void setMouseVirtual(bool);
      Window(kit::Window::Args const & a);

    private:
      kit::GLFWSingleton m_glfwSingleton;
      kit::GL m_glSingleton;

      static uint32_t                   m_instanceCount;
      static std::vector<kit::Window*>     m_windows;
      static kit::Window* kitWFromGLFW(GLFWwindow* ptr);
      static void __winfunc_position(GLFWwindow*, int newx, int newy);
      static void __winfunc_size(GLFWwindow*, int newwidth, int newheight);
      static void __winfunc_close(GLFWwindow*);
      static void __winfunc_refresh(GLFWwindow*);
      static void __winfunc_focus(GLFWwindow*, int glbool);
      static void __winfunc_minimize(GLFWwindow*, int glbool);
      static void __winfunc_framebuffersize(GLFWwindow*, int newwidth, int newheight);
      static void __infunc_mousebutton(GLFWwindow*, int button, int action, int mods);
      static void __infunc_cursorpos(GLFWwindow*, double newx, double newy);
      static void __infunc_cursorenter(GLFWwindow*, int glbool);
      static void __infunc_scroll(GLFWwindow*, double xoffset, double yoffset);
      static void __infunc_key(GLFWwindow*, int key, int scancode, int action, int mods);
      static void __infunc_char(GLFWwindow*, unsigned int codepoint);
    
      GLFWwindow                    * m_glfwHandle;
      std::queue<kit::WindowEvent>  m_eventList;
      bool                            m_eventsDistributed;
      
      bool m_isFocused;
      bool m_isMinimized;

      bool                            m_virtualMouse;
  };
}

#endif // KIT_WINDOW_HEADER
