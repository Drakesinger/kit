#ifndef IMPORTFUNC_HPP
#define IMPORTFUNC_HPP

#include<iostream>
#include<fstream>
#include<Kit/Types.hpp>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#ifdef _WIN32
#include<winsock2.h> // ntohl/htonl
#elif __unix
#include<arpa/inet.h> // ntohl/htonl
#endif
#include<cstdint>
#include<list>
#include<map>

glm::mat4 glmMat4(const aiMatrix4x4 & from);

struct DepthResult
{
  uint32_t foundDepth;
  aiNode* foundNode;
};

bool RecurseDepth(aiNode* currNode, std::string name, int level, DepthResult & out);

DepthResult FindDepth(aiNode* node, std::string name);

struct ImportResult
{

  struct BoneData
  {
    glm::mat4 m_LocalTransform;   // Bones current local transform
    glm::mat4 m_InvBindPose;
    std::string m_Name;
    std::string m_ParentName;
    uint32_t m_Id;
    uint32_t m_ParentId;       // ID to parent bone
  };

  struct KeyData3
  {
    float m_Time;
    float m_X;
    float m_Y;
    float m_Z;
  };

  struct KeyData4
  {
    float m_Time;
    float m_X;
    float m_Y;
    float m_Z;
    float m_W;
  };

  struct ChannelData
  {
    uint32_t m_BoneID;
    std::vector<KeyData3> m_TranslationKeys;
    std::vector<KeyData4> m_RotationKeys;
    std::vector<KeyData3> m_ScaleKeys;
  };

  struct AnimationData
  {
    std::string m_Name;
    float m_FPS;
    float m_Duration;
    std::vector<ChannelData> m_Channels;
  };

  struct SkeletonData
  {
    glm::mat4 m_GlobalInverseTransform;
    std::vector<BoneData>           m_BoneData;
    std::vector<AnimationData>      m_AnimationData;
  };


  struct SubmeshData
  {
    std::string m_MaterialName;
    std::string m_GeometryName;
  };

  struct MeshData
  {
    std::vector<SubmeshData> m_Submeshes;
  };

  std::map<std::string, kit::Geometry> m_Geometry;
  std::map<std::string, MeshData> m_Meshes;
  SkeletonData m_Skeleton;


  bool m_Error;
  std::string m_ErrorMessage;
};

ImportResult ImportMeshData(std::string filename);

#endif // IMPORTFUNC_H
