#include "Kit/PixelBuffer.hpp"
#include "Kit/IncOpenGL.hpp"

kit::PixelBuffer::AttachmentInfo::AttachmentInfo(kit::Texture * t, bool o)
{
  own = o;
  format = Texture::RGBA8;
  levels = 0;
  texture = t;
}

kit::PixelBuffer::AttachmentInfo::AttachmentInfo(kit::Texture::InternalFormat f, uint8_t l)
{

  format = f;
  levels = l;
  texture = nullptr;
}

kit::PixelBuffer::PixelBuffer()
{
  
  m_glHandle = 0;
  m_resolution = glm::uvec2(0, 0);
  m_depthAttachment = nullptr;
#ifndef KIT_SHITTY_INTEL
  glCreateFramebuffers(1, &m_glHandle);
#else
  glGenFramebuffers(1, &m_glHandle); 
#endif 
}

kit::PixelBuffer::~PixelBuffer()
{
  glDeleteFramebuffers(1, &m_glHandle);
  m_glHandle = 0;
  glGetError();
  
  if(m_ownDepth && m_depthAttachment)
    delete m_depthAttachment;
  
  for(auto & e : m_colorAttachments)
  {
    if(e.ownTexture && e.texture)
      delete e.texture;
  }
}

void kit::PixelBuffer::bind(kit::PixelBuffer::BindMethod method)
{
  switch(method)
  {
    case Read:
      glBindFramebuffer(GL_READ_FRAMEBUFFER, m_glHandle);
      break;
    case Draw:
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_glHandle);
      break;
    case Both:
      glBindFramebuffer(GL_FRAMEBUFFER, m_glHandle);
      break;
  }
  glViewport(0, 0, m_resolution.x, m_resolution.y);
}

void kit::PixelBuffer::unbind(kit::PixelBuffer::BindMethod method)
{
  
  switch(method)
  {
    case Read:
      glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
      break;
    case Draw:
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      break;
    case Both:
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      break;
  }
}

kit::PixelBuffer::PixelBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments) : kit::PixelBuffer()
{
  m_resolution = resolution;
  
  if(colorattachments.size() == 0)
  {
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferDrawBuffer(m_glHandle, GL_NONE);
#else 
    bind();
    glDrawBuffer(GL_NONE);
#endif 
  }
  else
  {
    // Keep track of attachments and create a drawbuffers
    std::vector<GLenum> drawBuffers(colorattachments.size());
    uint32_t currAttachment = 0;
    
    // Add/create color attachments
    for(auto & info : colorattachments)
    {
      // Fill the drawbuffer
      GLenum currEnum = GL_COLOR_ATTACHMENT0+ currAttachment;
      drawBuffers[currAttachment] = currEnum;
      currAttachment++;

      // Add texture if exists, otherwise create it 
      if(info.texture != nullptr)
      {
        // Assert resolution
        if(info.texture->getResolution().x != resolution.x || info.texture->getResolution().y != resolution.y)
        {
          KIT_THROW("Pixelbuffer attachments must be of the same size");
        }
        
        m_colorAttachments.push_back(AttachmentEntry(info.texture, info.own));
#ifndef KIT_SHITTY_INTEL
        glNamedFramebufferTexture(m_glHandle, currEnum, info.texture->getHandle(), 0);
#else 
        bind();
        glFramebufferTexture(GL_FRAMEBUFFER, currEnum, info.texture->getHandle(), 0);
#endif 
      }
      else
      {
        kit::Texture * adder = new kit::Texture(resolution, info.format, info.levels);
        m_colorAttachments.push_back(AttachmentEntry(adder, true));
#ifndef KIT_SHITTY_INTEL
        glNamedFramebufferTexture(m_glHandle, currEnum, adder->getHandle(), 0);
#else 
        bind();
        glFramebufferTexture(GL_FRAMEBUFFER, currEnum, adder->getHandle(), 0);
#endif 
      }
    }
    
    // Set drawbuffers
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferDrawBuffers(m_glHandle, (GLsizei)drawBuffers.size(), &drawBuffers[0]);
#else 
    bind();
    glDrawBuffers((GLsizei)drawBuffers.size(), &drawBuffers[0]);
#endif 
  }
}

kit::PixelBuffer::PixelBuffer(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments, kit::PixelBuffer::AttachmentInfo depthattachment) : kit::PixelBuffer()
{
  m_resolution = resolution;
  
  if(colorattachments.size() == 0)
  {
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferDrawBuffer(m_glHandle, GL_NONE);
#else 
    bind();
    glDrawBuffer(GL_NONE);
#endif 
  }
  else
  {
    // Keep track of attachments and create a drawbuffers
    std::vector<GLenum> drawBuffers(colorattachments.size());
    uint32_t currAttachment = 0;
    
    // Add/create color attachments
    for(auto & info : colorattachments)
    {
      // Fill the drawbuffer
      GLenum currEnum = GL_COLOR_ATTACHMENT0 + currAttachment;
      drawBuffers[currAttachment] = currEnum;
      currAttachment++;
      
      // Add texture if exists, otherwise create it 
      if(info.texture != nullptr)
      {
        // Assert resolution
        if(info.texture->getResolution().x != resolution.x || info.texture->getResolution().y != resolution.y)
        {
          KIT_THROW("Pixelbuffer attachments must be of the same size");
        }
        
        m_colorAttachments.push_back(AttachmentEntry(info.texture, info.own));
#ifndef KIT_SHITTY_INTEL
        glNamedFramebufferTexture(m_glHandle, currEnum, info.texture->getHandle(), 0);
#else 
        bind();
        glFramebufferTexture(GL_FRAMEBUFFER, currEnum, info.texture->getHandle(), 0);
#endif 
      }
      else
      {
        kit::Texture * adder = new kit::Texture(resolution, info.format, info.levels);
        m_colorAttachments.push_back(AttachmentEntry(adder, true));
#ifndef KIT_SHITTY_INTEL
        glNamedFramebufferTexture(m_glHandle, currEnum, adder->getHandle(), 0);
#else 
        bind();
        glFramebufferTexture(GL_FRAMEBUFFER, currEnum, adder->getHandle(), 0);
#endif 
      }
    }
    
    // Set drawbuffers
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferDrawBuffers(m_glHandle, (GLsizei)drawBuffers.size(), &drawBuffers[0]);
#else 
    bind();
    glDrawBuffers((GLsizei)drawBuffers.size(), &drawBuffers[0]);
#endif 
  }
  
  // Add depth attachment or create it if it doesnt exist
  if(depthattachment.texture != nullptr)
  {
    // Assert format
    kit::Texture::InternalFormat df = depthattachment.texture->getInternalFormat();
    if(df != kit::Texture::DepthComponent16 && df != kit::Texture::DepthComponent24 && df != kit::Texture::DepthComponent32F)
    {
      KIT_THROW("Wrong internal format of depth component");
    }
    
    // Assert resolution
    if(depthattachment.texture->getResolution().x != resolution.x || depthattachment.texture->getResolution().y != resolution.y)
    {
      KIT_THROW("Pixelbuffer attachments must be of the same size");
    }
    
    m_depthAttachment = depthattachment.texture;
    m_ownDepth = depthattachment.own;
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferTexture(m_glHandle, GL_DEPTH_ATTACHMENT, m_depthAttachment->getHandle(), 0);
#else 
    bind();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthAttachment->getHandle(), 0);
#endif 
  }
  else
  {
    // Assert format
    kit::Texture::InternalFormat df = depthattachment.format;
    if(df != kit::Texture::DepthComponent16 && df != kit::Texture::DepthComponent24 && df != kit::Texture::DepthComponent32F)
    {
      KIT_THROW("Wrong internal format of depth component");
    }
    
    m_depthAttachment = new kit::Texture(resolution, depthattachment.format, depthattachment.levels);
    m_ownDepth = true;
    
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferTexture(m_glHandle, GL_DEPTH_ATTACHMENT, m_depthAttachment->getHandle(), 0);
#else 
    bind();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthAttachment->getHandle(), 0);
#endif
  }
}

kit::PixelBuffer * kit::PixelBuffer::createShadowBuffer(glm::uvec2 resolution)
{
  return new kit::PixelBuffer(resolution, AttachmentList(), AttachmentInfo(kit::Texture::createShadowmap(resolution), true));
}

void kit::PixelBuffer::setDrawBuffers(std::vector< uint32_t > drawBuffers)
{
#ifndef KIT_SHITTY_INTEL
  glNamedFramebufferDrawBuffers(m_glHandle, (GLsizei)drawBuffers.size(), &drawBuffers[0]);
#else 
  bind();
  glDrawBuffers((GLsizei)drawBuffers.size(), &drawBuffers[0]);
#endif 
}


void kit::PixelBuffer::clear(std::vector<glm::vec4> colours, float depth)
{
  glDepthMask(GL_TRUE);
  bind();
  
  if(m_depthAttachment == nullptr)
  {
    KIT_THROW("Cant clear depth attachment as it does not exist, use the other clear method");
  }
  
  if(colours.size() != m_colorAttachments.size())
  {
    KIT_THROW("Wrong number of colors passed, one color per attachment is required.");
  }
  
  for(uint32_t i = 0; i < m_colorAttachments.size(); i++)
  {
    float currColor[4] = {colours[i].x, colours[i].y, colours[i].z, colours[i].w};
    glClearBufferfv(GL_COLOR, i, &currColor[0]);
  }

  glClearBufferfv(GL_DEPTH, 0, &depth);
}

void kit::PixelBuffer::clearAttachment(uint32_t attachment, glm::vec4 clearcolor)
{
  bind();
  if (attachment >= m_colorAttachments.size())
  {
    KIT_THROW("Cant clear attachment, index out of range.");
  }

  float color[4] = { clearcolor.x, clearcolor.y, clearcolor.z, clearcolor.w };
  glClearBufferfv(GL_COLOR, attachment, &color[0]);
}

void kit::PixelBuffer::clearAttachment(uint32_t attachment, glm::uvec4 clearcolor)
{
  bind();
  if (attachment >= m_colorAttachments.size())
  {
    KIT_THROW("Cant clear attachment, index out of range.");
  }

  uint32_t color[4] = { clearcolor.x, clearcolor.y, clearcolor.z, clearcolor.w };
  glClearBufferuiv(GL_COLOR, attachment, &color[0]);
}

void kit::PixelBuffer::clearAttachment(uint32_t attachment, glm::ivec4 clearcolor)
{
  bind();
  if (attachment >= m_colorAttachments.size())
  {
    KIT_THROW("Cant clear attachment, index out of range.");
  }

  int32_t color[4] = { clearcolor.x, clearcolor.y, clearcolor.z, clearcolor.w };
  glClearBufferiv(GL_COLOR, attachment, &color[0]);
}

void kit::PixelBuffer::clear(std::vector< glm::vec4 > colours)
{ 
  bind();
  if(colours.size() != m_colorAttachments.size())
  {
    KIT_THROW("Wrong number of colors passed, one color per attachment is required.");
  }

  for(uint32_t i = 0; i < m_colorAttachments.size(); i++)
  {
    float currColor[4] = {colours[i].x, colours[i].y, colours[i].z, colours[i].w};
    glClearBufferfv(GL_COLOR, i, &currColor[i]);
  }
}

void kit::PixelBuffer::clearDepth(float d)
{
  glDepthMask(GL_TRUE);
  bind();
  if(m_depthAttachment == nullptr)
  {
    KIT_THROW("Cant clear depth attachment as it does not exist, use the other clear method");
  }

  glClearBufferfv(GL_DEPTH, 0, &d);
}

uint32_t kit::PixelBuffer::getNumColorAttachments()
{
  return (uint32_t)m_colorAttachments.size();
}

kit::Texture * kit::PixelBuffer::getColorAttachment(uint32_t index)
{
  if(index >= m_colorAttachments.size())
  {
    KIT_THROW("Index out of range");
  }

  return m_colorAttachments[index].texture;
}

kit::Texture * kit::PixelBuffer::getDepthAttachment()
{
  return m_depthAttachment;
}

uint32_t kit::PixelBuffer::getHandle()
{
  return m_glHandle;
}

glm::uvec2 kit::PixelBuffer::getResolution()
{
  return m_resolution;
}

glm::vec4 kit::PixelBuffer::readPixel(uint32_t index, uint32_t x, uint32_t y)
{
  bind();
  glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
  float data[4];
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, &data[0]);
  kit::PixelBuffer::unbind();
  glFinish();
  
  glReadBuffer(GL_FRONT);
  
  return glm::vec4(data[0], data[1], data[2], data[3]);
}

std::vector<float> kit::PixelBuffer::readPixels(uint32_t index)
{
  std::vector<float> returner;
  uint32_t numPixels = m_resolution.x * m_resolution.y;
  float * data = new float[numPixels];
  returner.reserve(numPixels);
  
  bind();
  glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
  glReadPixels(0, 0, m_resolution.x, m_resolution.y, GL_RED, GL_FLOAT, &data[0]);
  kit::PixelBuffer::unbind();
  glFinish();
  glReadBuffer(GL_FRONT);

  for(uint32_t currRow = 0; currRow < m_resolution.y; currRow++)
  {
    for(uint32_t currCol = 0; currCol < m_resolution.x; currCol++)
    {
      int invRow = m_resolution.y - currRow - 1;
      returner.push_back(data[(invRow*m_resolution.x)+currCol]);
    }
  }

  delete[] data;

  return returner;
}

void kit::PixelBuffer::blitFrom(kit::PixelBuffer * source, bool colorMask, std::vector<std::array<bool, 4>> componentMask, bool depthMask, bool stencilMask)
{
  bool clearColorMask = false;
  GLbitfield mask = 0;

  if (colorMask)
  {
    mask |= GL_COLOR_BUFFER_BIT;
    if (m_colorAttachments.size() != source->getNumColorAttachments())
    {
      KIT_THROW("source: color attachment count mismatch");
      return;
    }
  }

  if (depthMask)
  {
    mask |= GL_DEPTH_BUFFER_BIT;
  }

  if (stencilMask)
  {
    mask |= GL_STENCIL_BUFFER_BIT;
  }

  if (componentMask.size() != 0 && colorMask)
  {
    if (componentMask.size() != m_colorAttachments.size())
    {
      KIT_ERR("componentMask: color attachment count mismatch");
      return;
    }

    for (uint32_t i = 0; i < m_colorAttachments.size(); i++)
    {
      glColorMaski(i, componentMask[i][0], componentMask[i][1], componentMask[i][2], componentMask[i][3]);
    }

    clearColorMask = true;
  }

  glBlitNamedFramebuffer(source->getHandle(), getHandle(), 0, 0, source->getResolution().x, source->getResolution().y, 0, 0, getResolution().x, getResolution().y, mask, GL_LINEAR);

  if (clearColorMask)
  {
    for (uint32_t i = 0; i < m_colorAttachments.size(); i++)
    {
      glColorMaski(i, true, true, true, true);
    }
  }
}

kit::PixelBuffer::AttachmentEntry::AttachmentEntry(kit::Texture* t, bool b)
{
  texture = t;
  ownTexture = b;
}

kit::PixelBuffer::AttachmentEntry::~AttachmentEntry()
{
}
