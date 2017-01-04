#include "Kit/Mesh.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Submesh.hpp"
#include "Kit/Camera.hpp"
#include "Kit/Material.hpp"
#include "Kit/ConvexHull.hpp"

#include <fstream>

kit::Mesh::Mesh()
{

}

kit::Mesh::~Mesh()
{
  std::cout << "Removing mesh" << std::endl;
  m_submeshEntries.clear();
}

kit::Mesh::Mesh(const std::string&filename)
{
  std::cout << "Creating mesh " << filename << std::endl;
  std::fstream fhandle(std::string("./data/meshes/") + filename);
  if(!fhandle)
  {
    KIT_THROW("Could not open mesh file for reading");
  }
  
  std::string currline = "";
  while(std::getline(fhandle, currline))
  {
    std::vector<std::string> currtokens = kit::splitString(currline);
    if(currtokens.size() != 0)
    {
      KIT_ASSERT(currtokens.size() >= 2 /* Invalid mesh parameter length */);
      
      std::string identifier = currtokens[0];
      if(identifier == std::string("submesh"))
      {
        KIT_ASSERT(currtokens.size() == 4 /* Submesh needs 3 arguments (submesh <name> <geometry-filename> <material-filename>) */);
        addSubmeshEntry(currtokens[1], kit::Submesh::load(currtokens[2]), kit::Material::load(currtokens[3]));
      }
      else if (identifier == std::string("skeleton"))
      {

      }
      else
      {
        KIT_THROW("Unknown mesh parameter");
      }
    }
  }
  
  fhandle.close();  
}

void kit::Mesh::setSubmeshEnabled(const std::string&name, bool b)
{
  m_submeshesEnabled.at(name) = b;
}

void kit::Mesh::addSubmeshEntry(const std::string&name, std::shared_ptr<kit::Submesh> geometry, std::shared_ptr<kit::Material> material)
{
  m_submeshEntries[name].m_material = material;
  m_submeshEntries[name].m_submesh = geometry;
  m_submeshesEnabled[name] = true;
}

kit::Mesh::SubmeshEntry* kit::Mesh::getSubmeshEntry(const std::string&name)
{
  if (m_submeshEntries.find(name) == m_submeshEntries.end())
  {
    KIT_THROW("No such submesh in mesh");
  }
  return &m_submeshEntries.at(name);
}

void kit::Mesh::render(kit::Camera * camera, const glm::mat4 & modelMatrix, bool isForwardPass, const std::vector<glm::mat4> & skinTransform, const std::vector<glm::mat4> & instanceTransform)
{
  for(auto & currSubmesh : m_submeshEntries)
  {
    if (m_submeshesEnabled.at(currSubmesh.first))
    {
      bool materialForward = currSubmesh.second.m_material->getFlags(skinTransform.size() > 0, instanceTransform.size() > 0).m_forward;
      if(isForwardPass != materialForward)
      {
        continue;
      }

      currSubmesh.second.m_material->use(camera, modelMatrix, skinTransform, instanceTransform);
      
      if(instanceTransform.size() > 0)
      {
        currSubmesh.second.m_submesh->renderGeometryInstanced(instanceTransform.size());
      }
      else
      {
        currSubmesh.second.m_submesh->renderGeometry();
      }
    }
  }
}

void kit::Mesh::renderGeometry()
{
  for (auto & currSubmesh : m_submeshEntries)
  {
    if (m_submeshesEnabled.at(currSubmesh.first))
    {
      currSubmesh.second.m_submesh->renderGeometry();
    }
  }
}

std::map< std::string, kit::Mesh::SubmeshEntry > & kit::Mesh::getSubmeshEntries()
{
  return m_submeshEntries;
}
