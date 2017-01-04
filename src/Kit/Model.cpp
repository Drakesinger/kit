#include "Kit/Model.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Mesh.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Material.hpp"
#include "Kit/Submesh.hpp"
#include "Kit/Program.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Renderer.hpp"
#include "Kit/Shader.hpp"
#include "Kit/Skeleton.hpp"

#include <sstream>
#include <glm/gtx/transform.hpp>

uint32_t kit::Model::m_instanceCount = 0;
std::map<kit::Model::ShadowProgramFlags, kit::Program*> kit::Model::m_shadowPrograms;

kit::Model::Model() : kit::Renderable::Renderable()
{
  kit::Model::m_instanceCount++;
  if (kit::Model::m_instanceCount == 1)
  {
    kit::Model::allocateShared();
  }
}

kit::Model::~Model()
{
  if(m_ownMesh && m_mesh) delete m_mesh;
  if(m_skeleton) delete m_skeleton;
  
  kit::Model::m_instanceCount--;
  if (kit::Model::m_instanceCount == 0)
  {
    kit::Model::releaseShared();
  }
}

void kit::Model::allocateShared()
{

}

void kit::Model::releaseShared()
{
  for(auto & t : m_shadowPrograms)
    if(t.second) delete t.second;
    
  m_shadowPrograms.clear();
}

kit::Program * kit::Model::getShadowProgram(bool skinned, bool opacityMapped, bool instanced)
{
  kit::Model::ShadowProgramFlags flags;
  flags.skinned = skinned;
  flags.opacityMapped = opacityMapped;
  flags.instanced = instanced;
  
  if(m_shadowPrograms.find(flags) != m_shadowPrograms.end())
  {
    return m_shadowPrograms.at(flags);
  }
  
  std::cout << "Generating shadow program for flags " << (skinned? "S":"-") << (opacityMapped? "O":"-") << (instanced? "I":"-") << std::endl;
  
  // Create a program and shaders
  auto newProgram = new kit::Program();
  
  // Vertex shader 
  std::stringstream vertexSource;
  auto vertexShader = new kit::Shader(Shader::Type::Vertex);
  {
    vertexSource << "#version 430 core" << std::endl;

    vertexSource << "layout(location = 0) in vec3 in_vertexPos;" << std::endl;
    vertexSource << "layout(location = 1) in vec2 in_uv;" << std::endl;
    vertexSource << "layout(location = 4) in ivec4 in_boneids;" << std::endl;
    vertexSource << "layout(location = 5) in vec4  in_boneweights;" << std::endl;
    vertexSource << "layout(location = 0) out vec2 out_uv;" << std::endl;

    vertexSource << "uniform mat4 uniform_mvpMatrix;" << std::endl;

    if(skinned)
    {
      vertexSource << "uniform mat4 uniform_bones[128];" << std::endl;
    }
    
    if(instanced)
    {
      vertexSource << "uniform mat4 uniform_instanceTransform[128];" << std::endl;
    }
    
    vertexSource << "void main()" << std::endl;
    vertexSource << "{" << std::endl;
    
    if(skinned)
    {
      vertexSource << "  mat4 boneTransform = uniform_bones[in_boneids[0]] * in_boneweights[0];" << std::endl;
      vertexSource << "  boneTransform += uniform_bones[in_boneids[1]] * in_boneweights[1];" << std::endl;
      vertexSource << "  boneTransform += uniform_bones[in_boneids[2]] * in_boneweights[2];" << std::endl;
      vertexSource << "  boneTransform += uniform_bones[in_boneids[3]] * in_boneweights[3];" << std::endl;
      vertexSource << "  vec4 position = boneTransform * vec4(in_vertexPos, 1.0);" << std::endl;
    }
    else 
    {
      vertexSource << "  vec4 position = vec4(in_vertexPos, 1.0);" << std::endl;
    }
    
    if(instanced)
    {
      vertexSource << "  gl_Position = uniform_mvpMatrix * uniform_instanceTransform[gl_InstanceID] * position;" << std::endl;
    }
    else
    {
      vertexSource << "  gl_Position = uniform_mvpMatrix * position;" << std::endl;
    }
    
    vertexSource << "  out_uv = in_uv;" << std::endl;
    vertexSource << "}" << std::endl;
  }
  
  // Pixel shader
  std::stringstream pixelSource;
  auto pixelShader = new kit::Shader(Shader::Type::Fragment);
  {
    pixelSource << "#version 430 core" << std::endl;

    pixelSource << "layout(location = 0) in vec2 in_uv;" << std::endl;
    
    if(opacityMapped)
    {
      pixelSource << "uniform sampler2D uniform_opacityMask;" << std::endl;
    }
    
    pixelSource << "void main()" << std::endl;
    pixelSource << "{" << std::endl;
    
    if(opacityMapped)
    {
      pixelSource << "  if(texture(uniform_opacityMask, in_uv).r < 0.5)" << std::endl;
      pixelSource << "  {" << std::endl;
      pixelSource << "    discard;" << std::endl;
      pixelSource << "  }" << std::endl;
    }
  
    pixelSource << "  gl_FragDepth = gl_FragCoord.z;" << std::endl;
    pixelSource << "}" << std::endl;

  }
  
  // Compile and link
  vertexShader->sourceFromString(vertexSource.str());
  vertexShader->compile();
  
  pixelShader->sourceFromString(pixelSource.str());
  pixelShader->compile();

  newProgram->attachShader(vertexShader);
  newProgram->attachShader(pixelShader);
  newProgram->link();
  newProgram->detachShader(pixelShader);
  newProgram->detachShader(vertexShader);
  
  delete vertexShader;
  delete pixelShader;
  
  kit::Model::m_shadowPrograms[flags] = newProgram;
  return kit::Model::m_shadowPrograms.at(flags);
}


kit::Model::Model(const std::string&mesh) : kit::Model()
{
  m_mesh = new kit::Mesh(mesh);
  m_ownMesh = true;
  m_skeleton = nullptr;
}

kit::Model::Model(kit::Mesh * mesh) : kit::Model()
{
  m_mesh = mesh;
  m_ownMesh = false;
  m_skeleton = nullptr;
}

kit::Model::Model(const std::string&mesh, const std::string& skeleton) : kit::Model()
{
  m_mesh = new kit::Mesh(mesh);
  m_ownMesh = true;
  m_skeleton = new kit::Skeleton(skeleton);
}

kit::Mesh * kit::Model::getMesh()
{
  return m_mesh;
}

kit::Skeleton * kit::Model::getSkeleton()
{
  return m_skeleton;
}

void kit::Model::setInstancing(bool enabled, std::vector< glm::mat4 > transforms)
{
  m_instanced = enabled;
  m_instanceTransform = transforms;
}


void kit::Model::update(const double & ms)
{
  if(m_skeleton != nullptr)
  {
    m_skeleton->update(ms);
  }
}

void kit::Model::renderDeferred(kit::Renderer* renderer)
{
  std::vector<glm::mat4> skinTransform;
  std::vector<glm::mat4> instanceTransform;
  
  if(m_skeleton)
  {
    skinTransform = m_skeleton->getSkin();
  }
  
  if(m_instanced)
  {
    instanceTransform = m_instanceTransform;
  }

  m_mesh->render(renderer->getActiveCamera(), getTransformMatrix(), false, skinTransform, instanceTransform);
}

void kit::Model::renderForward(kit::Renderer* renderer)
{
  std::vector<glm::mat4> skinTransform;
  std::vector<glm::mat4> instanceTransform;
  
  if(m_skeleton)
  {
    skinTransform = m_skeleton->getSkin();
  }
  
  if(m_instanced)
  {
    instanceTransform = m_instanceTransform;
  }

  m_mesh->render(renderer->getActiveCamera(), getTransformMatrix(), true, skinTransform, instanceTransform);
}

void kit::Model::renderShadows(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
  for (auto &currSubmeshIndex : m_mesh->getSubmeshEntries())
  {
    auto & currMaterial = currSubmeshIndex.second.m_material;
    auto & currSubmesh = currSubmeshIndex.second.m_submesh;

    if (!currMaterial->getCastShadows())
    {
      continue;
    }

    if (currMaterial->getDoubleSided())
    {
      glDisable(GL_CULL_FACE);
    }
    else
    {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
    }

    bool O = (currMaterial->getOpacityMask() != nullptr);
    bool S = (m_skeleton != nullptr);
    bool I = m_instanced;

    auto currProgram = kit::Model::getShadowProgram(S, O, I);

    currProgram->setUniformMat4("uniform_mvpMatrix", projectionMatrix * viewMatrix * getTransformMatrix());

    if(O)
    {
      currProgram->setUniformTexture("uniform_opacityMask", currMaterial->getOpacityMask().get());
    }

    if(S)
    {
      currProgram->setUniformMat4v("uniform_bones", m_skeleton->getSkin());
    }
    
    if(I)
    {
      currProgram->setUniformMat4v("uniform_instanceTransform", m_instanceTransform);
      currSubmesh->renderGeometryInstanced(m_instanceTransform.size());
    }
    else
    {
      currSubmesh->renderGeometry();
    }
  }
}

void kit::Model::renderGeometry()
{
  m_mesh->renderGeometry();
}

bool kit::Model::isSkinned()
{
  return (m_skeleton != nullptr);
}

std::vector<glm::mat4> kit::Model::getSkin()
{
  if (m_skeleton == nullptr)
  {
    KIT_ERR("Warning: tried to get skin from non-skinned model");
    return std::vector<glm::mat4>();
  }

  return m_skeleton->getSkin();
}

glm::vec3 kit::Model::getBoneWorldPosition(const std::string&bone)
{
  if (!m_skeleton)
  {
    KIT_ERR("Warning: tried to get bone position from non-skinned model");
    return glm::vec3();
  }

  kit::Skeleton::Bone * currBone = m_skeleton->getBone(bone);
  if (!currBone)
  {
    KIT_ERR("Warning: tried to get bone position from non-existent bone");
    return glm::vec3();
  }

  return glm::vec3( getTransformMatrix() * currBone->m_globalTransform * glm::vec4(0.0, 0.0, 0.0, 1.0));
}

glm::quat kit::Model::getBoneWorldRotation(const std::string&bone)
{
  if (!m_skeleton)
  {
    KIT_ERR("Warning: tried to get bone rotation from non-skinned model");
    return glm::quat();
  }

  kit::Skeleton::Bone* currBone = m_skeleton->getBone(bone);
  if (!currBone)
  {
    KIT_ERR("Warning: tried to get bone rotation from non-existent bone");
    return glm::quat();
  }

  // TODO: This (fodderFix) is needed for badly exported/imported models. Fix our importer/blenders exporter/whatever and then remove this.
  glm::quat fodderFix;
  fodderFix = glm::rotate(fodderFix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  //fodderFix = glm::rotate(fodderFix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

  return glm::quat_cast(getTransformMatrix() * currBone->m_globalTransform) * fodderFix;
}
