#include "Kit/BakedTerrain.hpp"

#include "Kit/IncOpenGL.hpp"

#include "Kit/Program.hpp"
#include "Kit/Material.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Renderer.hpp"
#include "Kit/Shader.hpp"
#include "Kit/Types.hpp"
#include "Kit/Model.hpp"

#include <string>
#include <fstream>
#include <cstdlib>
#include <sstream>

const char componentIndex[4] = {'r', 'g', 'b', 'a'};

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
    returner.push_back(componentIndex[layer-4]);
  }

  return returner;
}

kit::BakedTerrain::BakedTerrain(std::string const & name)
{
  glGenVertexArrays(1, &m_glVertexArray);
  glGenBuffers(1, &m_glVertexIndices);
  glGenBuffers(1, &m_glVertexBuffer);
  
  uint32_t vertexDataLen = 0;
  float * vertexData = nullptr;
  uint32_t * indexData = nullptr;
  
  std::string dataDirectory = "./data/terrains/" + name + "/baked/";
  
  // Load data
  {
    std::cout << "Loading vertex data from disk" << std::endl;
    std::ifstream f(dataDirectory + "vertexdata", std::ios_base::in | std::ios_base::binary);
    if(!f)
    {
      KIT_THROW("Failed to load terrain \"" + name + "\": could not load vertexdata.");
    }

    // Read index-data length (in ints)
    m_indexCount = kit::readUint32(f);

    // Read index data
    indexData = new uint32_t[m_indexCount];
    for (uint32_t i = 0; i < m_indexCount; i++)
    {
      indexData[i] = kit::readUint32(f);
    }
    
    // Read vertex-data length (in floats)
    vertexDataLen = kit::readUint32(f);
    
    // Read vertex data
    vertexData = new float[vertexDataLen];
    for (uint32_t i = 0; i < vertexDataLen; i++)
    {
      vertexData[i] = kit::readFloat(f);
    }
    f.close();
  }

  // Upload data
  {
    std::cout << "Uploading data to GPU" << std::endl;

    glBindVertexArray(m_glVertexArray);

    // Upload indices
    std::cout << "Uploading indices (" << (m_indexCount * sizeof(uint32_t)) << " bytes)" << std::endl;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glVertexIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(uint32_t), &indexData[0], GL_STATIC_DRAW);

    // Upload vertices 
    std::cout << "Uploading vertices (" << (vertexDataLen * sizeof(float)) << " bytes)" << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexDataLen * sizeof(float) , &vertexData[0], GL_STATIC_DRAW);
  }

  // Cleanup data
  {
    std::cout << "Cleaning up CPU copy" << std::endl;
    delete[] indexData;
    delete[] vertexData;
  }
  
  // Configure attributes
  {
    std::cout << "Setting up GPU attributes" << std::endl;
    static const uint32_t attributeSize = sizeof(float) * 14;

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*)0);

    // Texture coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 3) );

    // Normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 5) );

    // Tangents
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 8) );

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 11) );
  }

  // Load maps
  {
    std::cout << "Loading maps" << std::endl;
    m_arCache = new kit::Texture(dataDirectory + "arcache.tga");
    m_arCache->setEdgeSamplingMode(Texture::Repeat);
    m_arCache->setMinFilteringMode(Texture::LinearMipmapLinear);
    m_arCache->setMagFilteringMode(Texture::Linear);
    m_arCache->setAnisotropicLevel(8.0f);
    m_arCache->generateMipmap();
    
    m_nxCache = new kit::Texture(dataDirectory + "nxcache.tga");
    m_nxCache->setEdgeSamplingMode(Texture::Repeat);
    m_nxCache->setMinFilteringMode(Texture::LinearMipmapLinear);
    m_nxCache->setMagFilteringMode(Texture::Linear);
    m_nxCache->setAnisotropicLevel(8.0f);
    m_nxCache->generateMipmap();
    
    try
    {
      m_materialMask[0] = new kit::Texture(dataDirectory + "materialmask0.tga");
      m_materialMask[0]->setEdgeSamplingMode(Texture::ClampToEdge);
      m_materialMask[0]->setMinFilteringMode(Texture::LinearMipmapLinear);
      m_materialMask[0]->setMagFilteringMode(Texture::Linear);
      m_materialMask[0]->setAnisotropicLevel(8.0f);
      m_materialMask[0]->generateMipmap();
    
      m_materialMask[1] = new kit::Texture(dataDirectory + "materialmask1.tga");
      m_materialMask[1]->setEdgeSamplingMode(Texture::ClampToEdge);
      m_materialMask[1]->setMinFilteringMode(Texture::LinearMipmapLinear);
      m_materialMask[1]->setMagFilteringMode(Texture::Linear);
      m_materialMask[1]->setAnisotropicLevel(8.0f);
      m_materialMask[1]->generateMipmap();
    }
    catch (...)
    {

    }
  }
  
  // Load header
  {
    std::cout << "Loading header" << std::endl;
    std::string currLine;

    std::ifstream f(dataDirectory + "header", std::ios_base::in);
    if(!f)
    {
      KIT_THROW("Failed to load terrain \"" + name + "\": could not load header.");
    }

    while(std::getline(f, currLine))
    {
      if(kit::trim(currLine) == "")
      {
        continue;
      }

      auto args = kit::splitString(currLine);

      if(args[0] == "xzscale" && args.size() == 2)
      {
        m_xzScale = (float)std::atof(args[1].c_str());
      }

      if (args[0] == "yscale" && args.size() == 2)
      {
        m_yScale = (float)std::atof(args[1].c_str());
      }

      if(args[0] == "numlayers" && args.size() == 2)
      {
        m_numLayers = std::atoi(args[1].c_str());
        if(m_numLayers > 8)
        {
          KIT_THROW("Too many layers in terrain");
        }
      }

      if(args[0] == "size" && args.size() == 3)
      {
        m_size.x = std::atoi(args[1].c_str());
        m_size.y = std::atoi(args[2].c_str());
      }

      if(args[0] == "layer" && args.size() == 3)
      {
        int currLayer = std::atoi(args[1].c_str());
        if(currLayer >= 0 && currLayer <= 7 && currLayer < m_numLayers)
        {
          m_layerInfo[currLayer].uvScale = (float)std::atof(args[2].c_str());
          m_layerInfo[currLayer].used = true;

          std::stringstream currAr;
          currAr << dataDirectory << "arlayer" << currLayer << ".tga";
          m_layerInfo[currLayer].arCache = new kit::Texture(currAr.str());
          m_layerInfo[currLayer].arCache->setEdgeSamplingMode(Texture::Repeat);
          m_layerInfo[currLayer].arCache->setMinFilteringMode(Texture::LinearMipmapLinear);
          m_layerInfo[currLayer].arCache->setMagFilteringMode(Texture::Linear);
          m_layerInfo[currLayer].arCache->setAnisotropicLevel(8.0f);
          m_layerInfo[currLayer].arCache->generateMipmap();

          std::stringstream currNm;
          currNm << dataDirectory << "ndlayer" << currLayer << ".tga";
          m_layerInfo[currLayer].ndCache = new kit::Texture(currNm.str());
          m_layerInfo[currLayer].ndCache->setEdgeSamplingMode(Texture::Repeat);
          m_layerInfo[currLayer].ndCache->setMinFilteringMode(Texture::LinearMipmapLinear);
          m_layerInfo[currLayer].ndCache->setMagFilteringMode(Texture::Linear);
          m_layerInfo[currLayer].ndCache->setAnisotropicLevel(8.0f);
          m_layerInfo[currLayer].ndCache->generateMipmap();
        }
        else
        {
          KIT_THROW("Invalid layer id");
        }
      }
    }
  
    f.close();
  }

  // Load height data 
  {
    std::ifstream f(dataDirectory + "heightdata", std::ios_base::in | std::ios_base::binary);
    if (!f)
    {
      KIT_THROW("Failed to load terrain \"" + name + "\": could not load heightdata.");
    }

    // Reserve and load height data
    m_heightData.reserve(m_size.x * m_size.y);
    for (unsigned int i = 0; i < m_size.y * m_size.x; i++)
    {
      kit::BakedTerrain::Vertex adder;
      adder.m_height = kit::readFloat(f);
      adder.m_normal = kit::readVec3(f);
      m_heightData.push_back(adder);
    }

  }

  m_valid = true;
  std::cout << "Generating GPU program and verifying cache" << std::endl;
  updateGpuProgram();
}

kit::BakedTerrain::~BakedTerrain()
{
  glDeleteBuffers(1, &m_glVertexIndices);
  glDeleteBuffers(1, &m_glVertexBuffer);
  glDeleteVertexArrays(1, &m_glVertexArray);
  
  if(m_materialMask[0])
    delete m_materialMask[0];

  if(m_materialMask[1])
    delete m_materialMask[1];
  
  if(m_program)
    delete m_program;
  
  if(m_arCache)
    delete m_arCache;
  
  if(m_nxCache)
    delete m_nxCache;
  
  for(auto c : m_layerInfo)
  {
    if(c.arCache)
      delete c.arCache;
    
    if(c.ndCache)
      delete c.ndCache;
  }
}

void kit::BakedTerrain::renderDeferred(kit::Renderer * renderer)
{
  if(!m_valid)
  {
    return;
  }

  glm::mat4 modelViewMatrix = renderer->getActiveCamera()->getViewMatrix() * getWorldTransformMatrix();
  glm::mat4 modelViewProjectionMatrix = renderer->getActiveCamera()->getProjectionMatrix() * renderer->getActiveCamera()->getViewMatrix() * getWorldTransformMatrix();

  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  //glCullFace(GL_BACK);

  m_program->setUniformMat4("uniform_mvMatrix", modelViewMatrix);
  m_program->setUniformMat4("uniform_mvpMatrix", modelViewProjectionMatrix);

  m_program->use();
  
  renderGeometry();
}

void kit::BakedTerrain::renderShadows(glm::mat4 const & viewMatrix, glm::mat4 const & projectionMatrix)
{
  glDisable(GL_CULL_FACE);
  auto program = kit::Model::getShadowProgram(false, false, false);
  program->setUniformMat4("uniform_mvpMatrix", projectionMatrix * viewMatrix * getWorldTransformMatrix());
  program->use();
  renderGeometry();
}

void kit::BakedTerrain::renderGeometry()
{
  if(!m_valid)
  {
    return;
  }

  glBindVertexArray(m_glVertexArray);
  glDrawElements( GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

void kit::BakedTerrain::updateGpuProgram()
{
  if(!m_valid)
  {
    return;
  }

  // Generate vertexshader sourcecode
  std::stringstream vertexSource;
  {
    // Header
    vertexSource << "#version 410 core" << std::endl;
    vertexSource << std::endl;

    // In attributes
    vertexSource << "layout (location = 0) in vec3 in_position;" << std::endl;
    vertexSource << "layout (location = 1) in vec2 in_texcoord;" << std::endl;
    vertexSource << "layout (location = 2) in vec3 in_normal;" << std::endl;
    vertexSource << "layout (location = 3) in vec3 in_tangent;" << std::endl;
    vertexSource << "layout (location = 4) in vec3 in_bitangent;" << std::endl;
    vertexSource << std::endl;

    // Out attributes
    vertexSource << "layout (location = 0) out vec2 out_texcoord;" << std::endl;
    vertexSource << "layout (location = 1) out vec3 out_normal;" << std::endl;
    vertexSource << "layout (location = 2) out vec3 out_tangent;" << std::endl;
    vertexSource << "layout (location = 3) out vec3 out_bitangent;" << std::endl;
    vertexSource << "layout (location = 4) out vec4 out_position;" << std::endl;
    vertexSource << std::endl;

    // Uniforms
    vertexSource << "uniform mat4 uniform_mvpMatrix;" << std::endl;
    vertexSource << "uniform mat4 uniform_mvMatrix;" << std::endl;
    vertexSource << std::endl;

    // ---- Main code begins ---- //
    vertexSource << "void main()" << std::endl;
    vertexSource << "{" << std::endl;

    // Prepare variables
    vertexSource << "  vec4 position  = vec4(in_position, 1.0);" << std::endl;
    vertexSource << std::endl;

    // Write attributes
    vertexSource << "  out_position   = uniform_mvMatrix * position;" << std::endl;
    vertexSource << "  out_normal     = (uniform_mvMatrix * vec4(normalize(in_normal), 0.0)).xyz;" << std::endl;
    vertexSource << "  out_texcoord   = in_texcoord;" << std::endl;
    vertexSource << "  out_tangent    = (uniform_mvMatrix * vec4(normalize(in_tangent), 0.0)).xyz;" << std::endl;
    vertexSource << "  out_bitangent  = (uniform_mvMatrix * vec4(normalize(in_bitangent), 0.0)).xyz;" << std::endl;
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
    pixelSource << "layout (location = 0) in vec2 in_texCoords;" << std::endl;
    pixelSource << "layout (location = 1) in vec3 in_normal;" << std::endl;
    pixelSource << "layout (location = 2) in vec3 in_tangent;" << std::endl;
    pixelSource << "layout (location = 3) in vec3 in_bitangent;" << std::endl;
    pixelSource << "layout (location = 4) in vec4 in_position;" << std::endl;
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
    pixelSource << "uniform sampler2D uniform_nxCache;" << std::endl;
    pixelSource << "uniform float uniform_detailDistance;" << std::endl;
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
    pixelSource << "  vec2 detailUv = in_texCoords * vec2(" << (float)m_size.x * m_xzScale << ", " << (float)m_size.y  * m_xzScale << ");" << std::endl;
    pixelSource << "  float linearDistance = distance(vec3(0.0), in_position.xyz / in_position.w);" << std::endl;

    // Prepare output variables
    pixelSource << "  vec4 arOut; " << std::endl;
    pixelSource << "  vec3 nOut;" << std::endl;

    // If the distance to the fragment is higher than the detail distance, output the cached AR and NM maps
    pixelSource << "  if (linearDistance > uniform_detailDistance)" << std::endl;
    pixelSource << "  {" << std::endl;
    pixelSource << "    arOut = texture(uniform_arCache, fullUv);" << std::endl;
    pixelSource << "    nOut = texture(uniform_nxCache, fullUv).rgb;" << std::endl;
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
      pixelSource << "    vec4 ar" << i << " = texture(uniform_arLayer" << i << ", detailUv * " << m_layerInfo[i].uvScale << ");" << std::endl;
      pixelSource << "    vec4 nd" << i << " = texture(uniform_ndLayer" << i << ", detailUv * " << m_layerInfo[i].uvScale << ");" << std::endl;

      if (i == 0)
      {
        pixelSource << "    arOut = ar0;" << std::endl;
        pixelSource << "    nOut = nd0.rgb;" << std::endl;
      }
      else
      {
        pixelSource << "    arOut = blend2(arOut, ar" << i << ", materialMask" << getMaskSuffix(i) << " + nd" << i << ".a);" << std::endl;
        pixelSource << "    nOut = blend2(vec4(nOut, 0.0), vec4(nd" << i << ".rgb, 0.0), materialMask" << getMaskSuffix(i) << " + nd" << i << ".a).rgb;" << std::endl;
      }
    }

    // We're done with generating output
    pixelSource << "  }" << std::endl;
    pixelSource << std::endl;

    // Prepare normalmapping and output
    pixelSource << "  mat3 normalRotation= (mat3(normalize(in_tangent), normalize(in_bitangent), normalize(in_normal)));" << std::endl;
    pixelSource << "  vec3 normalTex     = normalize((nOut.rgb - 0.5)*2.0);" << std::endl;
    pixelSource << "  vec3 normal        = normalize(normalRotation * normalTex);" << std::endl;
    
    // Write output
    pixelSource << "  out_A = arOut;" << std::endl;
    pixelSource << "  out_B = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
    pixelSource << "  out_C.xyz = normal;" << std::endl;
    pixelSource << "  out_C.a = 1.0;" << std::endl;

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
  if(m_program)
    delete m_program;
  
  m_program = new kit::Program();
  m_program->attachShader(vertexShader);
  m_program->attachShader(pixelShader);
  m_program->link();
  m_program->detachShader(vertexShader);
  m_program->detachShader(pixelShader);

  delete vertexShader;
  delete pixelShader;
  
  // Update uniforms
  if (m_numLayers > 1)
  {
    m_program->setUniformTexture("uniform_materialMask0", m_materialMask[0]);
  }

  if (m_numLayers > 4)
  {
    m_program->setUniformTexture("uniform_materialMask1", m_materialMask[1]);
  }
  
  m_program->setUniformTexture("uniform_arCache", m_arCache);
  m_program->setUniformTexture("uniform_nxCache", m_nxCache);
  m_program->setUniform1f("uniform_detailDistance", 500.0f); //< TODO: Replace with configuration parameter

  // Layer-specific uniforms
  for(int i = 0; i < m_numLayers; i++)
  {
    m_program->setUniformTexture("uniform_arLayer" + std::to_string(i), m_layerInfo[i].arCache);
    m_program->setUniformTexture("uniform_ndLayer" + std::to_string(i), m_layerInfo[i].ndCache);
  }
}

kit::Texture * kit::BakedTerrain::getArCache()
{
  return m_arCache;
}

kit::Texture * kit::BakedTerrain::getNxCache()
{
  return m_nxCache;
}

kit::Texture * kit::BakedTerrain::getMaterialMask0()
{
  return m_materialMask[0];
}

kit::Texture * kit::BakedTerrain::getMaterialMask1()
{
  return m_materialMask[1];
}

kit::BakedTerrain::Vertex const & kit::BakedTerrain::getVertexAt(uint32_t x, uint32_t y)
{
  x = (glm::min)(m_size.x - 1, x);
  y = (glm::min)(m_size.y - 1, y);
  return m_heightData[(m_size.x * y) + x];
  //return m_heightData[(m_size.y * x) + x];
}

float kit::BakedTerrain::sampleHeight(float x, float z)
{
  glm::vec2 fullSize;
  fullSize.x = float(m_size.x) * m_xzScale;
  fullSize.y = float(m_size.y) * m_xzScale;

  glm::vec2 halfSize = fullSize / 2.0f;

  x += halfSize.x;
  z += halfSize.y;

  float xMap = x / m_xzScale;
  float zMap = z / m_xzScale;

  uint32_t px = (uint32_t)glm::floor(xMap);
  uint32_t pz = (uint32_t)glm::floor(zMap);

  uint32_t hx = px + 1;
  uint32_t hz = pz + 1;

  float v1 = getVertexAt(px, pz).m_height;
  float v2 = getVertexAt(hx, pz).m_height;
  float v3 = getVertexAt(px, hz).m_height;
  float v4 = getVertexAt(hx, hz).m_height;

  // Calculate the weights for each pixel
  float fx = (xMap - (float)px);
  float fz = (zMap - (float)pz);
  float fx1 = 1.0f - fx;
  float fz1 = 1.0f - fz;

  float w1 = fx1 * fz1;
  float w2 = fx  * fz1;
  float w3 = fx1 * fz;
  float w4 = fx  * fz;

  float height = v1 * w1 + v2 * w2 + v3 * w3 + v4 * w4;

  //std::cout << "Height at " << x << "x" << z << ": " << height << std::endl;
  return height * m_yScale;
}

glm::vec3 kit::BakedTerrain::sampleNormal(float x, float z)
{
  glm::vec2 fullSize;
  fullSize.x = float(m_size.x) * m_xzScale;
  fullSize.y = float(m_size.y) * m_xzScale;

  glm::vec2 halfSize = fullSize / 2.0f;

  x += halfSize.x;
  z += halfSize.y;

  float xMap = x / m_xzScale;
  float zMap = z / m_xzScale;

  uint32_t px = (uint32_t)glm::floor(xMap);
  uint32_t pz = (uint32_t)glm::floor(zMap);

  uint32_t hx = px + 1;
  uint32_t hz = pz + 1;

  glm::vec3 v1 = getVertexAt(px, pz).m_normal;
  glm::vec3 v2 = getVertexAt(hx, pz).m_normal;
  glm::vec3 v3 = getVertexAt(px, hz).m_normal;
  glm::vec3 v4 = getVertexAt(hx, hz).m_normal;

  // Calculate the weights for each pixel
  float fx = (xMap - (float)px);
  float fz = (zMap - (float)pz);
  float fx1 = 1.0f - fx;
  float fz1 = 1.0f - fz;

  float w1 = fx1 * fz1;
  float w2 = fx  * fz1;
  float w3 = fx1 * fz;
  float w4 = fx  * fz;

  glm::vec3 normal = v1 * w1 + v2 * w2 + v3 * w3 + v4 * w4;

  return normal;
}

const glm::uvec2 & kit::BakedTerrain::getSize()
{
  return m_size;
}

const float & kit::BakedTerrain::getXzScale()
{
  return m_xzScale;
}

bool kit::BakedTerrain::checkCollision(glm::vec3 point)
{
  return sampleHeight(point.x, point.z) >= point.y;
}

int32_t kit::BakedTerrain::getRenderPriority()
{
  // Render priority at 990. We want to render it after anything else, except water which is at 1000)
  // This is so that we only render visible fragments, since terrain gbuffer-shaders are pretty heavy
  return 990;
}

void kit::BakedTerrain::setDetailDistance(const float& meters)
{
  m_program->setUniform1f("uniform_detailDistance", meters);
}
