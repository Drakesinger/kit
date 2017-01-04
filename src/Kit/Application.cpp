#include "Kit/Application.hpp"
#include "Kit/Console.hpp"
#include "Kit/Window.hpp"
#include "Kit/ApplicationState.hpp"

#include <algorithm>

kit::Application::Application() : kit::Scriptable()
{
  kit::Window::Args args;
  args.mode = kit::Window::Windowed;
  args.resolution = glm::uvec2(1366, 768);
  args.resizable = true;
  args.title = "Kit Application";
  
  m_window = new kit::Window( args);
  
  m_console = new kit::Console(this);
  
  m_scriptEngine.add_global(chaiscript::var(this), "self");
  
  try
  {
    m_scriptEngine.eval_file("./data/autoexec.cs");
  }
  catch(chaiscript::exception::file_not_found_error)
  {
    print(std::string("Warning: autoexec.cs does not exist."));
  }
  catch (chaiscript::exception::eval_error e)
  {
    std::cout << "Couldn't evaluate autoexec.cs: " << e.reason << std::endl;
    std::cout << "Details: " << e.detail << std::endl;
  }
  try
  {
    m_scriptEngine.eval_file("./data/user.cs");
  }
  catch(chaiscript::exception::file_not_found_error)
  {
    print(std::string("Notice: user.cs does not exist."));
  }

  fillKeyIndex();

#ifdef KIT_EXPERIMENTAL_CONTROLLERS
  kit::Controller::refreshControllers();
#endif
}

kit::Application::~Application()
{
  if(m_console)
    delete m_console;
  
  if(m_window)
    delete m_window;
}

void kit::Application::fillKeyIndex()
{
  m_keyIndex.clear();
  m_mouseIndex.clear();
  
  m_keyIndex[kit::Space] = "Space";
  m_keyIndex[kit::Apostrophe] = "'";
  m_keyIndex[kit::Comma] = ",";
  m_keyIndex[kit::Minus] = "-";
  m_keyIndex[kit::Period] = ".";
  m_keyIndex[kit::Slash] = "/";
  m_keyIndex[kit::Num0] = "0";
  m_keyIndex[kit::Num1] = "1";
  m_keyIndex[kit::Num2] = "2";
  m_keyIndex[kit::Num3] = "3";
  m_keyIndex[kit::Num4] = "4";
  m_keyIndex[kit::Num5] = "5";
  m_keyIndex[kit::Num6] = "6";
  m_keyIndex[kit::Num7] = "7";
  m_keyIndex[kit::Num8] = "8";
  m_keyIndex[kit::Num9] = "9";
  m_keyIndex[kit::Semicolon] = ":";
  m_keyIndex[kit::Equal] = "=";
  m_keyIndex[kit::A] = "A";
  m_keyIndex[kit::B] = "B";
  m_keyIndex[kit::C] = "C";
  m_keyIndex[kit::D] = "D";
  m_keyIndex[kit::E] = "E";
  m_keyIndex[kit::F] = "F";
  m_keyIndex[kit::G] = "G";
  m_keyIndex[kit::H] = "H";
  m_keyIndex[kit::I] = "I";
  m_keyIndex[kit::J] = "J";
  m_keyIndex[kit::K] = "K";
  m_keyIndex[kit::L] = "L";
  m_keyIndex[kit::M] = "M";
  m_keyIndex[kit::N] = "N";
  m_keyIndex[kit::O] = "O";
  m_keyIndex[kit::P] = "P";
  m_keyIndex[kit::Q] = "Q";
  m_keyIndex[kit::R] = "R";
  m_keyIndex[kit::S] = "S";
  m_keyIndex[kit::T] = "T";
  m_keyIndex[kit::U] = "U";
  m_keyIndex[kit::V] = "V";
  m_keyIndex[kit::W] = "W";
  m_keyIndex[kit::X] = "X";
  m_keyIndex[kit::Y] = "Y";
  m_keyIndex[kit::Z] = "Z";
  m_keyIndex[kit::LeftBracket] = "[";
  m_keyIndex[kit::Backslash] = "\\";
  m_keyIndex[kit::GraveAccent] = "`";
  m_keyIndex[kit::World1] = "World1";
  m_keyIndex[kit::World2] = "World2";
  m_keyIndex[kit::Escape] = "Escape";
  m_keyIndex[kit::Enter] = "Enter";
  m_keyIndex[kit::Tab] = "Tab";
  m_keyIndex[kit::Backspace] = "Backspace";
  m_keyIndex[kit::Insert] = "Insert";
  m_keyIndex[kit::Delete] = "Delete";
  m_keyIndex[kit::Right] = "Right";
  m_keyIndex[kit::Left] = "Left";
  m_keyIndex[kit::Down] = "Down";
  m_keyIndex[kit::Up] = "Up";
  m_keyIndex[kit::Page_up] = "Page up";
  m_keyIndex[kit::Page_down] = "Page down";
  m_keyIndex[kit::Home] = "Home";
  m_keyIndex[kit::End] = "End";
  m_keyIndex[kit::CapsLock] = "Capslock";
  m_keyIndex[kit::ScrollLock] = "Scrollock";
  m_keyIndex[kit::NumLock] = "Numlock";
  m_keyIndex[kit::PrintScreen] = "Printscreen";
  m_keyIndex[kit::pause] = "Pause";
  m_keyIndex[kit::F1] = "F1";
  m_keyIndex[kit::F2] = "F2";
  m_keyIndex[kit::F3] = "F3";
  m_keyIndex[kit::F4] = "F4";
  m_keyIndex[kit::F5] = "F5";
  m_keyIndex[kit::F6] = "F6";
  m_keyIndex[kit::F7] = "F7";
  m_keyIndex[kit::F8] = "F8";
  m_keyIndex[kit::F9] = "F9";
  m_keyIndex[kit::F10] = "F10";
  m_keyIndex[kit::F11] = "F11";
  m_keyIndex[kit::F12] = "F12";
  m_keyIndex[kit::F13] = "F13";
  m_keyIndex[kit::F14] = "F14";
  m_keyIndex[kit::F15] = "F15";
  m_keyIndex[kit::F16] = "F16";
  m_keyIndex[kit::F17] = "F17";
  m_keyIndex[kit::F18] = "F18";
  m_keyIndex[kit::F19] = "F19";
  m_keyIndex[kit::F20] = "F20";
  m_keyIndex[kit::F21] = "F21";
  m_keyIndex[kit::F22] = "F22";
  m_keyIndex[kit::F23] = "F23";
  m_keyIndex[kit::F24] = "F24";
  m_keyIndex[kit::F25] = "F25";
  m_keyIndex[kit::Kp_0] = "Numpad 0";
  m_keyIndex[kit::Kp_1] = "Numpad 1";
  m_keyIndex[kit::Kp_2] = "Numpad 2";
  m_keyIndex[kit::Kp_3] = "Numpad 3";
  m_keyIndex[kit::Kp_4] = "Numpad 4";
  m_keyIndex[kit::Kp_5] = "Numpad 5";
  m_keyIndex[kit::Kp_6] = "Numpad 6";
  m_keyIndex[kit::Kp_7] = "Numpad 7";
  m_keyIndex[kit::Kp_8] = "Numpad 8";
  m_keyIndex[kit::Kp_9] = "Numpad 9";
  m_keyIndex[kit::KpDecimal] = "Numpad .";
  m_keyIndex[kit::KpDivide] = "Numpad /";
  m_keyIndex[kit::KpMultiply] = "Numpad *";
  m_keyIndex[kit::KpSubtract] = "Numpad -";
  m_keyIndex[kit::KpAdd] = "Numpad +";
  m_keyIndex[kit::KpEnter] = "Numpad enter";
  m_keyIndex[kit::KpEqual] = "Numpad =";
  m_keyIndex[kit::LeftShift] = "L. shift";
  m_keyIndex[kit::LeftControl] = "L. control";
  m_keyIndex[kit::LeftAlt] = "L. alt";
  m_keyIndex[kit::LeftSuper] = "L. super";
  m_keyIndex[kit::RightShift] = "R. shift";
  m_keyIndex[kit::RightControl] = "R. control";
  m_keyIndex[kit::RightAlt] = "R. alt";
  m_keyIndex[kit::RightSuper] = "R. super";
  m_keyIndex[kit::Menu] = "Menu";
  m_mouseIndex[kit::Mouse1] = "L. mouse";
  m_mouseIndex[kit::Mouse2] = "R. mouse";
  m_mouseIndex[kit::Mouse3] = "M. mouse";
  m_mouseIndex[kit::Mouse4] = "Mouse 4";
  m_mouseIndex[kit::Mouse5] = "Mouse 5";
  m_mouseIndex[kit::Mouse6] = "Mouse 6";
  m_mouseIndex[kit::Mouse7] = "Mouse 7";
  m_mouseIndex[kit::Mouse8] = "Mouse 8";
  
}

void kit::Application::onInitialize()
{

}


void kit::Application::initialize()
{
  onInitialize();
}

void kit::Application::render()
{
  onRender();
  
  if(m_states.size() > 0)
  {
    m_states.top()->render();
  }
  
  m_console->render();
  
  m_window->display();
}

void kit::Application::onRender()
{

}

void kit::Application::update(const double & mstime)
{
  if (m_needResize)
  {
    onResize(m_resizeSize);
    if (m_states.size() > 0)
    {
      m_states.top()->onResize(m_resizeSize);
    }
    m_needResize = false;
  }

  m_console->update(mstime);

  if(m_states.size() > 0)
  {
    m_states.top()->update(mstime);
  }
}

void kit::Application::onResize(glm::uvec2)
{

}

void kit::Application::handleEvent(const double & mstime, const kit::WindowEvent & evt)
{
  // close the window on closing events (pressing x, alt+f4 etc)
  if(evt.type == kit::WindowEvent::Closing)
  {
    m_window->close();
  }
  
  if (evt.type == kit::WindowEvent::FramebufferResized)
  {
    if (evt.resized.newSize.x != 0 && evt.resized.newSize.y != 0)
    {
      m_needResize = true;
      m_resizeSize = evt.resized.newSize;
    }
  }
  
  m_console->handleEvent(evt);
  
  if( (evt.type == kit::WindowEvent::KeyPressed || evt.type == kit::WindowEvent::KeyRepeated) && evt.keyboard.key == kit::Key::F11)
  {
    if(m_console->isActive())
    {
      m_console->hide();
      if(m_states.size() > 0)
      {
        m_states.top()->onConsoleInactive();
      }
    }
    else
    {
      if(m_states.size() > 0)
      {
        m_states.top()->onConsoleActive();
      }
      m_console->Show();
    }
  }

  if(!((evt.type == kit::WindowEvent::KeyPressed || evt.type == kit::WindowEvent::KeyReleased || evt.type == kit::WindowEvent::KeyRepeated) && evt.keyboard.key == kit::F11))
  {
    // Handle events for the state machine
    if(m_states.size() > 0)
    {
      m_states.top()->handleEvent(mstime, evt);
    }
  }
}

void kit::Application::run(ApplicationState * state)
{
  initialize();
  
  m_msSinceUpdate.restart();
  m_msSinceRender.restart();
  
  pushState(state);
  
  while(m_window->isOpen())
  {
    double currRenderRate = m_renderRate; 
    double currUpdateRate = m_updateRate;
    
    double curr_msSinceUpdate = double(m_msSinceUpdate.timeSinceStart().asMicroseconds()) / 1000.0;
    if(curr_msSinceUpdate >= currUpdateRate)
    {
      m_msSinceUpdate.restart();
      
      kit::WindowEvent evt;
      while(m_window->fetchEvent(evt))
      {
        handleEvent(curr_msSinceUpdate, evt);
      }
      
      update(curr_msSinceUpdate);
      m_lastUpdateTime = double(m_msSinceUpdate.timeSinceStart().asMicroseconds()) / 1000.0;
    }

    double curr_msSinceRender = double(m_msSinceRender.timeSinceStart().asMicroseconds()) / 1000.0;
    if(curr_msSinceRender >= currRenderRate)
    {
      m_msSinceRender.restart();
      render();
      m_lastRenderTime = double(m_msSinceRender.timeSinceStart().asMicroseconds()) / 1000.0;
    }
  }
  
  while(m_states.size() > 0)
  {
    popState();
  }
}

void kit::Application::pushState(kit::ApplicationState * state)
{
  if(m_states.size() > 0)
  {
    m_states.top()->onInactive();
  }
  
  state->registerApplication(this);
  m_states.push(state);
  m_states.top()->allocate();
  m_states.top()->onActive();
}

void kit::Application::popState()
{
  m_states.top()->onInactive();
  m_states.top()->release();
  m_states.top()->registerApplication(nullptr);
  m_states.pop();
  
  if(m_states.size() > 0)
  {
    m_states.top()->onActive();
  }
}

kit::Window* kit::Application::getWindow()
{
  return m_window;
}

kit::Console* kit::Application::getConsole()
{
  return m_console;
}

void kit::Application::quit()
{
  m_window->close();
}

void kit::Application::evaluate(const std::string&code)
{
  try
  {
    m_scriptEngine.eval(code);
  }
  catch(chaiscript::exception::eval_error & e)
  {
    print(std::string("Failed to evaluate string: ") + e.pretty_print());
  }
}

void kit::Application::print(const std::string&s)
{
  std::vector<std::string> ss = splitString(s, std::vector<char>{'\n'});
  for(auto  & cs : ss)
  {
    m_console->addLine(kit::stringToWide(cs));
  }
  std::cout << s << std::endl;
}

void kit::Application::setRenderRate(double ms)
{
  m_renderRate = ms;
}

void kit::Application::setUnfocusedRenderRate(double ms)
{
  m_unfocusedRenderRate = ms;
}

void kit::Application::setUpdateRate(double ms)
{
  m_updateRate = ms;
}

void kit::Application::setUnfocusedUpdateRate(double ms)
{
  m_unfocusedUpdateRate = ms;
}

void kit::Application::setVSync(bool enabled)
{
  getWindow()->setVSync(enabled);
}
