#ifndef KIT_QUAD_HPP
#define KIT_QUAD_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include "Kit/Texture.hpp"

#include <map>

namespace kit
{

  class Program;
  

  class KITAPI Quad 
  {
    public:
      
      Quad( glm::vec2 position = glm::vec2(0.0f, 0.0f),
                                    glm::vec2 size     = glm::vec2(1.0f, 1.0f),
                                    glm::vec4 color    = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                    kit::Texture * texture = nullptr );
      
      ~Quad();

      void render(kit::Program * customprogram = nullptr);
      static void renderGeometry();
      static void renderAltGeometry();



      glm::vec2 const & getPosition();
      glm::vec2 const & getSize();
      glm::vec4 const & getColor();
      kit::Texture * getTexture();
      float const & getDepth();
      
      void setPosition(glm::vec2 position);
      void setSize(glm::vec2 size);
      void setDepth(float d);
      void setColor(glm::vec4 color);
      void setTexture(kit::Texture * texture);
      void setTextureSubRect(glm::vec2 position, glm::vec2 size);
      
      void setBlending(bool b);
    private:

      
      void prepareProgram(kit::Program * customprogram = nullptr);
      
      
      // Shared GPU data
      static void allocateShared();
      static void releaseShared();
      static unsigned int m_instanceCount;
      static uint32_t m_glVertexArray;
      static uint32_t m_glVertexIndices;
      static uint32_t m_glVertexBuffer;
      
      static kit::Program * m_program;
      static kit::Program * m_programTextured;
      
      // Individual CPU cache
      glm::vec2 m_position;
      float      m_depth;
      glm::vec2 m_size;
      glm::vec2 m_texSubOffset;
      glm::vec2 m_texSubSize;
      glm::vec4 m_color;
      kit::Texture * m_texture;
      bool m_blending;
  };

}

#endif
