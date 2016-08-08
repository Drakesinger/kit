#ifndef KIT_MODEL_HPP
#define KIT_MODEL_HPP

#include "Kit/Export.hpp"
#include "Kit/Renderable.hpp"

#include "Kit/Skeleton.hpp"

#include <memory>

namespace kit 
{

  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class Mesh;
  typedef std::shared_ptr<Mesh> MeshPtr;

  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;

  class KITAPI Model : public kit::Renderable
  {
    public:
      typedef std::shared_ptr<Model> Ptr;
      
      ~Model();
      
      static kit::Model::Ptr create(kit::MeshPtr mesh);
      static kit::Model::Ptr create(std::string mesh);
      static kit::Model::Ptr create(std::string mesh, std::string skeleton);
      
      kit::MeshPtr getMesh();
      kit::Skeleton::Ptr getSkeleton();
      
      void setInstancing(bool enabled, std::vector<glm::mat4> transforms);
      
      void update(double const & ms);
      void renderDeferred(kit::RendererPtr renderer) override;
      void renderForward(kit::RendererPtr renderer) override;
      void renderShadows(glm::mat4 v, glm::mat4 p) override;
      void renderGeometry() override;
      
      virtual std::vector<glm::mat4> getSkin() override;
      virtual bool isSkinned() override;

      glm::vec3 getBoneWorldPosition(std::string bone);
      glm::quat getBoneWorldRotation(std::string bone);
      
      static kit::ProgramPtr getShadowProgram(bool skinned = false, bool opacitymapped = false);

      Model();
    private:

      kit::MeshPtr m_mesh;
      kit::Skeleton::Ptr m_skeleton;
      bool m_instanced;
      std::vector<glm::mat4> m_instanceTransform;
      

      static uint32_t               m_instanceCount;

      static void allocateShared();
      static void releaseShared();

      static kit::ProgramPtr           m_programShadow;    // Shadow program for non-skinned, non-opacitymapped meshes
      static kit::ProgramPtr           m_programShadowS;   // Shadow program for skinned, non-opacitymapped meshes
      static kit::ProgramPtr           m_programShadowO;   // Shadow program for non-skinned, opacitymapped meshes
      static kit::ProgramPtr           m_programShadowSO;  // Shadow program for skinned, opacitymapped meshes
  };

}

#endif
