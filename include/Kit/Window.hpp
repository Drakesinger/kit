#ifndef KIT_WINDOW_HEADER
#define KIT_WINDOW_HEADER

#include "Kit/WindowEvent.hpp"
#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include "Kit/Input.hpp"
#include "Kit/Monitor.hpp"

#include <queue>
#include <string>
#include <memory>

struct GLFWwindow;

namespace kit
{

  class Monitor;

  ///
  /// \brief An OpenGL-enabled window class
  ///
  class KITAPI Window
  {
    public:

      typedef std::shared_ptr<Window> Ptr;
      typedef std::weak_ptr<Window> WPtr;
      
      ///
      /// \brief Represents different window modes
      ///
      enum KITAPI Mode
      {
        Windowed   = 0, ///< Window will be windowed
        Fullscreen = 1, ///< Window will be fullscreen
        Borderless = 2  ///< Window will be borderless
      };

      ///
      /// \brief Holds a configuration for window creation
      ///
      struct KITAPI Args
      {
        public:
          
          /// 
          /// \brief Default constructor
          /// 
	  Args();
          
          ///
          /// \brief Constructor
          ///
          /// \param title The window title to set
          /// \param mode The window mode to use (fullscreen, windowed etc.)
          /// \param fullscreenMonitor If the window mode is fullscreen, this parameter specifies on which monitor the window should be on
          /// \param sharedWindow Specifies a window to share resources with
          /// \param resizable Specifies whether the window should be resizable
          ///
	  Args(const std::string& title, kit::Window::Mode mode, glm::uvec2 resolution, kit::Monitor::Ptr fullscreenMonitor = kit::Monitor::getPrimaryMonitor() , kit::Window::Ptr sharedWindow = nullptr, bool resizable = false);

	  std::string        title; ///< The window title to set upon creation
	  kit::Monitor::Ptr  fullscreenMonitor; ///< The monitor to use (for fullscreen windows, specify with mode)
	  kit::Window::Mode  mode; ///< The window mode to set upon creation
	  glm::uvec2         resolution; ///< The window resolution to set upon creation
	  kit::Window::Ptr   sharedWindow; ///< A window to share resources with, or nullptr
	  bool               resizable; ///< True if window should be resizable

	private:
	  kit::GLFWSingleton m_glfwSingleton;
      };

      /// 
      /// \brief Creates a window given a title, window-mode and reoslution
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param title The window title
      /// \param mode The window mode
      /// \param resolution The window resolution
      ///
      /// \returns A shared pointer pointing to the newly created window
      ///
      static kit::Window::Ptr create(const std::string& title, kit::Window::Mode mode, glm::uvec2 resolution);

      /// 
      /// \brief Creates a window given window arguments
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param windowArgs Contains the window arguments for creation of the window
      ///
      /// \returns A shared pointer pointing to the newly created window
      ///
      static kit::Window::Ptr create(kit::Window::Args const & windowArgs);

      ///
      /// \brief Constructor (FOR INTERNAL USE ONLY)
      /// 
      /// You should NEVER instance this as usual. ALWAYS use smart pointers (std::shared_ptr), and create them explicitly using the `create` methods!
      ///
      Window(kit::Window::Args const & a);

      ///
      /// \brief Destructor
      ///
      ~Window();

      ///
      /// \brief Makes the OpenGL context of the window active
      ///
      void activateContext();

      ///
      /// \brief Flips the buffers to display what has been drawn on the screen
      ///
      void display();

      ///
      /// \brief Clears the window
      ///
      /// \param color The color with which to clear the window
      ///
      void clear(glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));

      ///
      /// \returns A pointer to the internal GLFW instance of the window
      ///
      GLFWwindow * getGLFWHandle();

      ///
      /// \brief Retrieve a GLFW window attribute
      /// \returns The value of the GLFW window attribute `attrib`
      /// \param attrib The attribute to retrieve
      /// 
      int getGLFWAttribute(int attrib);

      ///
      /// \brief Prepares GLFW hints to be set upon creation of a window
      ///
      static void prepareGLFWHints(int target, int hint, bool restoreBeforePreparing = false);

      ///
      /// \brief Restores the GLFW hints to be set upon creation of a window
      ///
      static void restoreGLFWHints();

      ///
      /// \brief Binds the windows buffer and sets the glViewport
      ///
      void bind();

      ///
      /// \brief Signals for this window to be closed
      ///
      void close();
      
      ///
      /// \brief Returns true if this window is open
      /// \returns true if open, false if closed
      bool isOpen();

      ///
      /// \brief Sets the window title
      ///
      /// \param newTitle The new windowtitle
      ///
      void setTitle(const std::string& newTitle);

      ///
      /// \brief Retrieve the windows current position
      /// \returns The window position
      ///
      glm::ivec2 getPosition();
      
      ///
      /// \brief Sets the window position
      ///
      /// \param newPosition The new position
      ///
      void setPosition(glm::ivec2 newPosition);

      ///
      /// \brief Retrieve the windows current size
      /// \returns The window size
      ///
      glm::ivec2 getSize();
      
      ///
      /// \brief Sets the window size
      ///
      /// \param newSize The new suze
      ///
      void setSize(glm::ivec2 newSize);

      ///
      /// \brief Retrieve the size of the framebuffer. This only measures the actually paintable area
      /// \returns The framebuffer size
      ///
      glm::ivec2 getFramebufferSize();

      ///
      /// \brief Maximize the window
      ///
      void maximize();
      
      ///
      /// \brief Minimize the window
      ///
      void minimize();
      
      ///
      /// \brief Restores a minimized window
      ///
      void restore();

      ///
      /// \brief Make the window visible
      ///
      void show();
      
      ///
      /// \brief Make the window hidden
      ///
      void hide();

      ///
      /// \brief Sets vertical sync for the window context
      ///
      /// \param vsync true if vsync should be enabled, false othwerise
      ///
      void setVSync(bool vsync);

      ///
      /// \brief Adds an event to the window event queue
      ///
      /// Designed for internal use only
      ///
      void addEvent(kit::WindowEvent e);
      
      ///
      /// \brief Fetches the top-most event. Returns false if the eventlist is empty, true otherwise
      ///
      /// \param e Reference to windowevent instance to fill with new event
      /// \returns true if event successfully fetched, false otherwise
      ///
      bool fetchEvent(kit::WindowEvent & e);

      ///
      /// \brief Checks if window is currently focused
      /// \returns true if window is currently in focus
      ///
      bool isFocused();
      
      ///
      /// \brief Checks if window is minimized
      /// \returns True if minimized
      ///
      bool isMinimized();
      
      ///
      /// \brief Checks if a specified key is down
      /// \param k The specified key to check
      /// \returns true if specified key is down
      ///
      bool isKeyDown(kit::Key k);
      
      ///
      /// \brief Checks if a mouse button is down
      /// \param m The specified button to check
      /// \returns true if specified button is down
      ///
      bool isMouseButtonDown(kit::MouseButton m);
      
      ///
      /// \brief Retrieve the mouse position, relative to the topleft corner of the drawable area.
      /// \returns The mouse position
      ///
      glm::vec2 getMousePosition();
      
      
      ///
      /// \brief Sets the mouse position, relative to the topleft corner of the drawable area.
      /// \param m The new mouse position
      ///
      void setMousePosition(glm::vec2 m);
      
      ///
      /// \brief Set whether the mouse should be visible or not
      /// \param b true if mouse should be visible, false otherwise
      ///
      void mouseCursorVisible(bool b);

      ///
      /// \brief Set whether the mouse should be virtual
      /// \param b true if mouse should be virtual, false otherwise
      ///
      void setMouseVirtual(bool);


    private:
      static uint32_t                      m_instanceCount;
      static std::vector<kit::Window*>     m_windows;
      
      static kit::Window                   * kitWFromGLFW(GLFWwindow* ptr);
      
      static void                          __winfunc_position(GLFWwindow*, int newx, int newy);
      static void                          __winfunc_size(GLFWwindow*, int newwidth, int newheight);
      static void                          __winfunc_close(GLFWwindow*);
      static void                          __winfunc_focus(GLFWwindow*, int glbool);
      static void                          __winfunc_minimize(GLFWwindow*, int glbool);
      static void                          __winfunc_framebuffersize(GLFWwindow*, int newwidth, int newheight);
      static void                          __infunc_mousebutton(GLFWwindow*, int button, int action, int mods);
      static void                          __infunc_cursorpos(GLFWwindow*, double newx, double newy);
      static void                          __infunc_cursorenter(GLFWwindow*, int glbool);
      static void                          __infunc_scroll(GLFWwindow*, double xoffset, double yoffset);
      static void                          __infunc_key(GLFWwindow*, int key, int scancode, int action, int mods);
      static void                          __infunc_char(GLFWwindow*, unsigned int codepoint);

      kit::GLFWSingleton                   m_glfwSingleton;

      GLFWwindow                           * m_glfwHandle;
      std::queue<kit::WindowEvent>         m_eventList;
      bool                                 m_eventsDistributed;
      bool                                 m_isFocused;
      bool                                 m_isMinimized;
      bool                                 m_virtualMouse;
  };
}

#endif // KIT_WINDOW_HEADER
