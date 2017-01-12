#include "Kit/EditorTerrain.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Program.hpp"
#include "Kit/Material.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Renderer.hpp"
#include "Kit/Shader.hpp"
#include "Kit/Types.hpp"
#include "Kit/DoubleBuffer.hpp"
#include "Kit/PixelBuffer.hpp"

#include <string>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <algorithm>


#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp> 

const char componentIndex[4] = { 'r', 'g', 'b', 'a' };

static std::string getMaskSuffix(int layer)
{
  std::string returner;
  if (layer < 4)
  {
    returner = "0.";
    returner.push_back(componentIndex[layer]);
  }
  else
  {
    returner = "1.";
    returner.push_back(componentIndex[layer - 4]);
  }

  return returner;
}
kit::EditorTerrain::Triangle::Triangle(kit::EditorTerrain::Vertex* a, kit::EditorTerrain::Vertex* b, kit::EditorTerrain::Vertex* c)
{
  m_a = a;
  m_b = b;
  m_c = c;
  m_bitangent = glm::vec3(0.0f, 0.0f, 0.0f);
  m_normal = glm::vec3(0.0f, 0.0f, 0.0f);
  m_tangent = glm::vec3(0.0f, 0.0f, 0.0f);
}


kit::EditorTerrain::EditorTerrain() : kit::Renderable()
{
  // Initialize the layerinfo array with some sane defaults
  for(auto & currLayer : m_layerInfo)
  {
    currLayer.material = std::make_shared<kit::Material>();
  }

  m_name = "";
  
  m_bakeProgramArnx = nullptr;
  m_arnxCache = nullptr;
    
  m_materialMask = nullptr;
  m_materialPaintProgram = nullptr;
  m_heightPaintProgram = nullptr;

  m_indexCount = 0;
  m_numLayers = 0;
  m_program = nullptr;
  m_resolution = glm::uvec2(0, 0);
  m_valid = false;

  m_heightmap = nullptr;
  m_yScale = 0.0f;
  m_xzScale = 0.0f;

  m_decalBrush = nullptr;
  m_decalProgram = nullptr;
  m_decalBrushPosition = glm::vec2(0.0f, 0.0f);
  m_decalBrushSize = glm::vec2(0.0f, 0.0f);
  
  glGenVertexArrays(1, &m_glVertexArray);
  glGenBuffers(1, &m_glVertexIndices);
  glGenBuffers(1, &m_glVertexBuffer);
  
  // Configure attributes
  glBindVertexArray(m_glVertexArray);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glVertexIndices);
  glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
  
  static const uint32_t attributeSize = sizeof(float) * 4;

  // Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);

  // Texture coordinates
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 2) );
  
  // Compile static programs
  {
    m_materialPaintProgram = new kit::Program({"editor/terrain-material-paint.vert"}, {"editor/terrain-material-paint.frag"});
    m_heightPaintProgram = new kit::Program({"editor/terrain-height-paint.vert"}, {"editor/terrain-height-paint.frag"});
  }
}

kit::EditorTerrain::~EditorTerrain()
{
  glDeleteBuffers(1, &m_glVertexIndices);
  glDeleteBuffers(1, &m_glVertexBuffer);
  glDeleteVertexArrays(1, &m_glVertexArray);

  for(Triangle *currTriangle : m_triangles)
  {
    delete currTriangle;
  }
  
  for(Vertex *currVertex : m_vertices)
  {
    delete currVertex;
  }
  m_triangles.clear();
  m_vertices.clear();
  
  if(m_materialMask)
    delete m_materialMask;
  
  if(m_materialPaintProgram)
    delete m_materialPaintProgram ;
  
  if(m_heightmap)
    delete m_heightmap;
  
  if(m_heightPaintProgram)
    delete m_heightPaintProgram;
  
  if(m_program)
    delete m_program;
  
  if(m_pickProgram)
    delete m_pickProgram;
  
  if(m_shadowProgram)
    delete m_shadowProgram;
  
  if(m_bakeProgramArnx)
    delete m_bakeProgramArnx;
  
  if(m_arnxCache)
    delete m_arnxCache;
  
  if(m_decalBrush)
    delete m_decalBrush;
  
  if(m_decalProgram)
    delete m_decalProgram;
  
  if(m_wireProgram)
    delete m_wireProgram;
}

void kit::EditorTerrain::generateCache()
{
  // Clear old CPU data
  for(Triangle *currTriangle : m_triangles)
  {
    delete currTriangle;
  }
  
  for(Vertex *currVertex : m_vertices)
  {
    delete currVertex;
  }
  
  m_triangles.clear();
  m_vertices.clear();
  
  // Create CPU data
  glm::vec2 fullSize;
  fullSize.x = float(m_resolution.x) * m_xzScale;
  fullSize.y = float(m_resolution.y) * m_xzScale;

  glm::vec2 halfSize = fullSize / 2.0f;
  
  // Generate vertexdata on the CPU
  uint32_t currId = 0;
  for(uint32_t y = 0; y < m_resolution.y; y++)
  {
    for(uint32_t x = 0; x < m_resolution.x; x++)
    {
      Vertex * newVertex = new Vertex();
      newVertex->m_position.x = float(x) * m_xzScale - halfSize.x;
      newVertex->m_position.y = 0.0f;
      newVertex->m_position.z = float(y) * m_xzScale - halfSize.y;

      newVertex->m_normal.x = 0.0;
      newVertex->m_normal.y = 0.0;
      newVertex->m_normal.z = 0.0;

      newVertex->m_tangent.x = 0.0;
      newVertex->m_tangent.y = 0.0;
      newVertex->m_tangent.z = 0.0;

      newVertex->m_uv.x = float(x) * (1.0f/float(m_resolution.x));
      newVertex->m_uv.y = 1.0f - (float(y) * (1.0f/float(m_resolution.y)));
      newVertex->m_id = currId;

      m_vertices.push_back(newVertex);
      m_indexCache[newVertex] = currId++;

    }
  }

  // Generate triangles on the CPU
  bool xflip = false;
  bool yflip = false;
  for(unsigned int y = 0; y < m_resolution.y-1; y++)
  {
    xflip = false;
    for(unsigned int x = 0; x < m_resolution.x-1; x++)
    {
      Triangle* newTriangleA;
      Triangle* newTriangleB;

      if(xflip)
      {
        if(yflip)
        {
          newTriangleA = new Triangle(getVertexAt(x, y), getVertexAt(x+1, y+1), getVertexAt(x, y+1)); // X
          newTriangleB = new Triangle(getVertexAt(x, y), getVertexAt(x+1, y), getVertexAt(x+1, y+1)); // X
        }
        else
        {
          newTriangleA = new Triangle(getVertexAt(x, y), getVertexAt(x+1, y), getVertexAt(x, y+1));
          newTriangleB = new Triangle(getVertexAt(x, y+1), getVertexAt(x+1, y), getVertexAt(x+1, y+1));
        }
      }
      else
      {
        if(yflip)
        {
          newTriangleA = new Triangle(getVertexAt(x, y+1), getVertexAt(x+1, y), getVertexAt(x+1, y+1));
          newTriangleB = new Triangle(getVertexAt(x, y), getVertexAt(x+1, y), getVertexAt(x, y+1));
        }
        else
        {
          newTriangleA = new Triangle(getVertexAt(x, y), getVertexAt(x+1, y), getVertexAt(x+1, y+1));
          newTriangleB = new Triangle(getVertexAt(x, y), getVertexAt(x+1, y+1), getVertexAt(x, y+1));
        }
      }

      newTriangleA->m_a->m_triangles.push_back(newTriangleA);
      newTriangleA->m_b->m_triangles.push_back(newTriangleA);
      newTriangleA->m_c->m_triangles.push_back(newTriangleA);

      newTriangleB->m_a->m_triangles.push_back(newTriangleB);
      newTriangleB->m_b->m_triangles.push_back(newTriangleB);
      newTriangleB->m_c->m_triangles.push_back(newTriangleB);

      m_triangles.push_back(newTriangleA);
      m_triangles.push_back(newTriangleB);

      xflip = !xflip;
    }
    yflip = !yflip;
  }
  
  // Upload data 
  // Create indices
  std::vector<uint32_t> indexData;
  for(Triangle* currTriangle : m_triangles)
  {
    indexData.push_back(currTriangle->m_c->m_id);
    indexData.push_back(currTriangle->m_b->m_id);
    indexData.push_back(currTriangle->m_a->m_id);
  }

  m_indexCount = (uint32_t)m_triangles.size() * 3;

  // Create vertices
  std::vector<float> vertexData;
  for(Vertex* currVertex : m_vertices)
  {
    vertexData.push_back(currVertex->m_position.x);
    vertexData.push_back(currVertex->m_position.z);
    vertexData.push_back(currVertex->m_uv.x);
    vertexData.push_back(currVertex->m_uv.y);
  }

  glBindVertexArray(m_glVertexArray);

  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(uint32_t), &indexData[0], GL_STATIC_DRAW);

  // Upload vertices 
  glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float) , &vertexData[0], GL_STATIC_DRAW);
  
  // Create buffers etc
  glm::vec2 mapResolutionf = glm::vec2(m_resolution.x, m_resolution.y) * m_xzScale * 8.0f;// 8 fragments per meter, gives us 1.25dm  precision
  glm::uvec2 mapResolution = glm::uvec2(mapResolutionf.x, mapResolutionf.y);
  m_arnxCache = new kit::PixelBuffer(mapResolution, {PixelBuffer::AttachmentInfo(Texture::RGBA8), PixelBuffer::AttachmentInfo(Texture::RGBA8)});
  m_materialMask = new kit::DoubleBuffer(mapResolution, { PixelBuffer::AttachmentInfo(Texture::RGBA8) , PixelBuffer::AttachmentInfo(Texture::RGBA8) });
  m_heightmap = new kit::DoubleBuffer(m_resolution, {PixelBuffer::AttachmentInfo(Texture::R16F)});

  // Clear material mask
  m_materialMask->getFrontBuffer()->clearAttachment(0, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
  m_materialMask->getBackBuffer()->clearAttachment(0, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

  m_materialMask->getFrontBuffer()->clearAttachment(1, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  m_materialMask->getBackBuffer()->clearAttachment(1, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

  // Clear heightmap
  m_heightmap->getFrontBuffer()->clearAttachment(0, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  m_heightmap->getBackBuffer()->clearAttachment(0, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

kit::EditorTerrain::EditorTerrain(const std::string&name, glm::uvec2 resolution, float xzScale, float yScale) : kit::EditorTerrain()
{
  m_name = name,
  m_resolution = resolution;
  m_yScale = yScale;
  m_xzScale = xzScale;
  
  // Initialize layers
  {
    m_numLayers = 1;
    m_layerInfo[0].material = kit::Material::load("Dirt.material");
  }

  // Create and initialize buffers etc.
  {
    generateCache();
    
    // Generate, compile and link GPU program
    updateGpuProgram();

    // Bake ARNX-cache
    bakeARNXCache();
  }

  m_valid = true;
}

void kit::EditorTerrain::reset(const std::string&name, glm::uvec2 resolution, float xzScale, float yScale)
{
  m_name = name;
  m_resolution = resolution;
  m_xzScale = xzScale;
  m_yScale = yScale;
  
  // Initialize layers
  {
    m_numLayers = 1;
    m_layerInfo[0].material = kit::Material::load("Dirt.material");
  }

  // Create and initialize buffers etc.
  {
    generateCache();
    
    // Generate, compile and link GPU program
    updateGpuProgram();
    
    // Bake ARNM-cache
    bakeARNXCache();
  }
}

kit::EditorTerrain::EditorTerrain(const std::string&name) : kit::EditorTerrain()
{

  m_name = name;
  m_resolution = glm::uvec2(4, 4);
  m_xzScale = 1.0f;
  m_yScale = 1.0f;
  
  std::stringstream terrainPath;
  terrainPath << "./data/terrains/" << name << "/";
  
  std::string headerPath = terrainPath.str() + "header";
  
  std::ifstream header(headerPath);
  if(!header)
  {
    KIT_THROW("Failed to load terrain, could not open headerfile");
  }
  
  std::string currline;
  while(std::getline(header, currline))
  {
    auto args =  kit::splitString(currline);
    if(args.size() == 2)
    {
      if(args[0] == "xzscale")
      {
        m_xzScale = (float)std::atof(args[1].c_str());
      }
      if(args[0] == "yscale")
      {
        m_yScale = (float)std::atof(args[1].c_str());
      }
      if(args[0] == "numlayers")
      {
        m_numLayers = std::atoi(args[1].c_str());
      }
    }
    if(args.size() == 3)
    {
      if(args[0] == "size")
      {
        m_resolution = glm::uvec2(std::atoi(args[1].c_str()), std::atoi(args[2].c_str()));
      }
    }
    if(args.size() == 3)
    {
      if(args[0] == "layer")
      {
        int currLayer = std::atoi(args[1].c_str());
        if(currLayer < 0 || currLayer > 3)
        {
          KIT_ERR("Warning: Invalid layer declaration in terrain data");
        }
        m_layerInfo[currLayer].material = kit::Material::load(args[3]);
      }
    }
  }
  
  header.close();

  // Create and initialize buffers etc.
  {
    generateCache();
    
    // Generate, compile and link GPU program
    updateGpuProgram();
    
    // Bake ARNM-cache
    bakeARNXCache();
  }
  
  m_valid = true;
  updateGpuProgram();
}

void kit::EditorTerrain::renderDeferred(kit::Renderer * renderer)
{
  if(!m_valid)
  {
    return;
  }

  glm::mat4 modelViewMatrix = renderer->getActiveCamera()->getViewMatrix() * getWorldTransformMatrix();
  glm::mat4 modelViewProjectionMatrix = renderer->getActiveCamera()->getProjectionMatrix() * renderer->getActiveCamera()->getViewMatrix() * getWorldTransformMatrix();

  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  m_program->use();
  m_program->setUniformMat4("uniform_mvMatrix", modelViewMatrix);
  m_program->setUniformMat4("uniform_mvpMatrix", modelViewProjectionMatrix);

  renderGeometry();
}

void kit::EditorTerrain::renderShadows(glm::mat4 const & viewMatrix, glm::mat4 const & projectionMatrix)
{
  glDisable(GL_CULL_FACE);
  m_shadowProgram->use();
  m_shadowProgram->setUniformMat4("uniform_mvpMatrix", projectionMatrix * viewMatrix * getWorldTransformMatrix());
  renderGeometry();
}

void kit::EditorTerrain::renderForward(kit::Renderer * renderer)
{
  if(!m_valid)
  {
    return;
  }
  
  if(m_decalBrush == nullptr)
  {
    return;
  }

  glm::mat4 modelViewProjectionMatrix = renderer->getActiveCamera()->getProjectionMatrix() * renderer->getActiveCamera()->getViewMatrix() * getWorldTransformMatrix();

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  m_decalProgram->use();
  m_decalProgram->setUniformMat4("uniform_mvpMatrix", modelViewProjectionMatrix);
  m_decalProgram->setUniform1f("uniform_whitepoint", renderer->getWhitepoint());

  renderGeometry();
  
  m_wireProgram->use();
  m_wireProgram->setUniformMat4("uniform_mvpMatrix", modelViewProjectionMatrix);
  m_wireProgram->setUniform1f("uniform_whitepoint", renderer->getWhitepoint());

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_POLYGON_OFFSET_LINE);
  glPolygonOffset(-1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  renderGeometry();
  glPolygonOffset(0.0f, 0.0f);
  glDisable(GL_POLYGON_OFFSET_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


  // Reset defaults!
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void kit::EditorTerrain::renderGeometry()
{
  if(!m_valid)
  {
    return;
  }

  glBindVertexArray(m_glVertexArray);
  glDrawElements( GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

void kit::EditorTerrain::updateGpuProgram()
{
  // First generate our default rendering program, and a pickbuffer frag shader
  {
    // Generate vertexshader sourcecode
    std::stringstream vertexSource;
    {
      // Header
      vertexSource << "#version 410 core" << std::endl;
      vertexSource << std::endl;

      // In attributes
      vertexSource << "layout (location = 0) in vec2 in_position;" << std::endl;
      vertexSource << "layout (location = 1) in vec2 in_texcoord;" << std::endl;
      vertexSource << std::endl;

      // Out attributes
      vertexSource << "layout (location = 0) out vec4 out_position;" << std::endl;
      vertexSource << "layout (location = 1) out vec2 out_texcoord;" << std::endl;
      vertexSource << "layout (location = 2) out vec3 out_normal;" << std::endl;
      vertexSource << std::endl;

      // Uniforms
      vertexSource << "uniform mat4 uniform_mvpMatrix;" << std::endl;
      vertexSource << "uniform mat4 uniform_mvMatrix;" << std::endl;
      vertexSource << "uniform sampler2D uniform_heightmap;" << std::endl;
      vertexSource << std::endl;

      // Sampling functions
      vertexSource << "vec2 flipY(vec2 innn)" << std::endl;
      vertexSource << "{" << std::endl;
      vertexSource << "  innn.y = 1.0 - innn.y;" << std::endl;
      vertexSource << "  return innn;" << std::endl;
      vertexSource << "}" << std::endl;
      
      vertexSource << "vec4 sampleHeight(vec2 position)" << std::endl;
      vertexSource << "{" << std::endl;
      vertexSource << "  vec2 tsize = vec2(" << m_resolution.x << ", " << m_resolution.y << ");" << std::endl;
      vertexSource << "  vec2 tstep = vec2(1.0 / tsize);" << std::endl;
      vertexSource << "  vec3 off = vec3(1.0, 1.0, 0.0);" << std::endl;
      vertexSource << "  float cHeight = texture(uniform_heightmap, flipY(((position + (tsize / 2.0)) *tstep) + tstep*0.5)).r;" << std::endl;
      
      vertexSource << "  float hL = texture(uniform_heightmap, flipY(((position - off.xz + (tsize / 2.0)) *tstep) + tstep*0.5)).r;" << std::endl;
      vertexSource << "  float hR = texture(uniform_heightmap, flipY(((position + off.xz + (tsize / 2.0)) *tstep) + tstep*0.5)).r;" << std::endl;
      vertexSource << "  float hD = texture(uniform_heightmap, flipY(((position - off.zy + (tsize / 2.0)) *tstep) + tstep*0.5)).r;" << std::endl;
      vertexSource << "  float hU = texture(uniform_heightmap, flipY(((position + off.xy + (tsize / 2.0)) *tstep) + tstep*0.5)).r;" << std::endl;
      vertexSource << "  return vec4( normalize(vec3(hL - hR , 1.0 / " << m_yScale << ", hD - hU)) , cHeight);" << std::endl;
      vertexSource << "}" << std::endl;

      // ---- Main code begins ---- //
      vertexSource << "void main()" << std::endl;
      vertexSource << "{" << std::endl;

      // Prepare variables
      vertexSource << "  vec2 sampleUv = vec2(in_position / " << m_xzScale << ");" << std::endl;
      vertexSource << "  vec4 hmSample   = sampleHeight(sampleUv);" << std::endl;
      vertexSource << "  vec4 position  = vec4(in_position.x, hmSample.w * " << m_yScale << ", in_position.y, 1.0);" << std::endl;
      vertexSource << std::endl;

      // Write attributes
      vertexSource << "  out_position   = uniform_mvMatrix * position;" << std::endl;
      vertexSource << "  out_normal     = (uniform_mvMatrix * vec4(normalize(hmSample.xyz), 0.0)).xyz;" << std::endl;
      vertexSource << "  out_texcoord   = in_texcoord;" << std::endl;
      vertexSource << std::endl;

      // Write gl_* variables
      vertexSource << "  gl_Position    = uniform_mvpMatrix * position;" << std::endl;

      // ---- Main code ends ---- //
      vertexSource << "}" << std::endl;
    }

    // Generate pixelshader sourcecode
    std::stringstream pixelSource;
    {
      // Header
      pixelSource << "#version 410 core" << std::endl;
      pixelSource << std::endl;
      
      // In attributes
      pixelSource << "layout (location = 0) in vec4 in_position;" << std::endl;
      pixelSource << "layout (location = 1) in vec2 in_texCoords;" << std::endl;
      pixelSource << "layout (location = 2) in vec3 in_normal;" << std::endl;
      pixelSource << std::endl;
      
      // Out attributes
      pixelSource << "layout (location = 0) out vec4 out_A;" << std::endl;
      pixelSource << "layout (location = 1) out vec4 out_B;" << std::endl;
      pixelSource << "layout (location = 2) out vec4 out_C;" << std::endl;
      pixelSource << "layout (location = 3) out vec4 out_D;" << std::endl;
      
      // Function to blend two textures smoothly based on a depthmap
      pixelSource << "vec4 blend2(vec4 below, vec4 above, float a2)" << std::endl;
      pixelSource << "{" << std::endl;
      pixelSource << "  float depth = 0.1;" << std::endl;
      pixelSource << std::endl;
      pixelSource << "  float f1 = 1.1;" << std::endl;
      pixelSource << "  float f2 = a2;" << std::endl;
      pixelSource << "  float ma = max(f1, f2) - depth;" << std::endl;
      pixelSource << std::endl;
      pixelSource << "  float b1 = max(f1 - ma, 0);" << std::endl;
      pixelSource << "  float b2 = max(f2 - ma, 0);" << std::endl;
      pixelSource << std::endl;
      pixelSource << "  return (below.rgba * b1 + above.rgba * b2) / (b1 + b2);" << std::endl;
      pixelSource << "}" << std::endl;
      pixelSource << std::endl;

      // Uniforms
      if (m_numLayers > 1)
      {
        pixelSource << "uniform sampler2D uniform_materialMask0;" << std::endl;
      }

      if (m_numLayers > 4)
      {
        pixelSource << "uniform sampler2D uniform_materialMask1;" << std::endl;
      }

      pixelSource << "uniform sampler2D uniform_arCache;" << std::endl;
      pixelSource << "uniform float uniform_detailDistance;" << std::endl;
      pixelSource << std::endl;

      // Layerspecific uniforms
      for(int i = 0; i < m_numLayers; i++)
      {
        pixelSource << "uniform sampler2D uniform_arLayer" << i << ";" << std::endl;

        if (i != 0)
        {
          pixelSource << "uniform sampler2D uniform_ndLayer" << i << ";" << std::endl;
        }

        pixelSource << std::endl;
      }

      // ---- Main code begins ---- //
      pixelSource << "void main()" << std::endl;
      pixelSource << "{" << std::endl;

      // Prepare variables
      pixelSource << "  vec2 fullUv = in_texCoords;" << std::endl;
      pixelSource << "  vec2 detailUv = in_texCoords * vec2(" << float(m_resolution.x) * m_xzScale << ", " << float(m_resolution.y) * m_xzScale << ");" << std::endl;
      pixelSource << "  float linearDistance = distance(vec3(0.0), in_position.xyz / in_position.w);" << std::endl;

      // Prepare output variables
      pixelSource << "  vec4 arOut; " << std::endl;

      // If the distance to the fragment is higher than the detail distance, output the cached AR and NM maps
      pixelSource << "  if (linearDistance > uniform_detailDistance)" << std::endl;
      pixelSource << "  {" << std::endl;
      pixelSource << "    arOut = texture(uniform_arCache, fullUv);" << std::endl;
      pixelSource << "  }" << std::endl;

      // Otherwise, do some magic
      pixelSource << "  else" << std::endl;
      pixelSource << "  {" << std::endl;

      // Sample the materialmasks
      if (m_numLayers > 1)
      {
        pixelSource << "    vec4 materialMask0 = texture(uniform_materialMask0, fullUv);" << std::endl;
      }

      if (m_numLayers > 4)
      {
        pixelSource << "    vec4 materialMask1 = texture(uniform_materialMask1, fullUv);" << std::endl;
      }

      // Sample the layer maps
      for (int i = 0; i < m_numLayers; i++)
      {
        pixelSource << "    vec4 ar" << i << " = texture(uniform_arLayer" << i << ", detailUv * " << m_layerInfo[i].material->getUvScale() << ");" << std::endl;

        if (i == 0)
        {
          pixelSource << "    arOut = ar0;" << std::endl;
        }
        else
        {
          pixelSource << "    float d" << i << " = texture(uniform_ndLayer" << i << ", detailUv * " << m_layerInfo[i].material->getUvScale() << ").a;" << std::endl;
          pixelSource << "    arOut = blend2(arOut, ar" << i << ", materialMask" << getMaskSuffix(i) << " + d" << i << ");" << std::endl;
        }
      }

      // We're done with generating output
      pixelSource << "  }" << std::endl;
      pixelSource << std::endl;

      // Write output
      pixelSource << "  out_A = arOut;" << std::endl;
      pixelSource << "  out_B = vec4(0.0, 0.0, 0.0, 0.2);" << std::endl;
      pixelSource << "  out_C.xyz = normalize(in_normal);" << std::endl;
      pixelSource << "  out_C.a = 1.0;" << std::endl;

      // Mission complete!
      pixelSource << "}" << std::endl;
    }

    std::stringstream pickSource;
    {
       // Header
      pickSource << "#version 410 core" << std::endl;
      pickSource << std::endl;
      
      // In attributes
      pickSource << "layout (location = 0) in vec4 in_position;" << std::endl;
      pickSource << "layout (location = 1) in vec2 in_texCoords;" << std::endl;
      pickSource << std::endl;
      
      // Out attributes
      pickSource << "layout (location = 0) out vec4 out_A;" << std::endl;
      pickSource << "layout (location = 1) out vec4 out_B;" << std::endl;

      // ---- Main code begins ---- //
      pickSource << "void main()" << std::endl;
      pickSource << "{" << std::endl;

      // Write output
      pickSource << "  out_A = vec4(in_position.xyz, 1.0);" << std::endl;
      pickSource << "  out_B = vec4(in_texCoords, 1.0, 1.0);" << std::endl;

      // Mission complete!
      pickSource << "}" << std::endl;
    }
    
    std::stringstream decalSource;
    {
       // Header
      decalSource << "#version 410 core" << std::endl;
      decalSource << std::endl;
      
      // In attributes
      decalSource << "layout (location = 1) in vec2 in_texCoords;" << std::endl;
      decalSource << std::endl;
      
      // Out attributes
      decalSource << "layout (location = 0) out vec4 out_color;" << std::endl;

      // Uniforms
      decalSource << "uniform sampler2D uniform_brush;" << std::endl;
      decalSource << "uniform vec2 uniform_brushSize;" << std::endl;
      decalSource << "uniform vec2 uniform_brushPos;" << std::endl;
      decalSource << "uniform float uniform_whitepoint;" << std::endl;
      
      // ---- Main code begins ---- //
      decalSource << "void main()" << std::endl;
      decalSource << "{" << std::endl;

      decalSource << "  out_color = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
      
      decalSource << "  if(  in_texCoords.x > uniform_brushPos.x && in_texCoords.x < uniform_brushPos.x + uniform_brushSize.x" << std::endl;
      decalSource << "  && in_texCoords.y > uniform_brushPos.y && in_texCoords.y < uniform_brushPos.y + uniform_brushSize.y)" << std::endl;
      decalSource << "  {" << std::endl;
      decalSource << "    vec2 brushUv;" << std::endl;
      decalSource << "    brushUv = (in_texCoords / uniform_brushSize) - (uniform_brushPos / uniform_brushSize);" << std::endl;
      decalSource << "    float currBrush = (1.0 - texture(uniform_brush, brushUv ).r);" << std::endl;
      decalSource << "    out_color = vec4(vec3(currBrush) * uniform_whitepoint * 0.75, 1.0);" << std::endl;
      decalSource << "  }" << std::endl;


      // Mission complete!
      decalSource << "}" << std::endl;
    }
    
    std::stringstream wireSource;
    {
      // Header
      wireSource << "#version 410 core" << std::endl;
      wireSource << std::endl;

      wireSource << "layout (location = 2) in vec3 in_normal;" << std::endl;

      // Out attributes
      wireSource << "layout (location = 0) out vec4 out_color;" << std::endl;

      // Uniforms
      wireSource << "uniform float uniform_whitepoint;" << std::endl;

      // ---- Main code begins ---- //
      wireSource << "void main()" << std::endl;
      wireSource << "{" << std::endl;

      wireSource << "  float ndotv = max(0.01, dot(vec3(0.0, 0.0, -1.0), normalize(in_normal)));" << std::endl;

      wireSource << "  out_color = vec4(vec3(uniform_whitepoint) * ndotv, 1.0);" << std::endl;

      // Mission complete!
      wireSource << "}" << std::endl;
    }

    std::stringstream shadowSource;
    {
      shadowSource << "#version 410 core" << std::endl;

      shadowSource << "void main()" << std::endl;
      shadowSource << "{" << std::endl;
      shadowSource << "  gl_FragDepth = gl_FragCoord.z;" << std::endl;
      shadowSource << "}" << std::endl;
    }
    
    // Compile shader objects
    auto vertexShader = new kit::Shader(Shader::Type::Vertex);
    vertexShader->sourceFromString(vertexSource.str());
    vertexShader->compile();

    auto pixelShader = new kit::Shader(Shader::Type::Fragment);
    pixelShader->sourceFromString(pixelSource.str());
    pixelShader->compile();
    
    auto pickShader = new kit::Shader(Shader::Type::Fragment);
    pickShader->sourceFromString(pickSource.str());
    pickShader->compile();

    auto decalShader = new kit::Shader(Shader::Type::Fragment);
    decalShader->sourceFromString(decalSource.str());
    decalShader->compile();

    auto wireShader = new kit::Shader(Shader::Type::Fragment);
    wireShader->sourceFromString(wireSource.str());
    wireShader->compile();
    
    auto shadowShader = new kit::Shader(Shader::Type::Fragment);
    shadowShader->sourceFromString(shadowSource.str());
    shadowShader->compile();
    
    // Link program
    if(m_program)
      delete m_program;
    
    m_program = new kit::Program();
    m_program->attachShader(vertexShader);
    m_program->attachShader(pixelShader);
    m_program->link();
    m_program->detachShader(vertexShader);
    m_program->detachShader(pixelShader);

    // Update uniforms
    if (m_numLayers > 1)
    {
      m_program->setUniformTexture("uniform_materialMask0", m_materialMask->getFrontBuffer()->getColorAttachment(0));
    }

    if (m_numLayers > 4)
    {
      m_program->setUniformTexture("uniform_materialMask1", m_materialMask->getFrontBuffer()->getColorAttachment(1));
    }

    m_program->setUniformTexture("uniform_arCache", m_arnxCache->getColorAttachment(0));
    m_program->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
    m_program->setUniform1f("uniform_detailDistance", 25.0f); ///< TODO: Replace with configuration parameter

    // Layer-specific uniforms
    for(int i = 0; i < m_numLayers; i++)
    {
      m_program->setUniformTexture("uniform_arLayer" + std::to_string(i), m_layerInfo[i].material->getARCache());

      if (i != 0)
      {
        m_program->setUniformTexture("uniform_ndLayer" + std::to_string(i), m_layerInfo[i].material->getNDCache());
      }
    }
    
    // Link picking program
    if(m_pickProgram)
      delete m_pickProgram;
    
    m_pickProgram = new kit::Program();
    m_pickProgram->attachShader(vertexShader);
    m_pickProgram->attachShader(pickShader);
    m_pickProgram->link();
    m_pickProgram->detachShader(vertexShader);
    m_pickProgram->detachShader(pickShader);
    
    m_pickProgram->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
    
    // Link decal program
    if(m_decalProgram)
      delete m_decalProgram;
    
    m_decalProgram = new kit::Program();
    m_decalProgram->attachShader(vertexShader);
    m_decalProgram->attachShader(decalShader);
    m_decalProgram->link();
    m_decalProgram->detachShader(vertexShader);
    m_decalProgram->detachShader(decalShader);
    m_decalProgram->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
    
    // Link wire program
    if(m_wireProgram)
      delete m_wireProgram;
    
    m_wireProgram = new kit::Program();
    m_wireProgram->attachShader(vertexShader);
    m_wireProgram->attachShader(wireShader);
    m_wireProgram->link();
    m_wireProgram->detachShader(vertexShader);
    m_wireProgram->detachShader(wireShader);
    m_wireProgram->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));

    // Link shadow program
    if(m_shadowProgram)
      delete m_shadowProgram;
    
    m_shadowProgram = new kit::Program();
    m_shadowProgram->attachShader(vertexShader);
    m_shadowProgram->attachShader(decalShader);
    m_shadowProgram->link();
    m_shadowProgram->detachShader(vertexShader);
    m_shadowProgram->detachShader(decalShader);
    m_shadowProgram->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
    
    delete vertexShader;
    delete pixelShader;
    delete pickShader;
    delete decalShader;
    delete wireShader;
    delete shadowShader;

  }
  
  // Then render our ARNX baking program
  {
    // Generate vertexshader sourcecode
    std::stringstream vertexSource;
    {
      // Header
      vertexSource << "#version 410 core" << std::endl;
      vertexSource << std::endl;

      // In attributes
      vertexSource << "const vec2 quadVertices[4] = vec2[4]( vec2( -1.0, -1.0), vec2( 1.0, -1.0), vec2( -1.0, 1.0), vec2( 1.0, 1.0));" << std::endl;
      vertexSource << "const vec2 quadCoords[4]   = vec2[4]( vec2( 0.0, 1.0),   vec2( 1.0, 1.0),   vec2( 0.0, 0.0), vec2( 1.0, 0.0));" << std::endl;
      vertexSource << std::endl;

      // Out attributes
      vertexSource << "layout (location = 0) out vec2 out_texCoords;" << std::endl;
      vertexSource << std::endl;

      // ---- Main code begins ---- //
      vertexSource << "void main()" << std::endl;
      vertexSource << "{" << std::endl;

      // Write attributes
      vertexSource << "  out_texCoords   = vec2(quadCoords[gl_VertexID]);" << std::endl;
      vertexSource << "  out_texCoords.y = 1.0 - out_texCoords.y;" << std::endl;
      vertexSource << std::endl;

      // Write gl_* variables
      vertexSource << "  gl_Position    = vec4(quadVertices[gl_VertexID], 0.0, 1.0);" << std::endl;

      // ---- Main code ends ---- //
      vertexSource << "}" << std::endl;
    }

    // Generate pixelshader sourcecode
    std::stringstream pixelSource;
    {
      // Header
      pixelSource << "#version 410 core" << std::endl;
      pixelSource << std::endl;
      
      // In attributes
      pixelSource << "layout (location = 0) in vec2 in_texCoords;" << std::endl;
      pixelSource << std::endl;
      
      // Out attributes
      pixelSource << "layout (location = 0) out vec4 out_AR;" << std::endl;
      pixelSource << "layout (location = 1) out vec4 out_NM;" << std::endl;
      
      // Function to blend two textures smoothly based on a depthmap
      pixelSource << "vec4 blend2(vec4 below, vec4 above, float a2)" << std::endl;
      pixelSource << "{" << std::endl;
      pixelSource << "  float depth = 0.1;" << std::endl;
      pixelSource << std::endl;
      pixelSource << "  float f1 = 1.1;" << std::endl;
      pixelSource << "  float f2 = a2;" << std::endl;
      pixelSource << "  float ma = max(f1, f2) - depth;" << std::endl;
      pixelSource << std::endl;
      pixelSource << "  float b1 = max(f1 - ma, 0);" << std::endl;
      pixelSource << "  float b2 = max(f2 - ma, 0);" << std::endl;
      pixelSource << std::endl;
      pixelSource << "  return (below.rgba * b1 + above.rgba * b2) / (b1 + b2);" << std::endl;
      pixelSource << "}" << std::endl;
      pixelSource << std::endl;
      
      // Uniforms
      if (m_numLayers > 1)
      {
        pixelSource << "uniform sampler2D uniform_materialMask0;" << std::endl;
      }
      if (m_numLayers > 4)
      {
        pixelSource << "uniform sampler2D uniform_materialMask1;" << std::endl;
      }
      pixelSource << std::endl;

      // Layerspecific uniforms
      for(int i = 0; i < m_numLayers; i++)
      {
        pixelSource << "uniform sampler2D uniform_arLayer" << i << ";" << std::endl;
        pixelSource << "uniform sampler2D uniform_ndLayer" << i << ";" << std::endl;
        pixelSource << std::endl;
      }

      // ---- Main code begins ---- //
      pixelSource << "void main()" << std::endl;
      pixelSource << "{" << std::endl;

      // Prepare variables
      pixelSource << "  vec2 fullUv = in_texCoords;" << std::endl;
      pixelSource << "  vec2 detailUv = in_texCoords * vec2(" << float(m_resolution.x) * m_xzScale << ", " << float(m_resolution.y) * m_xzScale << ");" << std::endl;

      // Prepare output variables
      pixelSource << "  vec4 arOut; " << std::endl;
      pixelSource << "  vec4 nxOut; " << std::endl;

      // Sample the materialmasks
      if (m_numLayers > 1)
      {
        pixelSource << "    vec4 materialMask0 = texture(uniform_materialMask0, fullUv);" << std::endl;
      }

      if (m_numLayers > 4)
      {
        pixelSource << "    vec4 materialMask1 = texture(uniform_materialMask1, fullUv);" << std::endl;
      }

      // Sample the layer maps
      for (int i = 0; i < m_numLayers; i++)
      {
        pixelSource << "    vec4 ar" << i << " = texture(uniform_arLayer" << i << ", detailUv * " << m_layerInfo[i].material->getUvScale() << ");" << std::endl;
        pixelSource << "    vec4 nd" << i << " = texture(uniform_ndLayer" << i << ", detailUv * " << m_layerInfo[i].material->getUvScale() << ");" << std::endl;

        if (i == 0)
        {
          pixelSource << "    arOut = ar0;" << std::endl;
          pixelSource << "    nxOut = nd0;" << std::endl;
        }
        else
        {
          pixelSource << "    arOut = blend2(arOut, ar" << i << ", materialMask" << getMaskSuffix(i) << " + nd" << i << ".a);" << std::endl;
          pixelSource << "    nxOut = blend2(nxOut, nd" << i << ", materialMask" << getMaskSuffix(i) << " + nd" << i << ".a);" << std::endl;
        }
      }

      // Write output
      pixelSource << "  out_AR = arOut;" << std::endl;
      pixelSource << "  out_NM = nxOut;" << std::endl;

      // Mission complete!
      pixelSource << "}" << std::endl;
    }

    // Compile shader objects
    auto vertexShader = new kit::Shader(Shader::Type::Vertex);
    vertexShader->sourceFromString(vertexSource.str());
    vertexShader->compile();

    auto pixelShader = new kit::Shader(Shader::Type::Fragment);
    pixelShader->sourceFromString(pixelSource.str());
    pixelShader->compile();

    // Link program
    if(m_bakeProgramArnx)
      delete m_bakeProgramArnx;
    
    m_bakeProgramArnx = new kit::Program();
    m_bakeProgramArnx->attachShader(vertexShader);
    m_bakeProgramArnx->attachShader(pixelShader);
    m_bakeProgramArnx->link();
    m_bakeProgramArnx->detachShader(vertexShader);
    m_bakeProgramArnx->detachShader(pixelShader);

    delete vertexShader;
    delete pixelShader;
    
    // Update uniforms
    if (m_numLayers > 1)
    {
      m_bakeProgramArnx->setUniformTexture("uniform_materialMask0", m_materialMask->getFrontBuffer()->getColorAttachment(0));
    }
    if (m_numLayers > 4)
    {
      m_bakeProgramArnx->setUniformTexture("uniform_materialMask1", m_materialMask->getFrontBuffer()->getColorAttachment(1));
    }


    // Layer-specific uniforms
    for(int i = 0; i < m_numLayers; i++)
    {
      m_bakeProgramArnx->setUniformTexture("uniform_arLayer" + std::to_string(i), m_layerInfo[i].material->getARCache());
      m_bakeProgramArnx->setUniformTexture("uniform_ndLayer" + std::to_string(i), m_layerInfo[i].material->getNDCache());
    }
  }
}

void kit::EditorTerrain::bakeCPUNormals()
{
  // Calculate triangle normals and tangents
  for(Triangle *currTriangle : m_triangles)
  {
    // Calculate face normal
    glm::vec3 u(currTriangle->m_b->m_position - currTriangle->m_c->m_position);
    glm::vec3 v(currTriangle->m_a->m_position - currTriangle->m_c->m_position);

    currTriangle->m_normal.x = (u.y * v.z) - (u.z * v.y);
    currTriangle->m_normal.y = (u.z * v.x) - (u.x * v.z);
    currTriangle->m_normal.z = (u.x * v.y) - (u.y * v.x);

    // Calculate face tangents and bitangents
    glm::vec2 s(currTriangle->m_b->m_uv - currTriangle->m_c->m_uv);
    glm::vec2 t(currTriangle->m_a->m_uv - currTriangle->m_c->m_uv);

    float      r = 1.0f / ((s.x * t.y) - (s.y * t.x));
    glm::vec3  sDir((t.y * u.x - t.x * v.x) * r, (t.y * u.y - t.x * v.y) * r, (t.y * u.z - t.x * v.z) * r);
    glm::vec3  tDir((s.x * v.x - s.y * u.x) * r, (s.x * v.y - s.y * u.y) * r, (s.x * v.z - s.y * u.z) * r);

    // Gram-Schmidt orthogonalize
    glm::vec3 tangent = sDir - (currTriangle->m_normal * glm::dot(currTriangle->m_normal, sDir));
    tangent = glm::normalize(tangent);

    // Calculate handedness
    float tangentDir = (glm::dot(glm::cross(currTriangle->m_normal, sDir), tDir) >= 0.0f) ? 1.0f : -1.0f;  
    glm::vec3 bitangent = glm::cross(currTriangle->m_normal, tangent) * tangentDir;
    
    currTriangle->m_tangent = tangent;
    currTriangle->m_bitangent = bitangent;
  }
  
  // Calculate vertex normals and tangents
  for(Vertex *currVertex : m_vertices)
  {
    for(Triangle *currTriangle : currVertex->m_triangles)
    {
      currVertex->m_normal += currTriangle->m_normal;
      currVertex->m_tangent += currTriangle->m_tangent;
      currVertex->m_bitangent += currTriangle->m_bitangent;
    }
    currVertex->m_normal /= float(currVertex->m_triangles.size());
    currVertex->m_tangent /= float(currVertex->m_triangles.size());
    currVertex->m_bitangent /= float(currVertex->m_triangles.size());
  }
}

void kit::EditorTerrain::bakeCPUHeight()
{
  auto pixelData = m_heightmap->getFrontBuffer()->readPixels(0);

  for(uint32_t i = 0; i < m_resolution.x * m_resolution.y; i++)
  {
    m_vertices[i]->m_position.y = pixelData[i] * m_yScale;
  }
}

kit::EditorTerrain::Vertex *kit::EditorTerrain::getVertexAt(int32_t x, int32_t y)
{

  if((uint32_t)x >= m_resolution.x)
  {
    x = m_resolution.x-1;
  }
  if(x < 0)
  {
    x = 0;
  }
  
  if((uint32_t)y >= m_resolution.y)  {
    y = m_resolution.y-1;
  }
  
  if(y < 0)
  {
    y = 0;
  }
  
  uint32_t index = (y*m_resolution.x) + x;
  
  if(index >= m_vertices.size())
  {
    KIT_THROW("Invalid vertex index");
  }
  
  return m_vertices[index];
}

glm::vec3 kit::EditorTerrain::sampleHeightmap(int32_t x, int32_t y)
{
  glm::vec2 fullSize;
  fullSize.x = float(m_resolution.x) * m_xzScale;
  fullSize.y = float(m_resolution.y) * m_xzScale;

  glm::vec2 halfSize = fullSize / 2.0f;

  x = (glm::min)(m_resolution.x-1, (unsigned int)x);
  y = (glm::min)(m_resolution.y-1, (unsigned int)y);
  
  float ry = m_heightmap->getFrontBuffer()->readPixel(0, x, m_resolution.y - 1 - y).x * m_yScale;
  float rx = (float(x) * m_xzScale) - halfSize.x;
  float rz = (float(y) * m_xzScale) - halfSize.y;
  
  return glm::vec3(rx, ry, rz);
  
}

glm::vec3 kit::EditorTerrain::sampleBilinear(float x, float z)
{
  glm::vec2 fullSize;
  fullSize.x = float(m_resolution.x) * m_xzScale;
  fullSize.y = float(m_resolution.y) * m_xzScale;

  glm::vec2 halfSize = fullSize / 2.0f;
  
  x += halfSize.x;
  z += halfSize.y;
  
  float xMap = x / m_xzScale;
  float zMap = z / m_xzScale;
  
  uint32_t px = (uint32_t)glm::floor(xMap);
  uint32_t pz = (uint32_t)glm::floor(zMap);
  
  uint32_t hx = px + 1;
  uint32_t hz = pz + 1;
  
  auto v1 = sampleHeightmap(px, pz);
  auto v2 = sampleHeightmap(hx, pz);
  auto v3 = sampleHeightmap(px, hz);
  auto v4 = sampleHeightmap(hx, hz);
  

  
  // Calculate the weights for each pixel
  float fx = (xMap - (float)px);
  float fz = (zMap - (float)pz);
  float fx1 = 1.0f - fx;
  float fz1 = 1.0f - fz;

  float w1 = fx1 * fz1;
  float w2 = fx  * fz1;
  float w3 = fx1 * fz;
  float w4 = fx  * fz;
  
  float height = v1.y * w1 + v2.y * w2 + v3.y * w3 + v4.y * w4;
  
  //std::cout << "Weights are "  << w1 << " " << w2 << " " << w3 << " " << w4 << std::endl;
  //std::cout << "Height is " << height << " from " << v1.y << " " << v2.y << " " << v3.y << " " << v4.y << std::endl;
  
  // Calculate euler x and z rotations to set origins direction instead (yes pas)
  // First calculate angles of the edges
  
  glm::vec2 sv1 = glm::vec2(v1.x, v1.y);
  glm::vec2 sv2 = glm::vec2(v2.x, v2.y);
  glm::vec2 sv3 = glm::vec2(v3.x, v3.y);
  glm::vec2 sv4 = glm::vec2(v4.x, v4.y);
  
  glm::vec2 dv1 = glm::vec2(v1.z, v1.y);
  glm::vec2 dv2 = glm::vec2(v2.z, v2.y);
  glm::vec2 dv3 = glm::vec2(v3.z, v3.y);
  glm::vec2 dv4 = glm::vec2(v4.z, v4.y);
  
  glm::vec2 dtop = glm::normalize(sv2 - sv1);
  glm::vec2 dbottom = glm::normalize(sv4 - sv3);
  glm::vec2 dleft = glm::normalize(dv3 - dv1);
  glm::vec2 dright = glm::normalize(dv4 - dv2);
  
  glm::vec2 reference = glm::normalize(glm::vec2(1.0, 0.0));
  
  float top     = glm::orientedAngle(reference, dtop);
  float bottom  = glm::orientedAngle(reference, dbottom);
  float left    = glm::orientedAngle(dleft, reference);
  float right   = glm::orientedAngle(dright, reference);

  float zAngle = glm::degrees(glm::mix(top, bottom, fz));
  float xAngle = glm::degrees(glm::mix(left, right, fx));
  
  return glm::vec3(height, xAngle, zAngle);
}

void kit::EditorTerrain::bakeARNXCache()
{
  m_arnxCache->clearAttachment(0, glm::vec4(0.5f, 0.5f, 0.5f, 0.8f));
  m_arnxCache->clearAttachment(1, glm::vec4(0.0f, 0.0f, 1.0f, 0.2f));

  m_bakeProgramArnx->use();
  m_arnxCache->bind();
  
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

kit::Texture * kit::EditorTerrain::getARCache()
{
  return m_arnxCache->getColorAttachment(0);
}

kit::Texture * kit::EditorTerrain::getNXCache()
{
  return m_arnxCache->getColorAttachment(1);
}

kit::Texture * kit::EditorTerrain::getHeightmap()
{
  return m_heightmap->getFrontBuffer()->getColorAttachment(0);
}

void kit::EditorTerrain::renderPickbuffer(kit::Camera * cam)
{

  glm::mat4 modelViewProjectionMatrix = cam->getProjectionMatrix() * cam->getViewMatrix() * getWorldTransformMatrix();
  glm::mat4 modelViewMatrix = cam->getViewMatrix() * getWorldTransformMatrix();

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  m_pickProgram->use();
  m_pickProgram->setUniformMat4("uniform_mvpMatrix", modelViewProjectionMatrix);
  m_pickProgram->setUniformMat4("uniform_mvMatrix", modelViewMatrix);

  renderGeometry();
}

void kit::EditorTerrain::setDecalBrush(kit::Texture * brush, glm::vec2 positionUv, glm::vec2 sizeUv)
{
  m_decalBrush = brush;
  m_decalBrushPosition = positionUv;
  m_decalBrushSize = sizeUv;
  
  if(m_decalBrush != nullptr)
  {
    m_decalProgram->setUniform2f("uniform_brushPos", positionUv);
    m_decalProgram->setUniform2f("uniform_brushSize", sizeUv);
    m_decalProgram->setUniformTexture("uniform_brush", m_decalBrush);
  }
  
}

void kit::EditorTerrain::paintMaterialMask(uint8_t layerid, kit::Texture * brush, glm::vec2 positionUv, glm::vec2 sizeUv, PaintOperation op, float strength)
{
  //int compIndex = layerid > 3 ? layerid - 4 : layerid;

  m_materialPaintProgram->use();
  m_materialPaintProgram->setUniform1i("uniform_currLayer", layerid);
  m_materialPaintProgram->setUniform1i("uniform_opMode", (int)op);
  m_materialPaintProgram->setUniform2f("uniform_brushPos", positionUv);
  m_materialPaintProgram->setUniform2f("uniform_brushSize", sizeUv);
  m_materialPaintProgram->setUniformTexture("uniform_materialMask0", m_materialMask->getFrontBuffer()->getColorAttachment(0));
  m_materialPaintProgram->setUniformTexture("uniform_materialMask1", m_materialMask->getFrontBuffer()->getColorAttachment(1));
  m_materialPaintProgram->setUniformTexture("uniform_brush", brush);
  m_materialPaintProgram->setUniform1f("uniform_strength", strength);

  m_materialMask->getBackBuffer()->bind();
  m_materialMask->getBackBuffer()->clearAttachment(0, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  m_materialMask->getBackBuffer()->clearAttachment(1, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  m_materialMask->flip();
  m_materialMask->getFrontBuffer()->getColorAttachment(0)->generateMipmap();
  m_materialMask->getFrontBuffer()->getColorAttachment(1)->generateMipmap();


  if (m_numLayers > 1)
  {
    m_program->setUniformTexture("uniform_materialMask0", m_materialMask->getFrontBuffer()->getColorAttachment(0));
    m_bakeProgramArnx->setUniformTexture("uniform_materialMask0", m_materialMask->getFrontBuffer()->getColorAttachment(0));
  }
  if (m_numLayers > 4)
  {
    m_program->setUniformTexture("uniform_materialMask1", m_materialMask->getFrontBuffer()->getColorAttachment(1));
    m_bakeProgramArnx->setUniformTexture("uniform_materialMask1", m_materialMask->getFrontBuffer()->getColorAttachment(1));
  }

  bakeARNXCache();
}

void kit::EditorTerrain::paintHeightmap(kit::Texture * brush, glm::vec2 positionUv, glm::vec2 sizeUv, PaintOperation op, float strength)
{

  m_heightPaintProgram->use();
  m_heightPaintProgram->setUniform1i("uniform_opMode", (int)op);
  m_heightPaintProgram->setUniform2f("uniform_brushPos", positionUv);
  m_heightPaintProgram->setUniform2f("uniform_brushSize", sizeUv);
  m_heightPaintProgram->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
  m_heightPaintProgram->setUniformTexture("uniform_brush", brush);
  m_heightPaintProgram->setUniform1f("uniform_yScale", m_yScale);
  m_heightPaintProgram->setUniform1f("uniform_strength", strength);

  m_heightmap->getBackBuffer()->bind();
  m_heightmap->getBackBuffer()->clearAttachment(0, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  m_heightmap->flip();
  m_heightmap->getFrontBuffer()->getColorAttachment(0)->generateMipmap();
  m_program->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
  m_wireProgram->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
  m_pickProgram->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
  m_shadowProgram->setUniformTexture("uniform_heightmap", m_heightmap->getFrontBuffer()->getColorAttachment(0));
}

glm::uvec2 kit::EditorTerrain::getResolution()
{
  return m_resolution;
}

float kit::EditorTerrain::getXZScale()
{
  return m_xzScale;
}

glm::vec2 kit::EditorTerrain::getWorldSize()
{
  return glm::vec2((float)m_resolution.x, (float)m_resolution.y) * m_xzScale;
}

void kit::EditorTerrain::bake()
{
  bakeARNXCache();
  bakeCPUHeight();
  bakeCPUNormals();

  std::stringstream terrainPath;
  terrainPath << "./data/terrains/" << m_name;

  std::stringstream bakedPath;
  bakedPath << terrainPath.str() << "/baked";

  if(!kit::createDirectory(terrainPath.str()))
  {
    KIT_ERR("Failed to bake terrain, couldn't create terrain path");
    return;
  }

  if(!kit::createDirectory(bakedPath.str()))
  {
    KIT_ERR("Failed to bake terrain, couldn't create baked path");
    return;
  }

  m_arnxCache->getColorAttachment(0)->saveToFile(bakedPath.str() + std::string("/arcache.tga"));
  m_arnxCache->getColorAttachment(1)->saveToFile(bakedPath.str() + std::string("/nxcache.tga"));

  if (m_numLayers > 1)
  {
    m_materialMask->getFrontBuffer()->getColorAttachment(0)->saveToFile(bakedPath.str() + std::string("/materialmask0.tga"));
  }
  if (m_numLayers > 4)
  {
    m_materialMask->getFrontBuffer()->getColorAttachment(1)->saveToFile(bakedPath.str() + std::string("/materialmask1.tga"));
  }

  for(int i = 0; i < m_numLayers; i++)
  {
    if(m_layerInfo[i].material == nullptr)
    {
      KIT_ERR("Failed to bake terrain, layer is missing material");
      return;
    }
    
    m_layerInfo[i].material->getARCache()->saveToFile(bakedPath.str() + std::string("/arlayer") + std::to_string(i) + std::string(".tga"));
    m_layerInfo[i].material->getNDCache()->saveToFile(bakedPath.str() + std::string("/ndlayer") + std::to_string(i) + std::string(".tga"));
  }
  
  std::ofstream header(bakedPath.str() + std::string("/header"));
  if(!header)
  {
    KIT_ERR("Failed to bake terrain, could not create headerfile");
    return;
  }
  
  header << "xzscale " << m_xzScale << std::endl;
  header << "yscale " << m_yScale << std::endl;
  header << "numlayers " << (int)m_numLayers << std::endl;
  header << "size " << m_resolution.x << " " << m_resolution.y << std::endl;
  for(int i = 0; i < m_numLayers; i++)
  {
    header << "layer " << i << " " << m_layerInfo[i].material->getUvScale() << std::endl;
  }
  header.close();
  

  // WRITE VERTEXDATA FOR GPU
  std::ofstream data(bakedPath.str() + std::string("/vertexdata"), std::ios_base::binary);
  if(!data)
  {
    KIT_ERR("Failed to bake terrain, could not create vertexdata-file");
  }

  // Write indices
  kit::writeUint32(data, uint32_t(m_triangles.size() * 3));
  for (Triangle * currTriangle : m_triangles)
  {
    kit::writeUint32(data, currTriangle->m_c->m_id);
    kit::writeUint32(data, currTriangle->m_b->m_id);
    kit::writeUint32(data, currTriangle->m_a->m_id);
  }

  // Write vertices
  kit::writeUint32(data, uint32_t(m_vertices.size() * 14));
  for (Vertex * currVertex : m_vertices)
  {
    kit::writeVec3(data, currVertex->m_position);
    kit::writeVec2(data, currVertex->m_uv);
    kit::writeVec3(data, currVertex->m_normal);
    kit::writeVec3(data, currVertex->m_tangent);
    kit::writeVec3(data, currVertex->m_bitangent);
  }
  data.close();
  
  
  
  // WRITE HEIGHTDATA FOR CPU
  std::ofstream hdata(bakedPath.str() + std::string("/heightdata"), std::ios_base::out | std::ios_base::binary);
  if(!hdata)
  {
    KIT_ERR("Failed to bake terrain, could not create heightdata-file");
  }
  
  for (uint32_t y = 0; y < m_resolution.y; y++)
  {
    for (uint32_t x = 0; x < m_resolution.x; x++)
    {
      auto * v = getVertexAt(x, y);
      float h = v->m_position.y / m_yScale;

      kit::writeFloat(hdata, h);
      kit::writeVec3(hdata, v->m_normal);
    }
  }

  hdata.close();
}

void kit::EditorTerrain::save()
{
  std::stringstream terrainPath;
  terrainPath << "./data/terrains/" << m_name;

  if(!kit::createDirectory(terrainPath.str()))
  {
    KIT_ERR("Failed to save terrain, couldn't create terrain path");
    return;
  }

  m_materialMask->getFrontBuffer()->getColorAttachment(0)->saveToFile(terrainPath.str() + std::string("/materialmask0.tga"));
  m_materialMask->getFrontBuffer()->getColorAttachment(1)->saveToFile(terrainPath.str() + std::string("/materialmask1.tga"));

  m_heightmap->getFrontBuffer()->getColorAttachment(0)->saveToFile(terrainPath.str() + std::string("/heightmap.tga"));

  
  std::ofstream header(terrainPath.str() + std::string("/header"));
  if(!header)
  {
    KIT_ERR("Failed to save terrain, could not create headerfile");
    return;
  }
  
  header << "xzscale " << m_xzScale << std::endl;
  header << "yscale " << m_yScale << std::endl;
  header << "numlayers " << (int)m_numLayers << std::endl;
  header << "size " << m_resolution.x << " " << m_resolution.y << std::endl;
  for(int i = 0; i < m_numLayers; i++)
  {
    if(m_layerInfo[i].material == nullptr)
    {
      KIT_ERR("Failed to save terrain, invalid layer material");
      return;
    }
    header << "layer " << i << " \"" << m_layerInfo[i].material->getName() << "\"" << std::endl;
  }
  header.close();
}

void kit::EditorTerrain::setName(const std::string&name)
{
  m_name = name;
}

kit::EditorTerrain::LayerInfo& kit::EditorTerrain::getLayerInfo(int layer)
{
  if(layer > 7 || layer > m_numLayers-1)
  {
    KIT_ERR("Warning: Tried to get layerinfo out of bounds, returning highest");
    return m_layerInfo[m_numLayers - 1];
  }
  return m_layerInfo[layer];
}

void kit::EditorTerrain::setNumLayers(uint8_t l)
{
  if(l > 8 || l == 0)
  {
    KIT_ERR("Warning: Tried to set layercount to out of bounds");
    return;
  }
  m_numLayers = l;
}

uint8_t kit::EditorTerrain::getNumLayers()
{
  return m_numLayers;
}

void kit::EditorTerrain::saveAs(const std::string&name)
{
  m_name = name;
  save();
}

void kit::EditorTerrain::invalidateMaterials()
{
    for(int i = 0; i < m_numLayers; i++)
    {
      m_program->setUniformTexture("uniform_arLayer" + std::to_string(i), m_layerInfo[i].material->getARCache());
      
      if (i != 0)
      {
        m_program->setUniformTexture("uniform_ndLayer" + std::to_string(i), m_layerInfo[i].material->getNDCache());
      }

      m_bakeProgramArnx->setUniformTexture("uniform_arLayer" + std::to_string(i), m_layerInfo[i].material->getARCache());
      m_bakeProgramArnx->setUniformTexture("uniform_ndLayer" + std::to_string(i), m_layerInfo[i].material->getNDCache());
    }
    bakeARNXCache();
    
}

kit::PixelBuffer * kit::EditorTerrain::getMaterialMask()
{
  return m_materialMask->getFrontBuffer();
}
