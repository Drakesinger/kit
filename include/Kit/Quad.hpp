#ifndef KIT_QUAD_HPP
#define KIT_QUAD_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include "Kit/Texture.hpp"

#include <memory>
#include <map>

namespace kit
{

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class KITAPI Quad 
  {
    public:
      
      typedef std::shared_ptr<Quad> Ptr;
      
      ~Quad();

      void render(kit::ProgramPtr customprogram = nullptr);
      static void renderGeometry();
      static void renderAltGeometry();

      static kit::Quad::Ptr create( glm::vec2 position = glm::vec2(0.0f, 0.0f),
                                    glm::vec2 size     = glm::vec2(1.0f, 1.0f),
                                    glm::vec4 color    = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                    kit::Texture::WPtr   = kit::Texture::WPtr() );

      glm::vec2 const & getPosition();
      glm::vec2 const & getSize();
      glm::vec4 const & getColor();
      kit::Texture::WPtr & getTexture();
      float const & getDepth();
      
      void setPosition(glm::vec2 position);
      void setSize(glm::vec2 size);
      void setDepth(float d);
      void setColor(glm::vec4 color);
      void setTexture(kit::Texture::WPtr texture);

      void setBlending(bool b);
      
      Quad(glm::vec2 position,
        glm::vec2 size,
        glm::vec4 color,
        kit::Texture::WPtr);
    private:

      
      void PrepareProgram(kit::ProgramPtr customprogram = nullptr);
      
      
      // Shared GPU data
      static void allocateShared();
      static void releaseShared();
      static unsigned int m_instanceCount;
      static uint32_t m_glVertexArray;
      static uint32_t m_glVertexIndices;
      static uint32_t m_glVertexBuffer;
      
      static kit::ProgramPtr m_program;
      static kit::ProgramPtr m_ProgramTextured;
      
      // Individual CPU cache
      glm::vec2 m_position;
      float      m_depth;
      glm::vec2 m_size;
      glm::vec4 m_color;
      kit::Texture::WPtr m_texture;
      bool m_blending;
  };

}

#endif