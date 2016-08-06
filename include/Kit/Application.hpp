#ifndef KIT_APPLICATION_HPP
#define KIT_APPLICATION_HPP

#include "Kit/ScriptLib.hpp"  // Inheritance to kit::Scriptable
#include "Kit/Timer.hpp"      // m_msSinceRender, m_msSinceUpdate
#include "Kit/Input.hpp"      // Input bindings
#include "Kit/WindowEvent.hpp" // Window events

#include <vector>
#include <stack>
#include <memory>

namespace kit
{
  class Console;
  typedef std::shared_ptr<kit::Console> ConsolePtr;

  class Window;
  typedef std::shared_ptr<kit::Window> WindowPtr;

  class ApplicationState;
  typedef std::shared_ptr<kit::ApplicationState> ApplicationStatePtr;

  class KITAPI Application : public kit::Scriptable
  {
  public:
      Application();
      ~Application();

      void run(ApplicationStatePtr state);

      void pushState(ApplicationStatePtr state);
      void popState();

      kit::WindowPtr getWindow();
      kit::ConsolePtr getConsole();

      void evaluate(std::string code);
      void quit();
      void print(std::string);

      virtual void onResize(glm::uvec2);
 
      virtual void onInitialize();

      virtual void onRender();

      void setUpdateRate(double ms);
      void setUnfocusedUpdateRate(double ms);

      void setRenderRate(double ms);
      void setUnfocusedRenderRate(double ms);
      
      void setVSync(bool enabled);

  protected:
      void update(double const & mstime);
      void handleEvent(const double & mstime, kit::WindowEvent const & evt);
      void render();

  private:

      void initialize();
      void fillKeyIndex();

      std::map<kit::Key, std::string> m_keyIndex;
      std::map<kit::MouseButton, std::string> m_mouseIndex;

      bool m_needResize;
      glm::uvec2 m_resizeSize;

      // Window
      kit::WindowPtr  m_window;       //< Our window (provided by Kit)

      kit::Timer        m_msSinceUpdate;    //< How many milliseconds since the last frame render _started_ (Add m_lastRenderTime to get the time since last frame render ended)
      double            m_updateRate;       //< Time delay between update steps in milliseconds. Default 1000 / 90
      double            m_unfocusedUpdateRate; //< Updaterate when unfocused
      double            m_lastUpdateTime;   //< How many milliseconds the last update step took.

      kit::Timer        m_msSinceRender;    //< How many milliseconds since the last frame render _started_ (Add m_lastRenderTime to get the time since last frame render ended)
      double            m_renderRate;       //< Time delay between frame renders in milliseconds. Default 1000 / 30
      double            m_unfocusedRenderRate;    //< Render rate when unfocused
      double            m_lastRenderTime;   //< How many milliseconds the last frame render took.

      // State machine
      std::stack<kit::ApplicationStatePtr>        m_states;           //< List of game states

      // Console 
      kit::ConsolePtr m_console;
  };

}
#endif