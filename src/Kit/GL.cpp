#include "Kit/GL.hpp"
#include <map>

/*
  The initial value for each capability with the exception of GL_DITHER and GL_MULTISAMPLE is GL_FALSE.
  The initial value for GL_DITHER and GL_MULTISAMPLE is GL_TRUE.
 */
static const std::vector<GLenum> gl_capabilities = {
  GL_BLEND,
  GL_COLOR_LOGIC_OP,
  GL_CULL_FACE,
  GL_DEPTH_CLAMP,
  GL_DEBUG_OUTPUT,
  GL_DEBUG_OUTPUT_SYNCHRONOUS,
  GL_DEPTH_TEST,
  GL_DITHER,
  GL_FRAMEBUFFER_SRGB,
  GL_LINE_SMOOTH,
  GL_MULTISAMPLE,
  GL_POLYGON_SMOOTH,
  GL_POLYGON_OFFSET_FILL,
  GL_POLYGON_OFFSET_LINE,
  GL_POLYGON_OFFSET_POINT,
  GL_PROGRAM_POINT_SIZE,
  GL_PRIMITIVE_RESTART,
  GL_SAMPLE_ALPHA_TO_COVERAGE,
  GL_SAMPLE_ALPHA_TO_ONE,
  GL_SAMPLE_COVERAGE,
  GL_SAMPLE_MASK,
  GL_SCISSOR_TEST,
  GL_STENCIL_TEST,
  GL_TEXTURE_CUBE_MAP_SEAMLESS
};

static const std::vector<GLenum> gl_texturetargets = {
  GL_TEXTURE_1D,
  GL_TEXTURE_2D,
  GL_TEXTURE_3D,
  GL_TEXTURE_1D_ARRAY,
  GL_TEXTURE_2D_ARRAY,
  GL_TEXTURE_RECTANGLE,
  GL_TEXTURE_CUBE_MAP,
  GL_TEXTURE_CUBE_MAP_ARRAY,
  GL_TEXTURE_BUFFER,
  GL_TEXTURE_2D_MULTISAMPLE,
  GL_TEXTURE_2D_MULTISAMPLE_ARRAY
};

static const std::vector<GLenum> gl_buffertargets = {
  GL_ARRAY_BUFFER,
  GL_ATOMIC_COUNTER_BUFFER,
  GL_COPY_READ_BUFFER,
  GL_COPY_WRITE_BUFFER,
  GL_DISPATCH_INDIRECT_BUFFER,
  GL_DRAW_INDIRECT_BUFFER,
  GL_ELEMENT_ARRAY_BUFFER,
  GL_PIXEL_PACK_BUFFER,
  GL_PIXEL_UNPACK_BUFFER,
#ifdef __unix__
  GL_QUERY_BUFFER,
#endif
  GL_SHADER_STORAGE_BUFFER,
  GL_TEXTURE_BUFFER,
  GL_TRANSFORM_FEEDBACK_BUFFER,
  GL_UNIFORM_BUFFER
};


kit::GL::StateSet::StateSet()
{
  // initialize a new stateset with the GL defaults
  
  for(auto i = gl_capabilities.begin(); i != gl_capabilities.end(); i++)
  {
    this->m_states[(*i)] = false;
  }
  
  this->m_states[GL_DITHER] = true;
  this->m_states[GL_MULTISAMPLE] = true;
  
  for(auto i = gl_buffertargets.begin(); i != gl_buffertargets.end(); i++)
  {
    this->m_boundBuffers[(*i)] = 0;
  }
  
  for(auto i = gl_texturetargets.begin(); i != gl_texturetargets.end(); i++)
  {
    this->m_boundTextures[(*i)] = 0;
  }
  
  this->m_boundArray = 0;
  this->m_usedProgram = 0;
  this->m_boundFramebufferRead = 0;
  this->m_boundFramebufferDraw = 0;
  this->m_boundRenderbuffer = 0;
  
  this->m_blendFuncCache.m_srcFactor  = GL_ONE;
  this->m_blendFuncCache.m_dstFactor = GL_ZERO;
  this->m_blendFuncCacheValid = true;

  this->m_cullMode        = GL_BACK;

  this->m_depthFunction   = GL_LESS;
  this->m_depthRangeNear  = 0;
  this->m_depthRangeFar   = 1;
  this->m_depthMask       = GL_TRUE;

  this->m_scissor_x       = 0;
  this->m_scissor_y       = 0;
  this->m_scissor_width   = 0;
  this->m_scissor_height  = 0;

  this->m_stencilFunc     = GL_ALWAYS;
  this->m_stencilRefVal   = 0;
  this->m_stencilMask     = 0xFFFFFFFF;

  this->m_stencilOp_sfail = GL_KEEP;
  this->m_stencilOp_dpfail= GL_KEEP;
  this->m_stencilOp_dppass= GL_KEEP;
}

uint32_t           kit::GL::m_instanceCount = 0;
kit::GL::StateSet     kit::GL::m_cache;
kit::GL::StateSet     kit::GL::m_originalStates;

kit::GL::GL()
{
  
  kit::GL::m_instanceCount += 1;
  if(kit::GL::m_instanceCount == 1)
  {
   std::cout << "Allocating GL statemanager..." << std::endl;
   kit::GL::allocate();
  }
}

kit::GL::~GL()
{
  kit::GL::m_instanceCount -= 1;
  if(kit::GL::m_instanceCount < 1)
  {
   std::cout << "Releasing GL statemanager..." << std::endl;
   kit::GL::release();
  }
}

void kit::GL::allocate()
{
 // kit::GL::m_originalStates.ReadFromHardware();
 // kit::GL::m_cache = kit::GL::m_originalStates;

}

void kit::GL::release()
{
  //kit::GL::m_originalStates.WriteToHardware();
}

void kit::GL::enable(GLenum state)
{
  
  if(!kit::GL::m_cache.m_states[state])
  {
    KIT_GL(glEnable(state));
    kit::GL::m_cache.m_states[state] = true;
  }
}

void kit::GL::disable(GLenum state)
{
  
  if(kit::GL::m_cache.m_states[state])
  {
    KIT_GL(glDisable(state));
    kit::GL::m_cache.m_states[state] = false;
  }
}

void kit::GL::bindTexture(GLenum target, GLuint texture)
{
  
  if(kit::GL::m_cache.m_boundTextures[target] != texture)
  {
    KIT_GL(glBindTexture(target, texture));
    kit::GL::m_cache.m_boundTextures[target] = texture;
  }
}

void kit::GL::bindBuffer(GLenum target, GLuint buffer)
{
  
  if(kit::GL::m_cache.m_boundBuffers[target] != buffer)
  {
    KIT_GL(glBindBuffer(target, buffer));
    kit::GL::m_cache.m_boundBuffers[target] = buffer;
  }
}

void kit::GL::bindVertexArray(GLuint array)
{
  
  if(kit::GL::m_cache.m_boundArray != array)
  {
    KIT_GL(glBindVertexArray(array));
    kit::GL::m_cache.m_boundArray = array;
  }
}

void kit::GL::useProgram(GLuint program)
{
  
  if(kit::GL::m_cache.m_usedProgram != program)
  {
    KIT_GL(glUseProgram(program));
    kit::GL::m_cache.m_usedProgram = program;
  }
}

void kit::GL::bindFramebuffer(GLenum target, GLuint framebuffer)
{
  
  if(target == GL_READ_FRAMEBUFFER)
  {
    if(framebuffer != kit::GL::m_cache.m_boundFramebufferRead)
    {
      KIT_GL(glBindFramebuffer(target, framebuffer));
      kit::GL::m_cache.m_boundFramebufferRead = framebuffer;
    }
  }
  else if(target == GL_DRAW_FRAMEBUFFER)
  {
    if(framebuffer != kit::GL::m_cache.m_boundFramebufferDraw)
    {
      KIT_GL(glBindFramebuffer(target, framebuffer));
      kit::GL::m_cache.m_boundFramebufferDraw = framebuffer;
    }
  }
  else if(target == GL_FRAMEBUFFER)
  {
    if(kit::GL::m_cache.m_boundFramebufferRead != framebuffer || kit::GL::m_cache.m_boundFramebufferDraw != framebuffer)
    {
      KIT_GL(glBindFramebuffer(target, framebuffer));
      kit::GL::m_cache.m_boundFramebufferRead = framebuffer;
      kit::GL::m_cache.m_boundFramebufferDraw = framebuffer;
    }
  }
}

void kit::GL::bindRenderbuffer(GLenum target, GLuint renderbuffer)
{
  
  if(renderbuffer != kit::GL::m_cache.m_boundRenderbuffer)
  {
    KIT_GL(glBindRenderbuffer(target, renderbuffer));
    kit::GL::m_cache.m_boundRenderbuffer = renderbuffer;
  }
}

void kit::GL::blendFunc(GLenum src, GLenum dest)
{
  
  if(kit::GL::m_cache.m_blendFuncCacheValid && kit::GL::m_cache.m_blendFuncCache.m_srcFactor == src && kit::GL::m_cache.m_blendFuncCache.m_dstFactor == dest )
  {
    return;
  }
  
  KIT_GL(glBlendFunc(src, dest));
  kit::GL::StateSet::blendFunc func;
  func.m_srcFactor = src;
  func.m_dstFactor = dest;
  kit::GL::m_cache.m_blendFuncCacheValid = true;
  kit::GL::m_cache.m_blendFuncCache = func;
  
  kit::GL::m_cache.m_blendFuncs[GL_FRONT_LEFT] = func;
  kit::GL::m_cache.m_blendFuncs[GL_FRONT_RIGHT] = func;
  kit::GL::m_cache.m_blendFuncs[GL_BACK_LEFT] = func;
  kit::GL::m_cache.m_blendFuncs[GL_BACK_RIGHT] = func;
  
  GLint maxattachments;
  KIT_GL(glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxattachments));
  
  for(GLuint i = 0; i < GLuint(maxattachments); i++)
  {
    kit::GL::m_cache.m_blendFuncs[i] = func;
  }
}

void kit::GL::blendFunci(GLuint buf, GLenum src, GLenum dest)
{
  
  if(kit::GL::m_cache.m_blendFuncCacheValid && kit::GL::m_cache.m_blendFuncCache.m_srcFactor == src && kit::GL::m_cache.m_blendFuncCache.m_dstFactor == dest )
  {
    return;
  }
  kit::GL::m_cache.m_blendFuncCacheValid = false;
  auto finder = kit::GL::m_cache.m_blendFuncs.find(buf);
  if(finder != kit::GL::m_cache.m_blendFuncs.end())
  {
    if(finder->second.m_srcFactor == src && finder->second.m_dstFactor == dest)
    {
      return;
    }
  }

  kit::GL::StateSet::blendFunc func;
  func.m_srcFactor = src;
  func.m_dstFactor = dest;
  kit::GL::m_cache.m_blendFuncs[buf] = func;
  KIT_GL(glBlendFunci(buf, src, dest));
}

void kit::GL::cullFace(GLenum mode)
{
  
  if(kit::GL::m_cache.m_cullMode != mode)
  {
    KIT_GL(glCullFace(mode));
    kit::GL::m_cache.m_cullMode = mode;
  }
}

void kit::GL::depthFunc(GLenum func)
{
  
  if(kit::GL::m_cache.m_depthFunction != func)
  {
    KIT_GL(glDepthFunc(func));
    kit::GL::m_cache.m_depthFunction = func;
  }
}

void kit::GL::depthRange(double knear, double kfar)
{
  
  if(kit::GL::m_cache.m_depthRangeNear != knear || kit::GL::m_cache.m_depthRangeFar != kfar)
  {
    KIT_GL(glDepthRange(knear, kfar));
    kit::GL::m_cache.m_depthRangeNear = knear;
    kit::GL::m_cache.m_depthRangeFar = kfar;
  }
}

void kit::GL::depthMask(GLboolean b)
{
  
  if(kit::GL::m_cache.m_depthMask != b)
  {
    KIT_GL(glDepthMask(b));
    kit::GL::m_cache.m_depthMask = b;
  }
}

void kit::GL::scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
  
  if(   kit::GL::m_cache.m_scissor_x != x || kit::GL::m_cache.m_scissor_y != y
    ||  kit::GL::m_cache.m_scissor_width != width || kit::GL::m_cache.m_scissor_height != height )
  {
    KIT_GL(glScissor(x, y, width, height));
    kit::GL::m_cache.m_scissor_x = x;
    kit::GL::m_cache.m_scissor_y = y;
    kit::GL::m_cache.m_scissor_width = width;
    kit::GL::m_cache.m_scissor_height = height;
  }
}

void kit::GL::stencilFunc(GLenum func, GLint ref, GLuint mask)
{
  
  if(   kit::GL::m_cache.m_stencilFunc != func || kit::GL::m_cache.m_stencilRefVal != ref
    ||  kit::GL::m_cache.m_stencilMask != mask  )
  {
    KIT_GL(glStencilFunc(func, ref, mask));
    kit::GL::m_cache.m_stencilFunc = func;
    kit::GL::m_cache.m_stencilRefVal = ref;
    kit::GL::m_cache.m_stencilMask = mask;
  }
}

void kit::GL::stencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
  
  if(   kit::GL::m_cache.m_stencilOp_sfail != sfail || kit::GL::m_cache.m_stencilOp_dpfail != dpfail
    ||  kit::GL::m_cache.m_stencilOp_dppass != dppass  )
  {
    KIT_GL(glStencilOp(sfail, dpfail, dppass));
    kit::GL::m_cache.m_stencilOp_sfail = sfail;
    kit::GL::m_cache.m_stencilOp_dpfail = dpfail;
    kit::GL::m_cache.m_stencilOp_dppass = dppass;
  }
}

void kit::GL::attachShader(GLuint p, GLuint s)
{
  
  KIT_GL(glAttachShader(p, s));
}

void kit::GL::detachShader(GLuint p, GLuint s)
{
  
  KIT_GL(glDetachShader(p, s));
}