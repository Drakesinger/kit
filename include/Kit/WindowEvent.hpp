#ifndef KIT_WINDOWEVENT_HPP
#define KIT_WINDOWEVENT_HPP

#include "Kit/Export.hpp"
#include "Kit/Input.hpp"
#include "Kit/Types.hpp"

namespace kit
{
  ///
  /// \brief A window event
  ///
  struct KITAPI WindowEvent
  {
    ///
    /// \brief An enum representing the type of event
    ///
    enum KITAPI Type
    {
      FocusChanged,             ///< The window focus changed
      FramebufferResized,       ///< The framebuffer was resized
      Closing,                  ///< The window is closing
      Minimized,                ///< The window was minimized
      Restored,                 ///< The window was restored from a minimized state
      Moved,                    ///< The window was moved
      Resized,                  ///< The window was resized
      TextEntered,              ///< Text was entered
      KeyPressed,               ///< A key was pressed
      KeyReleased,              ///< A key was released
      KeyRepeated,              ///< A keypress was repeated
      MouseEntered,             ///< Mouse entered the window
      MouseLeft,                ///< Mouse left the window
      MouseMoved,               ///< Mouse was moved
      MouseButtonPressed,       ///< Mouse button was pressed
      MouseButtonReleased,      ///< Mouse button was released
      MouseScrolled             ///< Mouse wheel was scrolled
    };

    Type             type; ///< The type of event
    bool             hasFocus; ///< For the FocusChanged event type, true if focus was gained, false if it was lost

    struct KITAPI  s_Resized
    {
      glm::uvec2 newSize; ///< The new size for resize events
    } resized; ///< Data structure for the FramebufferResized and Resized events

    struct KITAPI s_Moved
    {
      glm::ivec2 newPosition; ///< The new position for move events
    } moved; ///< Data structure for Moved events

    struct KITAPI  s_Mouse
    {
      glm::vec2  scrollOffset; ///< The scroll offset for MouseScrolled event
      glm::vec2  newPosition; ///< The new position for MouseMoved event
      kit::MouseButton  button; ///< The button for MouseButtonPressed and MousebuttonReleased events
      int               modifiers; ///< Any modifiers that was active while the event was fired, for MouseButtonPressed and MouseButtonReleased events
    } mouse; ///< Data structure for MouseEntered, MouseLeft, MouseMoved, MouseButtonPressed, MouseButtonReleased and MouseScrolled events

    struct KITAPI s_Keyboard
    {
      kit::Key  key; ///< The key that was acted upon, for KeyPressed, KeyReleased and KeyRepeated events
      int       modifiers; ///< Modifiers that were held when the key was acted upon, for KeyPressed, KeyReleased and KeyRepeated events
      int       scancode; ///< System specific scan code, for KeyPressed, KeyReleased and KeyRepeated events
      unsigned int  unicode; ///< Native-endian UTF-32 codepoint, for TextEntered event
    } keyboard; ///< Data structure for TextEntered, KeyPressed, KeyReleased and KeyRepeated events
  };

}

#endif