#pragma once

#include "Kit/Export.hpp"
#include "Kit/Renderable.hpp"

namespace kit 
{

  class Camera;
  class Program;
  class Mesh;
  class Texture;
  class Skeleton;

  class KITAPI Model : public kit::Renderable
  {
    public:
      
      Model();
      ~Model();
      
      Model(kit::Mesh * mesh);
      Model(const std::string& mesh);
      Model(const std::string& mesh, const std::string& skeleton);
      
      kit::Mesh * getMesh();
      kit::Skeleton * getSkeleton();
      
      void setInstancing(bool enabled, std::vector<glm::mat4> transforms);
      
      void update(double const & ms);
      void renderDeferred(kit::Renderer * renderer) override;
      void renderForward(kit::Renderer * renderer) override;
      void renderShadows(glm::mat4 v, glm::mat4 p) override;
      void renderGeometry() override;
      
      virtual std::vector<glm::mat4> getSkin() override;
      virtual bool isSkinned() override;

      glm::vec3 getBoneWorldPosition(const std::string& bone);
      glm::quat getBoneWorldRotation(const std::string& bone);
      
      static kit::Program* getShadowProgram(bool skinned, bool opacityMapped, bool instanced);
    private:
      
      struct ShadowProgramFlags
      {
        bool skinned;
        bool opacityMapped;
        bool instanced;
        
        bool operator<(const ShadowProgramFlags& b) const {
          return std::tie(this->skinned, this->opacityMapped, this->instanced)  < std::tie(b.skinned,b. opacityMapped, b.instanced);
        }
      };
      
      bool m_ownMesh = false;
      kit::Mesh* m_mesh = nullptr;
      kit::Skeleton* m_skeleton = nullptr;
      bool m_instanced = false;
      std::vector<glm::mat4> m_instanceTransform;
      

      static uint32_t               m_instanceCount;

      static void allocateShared();
      static void releaseShared();

      static std::map<ShadowProgramFlags, kit::Program*> m_shadowPrograms;
  };

}
