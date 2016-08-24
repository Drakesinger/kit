#ifndef KIT_RENDERTEXTURE_HPP
#define KIT_RENDERTEXTURE_HPP

#include "Kit/Export.hpp"

#include "Kit/Types.hpp"
#include "Kit/Texture.hpp"

#include <vector>
#include <array>

namespace kit {

  ///
  /// \brief Encapsulates an OpenGL Framebuffer Object.
  ///
  class KITAPI PixelBuffer {
    public:
      typedef std::shared_ptr<PixelBuffer> Ptr;

      ///
      /// \brief Decides what to target when binding the FBO. Corresponds to GL_FRAMEBUFFER, GL_READ_FRAMEBUFFER, and GL_DRAW_FRAMEBUFFER.
      ///
      enum BindMethod
      {
        Read,
        Draw,
        Both
      };

      ///
      /// \brief Information about an attachment, for the construction of attachments.
      ///
      struct KITAPI AttachmentInfo
      {
        ///
        /// \brief Constructor. Constructs a new texture for the attachment, given the different parameters.
        /// \param format The internal format of the new texture
        /// \param edgemode The edge sampling mode of the new texture
        /// \param minfilter The minification filtering mode of the new texture.
        /// \param magfilter The magnification filtering mode of the new texture. Valid paramters are Nearest and Linear.
        ///
        AttachmentInfo(kit::Texture::InternalFormat format, kit::Texture::EdgeSamplingMode edgemode = kit::Texture::ClampToEdge, kit::Texture::FilteringMode minfilter = kit::Texture::Linear, kit::Texture::FilteringMode magfilter = kit::Texture::Linear);

        ///
        /// \brief Constructor. Uses pre-existing texture
        /// \param texture Texture to use for attachment
        ///
        AttachmentInfo(kit::Texture::Ptr texture);

        ///
        /// \brief If not null, the attachment will be created using this texture
        /// 
        kit::Texture::Ptr texture;

        ///
        /// \brief If texture is null, the attachment will be created using these parameters
        /// 
        kit::Texture::InternalFormat    format;
        kit::Texture::EdgeSamplingMode  edgeSamplingMode;
        kit::Texture::FilteringMode     minFilteringMode;
        kit::Texture::FilteringMode     magFilteringMode;
      };

      ///
      /// \brief A vector containing AttachmentInfo instances
      ///
      typedef std::vector<AttachmentInfo> AttachmentList;

      ///
      /// \brief Constructor (FOR INTERNAL USE ONLY)
      ///
      /// You should NEVER instance this as usual. ALWAYS use smart pointers (std::shared_ptr), and create them explicitly using the `create` methods!
      ///
      PixelBuffer();

      ///
      /// \brief Destructor.
      ///
      ~PixelBuffer();

      /// 
      /// \brief Creates a pixelbuffer given a resolution, a list of color attachments and a depth attachment
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param resolution Resolution of the pixelbuffer 
      /// \param colorattachments A list of color attachments to use 
      /// \param depthattachment The depth attachment to use
      ///
      /// \throws kit::Exception If any attachment texture has a resolution that does not equal the passed resolution parameter
      /// \throws kit::Exception If the texture format for the depth attachment is not kit::Texture::DepthComponent16, kit::Texture::DepthComponent24 or kit::Texture::DepthComponent32F
      ///
      /// \returns A shared pointer to the newly created pixelbuffer
      ///
      static kit::PixelBuffer::Ptr create(glm::uvec2 resolution, AttachmentList colorattachments, AttachmentInfo depthattachment);

      /// 
      /// \brief Creates a pixelbuffer given a resolution and a list of color attachments
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param resolution Resolution of the pixelbuffer 
      /// \param colorattachments A list of color attachments to use 
      ///
      /// \throws kit::Exception If any attachment texture has a resolution that does not equal the passed resolution parameter
      ///
      /// \returns A shared pointer to the newly created pixelbuffer
      ///
      static kit::PixelBuffer::Ptr create(glm::uvec2 resolution, AttachmentList colorattachments);

      /// 
      /// \brief Creates a pixelbuffer optimized for shadowmapping
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param resolution Resolution of the pixelbuffer 
      ///
      /// \returns A shared pointer to the newly created pixelbuffer
      ///
      static kit::PixelBuffer::Ptr createShadowBuffer(glm::uvec2 resolution);

      ///
      /// \brief Binds the underlying FBO
      /// \param bindmethod Whether to bind for reading, drawing or for both
      ///
      void bind(kit::PixelBuffer::BindMethod bindmethod = kit::PixelBuffer::BindMethod::Both);

      ///
      /// \brief Unbinds any currently bound FBO
      ///
      static void unbind(kit::PixelBuffer::BindMethod bindmethod = kit::PixelBuffer::BindMethod::Both);

      ///
      /// \brief Clear the attachments
      /// \param colorattachments A list of clear colors for the respective color attachments
      /// \param depthattachment The value to clear the depthbuffer with
      ///
      /// \throws kit::Exception If colorattachments contain the wrong number of colors (1 per attachment required)
      /// \throws kit::Exception If called without a depth attachment (Use the other clear method)
      ///
      void clear(std::vector<glm::vec4> colorattachments, float depthattachment);

      ///
      /// \brief Clear the attachments
      /// \param colorattachments A list of clear colors for the respective color attachments
      ///
      /// \throws kit::Exception If colorattachments contain the wrong number of colors (1 per attachment required)
      ///
      void clear(std::vector<glm::vec4> colorattachments);

      ///
      /// \brief Clear the depthattachment
      /// \param d The value to clear the depthbuffer with
      /// \throws kit::Exception If called without a depth attachment
      ///
      void clearDepth(float d);

      ///
      /// \brief Clear an attachment given an index
      /// \param clearcolor The clearcolor to use
      /// \throws kit::Exception If attachment index is not valid
      ///
      void clearAttachment(uint32_t attachment, glm::vec4 clearcolor);

      ///
      /// \brief Clear an attachment given an index
      /// \param clearcolor The clearcolor to use
      /// \throws kit::Exception If attachment index is not valid
      ///
      void clearAttachment(uint32_t attachment, glm::uvec4 clearcolor);

      ///
      /// \brief Clear an attachment given an index
      /// \param clearcolor The clearcolor to use
      /// \throws kit::Exception If attachment index is not valid
      ///
      void clearAttachment(uint32_t attachment, glm::ivec4 clearcolor);

      ///
      /// \brief Set the drawbuffers for this framebuffer object
      ///
      void setDrawBuffers(std::vector<uint32_t> drawbuffers);

      ///
      /// \returns The internal OpenGL name for this Framebuffer Object
      ///
      uint32_t getHandle();

      ///
      /// \returns The resolution
      ///
      glm::uvec2 getResolution();

      ///
      /// \returns The number of color attachments
      ///
      uint32_t getNumColorAttachments();

      ///
      /// \returns A shared pointer to the texture from a given attachment 
      /// \param index The attachment index
      /// \throws kit::Exception If index is out of range
      ///
      kit::Texture::Ptr getColorAttachment(uint32_t index);

      ///
      /// \returns A shared pointer to the texture from the depth attachment
      ///
      kit::Texture::Ptr getDepthAttachment();

      /// \brief Read a single pixel from a given attachment 
      /// \param index The attachment index
      /// \param x The x coordinate in pixelspace
      /// \param y The y coordinate in pixelspace
      glm::vec4 readPixel(uint32_t index, uint32_t x, uint32_t y);

      /// \brief Read all the pixels from a given attachment
      /// \param index The attachment index
      /// \returns A vector of pixels, encoded top to bottom, left to right, r g b a.
      std::vector<float> readPixels(uint32_t index);

      /// \brief Blits pixels from a given PixelBuffer. 
      /// \param source The source of the pixel data
      /// \param colorMask Whether to blit the colors
      /// \param componentMask What components to blit for each attachment
      /// \param depthMask Whether to blit the depthbuffer
      /// \param stencilMask Whether to blit the stencilmask
      /// \throws kit::Exception If the source pixelbuffer doesn't have the same amount of color attachments
      /// \throws kit::Exception If the componentmask doesn't have the same amount of color attachments
      void blitFrom(kit::PixelBuffer::Ptr source, bool colorMask, std::vector<std::array<bool, 4>> componentMask, bool depthMask, bool stencilMask);

    private:
      uint32_t                         m_glHandle;
      kit::Texture::Ptr              m_depthAttachment;
      std::vector<kit::Texture::Ptr> m_colorAttachments;
      glm::uvec2                   m_resolution;
  };
}

#endif // KIT_RENDERTEXTURE_HPP
