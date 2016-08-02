#ifndef KIT_RENDERTEXTURE_HPP
#define KIT_RENDERTEXTURE_HPP

#include "Kit/Export.hpp"
#include "Kit/GL.hpp"
#include "Kit/Types.hpp"
#include "Kit/Texture.hpp"

#include <vector>
#include <array>

namespace kit {
  class KITAPI PixelBuffer {
    public:
      typedef std::shared_ptr<PixelBuffer> Ptr;

      enum BindMethod
      {
        Read,
        Draw,
        Both
      };
      
      struct KITAPI AttachmentInfo
      {
        AttachmentInfo(kit::Texture::InternalFormat format, kit::Texture::EdgeSamplingMode edgemode = kit::Texture::ClampToEdge, kit::Texture::FilteringMode minfilter = kit::Texture::Linear, kit::Texture::FilteringMode magfilter = kit::Texture::Linear);
        AttachmentInfo(kit::Texture::Ptr texture);
        
        // If not null, use this texture instead of creating one. Make sure size is right.
        kit::Texture::Ptr texture;
        
        // If texture is null, create the attachment using these parameters
        kit::Texture::InternalFormat    format;
        kit::Texture::EdgeSamplingMode  edgeSamplingMode;
        kit::Texture::FilteringMode     minFilteringMode;
        kit::Texture::FilteringMode     magFilteringMode;
      };
      
      typedef std::vector<AttachmentInfo> AttachmentList;
      
      PixelBuffer();
      ~PixelBuffer();

      static kit::PixelBuffer::Ptr create(glm::uvec2 resolution, AttachmentList colorattachments, AttachmentInfo depthattachment);
      static kit::PixelBuffer::Ptr create(glm::uvec2 resolution, AttachmentList colorattachments);
      static kit::PixelBuffer::Ptr createShadowBuffer(glm::uvec2 resolution);

      void bind(kit::PixelBuffer::BindMethod = kit::PixelBuffer::BindMethod::Both);
      static void unbind(kit::PixelBuffer::BindMethod = kit::PixelBuffer::BindMethod::Both);
      
      void clear(std::vector<glm::vec4> colorattachments, float depthattachment);
      void clear(std::vector<glm::vec4> colorattachments);
      void clearDepth(float d);

      void clearAttachment(uint32_t attachment, glm::vec4 clearcolor);
      void clearAttachment(uint32_t attachment, glm::uvec4 clearcolor);
      void clearAttachment(uint32_t attachment, glm::ivec4 clearcolor);
      
      void              setDrawBuffers(std::vector<uint32_t> drawbuffers);
      GLuint            getHandle();
      glm::uvec2      getResolution();
      uint32_t       getNumColorAttachments();
      kit::Texture::Ptr getColorAttachment(uint32_t index);
      kit::Texture::Ptr getDepthAttachment();
      
      glm::vec4        readPixel(uint32_t index, uint32_t x, uint32_t y);
      std::vector<float> readPixels(uint32_t index);
      
      void blitFrom(kit::PixelBuffer::Ptr source, bool colorMask, std::vector<std::array<bool, 4>> componentMask, bool depthMask, bool stencilMask);

    private:
      kit::GL                        m_glSingleton;
      GLuint                         m_glHandle;
      kit::Texture::Ptr              m_depthAttachment;
      std::vector<kit::Texture::Ptr> m_colorAttachments;
      glm::uvec2                   m_resolution;
  };
}

#endif // KIT_RENDERTEXTURE_HPP
