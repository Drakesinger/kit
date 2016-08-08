#include "Kit/ScriptLib.hpp"
#include "Kit/Window.hpp"
#include "Kit/Application.hpp"
#include "Kit/Console.hpp"
#include "Kit/Renderer.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Texture.hpp"

#include <chaiscript/utility/utility.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

chaiscript::ModulePtr kit::getScriptLibrary()
{
  std::cout << "Retrieving script library" << std::endl;
  static chaiscript::ModulePtr moduleKit = std::make_shared<chaiscript::Module>();
  static bool wasInitialized = false;
  
  if(!wasInitialized)
  {
    std::cout << "Initializing script library" << std::endl;
    /*
    // --- Monitor --- //
    chaiscript::utility::add_class<kit::VideoMode>
    (
      *moduleKit,
      "VideoMode",
      {
        chaiscript::constructor<kit::VideoMode()>(),
        chaiscript::constructor<kit::VideoMode(const kit::VideoMode&)>()
      },
      {
        {chaiscript::fun(&kit::VideoMode::m_width), "m_width"},     
        {chaiscript::fun(&kit::VideoMode::m_Height), "m_Height"},
        {chaiscript::fun(&kit::VideoMode::m_RedBits), "m_RedBits"},
        {chaiscript::fun(&kit::VideoMode::m_GreenBits), "m_GreenBits"},
        {chaiscript::fun(&kit::VideoMode::m_BlueBits), "m_BlueBits"},
        {chaiscript::fun(&kit::VideoMode::m_RefreshRate), "m_RefreshRate"},
        {chaiscript::fun(&kit::VideoMode::Resolution), "Resolution"}
      }
    );
    
    chaiscript::utility::add_class<kit::Monitor>
    (
      *moduleKit,
      "Monitor",
      {
        chaiscript::constructor<kit::Monitor(const kit::Monitor&)>()
      },
      {
        {chaiscript::fun(&kit::Monitor::isConnected), "isConnected"},     
        {chaiscript::fun(&kit::Monitor::getPhysicalSize), "getPhysicalSize"},
        {chaiscript::fun(&kit::Monitor::getPosition), "getPosition"},
        {chaiscript::fun(&kit::Monitor::getName), "getName"},
        {chaiscript::fun(&kit::Monitor::getVideoModes), "getVideoModes"},
        {chaiscript::fun(&kit::Monitor::getVideoMode), "getVideoMode"},
        {chaiscript::fun(&kit::Monitor::getConnectedMonitors), "getConnectedMonitors"},
        {chaiscript::fun(&kit::Monitor::getPrimaryMonitor), "getPrimaryMonitor"}
      }
    );
    */
    // --- Kit types --- //
    chaiscript::utility::add_class<glm::uvec2>
    (
      *moduleKit,
      "uvec2",
      {
        chaiscript::constructor<glm::uvec2(uint32_t, uint32_t)>(),
        chaiscript::constructor<glm::uvec2()>(),
        chaiscript::constructor<glm::uvec2(const glm::uvec2&)>()
      },
      {
        {chaiscript::fun(&glm::uvec2::x), "x"},
        {chaiscript::fun(&glm::uvec2::y), "y"}
      }
    );
    
    chaiscript::utility::add_class<glm::ivec2>
    (
      *moduleKit,
      "ivec2",
      {
        chaiscript::constructor<glm::ivec2(int32_t, int32_t)>(),
        chaiscript::constructor<glm::ivec2()>(),
        chaiscript::constructor<glm::ivec2(const glm::ivec2&)>()
      },
      {
        {chaiscript::fun(&glm::ivec2::x), "x"},
        {chaiscript::fun(&glm::ivec2::y), "y"}
      }
    );
    
    chaiscript::utility::add_class<glm::vec2>
    (
      *moduleKit,
      "vec2",
      {
        chaiscript::constructor<glm::vec2(float, float)>(),
        chaiscript::constructor<glm::vec2()>(),
        chaiscript::constructor<glm::vec2(const glm::vec2 &)>()
      },
      {
        {chaiscript::fun(&glm::vec2::x), "x"},
        {chaiscript::fun(&glm::vec2::y), "y"}
      }
    );
    
    chaiscript::utility::add_class<glm::vec3>
    (
      *moduleKit,
      "vec3",
      {
        chaiscript::constructor<glm::vec3(float, float, float)>(),
        chaiscript::constructor<glm::vec3()>(),
        chaiscript::constructor<glm::vec3(const glm::vec3 &)>()
      },
      {
        {chaiscript::fun(&glm::vec3::x), "x"},
        {chaiscript::fun(&glm::vec3::y), "y"},
        {chaiscript::fun(&glm::vec3::z), "z"}
      }
    );
    
    chaiscript::utility::add_class<glm::vec4>
    (
      *moduleKit,
      "vec4",
      {
        chaiscript::constructor<glm::vec4(float, float, float, float)>(),
        chaiscript::constructor<glm::vec4()>(),
        chaiscript::constructor<glm::vec4(const glm::vec4 &)>()
      },
      {
        {chaiscript::fun(&glm::vec4::x), "x"},
        {chaiscript::fun(&glm::vec4::y), "y"},
        {chaiscript::fun(&glm::vec4::z), "z"},
        {chaiscript::fun(&glm::vec4::w), "w"}
      }
    );
    
    // --- kit::Window --- //
    
    // Mode
    moduleKit->add(chaiscript::user_type<kit::Window::Mode>(), "WindowMode");
    moduleKit->add(chaiscript::fun(&__script_enum_opequals<kit::Window::Mode>), "==");
    moduleKit->add_global_const(chaiscript::const_var(kit::Window::Mode::Borderless), "WindowModeBorderless");
    moduleKit->add_global_const(chaiscript::const_var(kit::Window::Mode::Windowed), "WindowModeWindowed");
    moduleKit->add_global_const(chaiscript::const_var(kit::Window::Mode::Fullscreen), "WindowModeFullscreen");

    /*
    // Args
    chaiscript::utility::add_class<kit::Window::Args>
    (
      *moduleKit,
      "WindowArgs",
      {
        chaiscript::constructor<kit::Window::Args()>(),
        chaiscript::constructor<kit::Window::Args(std::string, kit::Window::Mode, glm::uvec2, kit::Monitor::Ptr)>(),
        chaiscript::constructor<kit::Window::Args(const kit::Window::Args &)>()
      },
      {
        {chaiscript::fun(&kit::Window::Args::title), "title"},
        {chaiscript::fun(&kit::Window::Args::fullscreenMonitor), "fullscreenMonitor"},
        {chaiscript::fun(&kit::Window::Args::mode), "mode"},
        {chaiscript::fun(&kit::Window::Args::resolution), "resolution"},
        {chaiscript::fun(&kit::Window::Args::sharedWindow), "sharedWindow"},
        {chaiscript::fun(&kit::Window::Args::resizable), "resizable"},
      }
    );*/
    
    // Events & Input
    moduleKit->add(chaiscript::user_type<kit::Key>(), "Key");
    moduleKit->add(chaiscript::user_type<kit::MouseButton>(), "MouseButton");
    moduleKit->add(chaiscript::user_type<kit::WindowEvent::Type>(), "WindowEventType");
    moduleKit->add(chaiscript::fun(&__script_enum_opequals<kit::WindowEvent::Type>), "==");
    moduleKit->add(chaiscript::fun(&__script_enum_opequals<kit::Key>), "==");
    
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::Closing), "WindowEventTypeClosing");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::FocusChanged), "WindowEventTypeFocusChanged");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::FramebufferResized), "WindowEventTypeFramebufferResized");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::KeyPressed), "WindowEventTypeKeyPressed");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::KeyReleased), "WindowEventTypeKeyReleasd");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::Minimized), "WindowEventTypeMinimized");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::MouseButtonPressed), "WindowEventTypeMouseButtonPressed");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::MouseButtonReleased), "WindowEventTypeMouseButtonReleased");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::MouseEntered), "WindowEventTypeMouseEntered");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::MouseLeft), "WindowEventTypeMouseLeft");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::MouseMoved), "WindowEventTypeMouseMoved");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::MouseScrolled), "WindowEventTypeMouseScrolled");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::Moved), "WindowEventTypeMoved");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::Resized), "WindowEventTypeResized");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::Restored), "WindowEventTypeRestored");
    moduleKit->add_global_const(chaiscript::const_var(kit::WindowEvent::Type::TextEntered), "WindowEventTypeTextEntered");
    
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Unknown), "KeyUnknown");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Space), "KeySpace");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Apostrophe), "KeyApostrophe");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Comma), "KeyComma");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Minus), "KeyMinus");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Period), "KeyPeriod");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Slash), "KeySlash");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num0), "KeyNum0");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num1), "KeyNum1");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num2), "KeyNum2");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num3), "KeyNum3");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num4), "KeyNum4");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num5), "KeyNum5");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num6), "KeyNum6");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num7), "KeyNum7");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num8), "KeyNum8");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Num9), "KeyNum9");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Semicolon), "KeySemicolon");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Equal), "KeyEqual");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::A), "KeyA");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::B), "KeyB");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::C), "KeyC");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::D), "KeyD");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::E), "KeyE");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F), "KeyF");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::G), "KeyG");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::H), "KeyH");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::I), "KeyI");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::J), "KeyJ");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::K), "KeyK");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::L), "KeyL");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::M), "KeyM");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::N), "KeyN");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::O), "KeyO");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::P), "KeyP");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Q), "KeyQ");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::R), "KeyR");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::S), "KeyS");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::T), "KeyT");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::U), "KeyU");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::V), "KeyV");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::W), "KeyW");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::X), "KeyX");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Y), "KeyY");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Z), "KeyZ");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::LeftBracket), "KeyLeftBracket");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Backslash), "KeyBackslash");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::RightBracket), "KeyRightBracket");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::GraveAccent), "KeyGraveAccent");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::World1), "KeyWorld1");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::World2), "KeyWorld2");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Escape), "KeyEscape");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Enter ), "KeyEnter ");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Tab ), "KeyTab ");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Backspace), "KeyBackspace");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Insert), "KeyInsert");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Delete), "KeyDelete");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Right), "KeyRight");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Left), "KeyLeft");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Down), "KeyDown");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Up), "KeyUp");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Page_up), "KeyPage_up");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Page_down), "KeyPage_down");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Home), "KeyHome");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::End), "KeyEnd");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::CapsLock), "KeyCapsLock");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::ScrollLock), "KeyScrollLock");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::NumLock), "KeyNumLock");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::PrintScreen), "KeyPrintScreen");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::pause), "KeyPause");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F1), "KeyF1");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F2), "KeyF2");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F3), "KeyF3");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F4), "KeyF4");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F5), "KeyF5");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F6), "KeyF6");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F7), "KeyF7");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F8), "KeyF8");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F9), "KeyF9");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F10), "KeyF10");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F11), "KeyF11");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F12), "KeyF12");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F13), "KeyF13");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F14), "KeyF14");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F15), "KeyF15");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F16), "KeyF16");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F17), "KeyF17");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F18), "KeyF18");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F19), "KeyF19");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F20), "KeyF20");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F21), "KeyF21");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F22), "KeyF22");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F23), "KeyF23");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F24), "KeyF24");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::F25), "KeyF25");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_0), "KeyKp_0");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_1), "KeyKp_1");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_2), "KeyKp_2");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_3), "KeyKp_3");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_4), "KeyKp_4");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_5), "KeyKp_5");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_6), "KeyKp_6");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_7), "KeyKp_7");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_8), "KeyKp_8");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Kp_9), "KeyKp_9");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::KpDecimal), "KeyKpDecimal");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::KpDivide), "KeyKpDivide");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::KpMultiply), "KeyKpMultiply");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::KpSubtract), "KeyKpSubtract");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::KpAdd), "KeyKpAdd");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::KpEnter), "KeyKpEnter");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::KpEqual), "KeyKpEqual");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::LeftShift), "KeyLeftShift");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::LeftControl), "KeyLeftControl");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::LeftAlt), "KeyLeftAlt");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::LeftSuper), "KeyLeftSuper");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::RightShift), "KeyRightShift");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::RightControl), "KeyRightControl");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::RightAlt), "KeyRightAlt");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::RightSuper), "KeyRightSuper");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Menu), "KeyMenu");
    moduleKit->add_global_const(chaiscript::const_var(kit::Key::Last), "KeyLast");
    
    chaiscript::utility::add_class<kit::WindowEvent::s_Resized>
    (
      *moduleKit,
      "WindowEventResized",
      {
        chaiscript::constructor<kit::WindowEvent::s_Resized()>(),
        chaiscript::constructor<kit::WindowEvent::s_Resized(const kit::WindowEvent::s_Resized &)>()
      },
      {
        {chaiscript::fun(&kit::WindowEvent::s_Resized::newSize), "newSize"}
      }
    );
    
    chaiscript::utility::add_class<kit::WindowEvent::s_Moved>
    (
      *moduleKit,
      "WindowEventMoved",
      {
        chaiscript::constructor<kit::WindowEvent::s_Moved()>(),
        chaiscript::constructor<kit::WindowEvent::s_Moved(const kit::WindowEvent::s_Moved &)>()
      },
      {
        {chaiscript::fun(&kit::WindowEvent::s_Moved::newPosition), "newPosition"}
      }
    );
    
    chaiscript::utility::add_class<kit::WindowEvent::s_Mouse>
    (
      *moduleKit,
      "WindowEventMouse",
      {
        chaiscript::constructor<kit::WindowEvent::s_Mouse()>(),
        chaiscript::constructor<kit::WindowEvent::s_Mouse(const kit::WindowEvent::s_Mouse &)>()
      },
      {
        {chaiscript::fun(&kit::WindowEvent::s_Mouse::scrollOffset), "scrollOffset"},
        {chaiscript::fun(&kit::WindowEvent::s_Mouse::newPosition), "newPosition"},
        {chaiscript::fun(&kit::WindowEvent::s_Mouse::button), "button"},
        {chaiscript::fun(&kit::WindowEvent::s_Mouse::modifiers), "modifiers"}
      }
    );
    
    chaiscript::utility::add_class<kit::WindowEvent::s_Keyboard>
    (
      *moduleKit,
      "WindowEventKeyboard",
      {
        chaiscript::constructor<kit::WindowEvent::s_Keyboard()>(),
        chaiscript::constructor<kit::WindowEvent::s_Keyboard(const kit::WindowEvent::s_Keyboard &)>()
      },
      {
        {chaiscript::fun(&kit::WindowEvent::s_Keyboard::key), "key"},
        {chaiscript::fun(&kit::WindowEvent::s_Keyboard::modifiers), "modifiers"},
        {chaiscript::fun(&kit::WindowEvent::s_Keyboard::scancode), "scancode"},
        {chaiscript::fun(&kit::WindowEvent::s_Keyboard::unicode), "unicode"}
      }
    );
    
    chaiscript::utility::add_class<kit::WindowEvent>
    (
      *moduleKit,
      "WindowEvent",
      {
        chaiscript::constructor<kit::WindowEvent()>(),
        chaiscript::constructor<kit::WindowEvent(const kit::WindowEvent &)>()
      },
      {
        {chaiscript::fun(&kit::WindowEvent::type), "type"},
        {chaiscript::fun(&kit::WindowEvent::hasFocus), "hasFocus"},
        {chaiscript::fun(&kit::WindowEvent::resized), "isResized"},
        {chaiscript::fun(&kit::WindowEvent::moved), "moved"},
        {chaiscript::fun(&kit::WindowEvent::mouse), "mouse"},
        {chaiscript::fun(&kit::WindowEvent::keyboard), "keyboard"}
      }
    );
    
    /*
    // The actual window
    chaiscript::utility::add_class<kit::Window>
    (
      *moduleKit,
      "Window",
      {
        chaiscript::constructor<kit::Window(kit::Window::Args)>(),
        chaiscript::constructor<kit::Window(const kit::Window &)>()
      },
      {
        {chaiscript::fun(&kit::Window::activateContext), "activateContext"},
        {chaiscript::fun(&kit::Window::display), "display"},
        {chaiscript::fun(&kit::Window::clear), "clear"},
        {chaiscript::fun(&kit::Window::bind), "bind"},
        {chaiscript::fun(&kit::Window::fetchEvent), "fetchEvent" },
        {chaiscript::fun(&kit::Window::close), "close"},
        {chaiscript::fun(&kit::Window::isOpen), "isOpen"},
        {chaiscript::fun(&kit::Window::setTitle), "setTitle"},
        {chaiscript::fun(&kit::Window::getPosition), "getPosition"},
        {chaiscript::fun(&kit::Window::setPosition), "setPosition"},
        {chaiscript::fun(&kit::Window::getSize), "getSize"},
        {chaiscript::fun(&kit::Window::setSize), "setSize"},
        {chaiscript::fun(&kit::Window::getFramebufferSize), "getFramebufferSize"},
        {chaiscript::fun(&kit::Window::minimize), "minimize"},
        {chaiscript::fun(&kit::Window::restore), "restore"},
        {chaiscript::fun(&kit::Window::show), "show"},
        {chaiscript::fun(&kit::Window::hide), "hide"},
        {chaiscript::fun(&kit::Window::getFullscreenMonitor), "getFullscreenMonitor"},
        {chaiscript::fun(&kit::Window::isFocused), "isFocused"},
        {chaiscript::fun(&kit::Window::isMinimized), "isMinimized"},
        {chaiscript::fun(&kit::Window::isKeyDown), "isKeyDown"},
        {chaiscript::fun(&kit::Window::isMouseButtonDown), "isMouseButtonDown"},
        {chaiscript::fun(&kit::Window::getMousePosition), "getMousePosition"},
        {chaiscript::fun(&kit::Window::setMousePosition), "setMousePosition"},
        {chaiscript::fun(&kit::Window::mouseCursorVisible), "mouseCursorVisible"},
        {chaiscript::fun(&kit::Window::setMouseVirtual), "setMouseVirtual"}
      }
    );
    
    moduleKit->add(chaiscript::fun([](kit::Window::Args a){return kit::Window::create(a); }), "createWindow");
    moduleKit->add(chaiscript::fun([](const std::string&title, const kit::Window::Mode mode, glm::uvec2 resolution){return kit::Window::create(title, mode, resolution); }), "createWindow");
    */
    
    // --- Console --- //
    chaiscript::utility::add_class<kit::Console>
    (
      *moduleKit,
      "Console",
      {
      },
      {
        {chaiscript::fun(&kit::Console::hide), "hide"},
        {chaiscript::fun(&kit::Console::Show), "Show"}
      }
    );
    
    // --- Application --- //
    
    chaiscript::utility::add_class<kit::Application>
    (
      *moduleKit,
      "Application",
      {
        chaiscript::constructor<kit::Application()>(),
      },
      {

        {chaiscript::fun(&kit::Application::print), "print"},
        {chaiscript::fun(&kit::Application::evaluate), "evaluate"},
        {chaiscript::fun(&kit::Application::getWindow), "getWindow"},
        {chaiscript::fun(&kit::Application::getConsole), "getConsole"},
        {chaiscript::fun(&kit::Application::popState), "popState"},
        {chaiscript::fun(&kit::Application::quit), "quit"},
        // {chaiscript::fun<void(kit::Application&, bool)>([=](kit::Application& a, bool b){ a.getWindow()->setVSync(b); }), "setVSync"} // Lambdas broken in chaiscript 5.8, see issue #280
        {chaiscript::fun(&kit::Application::setVSync), "setVSync"}
      }
    );
    
    
    moduleKit->add_global_const(chaiscript::const_var(kit::Renderer::BloomQuality::High), "High");
    moduleKit->add_global_const(chaiscript::const_var(kit::Renderer::BloomQuality::Low), "Low");
    // --- Renderer --- //
    chaiscript::utility::add_class<kit::Renderer>
    (
      *moduleKit,
      "Renderer",
      {
        chaiscript::constructor<kit::Renderer(glm::uvec2)>(),
      },
      {
        {chaiscript::fun(&kit::Renderer::setActiveCamera), "setActiveCamera"},
        {chaiscript::fun(&kit::Renderer::setExposure), "setExposure"},
        {chaiscript::fun(&kit::Renderer::setWhitepoint), "setWhitepoint"},
        {chaiscript::fun(&kit::Renderer::setFXAA), "setFXAA"},
        {chaiscript::fun(&kit::Renderer::setInternalResolution), "setInternalResolution"},
        {chaiscript::fun(&kit::Renderer::setResolution), "setResolution"},
        {chaiscript::fun(&kit::Renderer::setShadows), "setShadows"},
        {chaiscript::fun(&kit::Renderer::setBloom), "setBloom"},
        {chaiscript::fun(&kit::Renderer::setBloomBlurLevels), "setBloomBlurLevels"},
        {chaiscript::fun(&kit::Renderer::setBloomDirtMask), "setBloomDirtMask"},
        {chaiscript::fun(&kit::Renderer::setBloomDirtMaskMultiplier), "setBloomDirtMaskMultiplier"},
        {chaiscript::fun(&kit::Renderer::setSceneFringe), "setSceneFringe"},
        {chaiscript::fun(&kit::Renderer::setSceneFringeExponential), "setSceneFringeExponential"},
        {chaiscript::fun(&kit::Renderer::setSceneFringeScale), "setSceneFringeScale"},
        {chaiscript::fun(&kit::Renderer::setBloomTresholdBias), "setBloomTresholdBias"},
        {chaiscript::fun(&kit::Renderer::setBloomQuality), "setBloomQuality"},
        {chaiscript::fun(&kit::Renderer::setGPUMetrics), "setGPUMetrics" }
     
      }
    );
    wasInitialized = true;
  }
  
  return moduleKit;
}

kit::Scriptable::Scriptable() : m_scriptEngine(chaiscript::Std_Lib::library())
{
  std::cout << "Constructing script library" << std::endl;
  this->m_scriptEngine.add(kit::getScriptLibrary());
  std::cout << "Constructed!" << std::endl;
}

kit::Scriptable::~Scriptable()
{

}

chaiscript::ChaiScript & kit::Scriptable::getScriptEngine()
{
  return this->m_scriptEngine;
}