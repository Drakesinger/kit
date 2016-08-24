#include "Kit/PixelBuffer.hpp"
#include "Kit/IncOpenGL.hpp"

kit::PixelBuffer::AttachmentInfo::AttachmentInfo(kit::Texture::Ptr texture)
{
  this->edgeSamplingMode = Texture::Repeat;
  this->format = Texture::RGBA8;
  this->magFilteringMode = Texture::Linear;
  this->minFilteringMode = Texture::LinearMipmapLinear;
  this->texture = texture;
}

kit::PixelBuffer::AttachmentInfo::AttachmentInfo(kit::Texture::InternalFormat format, kit::Texture::EdgeSamplingMode edgemode, kit::Texture::FilteringMode minfilter, kit::Texture::FilteringMode magfilter)
{
  this->edgeSamplingMode = edgemode;
  this->format = format;
  this->magFilteringMode = magfilter;
  this->minFilteringMode = minfilter;
  this->texture = nullptr;
}

kit::PixelBuffer::PixelBuffer()
{
  
  this->m_glHandle = 0;
  this->m_resolution = glm::uvec2(0, 0);
  this->m_depthAttachment = nullptr;
#ifndef KIT_SHITTY_INTEL
  glCreateFramebuffers(1, &this->m_glHandle);
#else
  glGenFramebuffers(1, &this->m_glHandle); 
#endif 
}

kit::PixelBuffer::~PixelBuffer()
{
  glDeleteFramebuffers(1, &this->m_glHandle);
  this->m_glHandle = 0;
  glGetError();
}

void kit::PixelBuffer::bind(kit::PixelBuffer::BindMethod method)
{
  switch(method)
  {
    case Read:
      glBindFramebuffer(GL_READ_FRAMEBUFFER, this->m_glHandle);
      break;
    case Draw:
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->m_glHandle);
      break;
    case Both:
      glBindFramebuffer(GL_FRAMEBUFFER, this->m_glHandle);
      break;
  }
  glViewport(0, 0, this->m_resolution.x, this->m_resolution.y);
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

kit::PixelBuffer::Ptr kit::PixelBuffer::create(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments)
{
  
  kit::PixelBuffer::Ptr returner = std::make_shared<kit::PixelBuffer>();
  
  returner->m_resolution = resolution;
  
  if(colorattachments.size() == 0)
  {
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferDrawBuffer(returner->m_glHandle, GL_NONE);
#else 
    returner->bind();
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
        
        returner->m_colorAttachments.push_back(info.texture);
#ifndef KIT_SHITTY_INTEL
        glNamedFramebufferTexture(returner->m_glHandle, currEnum, info.texture->getHandle(), 0);
#else 
        returner->bind();
        glFramebufferTexture(GL_FRAMEBUFFER, currEnum, info.texture->getHandle(), 0);
#endif 
      }
      else
      {
        kit::Texture::Ptr adder = kit::Texture::create2D(resolution, info.format, info.edgeSamplingMode, info.minFilteringMode, info.magFilteringMode);
        returner->m_colorAttachments.push_back(adder);
#ifndef KIT_SHITTY_INTEL
        glNamedFramebufferTexture(returner->m_glHandle, currEnum, adder->getHandle(), 0);
#else 
        returner->bind();
        glFramebufferTexture(GL_FRAMEBUFFER, currEnum, adder->getHandle(), 0);
#endif 
      }
    }

    
    // Set drawbuffers
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferDrawBuffers(returner->m_glHandle, (GLsizei)drawBuffers.size(), &drawBuffers[0]);
#else 
    returner->bind();
    glDrawBuffers((GLsizei)drawBuffers.size(), &drawBuffers[0]);
#endif 
  }
  
  return returner;
}

kit::PixelBuffer::Ptr kit::PixelBuffer::create(glm::uvec2 resolution, kit::PixelBuffer::AttachmentList colorattachments, kit::PixelBuffer::AttachmentInfo depthattachment)
{
  
  kit::PixelBuffer::Ptr returner = std::make_shared<kit::PixelBuffer>();
  returner->m_resolution = resolution;
  
  if(colorattachments.size() == 0)
  {
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferDrawBuffer(returner->m_glHandle, GL_NONE);
#else 
    returner->bind();
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
        
        returner->m_colorAttachments.push_back(info.texture);
#ifndef KIT_SHITTY_INTEL
        glNamedFramebufferTexture(returner->m_glHandle, currEnum, info.texture->getHandle(), 0);
#else 
        returner->bind();
        glFramebufferTexture(GL_FRAMEBUFFER, currEnum, info.texture->getHandle(), 0);
#endif 
      }
      else
      {
        kit::Texture::Ptr adder = kit::Texture::create2D(resolution, info.format, info.edgeSamplingMode, info.minFilteringMode, info.magFilteringMode);
        returner->m_colorAttachments.push_back(adder);
#ifndef KIT_SHITTY_INTEL
        glNamedFramebufferTexture(returner->m_glHandle, currEnum, adder->getHandle(), 0);
#else 
        returner->bind();
        glFramebufferTexture(GL_FRAMEBUFFER, currEnum, adder->getHandle(), 0);
#endif 
      }
    }
    
    // Set drawbuffers
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferDrawBuffers(returner->m_glHandle, (GLsizei)drawBuffers.size(), &drawBuffers[0]);
#else 
    returner->bind();
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
    
    returner->m_depthAttachment = depthattachment.texture;
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferTexture(returner->m_glHandle, GL_DEPTH_ATTACHMENT, returner->m_depthAttachment->getHandle(), 0);
#else 
    returner->bind();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, returner->m_depthAttachment->getHandle(), 0);
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
    
    returner->m_depthAttachment = kit::Texture::create2D(resolution, depthattachment.format, depthattachment.edgeSamplingMode, depthattachment.minFilteringMode, depthattachment.magFilteringMode);
#ifndef KIT_SHITTY_INTEL
    glNamedFramebufferTexture(returner->m_glHandle, GL_DEPTH_ATTACHMENT, returner->m_depthAttachment->getHandle(), 0);
#else 
    returner->bind();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, returner->m_depthAttachment->getHandle(), 0);
#endif
  }
  
  return returner;
}

kit::PixelBuffer::Ptr kit::PixelBuffer::createShadowBuffer(glm::uvec2 resolution)
{
  return kit::PixelBuffer::create(resolution, AttachmentList(), AttachmentInfo(kit::Texture::createShadowmap(resolution)));
}

void kit::PixelBuffer::setDrawBuffers(std::vector< uint32_t > drawBuffers)
{
#ifndef KIT_SHITTY_INTEL
  glNamedFramebufferDrawBuffers(this->m_glHandle, (GLsizei)drawBuffers.size(), &drawBuffers[0]);
#else 
  this->bind();
  glDrawBuffers((GLsizei)drawBuffers.size(), &drawBuffers[0]);
#endif 
}


void kit::PixelBuffer::clear(std::vector<glm::vec4> colours, float depth)
{
  glDepthMask(GL_TRUE);
  this->bind();
  
  if(this->m_depthAttachment == nullptr)
  {
    KIT_THROW("Cant clear depth attachment as it does not exist, use the other clear method");
  }
  
  if(colours.size() != this->m_colorAttachments.size())
  {
    KIT_THROW("Wrong number of colors passed, one color per attachment is required.");
  }
  
  for(uint32_t i = 0; i < this->m_colorAttachments.size(); i++)
  {
    float currColor[4] = {colours[i].x, colours[i].y, colours[i].z, colours[i].w};
    glClearBufferfv(GL_COLOR, i, &currColor[0]);
  }

  glClearBufferfv(GL_DEPTH, 0, &depth);
}

void kit::PixelBuffer::clearAttachment(uint32_t attachment, glm::vec4 clearcolor)
{
  this->bind();
  if (attachment >= this->m_colorAttachments.size())
  {
    KIT_THROW("Cant clear attachment, index out of range.");
  }

  float color[4] = { clearcolor.x, clearcolor.y, clearcolor.z, clearcolor.w };
  glClearBufferfv(GL_COLOR, attachment, &color[0]);
}

void kit::PixelBuffer::clearAttachment(uint32_t attachment, glm::uvec4 clearcolor)
{
  this->bind();
  if (attachment >= this->m_colorAttachments.size())
  {
    KIT_THROW("Cant clear attachment, index out of range.");
  }

  uint32_t color[4] = { clearcolor.x, clearcolor.y, clearcolor.z, clearcolor.w };
  glClearBufferuiv(GL_COLOR, attachment, &color[0]);
}

void kit::PixelBuffer::clearAttachment(uint32_t attachment, glm::ivec4 clearcolor)
{
  this->bind();
  if (attachment >= this->m_colorAttachments.size())
  {
    KIT_THROW("Cant clear attachment, index out of range.");
  }

  int32_t color[4] = { clearcolor.x, clearcolor.y, clearcolor.z, clearcolor.w };
  glClearBufferiv(GL_COLOR, attachment, &color[0]);
}

void kit::PixelBuffer::clear(std::vector< glm::vec4 > colours)
{ 
  this->bind();
  if(colours.size() != this->m_colorAttachments.size())
  {
    KIT_THROW("Wrong number of colors passed, one color per attachment is required.");
  }

  for(uint32_t i = 0; i < this->m_colorAttachments.size(); i++)
  {
    float currColor[4] = {colours[i].x, colours[i].y, colours[i].z, colours[i].w};
    glClearBufferfv(GL_COLOR, i, &currColor[i]);
  }
}

void kit::PixelBuffer::clearDepth(float d)
{
  glDepthMask(GL_TRUE);
  this->bind();
  if(this->m_depthAttachment == nullptr)
  {
    KIT_THROW("Cant clear depth attachment as it does not exist, use the other clear method");
  }

  glClearBufferfv(GL_DEPTH, 0, &d);
}

uint32_t kit::PixelBuffer::getNumColorAttachments()
{
  return (uint32_t)this->m_colorAttachments.size();
}

kit::Texture::Ptr kit::PixelBuffer::getColorAttachment(uint32_t index)
{
  if(index >= this->m_colorAttachments.size())
  {
    KIT_THROW("Index out of range");
  }

  return this->m_colorAttachments[index];
}

kit::Texture::Ptr kit::PixelBuffer::getDepthAttachment()
{
  return this->m_depthAttachment;
}

uint32_t kit::PixelBuffer::getHandle()
{
  return this->m_glHandle;
}

glm::uvec2 kit::PixelBuffer::getResolution()
{
  return this->m_resolution;
}

glm::vec4 kit::PixelBuffer::readPixel(uint32_t index, uint32_t x, uint32_t y)
{
  this->bind();
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
  uint32_t numPixels = this->m_resolution.x * this->m_resolution.y;
  float * data = new float[numPixels];
  returner.reserve(numPixels);
  
  this->bind();
  glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
  glReadPixels(0, 0, this->m_resolution.x, this->m_resolution.y, GL_RED, GL_FLOAT, &data[0]);
  kit::PixelBuffer::unbind();
  glFinish();
  glReadBuffer(GL_FRONT);

  for(uint32_t currRow = 0; currRow < this->m_resolution.y; currRow++)
  {
    for(uint32_t currCol = 0; currCol < this->m_resolution.x; currCol++)
    {
      int invRow = this->m_resolution.y - currRow - 1;
      returner.push_back(data[(invRow*this->m_resolution.x)+currCol]);
    }
  }

  delete[] data;

  return returner;
}

void kit::PixelBuffer::blitFrom(kit::PixelBuffer::Ptr source, bool colorMask, std::vector<std::array<bool, 4>> componentMask, bool depthMask, bool stencilMask)
{
  bool clearColorMask = false;
  GLbitfield mask = 0;

  if (colorMask)
  {
    mask |= GL_COLOR_BUFFER_BIT;
    if (this->m_colorAttachments.size() != source->getNumColorAttachments())
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
    if (componentMask.size() != this->m_colorAttachments.size())
    {
      KIT_ERR("componentMask: color attachment count mismatch");
      return;
    }

    for (uint32_t i = 0; i < this->m_colorAttachments.size(); i++)
    {
      glColorMaski(i, componentMask[i][0], componentMask[i][1], componentMask[i][2], componentMask[i][3]);
    }

    clearColorMask = true;
  }

  glBlitNamedFramebuffer(source->getHandle(), this->getHandle(), 0, 0, source->getResolution().x, source->getResolution().y, 0, 0, this->getResolution().x, this->getResolution().y, mask, GL_LINEAR);

  if (clearColorMask)
  {
    for (uint32_t i = 0; i < this->m_colorAttachments.size(); i++)
    {
      glColorMaski(i, true, true, true, true);
    }
  }
}