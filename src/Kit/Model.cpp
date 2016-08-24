#include "Kit/Model.hpp"
#include "Kit/Mesh.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Material.hpp"
#include "Kit/Submesh.hpp"
#include "Kit/Program.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Renderer.hpp"
#include "Kit/Shader.hpp"

#include <sstream>
#include <glm/gtx/transform.hpp>

uint32_t kit::Model::m_instanceCount = 0;
std::map<kit::Model::ShadowProgramFlags, kit::Program::Ptr> kit::Model::m_shadowPrograms;

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
  kit::Model::m_shadowPrograms.clear();
}

kit::ProgramPtr kit::Model::getShadowProgram(bool skinned, bool opacityMapped, bool instanced)
{
  kit::Model::ShadowProgramFlags flags;
  flags.skinned = skinned;
  flags.opacityMapped = opacityMapped;
  flags.instanced = instanced;
  
  if(kit::Model::m_shadowPrograms.find(flags) != kit::Model::m_shadowPrograms.end())
  {
    return kit::Model::m_shadowPrograms.at(flags);
  }
  
  std::cout << "Generating shadow program for flags " << (skinned? "S":"-") << (opacityMapped? "O":"-") << (instanced? "I":"-") << std::endl;
  
  // Create a program and shaders
  auto newProgram = kit::Program::create();
  
  // Vertex shader 
  std::stringstream vertexSource;
  auto vertexShader = kit::Shader::create(Shader::Type::Vertex);
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
  auto pixelShader = kit::Shader::create(Shader::Type::Fragment);
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
  
  kit::Model::m_shadowPrograms[flags] = newProgram;
  return kit::Model::m_shadowPrograms.at(flags);
}


kit::Model::Ptr kit::Model::create(const std::string&mesh)
{
  kit::Model::Ptr returner = std::make_shared<kit::Model>();
  returner->m_mesh = kit::Mesh::load(mesh);
  returner->m_skeleton = nullptr;
  return returner;
}

kit::Model::Ptr kit::Model::create(kit::Mesh::Ptr mesh)
{
  kit::Model::Ptr returner = std::make_shared<kit::Model>();
  returner->m_mesh = mesh;
  returner->m_skeleton = nullptr;
  return returner;
}

kit::Model::Ptr kit::Model::create(const std::string&mesh, const std::string& skeleton)
{
  kit::Model::Ptr returner = std::make_shared<kit::Model>();
  returner->m_mesh = kit::Mesh::load(mesh);
  returner->m_skeleton = kit::Skeleton::load(skeleton);
  return returner;
}

kit::Mesh::Ptr kit::Model::getMesh()
{
  return this->m_mesh;
}

kit::Skeleton::Ptr kit::Model::getSkeleton()
{
  return this->m_skeleton;
}

void kit::Model::setInstancing(bool enabled, std::vector< glm::mat4 > transforms)
{
  this->m_instanced = enabled;
  this->m_instanceTransform = transforms;
}


void kit::Model::update(const double & ms)
{
  if(this->m_skeleton != nullptr)
  {
    this->m_skeleton->update(ms);
  }
}

void kit::Model::renderDeferred(kit::Renderer::Ptr renderer)
{
  std::vector<glm::mat4> skinTransform;
  std::vector<glm::mat4> instanceTransform;
  
  if(this->m_skeleton)
  {
    skinTransform = this->m_skeleton->getSkin();
  }
  
  if(this->m_instanced)
  {
    instanceTransform = this->m_instanceTransform;
  }

  this->m_mesh->render(renderer->getActiveCamera(), this->getTransformMatrix(), false, skinTransform, instanceTransform);
}

void kit::Model::renderForward(kit::Renderer::Ptr renderer)
{
  std::vector<glm::mat4> skinTransform;
  std::vector<glm::mat4> instanceTransform;
  
  if(this->m_skeleton)
  {
    skinTransform = this->m_skeleton->getSkin();
  }
  
  if(this->m_instanced)
  {
    instanceTransform = this->m_instanceTransform;
  }

  this->m_mesh->render(renderer->getActiveCamera(), this->getTransformMatrix(), true, skinTransform, instanceTransform);
}

void kit::Model::renderShadows(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
  for (auto &currSubmeshIndex : this->m_mesh->getSubmeshEntries())
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
    bool S = (this->m_skeleton != nullptr);
    bool I = this->m_instanced;

    auto currProgram = kit::Model::getShadowProgram(S, O, I);

    currProgram->setUniformMat4("uniform_mvpMatrix", projectionMatrix * viewMatrix * this->getTransformMatrix());

    if(O)
    {
      currProgram->setUniformTexture("uniform_opacityMask", currMaterial->getOpacityMask());
    }

    if(S)
    {
      currProgram->setUniformMat4v("uniform_bones", this->m_skeleton->getSkin());
    }
    
    if(I)
    {
      currProgram->setUniformMat4v("uniform_instanceTransform", this->m_instanceTransform);
      currSubmesh->renderGeometryInstanced(this->m_instanceTransform.size());
    }
    else
    {
      currSubmesh->renderGeometry();
    }
  }
}

void kit::Model::renderGeometry()
{
  this->m_mesh->renderGeometry();
}

bool kit::Model::isSkinned()
{
  return (this->m_skeleton != nullptr);
}

std::vector<glm::mat4> kit::Model::getSkin()
{
  if (this->m_skeleton == nullptr)
  {
    KIT_ERR("Warning: tried to get skin from non-skinned model");
    return std::vector<glm::mat4>();
  }

  return this->m_skeleton->getSkin();
}

glm::vec3 kit::Model::getBoneWorldPosition(const std::string&bone)
{
  if (!this->m_skeleton)
  {
    KIT_ERR("Warning: tried to get bone position from non-skinned model");
    return glm::vec3();
  }

  kit::Skeleton::Bone::Ptr currBone = this->m_skeleton->getBone(bone);
  if (!currBone)
  {
    KIT_ERR("Warning: tried to get bone position from non-existent bone");
    return glm::vec3();
  }

  return glm::vec3( this->getTransformMatrix() * currBone->m_globalTransform * glm::vec4(0.0, 0.0, 0.0, 1.0));
}

glm::quat kit::Model::getBoneWorldRotation(const std::string&bone)
{
  if (!this->m_skeleton)
  {
    KIT_ERR("Warning: tried to get bone rotation from non-skinned model");
    return glm::quat();
  }

  kit::Skeleton::Bone::Ptr currBone = this->m_skeleton->getBone(bone);
  if (!currBone)
  {
    KIT_ERR("Warning: tried to get bone rotation from non-existent bone");
    return glm::quat();
  }

  // TODO: This (fodderFix) is needed for badly exported/imported models. Fix our importer/blenders exporter/whatever and then remove this.
  glm::quat fodderFix;
  fodderFix = glm::rotate(fodderFix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  //fodderFix = glm::rotate(fodderFix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

  return glm::quat_cast(this->getTransformMatrix() * currBone->m_globalTransform) * fodderFix;
}
