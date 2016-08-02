#ifndef KIT_WINDOWEVENT_HPP
#define KIT_WINDOWEVENT_HPP

#include "Kit/Export.hpp"
#include "Kit/Input.hpp"
#include "Kit/Types.hpp"

namespace kit
{
  struct KITAPI WindowEvent
  {
    enum KITAPI Type
    {
      FocusChanged,
      FramebufferResized,
      Closing,
      Minimized,
      Restored,
      Moved,
      Invalidated,
      Resized,
      TextEntered,
      KeyPressed,
      KeyReleased,
      KeyRepeated,
      MouseEntered,
      MouseLeft,
      MouseMoved,
      MouseButtonPressed,
      MouseButtonReleased,
      MouseScrolled
    };

    Type             type;
    bool             hasFocus;
    bool             isMinimized;

    struct KITAPI  s_Resized
    {
      glm::uvec2 newSize;
    } resized;

    struct KITAPI s_Moved
    {
      glm::ivec2 newPosition;
    } moved;

    struct KITAPI  s_Mouse
    {
      glm::vec2  scrollOffset;
      glm::vec2  newPosition;
      kit::MouseButton  button;
      int               modifiers;
    } mouse;

    struct KITAPI s_Keyboard
    {
      kit::Key  key;
      int       modifiers;
      int       scancode;
      unsigned int  unicode;
    } keyboard;
  };

}

#endif