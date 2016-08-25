#include "Kit/Application.hpp"
#include "Kit/Console.hpp"
#include "Kit/Window.hpp"
#include "Kit/ApplicationState.hpp"

#include <algorithm>

kit::Application::Application() : kit::Scriptable()
{
  // initialize variables with sane defaults
  this->m_window = nullptr;
  this->m_lastRenderTime = 0.0;
  this->m_lastUpdateTime = 0.0;
  
  // Configurable runtime variables
  this->m_renderRate = 1000.0 / 120.0;
  this->m_updateRate = 1000.0 / 120.0;
  this->m_unfocusedRenderRate = 1000.0 / 30.0;
  this->m_unfocusedUpdateRate = 1000.0 / 30.0;
  

}

kit::Application::~Application()
{

}

void kit::Application::fillKeyIndex()
{
  this->m_keyIndex.clear();
  this->m_mouseIndex.clear();
  
  this->m_keyIndex[kit::Space] = "Space";
  this->m_keyIndex[kit::Apostrophe] = "'";
  this->m_keyIndex[kit::Comma] = ",";
  this->m_keyIndex[kit::Minus] = "-";
  this->m_keyIndex[kit::Period] = ".";
  this->m_keyIndex[kit::Slash] = "/";
  this->m_keyIndex[kit::Num0] = "0";
  this->m_keyIndex[kit::Num1] = "1";
  this->m_keyIndex[kit::Num2] = "2";
  this->m_keyIndex[kit::Num3] = "3";
  this->m_keyIndex[kit::Num4] = "4";
  this->m_keyIndex[kit::Num5] = "5";
  this->m_keyIndex[kit::Num6] = "6";
  this->m_keyIndex[kit::Num7] = "7";
  this->m_keyIndex[kit::Num8] = "8";
  this->m_keyIndex[kit::Num9] = "9";
  this->m_keyIndex[kit::Semicolon] = ":";
  this->m_keyIndex[kit::Equal] = "=";
  this->m_keyIndex[kit::A] = "A";
  this->m_keyIndex[kit::B] = "B";
  this->m_keyIndex[kit::C] = "C";
  this->m_keyIndex[kit::D] = "D";
  this->m_keyIndex[kit::E] = "E";
  this->m_keyIndex[kit::F] = "F";
  this->m_keyIndex[kit::G] = "G";
  this->m_keyIndex[kit::H] = "H";
  this->m_keyIndex[kit::I] = "I";
  this->m_keyIndex[kit::J] = "J";
  this->m_keyIndex[kit::K] = "K";
  this->m_keyIndex[kit::L] = "L";
  this->m_keyIndex[kit::M] = "M";
  this->m_keyIndex[kit::N] = "N";
  this->m_keyIndex[kit::O] = "O";
  this->m_keyIndex[kit::P] = "P";
  this->m_keyIndex[kit::Q] = "Q";
  this->m_keyIndex[kit::R] = "R";
  this->m_keyIndex[kit::S] = "S";
  this->m_keyIndex[kit::T] = "T";
  this->m_keyIndex[kit::U] = "U";
  this->m_keyIndex[kit::V] = "V";
  this->m_keyIndex[kit::W] = "W";
  this->m_keyIndex[kit::X] = "X";
  this->m_keyIndex[kit::Y] = "Y";
  this->m_keyIndex[kit::Z] = "Z";
  this->m_keyIndex[kit::LeftBracket] = "[";
  this->m_keyIndex[kit::Backslash] = "\\";
  this->m_keyIndex[kit::GraveAccent] = "`";
  this->m_keyIndex[kit::World1] = "World1";
  this->m_keyIndex[kit::World2] = "World2";
  this->m_keyIndex[kit::Escape] = "Escape";
  this->m_keyIndex[kit::Enter] = "Enter";
  this->m_keyIndex[kit::Tab] = "Tab";
  this->m_keyIndex[kit::Backspace] = "Backspace";
  this->m_keyIndex[kit::Insert] = "Insert";
  this->m_keyIndex[kit::Delete] = "Delete";
  this->m_keyIndex[kit::Right] = "Right";
  this->m_keyIndex[kit::Left] = "Left";
  this->m_keyIndex[kit::Down] = "Down";
  this->m_keyIndex[kit::Up] = "Up";
  this->m_keyIndex[kit::Page_up] = "Page up";
  this->m_keyIndex[kit::Page_down] = "Page down";
  this->m_keyIndex[kit::Home] = "Home";
  this->m_keyIndex[kit::End] = "End";
  this->m_keyIndex[kit::CapsLock] = "Capslock";
  this->m_keyIndex[kit::ScrollLock] = "Scrollock";
  this->m_keyIndex[kit::NumLock] = "Numlock";
  this->m_keyIndex[kit::PrintScreen] = "Printscreen";
  this->m_keyIndex[kit::pause] = "Pause";
  this->m_keyIndex[kit::F1] = "F1";
  this->m_keyIndex[kit::F2] = "F2";
  this->m_keyIndex[kit::F3] = "F3";
  this->m_keyIndex[kit::F4] = "F4";
  this->m_keyIndex[kit::F5] = "F5";
  this->m_keyIndex[kit::F6] = "F6";
  this->m_keyIndex[kit::F7] = "F7";
  this->m_keyIndex[kit::F8] = "F8";
  this->m_keyIndex[kit::F9] = "F9";
  this->m_keyIndex[kit::F10] = "F10";
  this->m_keyIndex[kit::F11] = "F11";
  this->m_keyIndex[kit::F12] = "F12";
  this->m_keyIndex[kit::F13] = "F13";
  this->m_keyIndex[kit::F14] = "F14";
  this->m_keyIndex[kit::F15] = "F15";
  this->m_keyIndex[kit::F16] = "F16";
  this->m_keyIndex[kit::F17] = "F17";
  this->m_keyIndex[kit::F18] = "F18";
  this->m_keyIndex[kit::F19] = "F19";
  this->m_keyIndex[kit::F20] = "F20";
  this->m_keyIndex[kit::F21] = "F21";
  this->m_keyIndex[kit::F22] = "F22";
  this->m_keyIndex[kit::F23] = "F23";
  this->m_keyIndex[kit::F24] = "F24";
  this->m_keyIndex[kit::F25] = "F25";
  this->m_keyIndex[kit::Kp_0] = "Numpad 0";
  this->m_keyIndex[kit::Kp_1] = "Numpad 1";
  this->m_keyIndex[kit::Kp_2] = "Numpad 2";
  this->m_keyIndex[kit::Kp_3] = "Numpad 3";
  this->m_keyIndex[kit::Kp_4] = "Numpad 4";
  this->m_keyIndex[kit::Kp_5] = "Numpad 5";
  this->m_keyIndex[kit::Kp_6] = "Numpad 6";
  this->m_keyIndex[kit::Kp_7] = "Numpad 7";
  this->m_keyIndex[kit::Kp_8] = "Numpad 8";
  this->m_keyIndex[kit::Kp_9] = "Numpad 9";
  this->m_keyIndex[kit::KpDecimal] = "Numpad .";
  this->m_keyIndex[kit::KpDivide] = "Numpad /";
  this->m_keyIndex[kit::KpMultiply] = "Numpad *";
  this->m_keyIndex[kit::KpSubtract] = "Numpad -";
  this->m_keyIndex[kit::KpAdd] = "Numpad +";
  this->m_keyIndex[kit::KpEnter] = "Numpad enter";
  this->m_keyIndex[kit::KpEqual] = "Numpad =";
  this->m_keyIndex[kit::LeftShift] = "L. shift";
  this->m_keyIndex[kit::LeftControl] = "L. control";
  this->m_keyIndex[kit::LeftAlt] = "L. alt";
  this->m_keyIndex[kit::LeftSuper] = "L. super";
  this->m_keyIndex[kit::RightShift] = "R. shift";
  this->m_keyIndex[kit::RightControl] = "R. control";
  this->m_keyIndex[kit::RightAlt] = "R. alt";
  this->m_keyIndex[kit::RightSuper] = "R. super";
  this->m_keyIndex[kit::Menu] = "Menu";
  this->m_mouseIndex[kit::Mouse1] = "L. mouse";
  this->m_mouseIndex[kit::Mouse2] = "R. mouse";
  this->m_mouseIndex[kit::Mouse3] = "M. mouse";
  this->m_mouseIndex[kit::Mouse4] = "Mouse 4";
  this->m_mouseIndex[kit::Mouse5] = "Mouse 5";
  this->m_mouseIndex[kit::Mouse6] = "Mouse 6";
  this->m_mouseIndex[kit::Mouse7] = "Mouse 7";
  this->m_mouseIndex[kit::Mouse8] = "Mouse 8";
  
}

void kit::Application::onInitialize()
{

}


void kit::Application::initialize()
{
  kit::Window::Args args;
  args.mode = kit::Window::Windowed;
  args.resolution = glm::uvec2(1366, 768);
  args.resizable = true;
  args.title = "Kit Engine";
  this->m_window = kit::Window::create( args);
  
  this->m_console = kit::Console::create(this);
  
  this->m_scriptEngine.add_global(chaiscript::var(this), "self");
  
  try
  {
    this->m_scriptEngine.eval_file("./data/autoexec.cs");
  }
  catch(chaiscript::exception::file_not_found_error)
  {
    this->print(std::string("Warning: autoexec.cs does not exist."));
  }
  catch (chaiscript::exception::eval_error e)
  {
    std::cout << "Couldn't evaluate autoexec.cs: " << e.reason << std::endl;
    std::cout << "Details: " << e.detail << std::endl;
  }
  try
  {
    this->m_scriptEngine.eval_file("./data/user.cs");
  }
  catch(chaiscript::exception::file_not_found_error)
  {
    this->print(std::string("Notice: user.cs does not exist."));
  }

  this->fillKeyIndex();

#ifdef KIT_EXPERIMENTAL_CONTROLLERS
  kit::Controller::refreshControllers();
#endif

  this->onInitialize();
}

void kit::Application::render()
{
  this->m_window->clear(glm::vec4(0.0, 0.0, 1.0, 1.0));
  
  this->onRender();
  
  if(this->m_states.size() > 0)
  {
    this->m_states.top()->render();
  }
  
  this->m_console->render();
  
  this->m_window->display();
}

void kit::Application::onRender()
{

}

void kit::Application::update(const double & mstime)
{
  if (this->m_needResize)
  {
    this->onResize(this->m_resizeSize);
    if (this->m_states.size() > 0)
    {
      this->m_states.top()->onResize(this->m_resizeSize);
    }
    this->m_needResize = false;
  }

  this->m_console->update(mstime);

  if(this->m_states.size() > 0)
  {
    this->m_states.top()->update(mstime);
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
    this->m_window->close();
  }
  
  if (evt.type == kit::WindowEvent::FramebufferResized)
  {
    if (evt.resized.newSize.x != 0 && evt.resized.newSize.y != 0)
    {
      this->m_needResize = true;
      this->m_resizeSize = evt.resized.newSize;
    }
  }
  
  this->m_console->handleEvent(evt);
  
  if( (evt.type == kit::WindowEvent::KeyPressed || evt.type == kit::WindowEvent::KeyRepeated) && evt.keyboard.key == kit::Key::F11)
  {
    if(this->m_console->isActive())
    {
      this->m_console->hide();
      if(this->m_states.size() > 0)
      {
        this->m_states.top()->onConsoleInactive();
      }
    }
    else
    {
      if(this->m_states.size() > 0)
      {
        this->m_states.top()->onConsoleActive();
      }
      this->m_console->Show();
    }
  }

  if(!((evt.type == kit::WindowEvent::KeyPressed || evt.type == kit::WindowEvent::KeyReleased || evt.type == kit::WindowEvent::KeyRepeated) && evt.keyboard.key == kit::F11))
  {
    // Handle events for the state machine
    if(this->m_states.size() > 0)
    {
      this->m_states.top()->handleEvent(mstime, evt);
    }
  }
}

void kit::Application::run(ApplicationState::Ptr state)
{
  this->initialize();
  
  this->m_msSinceUpdate.restart();
  this->m_msSinceRender.restart();
  
  this->pushState(state);
  
  while(this->m_window->isOpen())
  {
    double currRenderRate = this->m_renderRate; 
    double currUpdateRate = this->m_updateRate;
    
    double curr_msSinceUpdate = double(this->m_msSinceUpdate.timeSinceStart().asMicroseconds()) / 1000.0;
    if(curr_msSinceUpdate >= currUpdateRate)
    {
      this->m_msSinceUpdate.restart();
      
      kit::WindowEvent evt;
      while(this->m_window->fetchEvent(evt))
      {
        this->handleEvent(curr_msSinceUpdate, evt);
      }
      
      this->update(curr_msSinceUpdate);
      this->m_lastUpdateTime = double(this->m_msSinceUpdate.timeSinceStart().asMicroseconds()) / 1000.0;
    }

    double curr_msSinceRender = double(this->m_msSinceRender.timeSinceStart().asMicroseconds()) / 1000.0;
    if(curr_msSinceRender >= currRenderRate)
    {
      this->m_msSinceRender.restart();
      this->render();
      this->m_lastRenderTime = double(this->m_msSinceRender.timeSinceStart().asMicroseconds()) / 1000.0;
    }
  }
}

void kit::Application::pushState(kit::ApplicationState::Ptr state)
{
  if(this->m_states.size() > 0)
  {
    this->m_states.top()->onInactive();
  }
  state->registerApplication(this);
  this->m_states.push(state);
  this->m_states.top()->allocate();
  this->m_states.top()->onActive();
}

void kit::Application::popState()
{
  this->m_states.top()->onInactive();
  this->m_states.top()->release();
  this->m_states.pop();
  
  if(this->m_states.size() > 0)
  {
    this->m_states.top()->onActive();
  }
}

kit::Window::Ptr kit::Application::getWindow()
{
  return this->m_window;
}

kit::Console::Ptr kit::Application::getConsole()
{
  return this->m_console;
}

void kit::Application::quit()
{
  this->m_window->close();
}

void kit::Application::evaluate(const std::string&code)
{
  try
  {
    this->m_scriptEngine.eval(code);
  }
  catch(chaiscript::exception::eval_error & e)
  {
    this->print(std::string("Failed to evaluate string: ") + e.pretty_print());
  }
}

void kit::Application::print(const std::string&s)
{
  std::vector<std::string> ss = splitString(s, std::vector<char>{'\n'});
  for(auto  & cs : ss)
  {
    this->m_console->addLine(kit::stringToWide(cs));
  }
  std::cout << s << std::endl;
}

void kit::Application::setRenderRate(double ms)
{
  this->m_renderRate = ms;
}

void kit::Application::setUnfocusedRenderRate(double ms)
{
  this->m_unfocusedRenderRate = ms;
}

void kit::Application::setUpdateRate(double ms)
{
  this->m_updateRate = ms;
}

void kit::Application::setUnfocusedUpdateRate(double ms)
{
  this->m_unfocusedUpdateRate = ms;
}

void kit::Application::setVSync(bool enabled)
{
  this->getWindow()->setVSync(enabled);
}