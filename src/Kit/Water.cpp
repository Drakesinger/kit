/*#include "Kit/Water.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Renderer.hpp"
#include "Kit/Program.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Cubemap.hpp"
#include "Kit/PixelBuffer.hpp"

// A structure to use as index for vertices, when generating our index buffer
struct xyPair
{
  float x;
  float y;

  xyPair(float xx, float yy)
  {
    this->x = xx;
    this->y = yy;
  }

  bool operator<(const xyPair& b) const { return std::tie(this->x, this->y) < std::tie(b.x, b.y); }
};

// Water constructor
kit::Water::Water(glm::uvec2 resolution)
{
  // Set values
  this->m_resolution = resolution;
  this->m_radianceMap = nullptr;
  this->m_environmentStrength = glm::vec3(1.0f, 1.0f, 1.0f);

  // Number of floats needed per vertex
  // We need 2: First float for x position and texcoord, second for y position and texcoord.
  uint32_t componentCount = 2; 

  // Create a list of floats for our vertex data
  std::vector<float> vertexData(resolution.x * resolution.y * componentCount, 0.0f);

  // Keep tab of vertex indices for our index generation
  std::map<xyPair, uint32_t> idIndex;

  // Precompute grid steps
  float xstep = 1.0f / (float)this->m_resolution.x;
  float ystep = 1.0f / (float)this->m_resolution.y;

  // Iterate each cell in the grid, and fill the vertex data
  uint32_t currIndex = 0;
  uint32_t currId = 0;
  for (uint32_t x = 0; x < this->m_resolution.x; x++)
  {
    for (uint32_t y = 0; y < this->m_resolution.y; y++)
    {
      // Update vertex data at current grid position
      float xcurr = xstep * (float)x;
      float ycurr = ystep * (float)y;
      vertexData[currIndex++] = xcurr;
      vertexData[currIndex++] = ycurr;

      // Store the current vertex ID in the ID index, at our current grid position
      idIndex[xyPair((float)x, (float)y)] = currId++;
    }
  }

  // Iterate each cell in the grid, and fill the index data
  currIndex = 0;
  std::vector<uint32_t> indexData(resolution.x * resolution.y * 6, 0);
  for (uint32_t x = 0; x < this->m_resolution.x-2; x++)
  {
    for (uint32_t y = 0; y < this->m_resolution.y-2; y++)
    {
      // Find the vertex ID of each corner in the grid, by searching our cache
      uint32_t tlIndex = idIndex.at(xyPair((float)x, (float)y));
      uint32_t trIndex = idIndex.at(xyPair((float)x+1.0f, (float)y));
      uint32_t blIndex = idIndex.at(xyPair((float)x, (float)y+1.0f));
      uint32_t brIndex = idIndex.at(xyPair((float)x+1.0f, (float)y+1.0f));

      // 2 triangles per grid cell
      indexData[currIndex++] = tlIndex;
      indexData[currIndex++] = trIndex;
      indexData[currIndex++] = brIndex;

      indexData[currIndex++] = tlIndex;
      indexData[currIndex++] = brIndex;
      indexData[currIndex++] = blIndex;
    }
  }

  // Create a vertex array object and two vertex buffer objects (for vertex data and index data)
  glGenVertexArrays(1, &this->m_glVao);
  glGenBuffers(1, &this->m_glVertexBuffer);
  glGenBuffers(1, &this->m_glIndexBuffer);

  // Use the newly created VAO
  glBindVertexArray(this->m_glVao);

  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glIndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(uint32_t), &indexData[0], GL_STATIC_DRAW);
  this->m_indexCount = (uint32_t)indexData.size();

  // Upload vertices 
  glBindBuffer(GL_ARRAY_BUFFER, this->m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);

  // Set the vertex data layout in our VAO
  uint32_t attributeSize = (sizeof(float) * 2);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);

  // Lets also create a shader program  and load some textures we need
  this->m_program = kit::Program::load({ "water.vert" }, {"lighting/cooktorrance.glsl", "water.frag" });
  this->m_heightmapA = kit::Texture::load("waterheight.tga", false);
  this->m_normalmapA = kit::Texture::load("waternormal.tga", false);
  this->m_heightmapB = kit::Texture::load("waterheight2.tga", false);
  this->m_normalmapB = kit::Texture::load("waternormal2.tga", false);
  this->m_brdf = kit::Texture::load("brdf.tga", false);
}

kit::Water::~Water()
{
  glDeleteBuffers(1, &this->m_glIndexBuffer);
  glDeleteBuffers(1, &this->m_glVertexBuffer);
  glDeleteVertexArrays(1, &this->m_glVao);
}

void kit::Water::renderForward(kit::Renderer::Ptr renderer)
{
  // Set OpenGL states
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE, GL_ONE);
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  // Calculate matrices to pass into shader
  glm::mat4 modelMatrix = this->getTransformMatrix();
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

  // Use our shader program, and update variables
  this->m_program->use();

  this->m_program->setUniformTexture("uniform_colormap", renderer->getAccumulationCopy()->getColorAttachment(0));
  this->m_program->setUniformTexture("uniform_depthmap", renderer->getAccumulationCopy()->getDepthAttachment());
  this->m_program->setUniform2f("uniform_projConst", glm::vec2(px, py));
  this->m_program->setUniformMat4("uniform_invViewMatrix", invViewMatrix);
  this->m_program->setUniformCubemap("uniform_reflection", this->m_radianceMap);
  this->m_program->setUniform3f("uniform_lightColor", this->m_environmentStrength);

  this->m_program->setUniformTexture("uniform_normalmapA", this->m_normalmapA);
  this->m_program->setUniformTexture("uniform_normalmapB", this->m_normalmapB);
  this->m_program->setUniformTexture("uniform_heightmapA", this->m_heightmapA);
  this->m_program->setUniformTexture("uniform_heightmapB", this->m_heightmapB);

  this->m_program->setUniformMat4("uniform_mvpMatrix", mvpMatrix);
  this->m_program->setUniformMat4("uniform_mvMatrix", mvMatrix);
  
  // Bind our OpenGL VAO and push a draw-call to the GPU to render our water
  glBindVertexArray(this->m_glVao);
  glDrawElements(GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

kit::Water::Ptr kit::Water::create(glm::uvec2 resolution)
{
  return std::make_shared<kit::Water>(resolution);
}

void kit::Water::update(double const & ms)
{
  // Update the current-time variable in our shaderprogram on update
  static double time = 0.0;
  time += ms;
  this->m_program->setUniform1f("uniform_time", (float)time);
}

int32_t kit::Water::getRenderPriority()
{
  // Render priority at 1000, since we want to render water after anything else
  return 1000;
}

bool kit::Water::requestAccumulationCopy()
{
  return true;
}

void kit::Water::setRadianceMap(kit::Cubemap::Ptr rad)
{
  // Radiance map to use for reflective lights
  this->m_radianceMap = rad;
}

void kit::Water::setEnvironmentStrength(glm::vec3 e)
{
  // Strength of reflective lights
  this->m_environmentStrength = e;
}

void kit::Water::setSunDirection(glm::vec3 d)
{
  this->m_sunDirection = d;
}

void kit::Water::setSunColor(glm::vec3 c)
{
  this->m_sunColor = c;
}
*/
