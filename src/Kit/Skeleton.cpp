#include "Kit/Skeleton.hpp"

#ifdef _WIN32
	#include <winsock2.h> // ntohl/htonl
#elif __unix
	#include <arpa/inet.h> // ntohl/htonl
#endif

#include <cstring> // memcmp

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <sstream>
#include <queue>

kit::Skeleton::Skeleton()
{
  this->m_currentTime = 0.0;
  this->m_isLooping = false;
  this->m_isPlaying = false;
}

kit::Skeleton::~Skeleton()
{

}

kit::Skeleton::Ptr kit::Skeleton::load(const std::string&filename)
{
  kit::Skeleton::Ptr returner = std::make_shared<kit::Skeleton>();

  std::ifstream s(std::string(std::string("./data/skeletons/") + filename).c_str(), std::ios::in | std::ios::binary);
  
  std::cout << "Loading skeleton " << filename.c_str() << std::endl;

  if(!s)
  {
    KIT_THROW("Couldn't open file for reading");
  }
  
  if (memcpy(&kit::readBytes(s, 4)[0], "KSKE", 4) != 0)
  {
    KIT_THROW("Bad signature");
  }

  
  uint32_t numBones = kit::readUint32(s);
  returner->m_boneIndexId = std::vector<Bone::Ptr>(numBones);
  returner->m_inverseBindPose = std::vector<glm::mat4>(numBones);
  returner->m_skin = std::vector<glm::mat4>(numBones);

  uint32_t numAnimations = kit::readUint32(s);
  
  returner->m_globalInverseTransform = kit::readMat4(s);
  
  // ... Bones
  for(uint32_t i = 0; i < numBones; i++)
  {
    kit::Skeleton::Bone::Ptr newBone = std::make_shared<Bone>();

    newBone->m_id              = kit::readUint32(s);
    newBone->m_parentId        = kit::readUint32(s);
    newBone->m_name            = kit::readString(s);
    newBone->m_localTransform  = kit::readMat4(s);
    newBone->m_globalTransform = glm::mat4(1.0);

    returner->m_inverseBindPose[newBone->m_id]  = kit::readMat4(s);
    returner->m_boneIndexId[newBone->m_id] = newBone;
    returner->m_boneIndexName[newBone->m_name] = newBone;
  }

  // Fill in parents, rootbones and children
  for (auto & currBone : returner->m_boneIndexId)
  {
    if (currBone->m_parentId == 1337)
    {
      returner->m_rootBones.push_back(currBone);
    }
    else
    {
      currBone->m_parent = returner->getBone(currBone->m_parentId);
    }

    for (auto & currBoneB : returner->m_boneIndexId)
    {
      if (currBoneB->m_parentId == currBone->m_id)
      {
        currBone->m_children.push_back(currBoneB);
      }
    }
  }

  
  // .. Animations
  for(uint32_t i = 0; i < numAnimations; i++)
  {
    kit::Skeleton::Animation::Ptr newAnimation = std::make_shared<Animation>();

    newAnimation->m_name = kit::readString(s);
    uint32_t numChannels = kit::readUint32(s);
    newAnimation->m_framesPerSecond = kit::readFloat(s);
    newAnimation->m_frameDuration = kit::readFloat(s);
    
    // Channels
    for(uint32_t ic = 0; ic < numChannels; ic++)
    {
      kit::Skeleton::AnimationChannel newChannel;
      
      uint32_t boneId = readUint32(s);
      
      uint32_t numtk = kit::readUint32(s);
      uint32_t numrk = kit::readUint32(s);
      uint32_t numsk = kit::readUint32(s);

      for(uint32_t ctk = 0; ctk < numtk; ctk++)
      {
        float time = kit::readFloat(s);
        newChannel.m_translationKeys[time] = kit::readVec3(s);
      }

      for(uint32_t crk = 0; crk < numrk; crk++)
      {
        float time = kit::readFloat(s);
        newChannel.m_rotationKeys[time] = kit::readQuat(s);
      }
      
      for(uint32_t csk = 0; csk < numsk; csk++)
      {
        float time = kit::readFloat(s);
        newChannel.m_scaleKeys[time] = kit::readVec3(s);
      }
      
      newAnimation->m_channels[boneId] = newChannel;
    }
    
    returner->m_animations[newAnimation->m_name] = newAnimation;
  }
  
  s.close();

  return returner;
}

void kit::Skeleton::update(const double & ms)
{
  if (this->m_isPlaying)
  {
    float totalAnimTime = this->m_currentAnimation->m_frameDuration * this->m_currentAnimation->m_framesPerSecond;
    float currDelta = float(this->m_currentTime) / totalAnimTime;
    this->m_currentFrame = currDelta * this->m_currentAnimation->m_frameDuration;

    this->m_currentTime += ms;
    if (this->m_currentTime >= totalAnimTime) {
      if (this->m_isLooping)
      {
        while (this->m_currentTime >= totalAnimTime)
        {
          this->m_currentTime -= totalAnimTime;
        }
      }
      else
      {
        this->stop();
      }
    }

    this->updateSkin();

  }
}

glm::quat kit::Skeleton::Bone::getCurrentRotation()
{
  kit::Transformable returner;

  glm::vec3 scale, translation, skew;
  glm::vec4 perspective;
  glm::quat orientation;
  glm::decompose(this->m_globalTransform, scale, orientation, translation, skew, perspective);

  return glm::inverse(orientation);
}

glm::vec3 kit::Skeleton::Bone::getCurrentPosition()
{
  kit::Transformable returner;

  glm::vec3 scale, translation, skew;
  glm::vec4 perspective;
  glm::quat orientation;
  glm::decompose(this->m_globalTransform, scale, orientation, translation, skew, perspective);

  return translation;
}

void kit::Skeleton::updateSkin()
{
  // Create a workload and fill it with the rootnodes
  std::queue<WorkPair> workload;
  for (auto & currBone : this->m_rootBones)
  {
    workload.push(WorkPair(glm::mat4(1.0), currBone));
  }

  // Work it off until empty
  while (!workload.empty())
  {
    // Fetch the workpair at the front of the line
    WorkPair currPair = workload.front();
    workload.pop();

    glm::mat4 parentTransform = currPair.first;
    Bone::Ptr currBone = currPair.second;

    //std::cout << "Updating bone " << currBone->m_name << ", son of " << (currBone->m_parent ? currBone->m_parent->m_name : "nobody") << ", has " << currBone->m_children.size() << " children" << std::endl;

    // Animate the current bones local transformation
    if (this->m_currentAnimation)
    {
      currBone->m_localTransform = this->m_currentAnimation->getBoneTransform(currBone->m_id, this->m_currentFrame);
    }

    // Inherit the global transformation
    currBone->m_globalTransform = parentTransform * currBone->m_localTransform;

    // Update the skin for this bone
    this->m_skin[currBone->m_id] = this->m_globalInverseTransform * currBone->m_globalTransform * this->m_inverseBindPose[currBone->m_id];

    // Add the children of this bone to the workload
    for (auto & currChild : currBone->m_children)
    {
      workload.push(WorkPair(currBone->m_globalTransform, currChild));
    }
  }
}

void kit::Skeleton::pause()
{
  this->m_isPlaying = false;
}

void kit::Skeleton::play(bool loop)
{
  this->m_isPlaying = true;
  this->m_isLooping = loop;
}

void kit::Skeleton::setAnimation(const std::string&name)
{
  auto newAnim = this->getAnimation(name);
  if (newAnim != this->m_currentAnimation)
  {
    this->m_currentAnimation = newAnim;
    this->stop();
  }
}

void kit::Skeleton::stop()
{
  this->m_isPlaying = false;
  this->m_currentTime = 0.0;
}

std::vector< glm::mat4 > kit::Skeleton::getSkin()
{
  return this->m_skin;
}

kit::Skeleton::Bone::Ptr kit::Skeleton::getBone(const std::string&name)
{
  if (this->m_boneIndexName.find(name) != this->m_boneIndexName.end())
  {
    return this->m_boneIndexName.at(name);
  }
  else
  {
    KIT_ERR("Warning: Could not find bone by name");
    return nullptr;
  }
}

kit::Skeleton::Bone::Ptr kit::Skeleton::getBone(uint32_t id)
{
  return this->m_boneIndexId[id];
}

kit::Skeleton::Animation::Ptr kit::Skeleton::getAnimation(const std::string&animationname)
{
  if (this->m_animations.find(animationname) != this->m_animations.end())
  {
    return this->m_animations.at(animationname);
  }
  else
  {
    KIT_ERR("Warning: Could not find animation by name");
    return nullptr;
  }
}

glm::quat kit::Skeleton::AnimationChannel::getRotationAt(float mstime)
{
  glm::quat a, b;
  float ad;//, bd;
  std::map<float, glm::quat>::iterator curr = this->m_rotationKeys.find(mstime);
  
  if(curr != this->m_rotationKeys.end())
  {
    a = curr->second;
    ad = curr->first;
  }
  else
  {
    std::map<float, glm::quat>::iterator lwr = this->m_rotationKeys.lower_bound(mstime);
    if(lwr != this->m_rotationKeys.begin())
    {
    lwr--; 
    }
    
    a = lwr->second;
    ad =  lwr->first;
  }
  
  std::map<float, glm::quat>::iterator hgr = this->m_rotationKeys.upper_bound(mstime);
  if (hgr != this->m_rotationKeys.end())
  {
    b = hgr->second;
  }
  else
  {
    b = this->m_rotationKeys.at(this->m_rotationKeys.rbegin()->first);
  }
  //bd = hgr->first;
  
  return glm::normalize(glm::slerp(a, b, mstime -  ad));
  //return a;
}

glm::vec3 kit::Skeleton::AnimationChannel::getScaleAt(float mstime)
{
  glm::vec3 a, b;
  float ad;//, bd;
  std::map<float, glm::vec3>::iterator curr = this->m_scaleKeys.find(mstime);

  if (curr != this->m_scaleKeys.end())
  {
    a = curr->second;
    ad = curr->first;
  }
  else
  {
    std::map<float, glm::vec3>::iterator lwr = this->m_scaleKeys.lower_bound(mstime);
    if (lwr != this->m_scaleKeys.begin())
    {
      lwr--;
    }

    a = lwr->second;
    ad = lwr->first;
  }

  std::map<float, glm::vec3>::iterator hgr = this->m_scaleKeys.upper_bound(mstime);
  if (hgr != this->m_scaleKeys.end())
  {
    b = hgr->second;
  }
  else
  {
    b = this->m_scaleKeys.at(this->m_scaleKeys.rbegin()->first);
  }
  //bd = hgr->first;

  return glm::mix(a, b, mstime - ad);
  //return a;
}

glm::mat4 kit::Skeleton::Animation::getBoneTransform(uint32_t id, float mstime)
{
  return this->m_channels.at(id).getTransformMatrix(mstime);
}

glm::mat4 kit::Skeleton::AnimationChannel::getTransformMatrix(float mstime)
{
  glm::mat4 S = glm::scale(glm::mat4(1), this->getScaleAt(mstime));
  glm::mat4 R = glm::mat4_cast(this->getRotationAt(mstime));
  glm::mat4 T = glm::translate(glm::mat4(1), this->getTranslationAt(mstime));

  glm::mat4 M = T * R * S;
  return M;
}

glm::vec3 kit::Skeleton::AnimationChannel::getTranslationAt(float mstime)
{
  glm::vec3 a, b;
  float ad;//, bd;
  std::map<float, glm::vec3>::iterator curr = this->m_translationKeys.find(mstime);
  
  if(curr != this->m_translationKeys.end())
  {
    a = curr->second;
    ad = curr->first;
  }
  else
  {
    std::map<float, glm::vec3>::iterator lwr = this->m_translationKeys.lower_bound(mstime);
    if(lwr != this->m_translationKeys.begin())
    {
    lwr--; 
    }
    
    a = lwr->second;
    ad =  lwr->first;
  }
  
  std::map<float, glm::vec3>::iterator hgr = this->m_translationKeys.upper_bound(mstime);
  if (hgr != this->m_translationKeys.end())
  {
    b = hgr->second;
  }
  else
  {
    b = this->m_translationKeys.at(this->m_translationKeys.rbegin()->first);
  }
  
  return glm::mix(a, b, mstime -  ad);
  //return a;
}
