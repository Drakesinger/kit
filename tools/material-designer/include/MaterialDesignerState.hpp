#ifndef KMD_MATERIALDESIGNER_HPP
#define KMD_MATERIALDESIGNER_HPP

#include <Kit/ApplicationState.hpp>
#include <Kit/ImguiImpl.hpp>
#include <Kit/Renderer.hpp>

namespace kit
{
  class Model;
  typedef std::shared_ptr<Model> ModelPtr;
  typedef std::weak_ptr<Model> ModelWPtr;

  class Submesh;
  typedef std::shared_ptr<Submesh> SubmeshPtr;

  class Light;
  typedef std::shared_ptr<Light> LightPtr;
  typedef std::weak_ptr<Light> LightWPtr;

  class Skybox;
  typedef std::shared_ptr<Skybox> SkyboxPtr;

  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class GridFloor;
  typedef std::shared_ptr<GridFloor> GridFloorPtr;

  class Quad;
  typedef std::shared_ptr<Quad> QuadPtr;

  class PixelBuffer;
  typedef std::shared_ptr<PixelBuffer> PixelBufferPtr;
  
  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class Material;
  typedef std::weak_ptr<Material> MaterialWPtr;
  typedef std::shared_ptr<Material> MaterialPtr;
}

namespace kmd
{
  
  class Application;
  typedef std::shared_ptr<kmd::Application> ApplicationPtr;
  
  class MaterialDesignerState : public kit::ApplicationState, public std::enable_shared_from_this<MaterialDesignerState>
  {
  public:

    typedef std::shared_ptr<MaterialDesignerState> Ptr;
    
    static Ptr create();
    
    virtual void allocate();
    virtual void release();
    
    virtual void onActive();
    virtual void onInactive();
    
    virtual void handleEvent(const double & mstime, const kit::WindowEvent& evt);
    virtual void render();
    virtual void update(const double& ms);
    
    virtual void onConsoleInactive(); //< Called when the console is inactivated/hidden
    virtual void onConsoleActive();   //< Called when the console is activated/shown
    virtual void onResize(glm::uvec2);
    
    
  private:
    bool             m_active;
    glm::vec2       m_lastMousePosition;
    
    void generateMaterialThumbs();
    
    // Camera
    enum InputMode
    {
      Maya,
      Max,
      Blender
    };

    void              updateCamera(double const & ms);
    kit::CameraPtr    m_camera;
    float             m_horizontalAngle;
    float             m_verticalAngle;
    float             m_distance;
    glm::vec3         m_origin;
    InputMode         m_inputMode;

    // Environment
    kit::LightPtr     m_sun;
    kit::LightPtr     m_envLight;
    kit::SkyboxPtr    m_skybox;
    kit::GridFloorPtr m_gridfloor;

    // Picking & rendering
    kit::QuadPtr                  m_fullscreenQuad;
    kit::Renderer::Payload::Ptr   m_renderPayload;

    void                          renderPickBuffer();
    kit::ProgramPtr               m_pickProgram;
    kit::PixelBufferPtr           m_pickBuffer;
    
    void                          renderGizmo();
    kit::ProgramPtr               m_gizmoProgram;
    kit::SubmeshPtr               m_gizmoTranslateX;
    kit::SubmeshPtr               m_gizmoTranslateY;
    kit::SubmeshPtr               m_gizmoTranslateZ;
    kit::SubmeshPtr               m_gizmoRotateX;
    kit::SubmeshPtr               m_gizmoRotateY;
    kit::SubmeshPtr               m_gizmoRotateZ;
    kit::SubmeshPtr               m_gizmoScaleX;
    kit::SubmeshPtr               m_gizmoScaleY;
    kit::SubmeshPtr               m_gizmoScaleZ;

    // UI
    kit::UISystem m_UISystem;
    void prepareUI(double const & ms);
    void prepareMaterialProperties(double const & ms);
    void prepareRenderProperties(double const & ms);
    void prepareEnvironmentProperties(double const & ms);
    void prepareMainMenu(double const & ms);

    // Editor states
    enum SelectMode
    {
      None = 0,
      Model = 1,
      Light = 2,
      Terrain = 3
    };

    enum TransformMode
    {
      Translate,
      Rotate,
      Scale /// Hold down shift to uniform scale
    };

    enum TransformSpace
    {
      Global,
      Local,
      View
    };

    enum TransformAxis
    {
      X,
      Y,
      Z,
      XY,
      XYZ
    };

    SelectMode                 m_selectedMode;
    TransformMode              m_transformMode;
    TransformAxis              m_transformAxis;
    TransformSpace             m_transformSpace;
    bool m_isTransforming;

    void deselect();

    void selectModel(kit::ModelPtr pointer);
    void selectMaterial(kit::MaterialPtr pointer);
    std::vector<kit::ModelPtr>  m_models;
    kit::ModelWPtr              m_selectedModel;
    kit::MaterialWPtr           m_selectedMaterial;

    void selectLight(kit::LightPtr);
    std::vector<kit::LightPtr> m_lights;
    kit::LightWPtr              m_selectedLight;

  };
}
#endif
