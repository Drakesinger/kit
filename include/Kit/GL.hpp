#ifndef KIT_GLSTATEMANAGER_HPP
#define KIT_GLSTATEMANAGER_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/OpenGL.hpp"
#include "Kit/IncOpenGL.hpp"

#include <map>
/* 
 * Anything on this list is planned feature support. Anything with an X to the right of it is fully implemented.
 * 
 *  Bound..
 *    VAO           X
 *    VBO           X
 *    Texture       X
 *    Program       X
 *    Framebuffer   X
 *    Renderbuffer  X
 * 
 *  Enabled/Disabled..
 *    GL_BLEND                                            X
 *      glBlendFunc()                                     X
 *      glBlendFunci()                                    
 *    GL_CULL_FACE                                        X
 *      glCullFace()                                      X
 *    GL_DEPTH_TEST                                       X
 *      glDepthFunc(), glDepthRange(), glDepthMask()      X
 *    GL_SCISSOR_TEST                                     X
 *      glScissor()                                       X
 *    GL_STENCIL_TEST                                     X
 *      glStencilFunc() and glStencilOp()                 X
 *    GL_TEXTURE_CUBE_MAP_SEAMLESS                        X
 */

namespace kit 
{
 class KITAPI GL 
 {
  public:
    
    struct StateSet
    {
      struct blendFunc 
      {
        GLenum m_srcFactor;
        GLenum m_dstFactor;
      };
      
      StateSet();
      
      std::map<GLenum, bool>   m_states;         // Currently enabled/disabled states
      std::map<GLenum, GLuint> m_boundTextures;  // Currently bound textures
      std::map<GLenum, GLuint> m_boundBuffers;   // Currently bound VBO's
      GLuint                   m_boundArray;     // Currently bound VAO
      GLuint                   m_usedProgram;    // Currently used program
      GLuint                   m_boundFramebufferRead; // Currently bound framebuffer for reading
      GLuint                   m_boundFramebufferDraw; // Currently bound framebuffer for drawing
      GLuint                   m_boundRenderbuffer;    // Currently bound renderbuffer
      
      //GLenum                   m_BlendSrcFactor; // Initially GL_ONE
      //GLenum                   m_BlendDestFactor;// Initially GL_ZERO
      std::map<GLuint, blendFunc> m_blendFuncs;  // 
      blendFunc                   m_blendFuncCache;
      bool                        m_blendFuncCacheValid;
      
      GLenum                   m_cullMode;       // Initially GL_BACK
      
      GLenum                   m_depthFunction;  // Initially GL_LESS
      double                   m_depthRangeNear; // Initially 0
      double                   m_depthRangeFar;  // Initially 1
      GLboolean                m_depthMask;      // Initially GL_TRUE
      
      GLint                    m_scissor_x;      // Initially 0
      GLint                    m_scissor_y;      // Initially 0
      GLsizei                  m_scissor_width; // Initially width of window
      GLsizei                  m_scissor_height; // Initially height of window
      
      GLenum                   m_stencilFunc;    // Initially GL_ALWAYS
      GLint                    m_stencilRefVal;  // Initially 0
      GLuint                   m_stencilMask;    // Initially all 1's
      
      GLenum                   m_stencilOp_sfail;   // Initially GL_KEEP
      GLenum                   m_stencilOp_dpfail;  // Initially GL_KEEP
      GLenum                   m_stencilOp_dppass;  // Initially GL_KEEP

    };
    
    GL();
    ~GL();
    
    static void enable(GLenum state);
    static void disable(GLenum state);
    
    static void bindTexture(GLenum target, GLuint texture);
    static void bindBuffer(GLenum target, GLuint buffer);
    static void bindVertexArray(GLuint array);
    static void useProgram(GLuint program);
    static void bindFramebuffer(GLenum target, GLuint framebuffer);
    static void bindRenderbuffer(GLenum target, GLuint renderbuffer);
    
    static void blendFunc(GLenum src, GLenum dest);
    static void blendFunci(GLuint buf, GLenum sfactor, GLenum dfactor);
    
    static void cullFace(GLenum mode);
    
    static void depthFunc(GLenum func);
    static void depthRange(double knear, double kfar);
    static void depthMask(GLboolean b);
    
    static void scissor(GLint x, GLint y, GLsizei width, GLsizei height);
    
    static void stencilFunc(GLenum func, GLint ref, GLuint mask);
    static void stencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
    
    static void attachShader(GLuint p, GLuint s);
    static void detachShader(GLuint p, GLuint s);

  private:
    
    static void allocate();
    static void release();
    static uint32_t              m_instanceCount;
    static StateSet                 m_cache;
    static StateSet                 m_originalStates;
 };
}

#endif