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

kit::BakedTerrain::BakedTerrain()
{
  // Initialize the layerinfo array with some sane defaults
  for(auto & currLayer : this->m_layerInfo)
  {
    currLayer.arCache = nullptr;
    currLayer.ndCache = nullptr;
    currLayer.used    = false;
    currLayer.uvScale = 1.0f;
  }

  this->m_arCache = nullptr;
  this->m_nxCache = nullptr;
  this->m_materialMask[0] = nullptr;
  this->m_materialMask[1] = nullptr;

  this->m_indexCount = 0;
  this->m_numLayers = 0;
  this->m_program = nullptr;
  this->m_size = glm::uvec2(0, 0);
  this->m_valid = false;
  
  this->m_xzScale = 1.0f;
  this->m_yScale = 1.0f;

  glGenVertexArrays(1, &this->m_glVertexArray);
  glGenBuffers(1, &this->m_glVertexIndices);
  glGenBuffers(1, &this->m_glVertexBuffer);
}

kit::BakedTerrain::~BakedTerrain()
{
  glDeleteBuffers(1, &this->m_glVertexIndices);
  glDeleteBuffers(1, &this->m_glVertexBuffer);
  glDeleteVertexArrays(1, &this->m_glVertexArray);
}

kit::BakedTerrain::Ptr kit::BakedTerrain::load(const std::string&name)
{
  std::cout << "Loading baked terrain \"" << name.c_str() << "\"" << std::endl;
  auto returner = std::make_shared<kit::BakedTerrain>();
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
      KIT_ERR("Failed to load terrain \"" + name + "\": could not load vertexdata.");
      return returner;
    }

    // Read index-data length (in ints)
    returner->m_indexCount = kit::readUint32(f);

    // Read index data
    indexData = new uint32_t[returner->m_indexCount];
    for (uint32_t i = 0; i < returner->m_indexCount; i++)
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

    glBindVertexArray(returner->m_glVertexArray);

    // Upload indices
    std::cout << "Uploading indices (" << (returner->m_indexCount * sizeof(uint32_t)) << " bytes)" << std::endl;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, returner->m_glVertexIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, returner->m_indexCount * sizeof(uint32_t), &indexData[0], GL_STATIC_DRAW);

    // Upload vertices 
    std::cout << "Uploading vertices (" << (vertexDataLen * sizeof(float)) << " bytes)" << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, returner->m_glVertexBuffer);
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
    returner->m_arCache = kit::Texture::create2DFromFile(dataDirectory + "arcache.tga", kit::Texture::RGBA8, kit::Texture::Repeat, kit::Texture::LinearMipmapLinear, kit::Texture::Linear);
    returner->m_nxCache = kit::Texture::create2DFromFile(dataDirectory + "nxcache.tga", kit::Texture::RGBA8, kit::Texture::Repeat, kit::Texture::LinearMipmapLinear, kit::Texture::Linear);
    try
    {
      returner->m_materialMask[0] = kit::Texture::create2DFromFile(dataDirectory + "materialmask0.tga", kit::Texture::RGBA8, kit::Texture::ClampToEdge, kit::Texture::LinearMipmapLinear, kit::Texture::Linear);
      returner->m_materialMask[1] = kit::Texture::create2DFromFile(dataDirectory + "materialmask1.tga", kit::Texture::RGBA8, kit::Texture::ClampToEdge, kit::Texture::LinearMipmapLinear, kit::Texture::Linear);
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
      KIT_ERR("Failed to load terrain \"" + name + "\": could not load header.");
      return returner;
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
        returner->m_xzScale = (float)std::atof(args[1].c_str());
      }

      if (args[0] == "yscale" && args.size() == 2)
      {
        returner->m_yScale = (float)std::atof(args[1].c_str());
      }

      if(args[0] == "numlayers" && args.size() == 2)
      {
        returner->m_numLayers = std::atoi(args[1].c_str());
        if(returner->m_numLayers > 8)
        {
          KIT_ERR("Too many layers in terrain");
          return returner;
        }
      }

      if(args[0] == "size" && args.size() == 3)
      {
        returner->m_size.x = std::atoi(args[1].c_str());
        returner->m_size.y = std::atoi(args[2].c_str());
      }

      if(args[0] == "layer" && args.size() == 3)
      {
        int currLayer = std::atoi(args[1].c_str());
        if(currLayer >= 0 && currLayer <= 7 && currLayer < returner->m_numLayers)
        {
          returner->m_layerInfo[currLayer].uvScale = (float)std::atof(args[2].c_str());
          returner->m_layerInfo[currLayer].used = true;

          std::stringstream currAr;
          currAr << dataDirectory << "arlayer" << currLayer << ".tga";
          returner->m_layerInfo[currLayer].arCache = kit::Texture::create2DFromFile(currAr.str(), kit::Texture::RGBA8, kit::Texture::Repeat, kit::Texture::LinearMipmapLinear, kit::Texture::Linear);

          std::stringstream currNm;
          currNm << dataDirectory << "ndlayer" << currLayer << ".tga";
          returner->m_layerInfo[currLayer].ndCache = kit::Texture::create2DFromFile(currNm.str(), kit::Texture::RGBA8, kit::Texture::Repeat, kit::Texture::LinearMipmapLinear, kit::Texture::Linear);
        }
        else
        {
          KIT_ERR("Invalid layer id");
          return returner;
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
      KIT_ERR("Failed to load terrain \"" + name + "\": could not load heightdata.");
      return returner;
    }

    // Reserve and load height data
    returner->m_heightData.reserve(returner->m_size.x * returner->m_size.y);
    for (unsigned int i = 0; i < returner->m_size.y * returner->m_size.x; i++)
    {
      kit::BakedTerrain::Vertex adder;
      adder.m_height = kit::readFloat(f);
      adder.m_normal = kit::readVec3(f);
      returner->m_heightData.push_back(adder);
    }

  }

  returner->m_valid = true;
  std::cout << "Generating GPU program and verifying cache" << std::endl;
  returner->updateGpuProgram();
  std::cout << "Done!" << std::endl;
  return returner;
}

void kit::BakedTerrain::renderDeferred(kit::Renderer::Ptr renderer)
{
  if(!this->m_valid)
  {
    return;
  }

  glm::mat4 modelViewMatrix = renderer->getActiveCamera()->getViewMatrix() * this->getTransformMatrix();
  glm::mat4 modelViewProjectionMatrix = renderer->getActiveCamera()->getProjectionMatrix() * renderer->getActiveCamera()->getViewMatrix() * this->getTransformMatrix();

  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  //glCullFace(GL_BACK);

  this->m_program->use();
  this->m_program->setUniformMat4("uniform_mvMatrix", modelViewMatrix);
  this->m_program->setUniformMat4("uniform_mvpMatrix", modelViewProjectionMatrix);

  this->renderGeometry();
}

void kit::BakedTerrain::renderShadows(glm::mat4 viewmatrix, glm::mat4 projectionmatrix)
{
  glDisable(GL_CULL_FACE);
  kit::Model::getShadowProgram(false, false, false)->use();
  kit::Model::getShadowProgram(false, false, false)->setUniformMat4("uniform_mvpMatrix", projectionmatrix * viewmatrix * this->getTransformMatrix());
  this->renderGeometry();
}

void kit::BakedTerrain::renderGeometry()
{
  if(!this->m_valid)
  {
    return;
  }

  glBindVertexArray(this->m_glVertexArray);
  glDrawElements( GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0);
}

void kit::BakedTerrain::updateGpuProgram()
{
  if(!this->m_valid)
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
    if (this->m_numLayers > 1)
    {
      pixelSource << "uniform sampler2D uniform_materialMask0;" << std::endl;
    }

    if (this->m_numLayers > 4)
    {
      pixelSource << "uniform sampler2D uniform_materialMask1;" << std::endl;
    }

    pixelSource << "uniform sampler2D uniform_arCache;" << std::endl;
    pixelSource << "uniform sampler2D uniform_nxCache;" << std::endl;
    pixelSource << "uniform float uniform_detailDistance;" << std::endl;
    pixelSource << std::endl;

    // Layerspecific uniforms
    for(int i = 0; i < this->m_numLayers; i++)
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
    pixelSource << "  vec2 detailUv = in_texCoords * vec2(" << (float)this->m_size.x * this->m_xzScale << ", " << (float)this->m_size.y  * this->m_xzScale << ");" << std::endl;
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
    if (this->m_numLayers > 1)
    {
      pixelSource << "    vec4 materialMask0 = texture(uniform_materialMask0, fullUv);" << std::endl;
    }

    if (this->m_numLayers > 4)
    {
      pixelSource << "    vec4 materialMask1 = texture(uniform_materialMask1, fullUv);" << std::endl;
    }

    // Sample the layer maps
    for (int i = 0; i < this->m_numLayers; i++)
    {
      pixelSource << "    vec4 ar" << i << " = texture(uniform_arLayer" << i << ", detailUv * " << this->m_layerInfo[i].uvScale << ");" << std::endl;
      pixelSource << "    vec4 nd" << i << " = texture(uniform_ndLayer" << i << ", detailUv * " << this->m_layerInfo[i].uvScale << ");" << std::endl;

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
  auto vertexShader = kit::Shader::create(Shader::Type::Vertex);
  vertexShader->sourceFromString(vertexSource.str());
  vertexShader->compile();

  auto pixelShader = kit::Shader::create(Shader::Type::Fragment);
  pixelShader->sourceFromString(pixelSource.str());
  pixelShader->compile();

  // Link program
  this->m_program = kit::Program::create();
  this->m_program->attachShader(vertexShader);
  this->m_program->attachShader(pixelShader);
  this->m_program->link();
  this->m_program->detachShader(vertexShader);
  this->m_program->detachShader(pixelShader);

  // Update uniforms
  if (this->m_numLayers > 1)
  {
    this->m_program->setUniformTexture("uniform_materialMask0", this->m_materialMask[0]);
  }

  if (this->m_numLayers > 4)
  {
    this->m_program->setUniformTexture("uniform_materialMask1", this->m_materialMask[1]);
  }
  
  this->m_program->setUniformTexture("uniform_arCache", this->m_arCache);
  this->m_program->setUniformTexture("uniform_nxCache", this->m_nxCache);
  this->m_program->setUniform1f("uniform_detailDistance", 25.0f); //< TODO: Replace with configuration parameter

  // Layer-specific uniforms
  for(int i = 0; i < this->m_numLayers; i++)
  {
    this->m_program->setUniformTexture("uniform_arLayer" + std::to_string(i), this->m_layerInfo[i].arCache);
    this->m_program->setUniformTexture("uniform_ndLayer" + std::to_string(i), this->m_layerInfo[i].ndCache);
  }
}

kit::Texture::Ptr kit::BakedTerrain::getArCache()
{
  return this->m_arCache;
}

kit::Texture::Ptr kit::BakedTerrain::getNxCache()
{
  return this->m_nxCache;
}

kit::Texture::Ptr kit::BakedTerrain::getMaterialMask0()
{
  return this->m_materialMask[0];
}

kit::Texture::Ptr kit::BakedTerrain::getMaterialMask1()
{
  return this->m_materialMask[1];
}

kit::BakedTerrain::Vertex const & kit::BakedTerrain::getVertexAt(uint32_t x, uint32_t y)
{
  x = (glm::min)(this->m_size.x - 1, x);
  y = (glm::min)(this->m_size.y - 1, y);
  return this->m_heightData[(this->m_size.x * y) + x];
  //return this->m_heightData[(this->m_size.y * x) + x];
}

float kit::BakedTerrain::sampleHeight(float x, float z)
{
  glm::vec2 fullSize;
  fullSize.x = float(this->m_size.x) * this->m_xzScale;
  fullSize.y = float(this->m_size.y) * this->m_xzScale;

  glm::vec2 halfSize = fullSize / 2.0f;

  x += halfSize.x;
  z += halfSize.y;

  float xMap = x / this->m_xzScale;
  float zMap = z / this->m_xzScale;

  uint32_t px = (uint32_t)glm::floor(xMap);
  uint32_t pz = (uint32_t)glm::floor(zMap);

  uint32_t hx = px + 1;
  uint32_t hz = pz + 1;

  float v1 = this->getVertexAt(px, pz).m_height;
  float v2 = this->getVertexAt(hx, pz).m_height;
  float v3 = this->getVertexAt(px, hz).m_height;
  float v4 = this->getVertexAt(hx, hz).m_height;

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
  return height * this->m_yScale;
}

glm::vec3 kit::BakedTerrain::sampleNormal(float x, float z)
{
  glm::vec2 fullSize;
  fullSize.x = float(this->m_size.x) * this->m_xzScale;
  fullSize.y = float(this->m_size.y) * this->m_xzScale;

  glm::vec2 halfSize = fullSize / 2.0f;

  x += halfSize.x;
  z += halfSize.y;

  float xMap = x / this->m_xzScale;
  float zMap = z / this->m_xzScale;

  uint32_t px = (uint32_t)glm::floor(xMap);
  uint32_t pz = (uint32_t)glm::floor(zMap);

  uint32_t hx = px + 1;
  uint32_t hz = pz + 1;

  glm::vec3 v1 = this->getVertexAt(px, pz).m_normal;
  glm::vec3 v2 = this->getVertexAt(hx, pz).m_normal;
  glm::vec3 v3 = this->getVertexAt(px, hz).m_normal;
  glm::vec3 v4 = this->getVertexAt(hx, hz).m_normal;

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
  return this->m_size;
}

const float & kit::BakedTerrain::getXzScale()
{
  return this->m_xzScale;
}

bool kit::BakedTerrain::checkCollision(glm::vec3 point)
{
  return this->sampleHeight(point.x, point.z) >= point.y;
}

int32_t kit::BakedTerrain::getRenderPriority()
{
  // Render priority at 990. We want to render it after anything else, except water which is at 1000)
  // This is so that we only render visible fragments, since terrain gbuffer-shaders are pretty heavy
  return 990;
}

void kit::BakedTerrain::setDetailDistance(const float& meters)
{
  this->m_program->setUniform1f("uniform_detailDistance", meters);
}