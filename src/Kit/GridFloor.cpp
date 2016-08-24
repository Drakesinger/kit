#include "Kit/GridFloor.hpp"
#include "Kit/Program.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Renderer.hpp"

uint32_t kit::GridFloor::m_instanceCount = 0;
GLuint kit::GridFloor::m_glVertexArray = 0;
GLuint kit::GridFloor::m_glVertexIndices = 0;
GLuint kit::GridFloor::m_glVertexBuffer = 0;
kit::Program::Ptr kit::GridFloor::m_program = nullptr;
uint32_t kit::GridFloor::m_indexCount = 0;

kit::GridFloor::GridFloor() : kit::Renderable::Renderable()
{
  kit::GridFloor::m_instanceCount++;
  if(kit::GridFloor::m_instanceCount == 1)
  {
    kit::GridFloor::allocateShared();
  }
}

kit::GridFloor::~GridFloor()
{
  kit::GridFloor::m_instanceCount--;
  if(kit::GridFloor::m_instanceCount == 0)
  {
    kit::GridFloor::releaseShared();
  }
}

kit::GridFloor::Ptr kit::GridFloor::create()
{
  return std::make_shared<kit::GridFloor>();
}

void kit::GridFloor::allocateShared()
{
  
  std::vector<GLfloat> vertexData;
  std::vector<GLuint> indexData;
  GLuint currIndex = 0;
  
  float majorAlpha = 0.40f;
  float minorAlpha = 0.10f;
  
  int32_t size = 10;
  for(int32_t i = -size; i < size+1; i++)
  {
    float currPos = (float)i;
    
    vertexData.push_back(currPos);  // X
    vertexData.push_back(0.0f);     // Y
    vertexData.push_back((GLfloat)-size);    // Z
    vertexData.push_back(1.0f);     // R
    vertexData.push_back(1.0f);     // G
    vertexData.push_back(1.0f);     // B
    vertexData.push_back(majorAlpha);    // A
    
    vertexData.push_back(currPos);  // X
    vertexData.push_back(0.0f);     // Y
    vertexData.push_back((GLfloat)size);     // Z
    vertexData.push_back(1.0f);     // R
    vertexData.push_back(1.0f);     // G
    vertexData.push_back(1.0f);     // B
    vertexData.push_back(majorAlpha);    // A
    
    indexData.push_back(currIndex++);
    indexData.push_back(currIndex++);
    
    vertexData.push_back((GLfloat)-size);    // X
    vertexData.push_back(0.0f);     // Y
    vertexData.push_back(currPos);  // Z
    vertexData.push_back(1.0f);     // R
    vertexData.push_back(1.0f);     // G
    vertexData.push_back(1.0f);     // B
    vertexData.push_back(majorAlpha);    // A
    
    vertexData.push_back((GLfloat)size);     // X
    vertexData.push_back(0.0f);     // Y
    vertexData.push_back(currPos);  // Z
    vertexData.push_back(1.0f);     // R
    vertexData.push_back(1.0f);     // G
    vertexData.push_back(1.0f);     // B
    vertexData.push_back(majorAlpha);    // A
    
    indexData.push_back(currIndex++);
    indexData.push_back(currIndex++);
    
    if(i < size)
    {
      vertexData.push_back(currPos+0.5f);  // X
      vertexData.push_back(0.0f);     // Y
      vertexData.push_back((GLfloat)-size);    // Z
      vertexData.push_back(1.0f);     // R
      vertexData.push_back(1.0f);     // G
      vertexData.push_back(1.0f);     // B
      vertexData.push_back(minorAlpha);    // A
      
      vertexData.push_back(currPos+0.5f);  // X
      vertexData.push_back(0.0f);     // Y
      vertexData.push_back((GLfloat)size);     // Z
      vertexData.push_back(1.0f);     // R
      vertexData.push_back(1.0f);     // G
      vertexData.push_back(1.0f);     // B
      vertexData.push_back(minorAlpha);    // A
      
      indexData.push_back(currIndex++);
      indexData.push_back(currIndex++);
      
      vertexData.push_back((GLfloat)-size);    // X
      vertexData.push_back(0.0f);     // Y
      vertexData.push_back(currPos+0.5f);  // Z
      vertexData.push_back(1.0f);     // R
      vertexData.push_back(1.0f);     // G
      vertexData.push_back(1.0f);     // B
      vertexData.push_back(minorAlpha);    // A
      
      vertexData.push_back((GLfloat)size);     // X
      vertexData.push_back(0.0f);     // Y
      vertexData.push_back(currPos+0.5f);  // Z
      vertexData.push_back(1.0f);     // R
      vertexData.push_back(1.0f);     // G
      vertexData.push_back(1.0f);     // B
      vertexData.push_back(minorAlpha);    // A
      
      indexData.push_back(currIndex++);
      indexData.push_back(currIndex++);
    }
  }
  
  // Cross X
  vertexData.push_back(0.0f);    // X
  vertexData.push_back(0.05f);    // Y
  vertexData.push_back(0.0f);     // Z
  vertexData.push_back(1.0f);     // R
  vertexData.push_back(0.0f);     // G
  vertexData.push_back(0.0f);     // B
  vertexData.push_back(1.0f);     // A
  
  vertexData.push_back((GLfloat)size);     // X
  vertexData.push_back(0.05f);    // Y
  vertexData.push_back(0.0f);     // Z
  vertexData.push_back(1.0f);     // R
  vertexData.push_back(0.0f);     // G
  vertexData.push_back(0.0f);     // B
  vertexData.push_back(1.0f);     // A
  
  indexData.push_back(currIndex++);
  indexData.push_back(currIndex++);
  
  // Cross Y
  vertexData.push_back(0.0f);    // X
  vertexData.push_back(0.05f);    // Y
  vertexData.push_back(0.0f);     // Z
  vertexData.push_back(0.0f);     // R
  vertexData.push_back(1.0f);     // G
  vertexData.push_back(0.0f);     // B
  vertexData.push_back(1.0f);     // A
  
  vertexData.push_back(0.0f);     // X
  vertexData.push_back((GLfloat)size);    // Y
  vertexData.push_back(0.0f);     // Z
  vertexData.push_back(0.0f);     // R
  vertexData.push_back(1.0f);     // G
  vertexData.push_back(0.0f);     // B
  vertexData.push_back(1.0f);     // A
  
  indexData.push_back(currIndex++);
  indexData.push_back(currIndex++);
  
  // Cross Z
  vertexData.push_back(0.0f);    // X
  vertexData.push_back(0.05f);    // Y
  vertexData.push_back(0.0f);     // Z
  vertexData.push_back(0.0f);     // R
  vertexData.push_back(0.0f);     // G
  vertexData.push_back(1.0f);     // B
  vertexData.push_back(1.0f);     // A
  
  vertexData.push_back(0.0f);     // X
  vertexData.push_back(0.05f);    // Y
  vertexData.push_back((GLfloat)size);     // Z
  vertexData.push_back(0.0f);     // R
  vertexData.push_back(0.0f);     // G
  vertexData.push_back(1.0f);     // B
  vertexData.push_back(1.0f);     // A
  
  indexData.push_back(currIndex++);
  indexData.push_back(currIndex++);
  
  kit::GridFloor::m_indexCount = (uint32_t)indexData.size();
  
  // Generate and bind our Vertex Array Object
  glGenVertexArrays(1, &kit::GridFloor::m_glVertexArray);
  glBindVertexArray(kit::GridFloor::m_glVertexArray);

  // Generate our Vertex Index Buffer Object
  glGenBuffers(1, &kit::GridFloor::m_glVertexIndices);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kit::GridFloor::m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), &indexData[0], GL_STATIC_DRAW);

  // Generate our Vertex Buffer Object
  glGenBuffers(1, &kit::GridFloor::m_glVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, kit::GridFloor::m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), &vertexData[0], GL_STATIC_DRAW);
  
  // XYZ
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 7, (void*)0);
  
  // RGBA
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 7, (void*) (sizeof(GLfloat) * 3));
  
  kit::GridFloor::m_program = kit::Program::load({"gridfloor.vert"}, {}, {"gridfloor.frag"});
}

void kit::GridFloor::releaseShared()
{
  
  kit::GridFloor::m_program.reset();
  
  glDeleteBuffers(1, &kit::GridFloor::m_glVertexIndices);
  glDeleteBuffers(1, &kit::GridFloor::m_glVertexBuffer);
  glDeleteVertexArrays(1, &kit::GridFloor::m_glVertexArray);
}

void kit::GridFloor::renderForward(kit::Renderer::Ptr renderer)
{
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  
  glm::mat4 modelViewProjectionMatrix = renderer->getActiveCamera()->getProjectionMatrix() * renderer->getActiveCamera()->getViewMatrix() * this->getTransformMatrix();
  
  kit::GridFloor::m_program->setUniformMat4("uniform_mvpMatrix", modelViewProjectionMatrix);
  kit::GridFloor::m_program->setUniform1f("uniform_whitepoint", renderer->getActiveCamera()->getWhitepoint());
  kit::GridFloor::m_program->use();
  kit::GridFloor::renderGeometry();
  kit::Program::useFixed();
}

void kit::GridFloor::renderGeometry()
{
  glBindVertexArray(kit::GridFloor::m_glVertexArray);
  glDrawElements(GL_LINES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

bool kit::GridFloor::isShadowCaster()
{
  return false;
}
