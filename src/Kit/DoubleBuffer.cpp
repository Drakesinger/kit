#include "Kit/DoubleBuffer.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Texture.hpp"
#include "Kit/PixelBuffer.hpp"

kit::DoubleBuffer::DoubleBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments, kit::PixelBuffer::AttachmentInfo depthattachment)
{
  m_resolution = resolution;
  m_frontBuffer = new kit::PixelBuffer(resolution, colorattachments, depthattachment);
  m_backBuffer = new kit::PixelBuffer(resolution, colorattachments, depthattachment);
}

kit::DoubleBuffer::DoubleBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments)
{
  m_resolution = resolution;
  m_frontBuffer = new kit::PixelBuffer(resolution, colorattachments);
  m_backBuffer = new kit::PixelBuffer(resolution, colorattachments);
}

kit::DoubleBuffer::DoubleBuffer(kit::Texture * front, kit::Texture * back)
{
  m_resolution = glm::uvec2(front->getResolution().x, front->getResolution().y);
  m_frontBuffer = new kit::PixelBuffer(m_resolution, {PixelBuffer::AttachmentInfo(front)});
  m_backBuffer = new kit::PixelBuffer(m_resolution, {PixelBuffer::AttachmentInfo(back)});
}

void kit::DoubleBuffer::flip()
{
  kit::PixelBuffer * f = m_frontBuffer;;
  m_frontBuffer = m_backBuffer;
  m_backBuffer = f;
}

void kit::DoubleBuffer::clear(std::vector<glm::vec4> colors, float depth)
{
  m_backBuffer->clear(colors, depth);
  m_backBuffer->bind();
}

void kit::DoubleBuffer::clear(std::vector<glm::vec4> colors)
{
  m_backBuffer->clear(colors);
  m_backBuffer->bind();
}

kit::DoubleBuffer::~DoubleBuffer()
{
  delete m_frontBuffer;
  delete m_backBuffer;
}


kit::PixelBuffer * kit::DoubleBuffer::getFrontBuffer(){
  return m_frontBuffer;
}

kit::PixelBuffer * kit::DoubleBuffer::getBackBuffer(){
  return m_backBuffer;
}

void kit::DoubleBuffer::blitFrom(kit::DoubleBuffer * source)
{
#ifndef KIT_SHITTY_INTEL
  glBlitNamedFramebuffer(
    source->getFrontBuffer()->getHandle(),
    getBackBuffer()->getHandle(),
    0, 0, source->getResolution().x, source->getResolution().y,
    0, 0, getResolution().x, getResolution().y,
    GL_COLOR_BUFFER_BIT, GL_LINEAR
  );
#else 
  source->getFrontBuffer()->bind(kit::PixelBuffer::Read);
  getFrontBuffer()->bind(kit::PixelBuffer::Draw);
  glBlitFramebuffer(
    0, 0, source->getResolution().x, source->getResolution().y,
    0, 0, getResolution().x, getResolution().y,
    GL_COLOR_BUFFER_BIT, GL_LINEAR
  );
#endif
}

glm::uvec2 kit::DoubleBuffer::getResolution()
{
  return m_resolution;
}

/*
 void glBlitNamedFramebuffer( uint32_t readFramebuffer,
  uint32_t drawFramebuffer,
  int32_t srcX0,
  int32_t srcY0,
  int32_t srcX1,
  int32_t srcY1,
  int32_t dstX0,
  int32_t dstY0,
  int32_t dstX1,
  int32_t dstY1,
  GLbitfield mask,
  GLenum filter);
 */
