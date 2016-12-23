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
}

kit::Skeleton::~Skeleton()
{
  for(auto c : m_boneIndexId)
  {
    if(c) delete c;
  }
  
  for(auto c : m_animations)
  {
    if(c.second) delete c.second;
  }
}

kit::Skeleton::Skeleton(const std::string&filename)
{
  std::ifstream s(std::string(std::string("./data/skeletons/") + filename).c_str(), std::ios::in | std::ios::binary);

  if(!s)
  {
    KIT_THROW("Couldn't open file for reading");
  }
  
  if (memcmp(&kit::readBytes(s, 4)[0], "KSKE", 4) != 0)
  {
    KIT_THROW("Bad signature");
  }

  
  uint32_t numBones = kit::readUint32(s);
  m_boneIndexId = std::vector<Bone*>(numBones);
  m_inverseBindPose = std::vector<glm::mat4>(numBones);
  m_skin = std::vector<glm::mat4>(numBones);

  uint32_t numAnimations = kit::readUint32(s);
  
  m_globalInverseTransform = kit::readMat4(s);
  
  // ... Bones
  for(uint32_t i = 0; i < numBones; i++)
  {
    kit::Skeleton::Bone * newBone = new Bone();

    newBone->m_id              = kit::readUint32(s);
    newBone->m_parentId        = kit::readUint32(s);
    newBone->m_name            = kit::readString(s);
    newBone->m_localTransform  = kit::readMat4(s);
    newBone->m_globalTransform = glm::mat4(1.0);

    m_inverseBindPose[newBone->m_id]  = kit::readMat4(s);
    m_boneIndexId[newBone->m_id] = newBone;
    m_boneIndexName[newBone->m_name] = newBone;
  }

  // Fill in parents, rootbones and children
  for (auto & currBone : m_boneIndexId)
  {
    if (currBone->m_parentId == 1337)
    {
      m_rootBones.push_back(currBone);
    }
    else
    {
      currBone->m_parent = getBone(currBone->m_parentId);
    }

    for (auto & currBoneB : m_boneIndexId)
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
    kit::Skeleton::Animation* newAnimation = new kit::Skeleton::Animation();

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
    
    m_animations[newAnimation->m_name] = newAnimation;
  }
  
  s.close();
}

void kit::Skeleton::update(const double & ms)
{
  if (m_isPlaying)
  {
    float totalAnimTime = m_currentAnimation->m_frameDuration * m_currentAnimation->m_framesPerSecond;
    float currDelta = float(m_currentTime) / totalAnimTime;
    m_currentFrame = currDelta * m_currentAnimation->m_frameDuration;

    m_currentTime += ms;
    if (m_currentTime >= totalAnimTime) {
      if (m_isLooping)
      {
        while (m_currentTime >= totalAnimTime)
        {
          m_currentTime -= totalAnimTime;
        }
      }
      else
      {
        stop();
        if(m_callbackOnDone)
        {
          m_callback();
          m_callback = nullptr;
          m_callbackOnDone = false;
        }
      }
    }

    updateSkin();

  }
}

glm::quat kit::Skeleton::Bone::getCurrentRotation()
{
  kit::Transformable returner;

  glm::vec3 scale, translation, skew;
  glm::vec4 perspective;
  glm::quat orientation;
  glm::decompose(m_globalTransform, scale, orientation, translation, skew, perspective);

  return glm::inverse(orientation);
}

glm::vec3 kit::Skeleton::Bone::getCurrentPosition()
{
  kit::Transformable returner;

  glm::vec3 scale, translation, skew;
  glm::vec4 perspective;
  glm::quat orientation;
  glm::decompose(m_globalTransform, scale, orientation, translation, skew, perspective);

  return translation;
}

void kit::Skeleton::updateSkin()
{
  // Create a workload and fill it with the rootnodes
  std::queue<WorkPair> workload;
  for (auto & currBone : m_rootBones)
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
    Bone* currBone = currPair.second;

    //std::cout << "Updating bone " << currBone->m_name << ", son of " << (currBone->m_parent ? currBone->m_parent->m_name : "nobody") << ", has " << currBone->m_children.size() << " children" << std::endl;

    // Animate the current bones local transformation
    if (m_currentAnimation)
    {
      currBone->m_localTransform = m_currentAnimation->getBoneTransform(currBone->m_id, m_currentFrame);
    }

    // Inherit the global transformation
    currBone->m_globalTransform = parentTransform * currBone->m_localTransform;

    // Update the skin for this bone
    m_skin[currBone->m_id] = m_globalInverseTransform * currBone->m_globalTransform * m_inverseBindPose[currBone->m_id];

    // Add the children of this bone to the workload
    for (auto & currChild : currBone->m_children)
    {
      workload.push(WorkPair(currBone->m_globalTransform, currChild));
    }
  }
}

void kit::Skeleton::pause()
{
  m_isPlaying = false;
}

void kit::Skeleton::play(bool loop)
{
  m_isPlaying = true;
  m_isLooping = loop;
}

void kit::Skeleton::setAnimation(const std::string&name)
{
  auto newAnim = getAnimation(name);
  if (newAnim != m_currentAnimation)
  {
    m_currentAnimation = newAnim;
    stop();
  }
}

void kit::Skeleton::stop()
{
  m_isPlaying = false;
  m_currentTime = 0.0;
}

std::vector< glm::mat4 > kit::Skeleton::getSkin()
{
  return m_skin;
}

kit::Skeleton::Bone * kit::Skeleton::getBone(const std::string&name)
{
  if (m_boneIndexName.find(name) != m_boneIndexName.end())
  {
    return m_boneIndexName.at(name);
  }
  else
  {
    KIT_ERR("Warning: Could not find bone by name");
    return nullptr;
  }
}

kit::Skeleton::Bone * kit::Skeleton::getBone(uint32_t id)
{
  return m_boneIndexId[id];
}

kit::Skeleton::Animation * kit::Skeleton::getAnimation(const std::string&animationname)
{
  if (m_animations.find(animationname) != m_animations.end())
  {
    return m_animations.at(animationname);
  }
  else
  {
    KIT_ERR("Warning: Could not find animation by name");
    return nullptr;
  }
}

glm::quat kit::Skeleton::AnimationChannel::getRotationAt(float mstime)
{
  if(m_rotationKeys.size() == 0)
  {
    return glm::quat();
  }
  
  if(m_rotationKeys.size() == 1)
  {
    return m_rotationKeys.begin()->second;
  }
  
  glm::quat a, b;
  float ad;//, bd;
  std::map<float, glm::quat>::iterator curr = m_rotationKeys.find(mstime);
  
  if(curr != m_rotationKeys.end())
  {
    a = curr->second;
    ad = curr->first;
  }
  else
  {
    std::map<float, glm::quat>::iterator lwr = m_rotationKeys.lower_bound(mstime);
    if(lwr != m_rotationKeys.begin())
    {
    lwr--; 
    }
    
    a = lwr->second;
    ad =  lwr->first;
  }
  
  std::map<float, glm::quat>::iterator hgr = m_rotationKeys.upper_bound(mstime);
  if (hgr != m_rotationKeys.end())
  {
    b = hgr->second;
  }
  else
  {
    b = m_rotationKeys.at(m_rotationKeys.rbegin()->first);
  }
  //bd = hgr->first;
  
  return glm::normalize(glm::slerp(a, b, mstime -  ad));
  //return a;
}

glm::vec3 kit::Skeleton::AnimationChannel::getScaleAt(float mstime)
{

  if(m_scaleKeys.size() == 0)
  {
    return glm::vec3(1.0f, 1.0f, 1.0f);
  }
  
  if(m_scaleKeys.size() == 1)
  {
    return m_scaleKeys.begin()->second;
  }
  
  glm::vec3 a, b;
  float ad;//, bd;
  std::map<float, glm::vec3>::iterator curr = m_scaleKeys.find(mstime);

  if (curr != m_scaleKeys.end())
  {
    a = curr->second;
    ad = curr->first;
  }
  else
  {
    std::map<float, glm::vec3>::iterator lwr = m_scaleKeys.lower_bound(mstime);
    if (lwr != m_scaleKeys.begin())
    {
      lwr--;
    }

    a = lwr->second;
    ad = lwr->first;
  }

  std::map<float, glm::vec3>::iterator hgr = m_scaleKeys.upper_bound(mstime);
  if (hgr != m_scaleKeys.end())
  {
    b = hgr->second;
  }
  else
  {
    b = m_scaleKeys.at(m_scaleKeys.rbegin()->first);
  }
  //bd = hgr->first;

  return glm::mix(a, b, mstime - ad);
  //return a;
}

glm::mat4 kit::Skeleton::Animation::getBoneTransform(uint32_t id, float mstime)
{
  auto channel = m_channels.at(id);
  return channel.getTransformMatrix(mstime);
}

glm::mat4 kit::Skeleton::AnimationChannel::getTransformMatrix(float mstime)
{
  
  glm::mat4 S = glm::scale(glm::mat4(1.0f), getScaleAt(mstime));
  glm::mat4 R = glm::mat4_cast(getRotationAt(mstime));
  glm::mat4 T = glm::translate(glm::mat4(1.0f), getTranslationAt(mstime));

  glm::mat4 M = T * R * S;
  return M;
}

glm::vec3 kit::Skeleton::AnimationChannel::getTranslationAt(float mstime)
{
  if(m_translationKeys.size() == 0)
  {
    return glm::vec3(0.0f, 0.0f, 0.0f);
  }
  
  if(m_translationKeys.size() == 1)
  {
    return m_translationKeys.begin()->second;
  }
  
  glm::vec3 a, b;
  float ad;//, bd;
  std::map<float, glm::vec3>::iterator curr = m_translationKeys.find(mstime);
  
  if(curr != m_translationKeys.end())
  {
    a = curr->second;
    ad = curr->first;
  }
  else
  {
    std::map<float, glm::vec3>::iterator lwr = m_translationKeys.lower_bound(mstime);
    if(lwr != m_translationKeys.begin())
    {
    lwr--; 
    }
    
    a = lwr->second;
    ad =  lwr->first;
  }
  
  std::map<float, glm::vec3>::iterator hgr = m_translationKeys.upper_bound(mstime);
  if (hgr != m_translationKeys.end())
  {
    b = hgr->second;
  }
  else
  {
    b = m_translationKeys.at(m_translationKeys.rbegin()->first);
  }
  
  return glm::mix(a, b, mstime -  ad);
  //return a;
}

bool kit::Skeleton::isPlaying()
{
  return m_isPlaying;
}

void kit::Skeleton::playAnimation(const std::string& name, kit::Skeleton::PlaybackDoneCallback callback)
{
  setAnimation(name);
  play(false);
  m_callbackOnDone = true;
  m_callback = callback;
}

