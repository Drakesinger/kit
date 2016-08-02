#ifndef KWE_CAMERA_HPP
#define KWE_CAMERA_HPP

#include "EditorComponent.hpp"

#include <glm/glm.hpp>

namespace kit
{
  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;
  class Model;
  typedef std::shared_ptr<Model> ModelPtr;
}

namespace kwe 
{

  class EditorCamera : public EditorComponent
  {
    public:
      EditorCamera();
      ~EditorCamera();

      virtual void allocate(WorldEditorState * ref) override;
      virtual void handleEvent(const double & mstime, const kit::WindowEvent& evt) override;
      virtual void update(double const & ms, glm::vec2 const & mouseOffset) override;
      virtual void render() override;

      virtual void renderUI(const double & mstime) override;

      kit::CameraPtr getCamera();
      
      glm::vec3 getOrigin();
      float getDistance();
      
      void brushesUpdated() override;
      void texturesUpdated() override;
      void materialsUpdated() override;
      void meshesUpdated() override;
      
    private:
      
    void updateOrigin();
    void updateCamera(double const & ms);
    
    
    kit::ModelPtr     m_markers[5];
    
    
    // Settings
    float             m_targetDistance;
    float             m_targetOriginHeight;

      // States
    kit::CameraPtr    m_camera;
    float             m_horizontalAngle;
    float             m_verticalAngle;

    float             m_distance;   //< Zoom value
    glm::vec3         m_origin;     //< Set on top of terrain
  };
}

#endif