#include <iostream>
#include <string>
#include "../include/importfunc.hpp"
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#elif __unix__
#include <QApplication>
#include <QObject>
#include <QFileDialog>

#endif

std::string selectFile()
{
#ifdef _WIN32
  OPENFILENAME ofn;
  char szFile[100];
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "All\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

  char CWD[MAX_PATH];
  _getcwd(CWD, MAX_PATH);
  if (GetOpenFileName(&ofn) == 0)
  {
    return "";
  }
  _chdir(CWD);
  return std::string(ofn.lpstrFile);
#elif __unix__
  int argc = 2;
  char const *argv[] = {"kit-importer", "foo"};
  QApplication a(argc, const_cast<char**>(argv));
  std::string filename=  QFileDialog::getOpenFileName(nullptr, QObject::tr("Select .FBX"), "/home", QObject::tr("FBX files (*.fbx)")).toStdString();
  //a.exec();
  return filename;
#endif
}

int main(int argc, char * argv[])
{
  kit::createDirectory("./import");
  kit::createDirectory("./import/meshes");
  kit::createDirectory("./import/geometry");
  kit::createDirectory("./import/skeletons");
  std::string filename = selectFile();
  if (filename.size() == 0)
  {
    return 0;
  }

  ImportResult result = ImportMeshData(filename);
  if (result.m_Error)
  {
    std::cout << "Error: " << result.m_ErrorMessage << std::endl;
    return 1;
  }

  std::string gdatadir = "./import/geometry/";
  std::string mdatadir = "./import/meshes/";
  std::string sdatadir = "./import/skeletons/";

  std::string name = "";

  for (auto & currGeometry : result.m_Geometry)
  {
    std::cout << "Importing geometry " << currGeometry.first << ", " << currGeometry.second.m_vertices.size() << " vertices, " << (currGeometry.second.m_indices.size() / 3) << " triangles" << std::endl;
    if (!currGeometry.second.save(gdatadir + currGeometry.first + std::string(".geometry")))
    {
      std::cout << "Error while importing geometry!" << std::endl;
    }
  }

  for (auto & currMesh : result.m_Meshes)
  {
    if (name == std::string("")) name = currMesh.first;
    std::cout << "Importing mesh " << currMesh.first << std::endl;
    std::ofstream handle(mdatadir + currMesh.first + std::string(".mesh"));
    if (!handle.is_open())
    {
      std::cout << "Error while writing mesh!" << std::endl;
      continue;
    }

    int i = 0;
    for (auto & currSubmesh : currMesh.second.m_Submeshes)
    {
      handle << "submesh \"submesh-" << i++ << "\" \"" << currSubmesh.m_GeometryName << ".geometry\" \"" << currSubmesh.m_MaterialName << ".material\"" << std::endl;
    }

    handle.close();
  }

  std::ofstream shandle(sdatadir + name + std::string(".skeleton"), std::ios::binary | std::ios::out);
  auto & skeleton = result.m_Skeleton;
  if (shandle.is_open())
  {
    std::cout << "Writing skeleton " << name << std::endl;
    kit::writeBytes(shandle, {'K', 'S', 'K', 'E'});
    kit::writeUint32(shandle, skeleton.m_BoneData.size());
    kit::writeUint32(shandle, skeleton.m_AnimationData.size());
    kit::writeMat4(shandle, skeleton.m_GlobalInverseTransform);

    for (int i = 0; i < skeleton.m_BoneData.size(); i++)
    {
      auto & currBone = skeleton.m_BoneData[i];
      std::cout << "Writing bone " << currBone.m_Id << " with parent " << currBone.m_ParentId << " (" << currBone.m_Name << ")" << std::endl;
      kit::writeUint32(shandle, currBone.m_Id);
      kit::writeUint32(shandle, currBone.m_ParentId);
      kit::writeString(shandle, currBone.m_Name);
      kit::writeMat4(shandle, currBone.m_LocalTransform);
      kit::writeMat4(shandle, currBone.m_InvBindPose);
    }

    for (uint32_t i = 0; i < skeleton.m_AnimationData.size(); i++)
    {
      auto & currAnim = skeleton.m_AnimationData[i];
      std::cout << "Writing animation " << currAnim.m_Name << ", " << currAnim.m_Channels.size() << " channels, " << currAnim.m_Duration << " frames @ " << currAnim.m_FPS << " FPS" << std::endl;
      kit::writeString(shandle, currAnim.m_Name);
      kit::writeUint32(shandle, currAnim.m_Channels.size());
      kit::writeFloat(shandle, currAnim.m_FPS);
      kit::writeFloat(shandle, currAnim.m_Duration);

      for (int j = 0; j < currAnim.m_Channels.size(); j++)
      {
        auto & currChannel = currAnim.m_Channels[j];
        kit::writeUint32(shandle, currChannel.m_BoneID);

        kit::writeUint32(shandle, currChannel.m_TranslationKeys.size());
        kit::writeUint32(shandle, currChannel.m_RotationKeys.size());
        kit::writeUint32(shandle, currChannel.m_ScaleKeys.size());

        for (int ctk = 0; ctk < currChannel.m_TranslationKeys.size(); ctk++)
        {
          kit::writeFloat(shandle, currChannel.m_TranslationKeys[ctk].m_Time);
          kit::writeFloat(shandle, currChannel.m_TranslationKeys[ctk].m_X);
          kit::writeFloat(shandle, currChannel.m_TranslationKeys[ctk].m_Y);
          kit::writeFloat(shandle, currChannel.m_TranslationKeys[ctk].m_Z);
        }

        for (int rtk = 0; rtk < currChannel.m_RotationKeys.size(); rtk++)
        {
          kit::writeFloat(shandle, currChannel.m_RotationKeys[rtk].m_Time);
          kit::writeFloat(shandle, currChannel.m_RotationKeys[rtk].m_X);
          kit::writeFloat(shandle, currChannel.m_RotationKeys[rtk].m_Y);
          kit::writeFloat(shandle, currChannel.m_RotationKeys[rtk].m_Z);
          kit::writeFloat(shandle, currChannel.m_RotationKeys[rtk].m_W);
        }

        for (int stk = 0; stk < currChannel.m_ScaleKeys.size(); stk++)
        {
          kit::writeFloat(shandle, currChannel.m_ScaleKeys[stk].m_Time);
          kit::writeFloat(shandle, currChannel.m_ScaleKeys[stk].m_X);
          kit::writeFloat(shandle, currChannel.m_ScaleKeys[stk].m_Y);
          kit::writeFloat(shandle, currChannel.m_ScaleKeys[stk].m_Z);
        }
      }
    }

    shandle.close();
  }
  else
  {
    std::cout << "Error while writing skeleton!" << std::endl;
  }

#ifdef _WIN32
  std::cout << "Press any key to quit..." << std::endl;
  std::cin.get();
#endif

  return 0;
}