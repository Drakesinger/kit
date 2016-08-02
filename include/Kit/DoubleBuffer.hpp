#ifndef KIT_DOUBLEBUFFER_HPP
#define KIT_DOUBLEBUFFER_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"
#include "Kit/PixelBuffer.hpp"

namespace kit {
  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;
  
  class KITAPI DoubleBuffer {
    public:
      
      typedef std::shared_ptr<DoubleBuffer> Ptr;

      DoubleBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments, kit::PixelBuffer::AttachmentInfo depthattachment);
      DoubleBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments);
      DoubleBuffer(kit::TexturePtr front,  kit::TexturePtr back);
      ~DoubleBuffer();

      static kit::DoubleBuffer::Ptr create(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments, kit::PixelBuffer::AttachmentInfo depthattachment);
      static kit::DoubleBuffer::Ptr create(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments);
      static kit::DoubleBuffer::Ptr create(kit::TexturePtr front,  kit::TexturePtr back);
      
      void flip();
      
      void clear(std::vector<glm::vec4> colorattachments, float depthattachment);
      void clear(std::vector<glm::vec4> colorattachments);
      
      kit::PixelBuffer::Ptr getFrontBuffer();
      kit::PixelBuffer::Ptr getBackBuffer();

      glm::uvec2 getResolution();
      
      void blitFrom(kit::DoubleBuffer::Ptr source); //< Blits from the source front buffer to the destination(this) backbuffer
      
    private:
      kit::GL               m_glSingleton;
      kit::PixelBuffer::Ptr m_frontBuffer;
      kit::PixelBuffer::Ptr m_backBuffer;
      glm::uvec2          m_resolution;

  };
}

#endif 