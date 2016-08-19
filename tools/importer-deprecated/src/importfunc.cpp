#include "../include/importfunc.hpp"

#include<Kit/Types.hpp>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#include<list>
#include<map>
#include <sstream>

glm::mat4 glmMat4(const aiMatrix4x4 & from)
{
  glm::mat4 to;
  to[0][0] = from.a1;
  to[1][0] = from.a2;
  to[2][0] = from.a3;
  to[3][0] = from.a4;

  to[0][1] = from.b1;
  to[1][1] = from.b2;
  to[2][1] = from.b3;
  to[3][1] = from.b4;

  to[0][2] = from.c1;
  to[1][2] = from.c2;
  to[2][2] = from.c3;
  to[3][2] = from.c4;

  to[0][3] = from.d1;
  to[1][3] = from.d2;
  to[2][3] = from.d3;
  to[3][3] = from.d4;

  return to;
}

void CopyaiMat(const aiMatrix4x4 *from, glm::mat4 &to) {
  to[0][0] = from->a1; to[1][0] = from->a2;
  to[2][0] = from->a3; to[3][0] = from->a4;
  to[0][1] = from->b1; to[1][1] = from->b2;
  to[2][1] = from->b3; to[3][1] = from->b4;
  to[0][2] = from->c1; to[1][2] = from->c2;
  to[2][2] = from->c3; to[3][2] = from->c4;
  to[0][3] = from->d1; to[1][3] = from->d2;
  to[2][3] = from->d3; to[3][3] = from->d4;
}

bool RecurseDepth(aiNode* currNode, std::string name, int level, DepthResult & out)
{
  if (std::string(currNode->mName.C_Str()).compare(name) == 0)
  {
    out.foundDepth = level;
    out.foundNode = currNode;
    return true;
  }

  for (unsigned int c = 0; c < currNode->mNumChildren; c++)
  {
    if (RecurseDepth(currNode->mChildren[c], name, level + 1, out))
    {
      return true;
    }
  }

  return false;
}

DepthResult FindDepth(aiNode* node, std::string name)
{

  DepthResult returner;
  returner.foundDepth = 0;
  returner.foundNode = nullptr;

  RecurseDepth(node, name, 1, returner);

  return returner;
}

ImportResult ImportMeshData(std::string filename)
{
  ImportResult returner;
  returner.m_Error = true;
  returner.m_ErrorMessage = "Unspecified";

  Assimp::Importer importer;
  importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
  const aiScene* scene = importer.ReadFile(filename, aiProcess_CalcTangentSpace
    | aiProcess_Triangulate
    | aiProcess_JoinIdenticalVertices
    | aiProcess_SortByPType
    | aiProcess_ValidateDataStructure
    | aiProcess_ImproveCacheLocality);


  //glm::mat4 conversionMatrix = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, -1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
  //glm::mat4 normalMatrix = glm::inverse(glm::transpose(conversionMatrix));

  // If the import failed, report it
  if (!scene)
  {
    std::stringstream ss;
    ss << "Importer error: " << importer.GetErrorString();
    returner.m_ErrorMessage = ss.str();
    return returner;
  }

  std::map<std::string, uint32_t> boneIndex;
  std::vector<ImportResult::BoneData> bones;
  uint32_t boneIdMaker = 0;
  returner.m_Skeleton.m_GlobalInverseTransform = glm::inverse(glmMat4(scene->mRootNode->mTransformation));// *conversionMatrix;
  // Import geometry and skeletons
  for (unsigned int currMesh = 0; currMesh < scene->mNumMeshes; currMesh++)
  {
    kit::Geometry currGeometry;
    aiMesh* currMeshPtr = scene->mMeshes[currMesh];
    uint32_t vertexCount = currMeshPtr->mNumVertices;
    uint32_t triangleCount = currMeshPtr->mNumFaces;
    aiString matname;
    scene->mMaterials[currMeshPtr->mMaterialIndex]->Get(AI_MATKEY_NAME, matname);
    std::string currMeshMaterial = matname.C_Str();
    std::string currMeshName = currMeshPtr->mName.C_Str();

    // Fill vertices
    for (unsigned int currVertex = 0; currVertex < vertexCount; currVertex++)
    {
      kit::Vertex currVertexData;

      currVertexData.m_position.x = currMeshPtr->mVertices[currVertex].x;
      currVertexData.m_position.y = currMeshPtr->mVertices[currVertex].y;
      currVertexData.m_position.z = currMeshPtr->mVertices[currVertex].z;
      
      //currVertexData.m_position.x = currMeshPtr->mVertices[currVertex].x;
      //currVertexData.m_position.z = -currMeshPtr->mVertices[currVertex].y;
      //currVertexData.m_position.y = currMeshPtr->mVertices[currVertex].z;

      //currVertexData.m_position = glm::vec3(conversionMatrix * glm::vec4(currMeshPtr->mVertices[currVertex].x, currMeshPtr->mVertices[currVertex].y, currMeshPtr->mVertices[currVertex].z, 1.0));

      if (currMeshPtr->mTextureCoords[0] != nullptr)
      {
        currVertexData.m_texCoords.x = currMeshPtr->mTextureCoords[0][currVertex].x;
        currVertexData.m_texCoords.y = currMeshPtr->mTextureCoords[0][currVertex].y;
      }
      else
      {
        currVertexData.m_texCoords.x = 0.0;
        currVertexData.m_texCoords.y = 0.0;
      }

      if (currMeshPtr->mNormals != nullptr)
      {
        currVertexData.m_normal.x = currMeshPtr->mNormals[currVertex].x;
        currVertexData.m_normal.y = currMeshPtr->mNormals[currVertex].y;
        currVertexData.m_normal.z = currMeshPtr->mNormals[currVertex].z;

        //currVertexData.m_normal.x = currMeshPtr->mNormals[currVertex].x;
        //currVertexData.m_normal.z = -currMeshPtr->mNormals[currVertex].y;
        //currVertexData.m_normal.y = currMeshPtr->mNormals[currVertex].z;

        //currVertexData.m_normal = glm::vec3(normalMatrix * glm::vec4(currMeshPtr->mNormals[currVertex].x, currMeshPtr->mNormals[currVertex].y, currMeshPtr->mNormals[currVertex].z, 0.0));

      }
      else
      {
        currVertexData.m_normal.x = 0.0;
        currVertexData.m_normal.y = 0.0;
        currVertexData.m_normal.z = 0.0;
      }

      if (currMeshPtr->mTangents != nullptr)
      {
        currVertexData.m_tangent.x = currMeshPtr->mTangents[currVertex].x;
        currVertexData.m_tangent.y = currMeshPtr->mTangents[currVertex].y;
        currVertexData.m_tangent.z = currMeshPtr->mTangents[currVertex].z;

        //currVertexData.m_tangent.x = currMeshPtr->mTangents[currVertex].x;
        //currVertexData.m_tangent.z = -currMeshPtr->mTangents[currVertex].y;
        //currVertexData.m_tangent.y = currMeshPtr->mTangents[currVertex].z;

        //currVertexData.m_tangent = glm::vec3(normalMatrix * glm::vec4(currMeshPtr->mTangents[currVertex].x, currMeshPtr->mTangents[currVertex].y, currMeshPtr->mTangents[currVertex].z, 0.0));
      }
      else
      {
        currVertexData.m_tangent.x = 0.0;
        currVertexData.m_tangent.y = 0.0;
        currVertexData.m_tangent.z = 0.0;
      }

      currVertexData.m_boneIDs.x = 0;
      currVertexData.m_boneIDs.y = 0;
      currVertexData.m_boneIDs.z = 0;
      currVertexData.m_boneIDs.w = 0;

      currVertexData.m_boneWeights.x = 0.0f;
      currVertexData.m_boneWeights.y = 0.0f;
      currVertexData.m_boneWeights.z = 0.0f;
      currVertexData.m_boneWeights.w = 0.0f;

      currGeometry.m_vertices.push_back(currVertexData);
    }

    // Fill faces
    for (unsigned int currTriangle = 0; currTriangle < triangleCount; currTriangle++)
    {
      aiFace currFacePtr = currMeshPtr->mFaces[currTriangle];
      if (currFacePtr.mNumIndices == 3)
      {
        currGeometry.m_indices.push_back(currFacePtr.mIndices[0]);
        currGeometry.m_indices.push_back(currFacePtr.mIndices[1]);
        currGeometry.m_indices.push_back(currFacePtr.mIndices[2]);
      }
    }

    // Import skeleton
    {
      // Initialize a cache that helps us keep track of how many weights each vertex has
      std::map<uint32_t, uint32_t> vertexWeightCount;
      for (int i = 0; i < currGeometry.m_vertices.size(); i++)
      {
        vertexWeightCount[i] = 0;
      }

      // Add bones from this mesh
      for (unsigned int currBone = 0; currBone < currMeshPtr->mNumBones; currBone++)
      {
        aiBone* currBonePtr = currMeshPtr->mBones[currBone];
        aiNode* boneNode = scene->mRootNode->FindNode(currBonePtr->mName.C_Str());

        if (boneNode != nullptr)
        {
          // If we dont have this bone already, add it
          if (boneIndex.find(currBonePtr->mName.C_Str()) == boneIndex.end())
          {
            ImportResult::BoneData newBone;
            newBone.m_InvBindPose = glmMat4(currBonePtr->mOffsetMatrix);// *conversionMatrix;
            newBone.m_LocalTransform = glmMat4(boneNode->mTransformation);// *conversionMatrix;
            newBone.m_Id = boneIdMaker++;
            newBone.m_ParentId = 0;
            newBone.m_ParentName = (boneNode->mParent == NULL ? "__NOPARENT__" : boneNode->mParent->mName.C_Str());
            newBone.m_Name = boneNode->mName.C_Str();

            boneIndex[newBone.m_Name] = newBone.m_Id;
            bones.push_back(newBone);
          }

          uint32_t currId = boneIndex[currBonePtr->mName.C_Str()];

          // Add weights from this bone
          for (unsigned int currWeight = 0; currWeight < currBonePtr->mNumWeights; currWeight++)
          {
            aiVertexWeight* currWeightPtr = &currBonePtr->mWeights[currWeight];
            kit::Vertex * currVertex = &currGeometry.m_vertices[currWeightPtr->mVertexId];
            switch (vertexWeightCount.at(currWeightPtr->mVertexId))
            {
            case 0:
              currVertex->m_boneIDs.x = currId;
              currVertex->m_boneWeights.x = currWeightPtr->mWeight;
              vertexWeightCount.at(currWeightPtr->mVertexId)++;
              break;

            case 1:
              currVertex->m_boneIDs.y = currId;
              currVertex->m_boneWeights.y = currWeightPtr->mWeight;
              vertexWeightCount.at(currWeightPtr->mVertexId)++;
              break;

            case 2:
              currVertex->m_boneIDs.z = currId;
              currVertex->m_boneWeights.z = currWeightPtr->mWeight;
              vertexWeightCount.at(currWeightPtr->mVertexId)++;
              break;

            case 3:
              currVertex->m_boneIDs.w = currId;
              currVertex->m_boneWeights.w = currWeightPtr->mWeight;
              vertexWeightCount.at(currWeightPtr->mVertexId)++;
              break;

            default:
              break;
            }
          }

        }
      }
    }

    ImportResult::SubmeshData submesh;
    if (scene->mNumMeshes == 1)
    {
      returner.m_Geometry[currMeshName] = currGeometry;
      submesh.m_GeometryName = currMeshName;
    }
    else
    {
      returner.m_Geometry[currMeshName + std::string(".") + currMeshMaterial] = currGeometry;
      submesh.m_GeometryName = currMeshName + std::string(".") + currMeshMaterial;
    }

    submesh.m_MaterialName = currMeshMaterial;
    returner.m_Meshes[currMeshName].m_Submeshes.push_back(submesh);
  }

  // Find each bones parent,  and then add each bone in our index to our resulting data.
  for (auto & currBone : bones)
  {
    aiNode* boneNode = scene->mRootNode->FindNode(currBone.m_Name.c_str());
    if (boneNode == NULL)
    {
      std::cout << "WARNING: Added bone can't recall its scenenode!" << std::endl;
      continue;
    }

    std::cout << "finding parent for " << currBone.m_Name << std::endl;

    if (boneNode->mParent == NULL)
    {
      std::cout << "No parent scenenode" << std::endl;
      currBone.m_ParentId = 1337;
    }
    else
    {
      bool found = false;
      for (auto & currSubBone : bones)
      {
        if (currSubBone.m_Name == std::string(boneNode->mParent->mName.C_Str()))
        {
          currBone.m_ParentId = boneIndex.at(std::string(boneNode->mParent->mName.C_Str()));
          found = true;
          std::cout << "Found in boneindex: " << currBone.m_ParentId << " (" << boneNode->mParent->mName.C_Str() << ")" << std::endl;
        }
      }
      if (!found)
      {
        currBone.m_ParentId = 1337;
        std::cout << "Not found in index either " << std::endl;
      }
    }
    returner.m_Skeleton.m_BoneData.push_back(currBone);
  }

  // Import animations
  for (uint32_t currAnimation = 0; currAnimation < scene->mNumAnimations; currAnimation++)
  {
    aiAnimation* currAnimPtr = scene->mAnimations[currAnimation];
    std::string currAnimName = currAnimPtr->mName.C_Str();
    if (currAnimName.substr(0, 11) == std::string("AnimStack::"))
    {
      currAnimName = currAnimName.substr(11);
    }

    ImportResult::AnimationData newAnimation;
    newAnimation.m_Name = currAnimName;
    newAnimation.m_FPS = (float)currAnimPtr->mTicksPerSecond;
    newAnimation.m_Duration = (float)currAnimPtr->mDuration;

    std::cout << "Importing animation " << newAnimation.m_Name.c_str() << ": " << newAnimation.m_FPS << " FPS, duration " <<  newAnimation.m_Duration << std::endl;
    
    bool IgnoreAnimation = true; //< Set to false if we find channel matching a bone in the current mesh
    for (uint32_t currChannel = 0; currChannel < currAnimPtr->mNumChannels; currChannel++)
    {
      aiNodeAnim* currChannelPtr = currAnimPtr->mChannels[currChannel];
      std::string currChannelName = currChannelPtr->mNodeName.C_Str();

      std::cout << "Importing from channel " << currChannel << ", named " << currChannelName.c_str() << std::endl;
      
      // Only import data from this channel if it affects any of our bones
      if (boneIndex.find(currChannelName) != boneIndex.end())
      {
        IgnoreAnimation = false;

        ImportResult::ChannelData newChannel;
        newChannel.m_BoneID = boneIndex.at(currChannelName);
        
        std::cout << "Channel is active for bone #" << newChannel.m_BoneID << std::endl;
        std::cout << "Importing " << currChannelPtr->mNumPositionKeys << " pos, " << currChannelPtr->mNumRotationKeys << " rot and " <<  currChannelPtr->mNumScalingKeys<< " sca keys " << std::endl;
        
        // Translation keys
        for (uint32_t currTKey = 0; currTKey < currChannelPtr->mNumPositionKeys; currTKey++)
        {
          aiVectorKey currKey = currChannelPtr->mPositionKeys[currTKey];
          ImportResult::KeyData3 newKey;

          // glm::vec3 key = glm::vec3(conversionMatrix * glm::vec4(currKey.mValue.x, currKey.mValue.y, currKey.mValue.z, 1.0));

          glm::vec3 key = glm::vec3(currKey.mValue.x, currKey.mValue.y, currKey.mValue.z);

          newKey.m_Time = (float)currKey.mTime;
          newKey.m_X = key.x;
          newKey.m_Y = key.y;
          newKey.m_Z = key.z;
          newChannel.m_TranslationKeys.push_back(newKey);
        }

        // Rotation keys
        for (uint32_t currRKey = 0; currRKey < currChannelPtr->mNumRotationKeys; currRKey++)
        {
          aiQuatKey currKey = currChannelPtr->mRotationKeys[currRKey];
          ImportResult::KeyData4 newKey;
          newKey.m_Time = (float)currKey.mTime;
          newKey.m_X = currKey.mValue.x;
          newKey.m_Y = currKey.mValue.y;
          newKey.m_Z = currKey.mValue.z;
          newKey.m_W = currKey.mValue.w;
          // (a,-b,-d,-c)
          newChannel.m_RotationKeys.push_back(newKey);
        }

        // Scale keys
        for (uint32_t currSKey = 0; currSKey < currChannelPtr->mNumScalingKeys; currSKey++)
        {
          aiVectorKey currKey = currChannelPtr->mScalingKeys[currSKey];
          ImportResult::KeyData3 newKey;
          newKey.m_Time = (float)currKey.mTime;
          newKey.m_X = currKey.mValue.x;
          newKey.m_Y = currKey.mValue.y;
          newKey.m_Z = currKey.mValue.z;
          newChannel.m_ScaleKeys.push_back(newKey);
        }

        newAnimation.m_Channels.push_back(newChannel);
      }
    }

    // If we found any relevant channels for this mesh in this animation, add it to our result.
    if (!IgnoreAnimation)
    {
      std::map<uint32_t, ImportResult::ChannelData> sorter;
      for (auto & currChannel : newAnimation.m_Channels)
      {
        sorter[currChannel.m_BoneID] = currChannel;
      }

      newAnimation.m_Channels.clear();
      for (auto & currSort : sorter)
      {
        newAnimation.m_Channels.push_back(currSort.second);
      }

      returner.m_Skeleton.m_AnimationData.push_back(newAnimation);
    }
  }

  returner.m_Error = false;
  return returner;
}