#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include "Kit/PixelBuffer.hpp"

namespace kit {
  class Texture;
  
  class KITAPI DoubleBuffer {
    public:
      
      DoubleBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments, kit::PixelBuffer::AttachmentInfo depthattachment);
      DoubleBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments);
      DoubleBuffer(kit::Texture * front,  kit::Texture * back);
      ~DoubleBuffer();

      void flip();
      
      void clear(std::vector<glm::vec4> colorattachments, float depthattachment);
      void clear(std::vector<glm::vec4> colorattachments);
      
      kit::PixelBuffer * getFrontBuffer();
      kit::PixelBuffer * getBackBuffer();

      glm::uvec2 getResolution();
      
      void blitFrom(kit::DoubleBuffer * source); ///< Blits from the source front buffer to the destination(this) backbuffer
      
    private:
      kit::PixelBuffer * m_frontBuffer = nullptr;
      kit::PixelBuffer * m_backBuffer = nullptr;
      glm::uvec2          m_resolution;

  };
}
