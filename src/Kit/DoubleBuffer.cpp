#include "Kit/DoubleBuffer.hpp"
#include "Kit/Texture.hpp"

kit::DoubleBuffer::DoubleBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments, kit::PixelBuffer::AttachmentInfo depthattachment)
{
  this->m_resolution = resolution;
  this->m_frontBuffer = kit::PixelBuffer::create(resolution, colorattachments, depthattachment);
  this->m_backBuffer = kit::PixelBuffer::create(resolution, colorattachments, depthattachment);
}

kit::DoubleBuffer::DoubleBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments)
{
  this->m_resolution = resolution;
  this->m_frontBuffer = kit::PixelBuffer::create(resolution, colorattachments);
  this->m_backBuffer = kit::PixelBuffer::create(resolution, colorattachments);
}

kit::DoubleBuffer::DoubleBuffer(kit::Texture::Ptr front, kit::Texture::Ptr back)
{
  this->m_resolution = glm::uvec2(front->getResolution().x, front->getResolution().y);
  this->m_frontBuffer = kit::PixelBuffer::create(this->m_resolution, {PixelBuffer::AttachmentInfo(front)});
  this->m_backBuffer = kit::PixelBuffer::create(this->m_resolution, {PixelBuffer::AttachmentInfo(back)});
}

void kit::DoubleBuffer::flip()
{
  this->m_frontBuffer.swap(this->m_backBuffer);
}

void kit::DoubleBuffer::clear(std::vector<glm::vec4> colors, float depth)
{
  this->m_backBuffer->clear(colors, depth);
  this->m_backBuffer->bind();
}

void kit::DoubleBuffer::clear(std::vector<glm::vec4> colors)
{
  this->m_backBuffer->clear(colors);
  this->m_backBuffer->bind();
}

kit::DoubleBuffer::~DoubleBuffer()
{

}

kit::DoubleBuffer::Ptr kit::DoubleBuffer::create(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments, kit::PixelBuffer::AttachmentInfo depthattachment)
{
  return std::make_shared<kit::DoubleBuffer>(resolution, colorattachments, depthattachment);
}

kit::DoubleBuffer::Ptr kit::DoubleBuffer::create(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments)
{
  return std::make_shared<kit::DoubleBuffer>(resolution, colorattachments);
}

kit::DoubleBuffer::Ptr kit::DoubleBuffer::create(kit::Texture::Ptr front, kit::Texture::Ptr back)
{
  return std::make_shared<kit::DoubleBuffer>(front, back);
}

kit::PixelBuffer::Ptr kit::DoubleBuffer::getFrontBuffer(){
  return this->m_frontBuffer;
}

kit::PixelBuffer::Ptr kit::DoubleBuffer::getBackBuffer(){
  return this->m_backBuffer;
}

void kit::DoubleBuffer::blitFrom(kit::DoubleBuffer::Ptr source)
{
  
  KIT_GL(glBlitNamedFramebuffer(
    source->getFrontBuffer()->getHandle(),
    this->getBackBuffer()->getHandle(),
    0, 0, source->getResolution().x, source->getResolution().y,
    0, 0, this->getResolution().x, this->getResolution().y,
    GL_COLOR_BUFFER_BIT, GL_LINEAR
  ));
}

glm::uvec2 kit::DoubleBuffer::getResolution()
{
  return this->m_resolution;
}

/*
 void glBlitNamedFramebuffer( GLuint readFramebuffer,
  GLuint drawFramebuffer,
  GLint srcX0,
  GLint srcY0,
  GLint srcX1,
  GLint srcY1,
  GLint dstX0,
  GLint dstY0,
  GLint dstX1,
  GLint dstY1,
  GLbitfield mask,
  GLenum filter);
 */
