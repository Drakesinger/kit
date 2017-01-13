#include "Kit/Water.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Renderer.hpp"
#include "Kit/Program.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Cubemap.hpp"
#include "Kit/PixelBuffer.hpp"
#include "Kit/DoubleBuffer.hpp"
#include "Kit/Quad.hpp"

// Water constructor
kit::Water::Water()
{
  // Number of floats needed per vertex
  // We need 2: First float for x position and texcoord, second for y position and texcoord.

  // BOTTOMLEFT TOPLEFT TOPRIGHT BOTTOMRIGHT
  std::vector<float> vertexData = {
    -0.5f, -0.5f,
    0.5f, -0.5f, 
    0.5f, 0.5f,
    -0.5f, 0.5f
  };
  
  // TR TL BL, BR TR BL
  std::vector<uint32_t> indexData = {
    0, 1, 2,
    0, 2, 3
  };


  // Create a vertex array object and two vertex buffer objects (for vertex data and index data)
  glGenVertexArrays(1, &m_glVao);
  glGenBuffers(1, &m_glVertexBuffer);
  glGenBuffers(1, &m_glIndexBuffer);

  // Use the newly created VAO
  glBindVertexArray(m_glVao);

  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(uint32_t), &indexData[0], GL_STATIC_DRAW);
  m_indexCount = (uint32_t)indexData.size();

  // Upload vertices 
  glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);

  // Set the vertex data layout in our VAO
  uint32_t attributeSize = (sizeof(float) * 2);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);

  // Lets also create a shader program  and load some textures we need
  m_program = new kit::Program({ "water.vert" }, {"water.frag" });
  m_belowProgram = new kit::Program({ "waterbelow.vert" }, {"waterbelow.frag" });
  m_heightmapA = kit::Texture::load("waterheight.tga", false);
  m_normalmapA = kit::Texture::load("waternormal.tga", false);
  m_heightmapB = kit::Texture::load("waterheight2.tga", false);
  m_normalmapB = kit::Texture::load("waternormal2.tga", false);
  
  m_underwaterProgram =new kit::Program({ "underwater.vert" }, {"underwater.frag" });
  m_underwaterQuad = new kit::Quad();
  m_vignette = kit::Texture::load("watervignette.png");
}

kit::Water::~Water()
{
  glDeleteBuffers(1, &m_glIndexBuffer);
  glDeleteBuffers(1, &m_glVertexBuffer);
  glDeleteVertexArrays(1, &m_glVao);
  
  if(m_program) delete m_program;
  if(m_belowProgram) delete m_belowProgram;
  if(m_underwaterProgram) delete m_underwaterProgram;
  if(m_underwaterQuad) delete m_underwaterQuad;
}

void kit::Water::renderForward(kit::Renderer* renderer)
{

  // Set OpenGL states
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);


  
  // Calculate matrices to pass into shader
  glm::mat4 modelMatrix = getWorldTransformMatrix();
  glm::mat4 viewMatrix = renderer->getActiveCamera()->getViewMatrix();
  glm::mat4 invViewMatrix = glm::inverse(viewMatrix);
  glm::mat4 projectionMatrix = renderer->getActiveCamera()->getProjectionMatrix();
  glm::mat4 mvMatrix = viewMatrix * modelMatrix;
  //glm::mat4 normalMatrix = glm::transpose(glm::inverse(mvMatrix));
  glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
  
  // Calculate px and py to linearize depth in shader
  glm::vec2 clip = renderer->getActiveCamera()->getClipRange();
  float znear = clip.x;
  float zfar = clip.y;
  float px = (-zfar * znear) / (zfar - znear);
  float py = zfar / (zfar - znear);
  

  // Render below
  if(renderer->getActiveCamera()->getWorldPosition().y < getWorldPosition().y)
  {
    // Render surface from below
    {
      // Set OpenGL states
      glCullFace(GL_BACK);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);

      //glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
      //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      //glStencilMask(0xFF); // Write to stencil buffer
      //glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)
      
      // Use our shader program, and update variables
      m_belowProgram->setUniformTexture("uniform_colormap", renderer->getAccumulationCopy()->getColorAttachment(0));
      m_belowProgram->setUniformTexture("uniform_depthmap", renderer->getAccumulationCopy()->getDepthAttachment());
      m_belowProgram->setUniformTexture("uniform_positionmap", renderer->getPositionBuffer()->getColorAttachment(0));
      m_belowProgram->setUniformMat4("uniform_invViewMatrix", invViewMatrix);
      m_belowProgram->setUniform3f("uniform_camerawp", renderer->getActiveCamera()->getWorldPosition());

      m_belowProgram->setUniformTexture("uniform_normalmapA", m_normalmapA.get());
      m_belowProgram->setUniformTexture("uniform_normalmapB", m_normalmapB.get());
      //m_belowProgram->setUniformTexture("uniform_heightmapA", m_heightmapA.get());
      //m_belowProgram->setUniformTexture("uniform_heightmapB", m_heightmapB.get());

      m_belowProgram->setUniformMat4("uniform_mvpMatrix", mvpMatrix);
      m_belowProgram->setUniformMat4("uniform_mvMatrix", mvMatrix);
      
      m_belowProgram->use();
      
      glBindVertexArray(m_glVao);
      glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, (void*)0);
    }
    
    // Render fullscreen effects
    {
      renderer->updateAccumulationCopy();
      renderer->updatePositionBuffer();
      
      glDisable(GL_BLEND);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glDisable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      //glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
      //glStencilMask(0x00); // Don't write anything to stencil buffer
      
      m_underwaterProgram->setUniformTexture("uniform_normalmapA", m_normalmapA.get());
      m_underwaterProgram->setUniformTexture("uniform_normalmapB", m_normalmapB.get());
      m_underwaterProgram->setUniformTexture("uniform_colormap", renderer->getAccumulationCopy()->getColorAttachment(0));
      m_underwaterProgram->setUniformTexture("uniform_positionmap", renderer->getPositionBuffer()->getColorAttachment(0));
      m_underwaterProgram->setUniformMat4("uniform_invViewMatrix", invViewMatrix);
      m_underwaterProgram->setUniformTexture("uniform_vignette", m_vignette.get());
      m_underwaterProgram->use();

      m_underwaterQuad->setBlending(false);
      m_underwaterQuad->render(m_underwaterProgram);
    }

  }
  // Render above
  else
  {
    // Use our shader program, and update variables
    m_program->setUniformTexture("uniform_colormap", renderer->getAccumulationCopy()->getColorAttachment(0));
    m_program->setUniformTexture("uniform_depthmap", renderer->getAccumulationCopy()->getDepthAttachment());
    m_program->setUniformTexture("uniform_positionmap", renderer->getPositionBuffer()->getColorAttachment(0));
    m_program->setUniformMat4("uniform_invViewMatrix", invViewMatrix);
    m_program->setUniformMat4("uniform_viewMatrix", viewMatrix);
    //m_program->setUniform3f("uniform_camerawp", renderer->getActiveCamera()->getWorldPosition());
    //m_program->setUniform3f("uniform_camerafwd", renderer->getActiveCamera()->getWorldForward());

    
    
    m_program->setUniformTexture("uniform_normalmapA", m_normalmapA.get());
    m_program->setUniformTexture("uniform_normalmapB", m_normalmapB.get());
    m_program->setUniformTexture("uniform_reflection", renderer->getReflectionMap());
    //m_program->setUniformTexture("uniform_heightmapA", m_heightmapA.get());
    //m_program->setUniformTexture("uniform_heightmapB", m_heightmapB.get());

    m_program->setUniformMat4("uniform_mvpMatrix", mvpMatrix);
    m_program->setUniformMat4("uniform_mvMatrix", mvMatrix);
    
    m_program->use();

    glBindVertexArray(m_glVao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, (void*)0);
  }
  
}

void kit::Water::update(double const & ms)
{
  // Update the current-time variable in our shaderprogram on update
  static double time = 0.0;
  time += ms;
  m_program->setUniform1f("uniform_time", (float)time);
  m_belowProgram->setUniform1f("uniform_time", (float)time);
  m_underwaterProgram->setUniform1f("uniform_time", (float)time);
}

int32_t kit::Water::getRenderPriority()
{
  // Render priority at 1000, since we want to render water after anything else
  return 0;
}

bool kit::Water::requestAccumulationCopy()
{
  return true;
}

bool kit::Water::requestPositionBuffer()
{
  return true;
}


void kit::Water::setSunDirection(glm::vec3 d)
{
  m_sunDirection = d;
}

void kit::Water::setSunColor(glm::vec3 c)
{
  m_sunColor = c;
}
