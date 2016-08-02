#ifndef KIT_TEXT_HPP
#define KIT_TEXT_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"

#include <memory>

namespace kit 
{

  class Font;
  typedef std::shared_ptr<Font> FontPtr;

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;
 
  class KITAPI Text
  {
    public:
      
      enum HAlignment
      {
        Left,
        Centered,
        Right
      };
      
      enum VAlignment
      {
        Top,
        Middle,
        Bottom
      };

      typedef std::shared_ptr<kit::Text> Ptr;
      typedef std::shared_ptr<kit::Text> WPtr;
      
      static kit::Text::Ptr create(kit::FontPtr font, float fontsize, std::wstring text, glm::vec2 position = glm::vec2(0.0, 0.0));
      
      void renderShadowed(glm::ivec2 resolution, glm::vec2 shadowOffset, glm::vec4 shadowColor);
      void render(glm::ivec2 resolution);
      
      void setText(std::wstring text);
      std::wstring const & getText();
      
      void setFont(kit::FontPtr font);
      kit::FontPtr getFont();
      
      void setFontSize(float fontsize);
      float const & getFontSize();
      
      float getLineAdvance();
      float getHeight();
      
      void setPosition(glm::vec2 position);
      glm::vec2 const & getPosition();
      
      void setColor(glm::vec4 color);
      glm::vec4 const & getColor();
      
      void setAlignment(HAlignment h, VAlignment v);
      
      Text();
      ~Text();
      
    private:
      
      static void allocateShared();
      static void releaseShared();
      
      void updateBuffers();
      kit::GL m_glSingleton;
      GLuint m_glVertexArray;
      GLuint m_glVertexIndices; 
      GLuint m_glVertexBuffer;
      
      uint32_t m_indexCount;
      
      static kit::ProgramPtr m_renderProgram;
      static uint32_t m_instanceCount;
      
      kit::FontPtr m_font;
      float m_fontSize;
      glm::vec4 m_color;
      HAlignment m_hAlignment;
      VAlignment m_vAlignment;
      
      std::wstring m_text;
      
      glm::vec2 m_position;
      float m_width;
  };
  
}

#endif
