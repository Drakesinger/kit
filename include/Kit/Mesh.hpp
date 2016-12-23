#pragma once

#include "Kit/Export.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <map>

namespace kit
{
  class Submesh;

  class Material;

  class ConvexHull;
  
  class Camera;

  class KITAPI Mesh 
  {
    public:
      
      struct SubmeshEntry 
      {
        kit::Submesh *  m_submesh;
        kit::Material * m_material;
      };
      
      ~Mesh();
      Mesh();
      Mesh(const std::string& filename);
      
      void render(kit::Camera* camera, const glm::mat4 & modelMatrix, bool isForwardPass, const std::vector<glm::mat4> & skinTransform = std::vector<glm::mat4>(), const std::vector<glm::mat4> & instanceTransform = std::vector<glm::mat4>());
      void renderGeometry();

      void addSubmeshEntry(const std::string& name, kit::Submesh* geometry, kit::Material* material);
      
      void setSubmeshEnabled(const std::string& name, bool s);

      std::vector<kit::ConvexHull*> & getHull();
      
      kit::Mesh::SubmeshEntry * getSubmeshEntry(const std::string& name);
      std::map<std::string, kit::Mesh::SubmeshEntry> & getSubmeshEntries();

    private:
      std::map<std::string, kit::Mesh::SubmeshEntry> m_submeshEntries;
      std::map<std::string, bool> m_submeshesEnabled;
      
      std::vector<kit::ConvexHull*> m_hull;

  };

}
