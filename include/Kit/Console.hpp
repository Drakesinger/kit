#ifndef KIT_CONSOLE_H
#define KIT_CONSOLE_H

#include "Kit/WindowEvent.hpp"

#include <list>
#include <memory>

namespace kit {

  /*
   *   Each line is 16 px high, with a fontsize of 14px
   *   There is a total of 20 lines (one of which is the buffer)
   *   Console is a total of 320px high
   */

  class Application;
  
  class Text;
  typedef std::shared_ptr<Text> TextPtr;

  class Font;
  typedef std::shared_ptr<Font> FontPtr;
  
  class Quad;
  typedef std::shared_ptr<Quad> QuadPtr;

  struct KITAPI ConsoleLine 
  {
    std::wstring string;
    kit::TextPtr text;
  };
  
  class KITAPI Console
  {
    public:
      typedef std::shared_ptr<Console> Ptr;
      typedef std::weak_ptr<Console> WPtr;
      
      Console(kit::Application* app);
      ~Console();
      
      static Ptr create(kit::Application* app);
      
      void hide();
      void Show();
      
      void handleEvent(const kit::WindowEvent & evt);
      void update(double const & ms);
      void render();
      
      void addLine(std::wstring s);
      
      bool isActive();
      
    private:
      void updateBufferText();
      
      std::list<std::wstring>::iterator getCurrentBuffer();
      
      kit::Application* m_application;   //< reference to the application, for evaluation and layout
      
      bool m_isActive;                      //< Set to true/false to show/hide the console
      float m_heightCoeff;                  //< Height coefficient for animating console. 1.0 = visible, 0.0 = hidden.
      
      kit::FontPtr m_font;                //< Font for text
      kit::QuadPtr m_quad;                //< Background quad
      
      std::list<ConsoleLine> m_lines;       //< Console output
      uint32_t m_lineOffset;             //< Line offset for browsing the console output
      
      std::list<std::wstring> m_buffer;     //< Command buffer, begin of is the current buffer, the rest is history (no pun intended)
      uint32_t m_bufferPosition;         //< Current position in buffer history, if 0 you are at current
      std::wstring m_bufferSave;            //< Save for the current buffer, when browsing history and returns to 0 again
      kit::TextPtr m_bufferText;          //< display text object for input buffer
      uint32_t m_cursorPosition;         //< Cursor position for input buffer
      
  };
}

#endif // KIT_CONSOLE_H
