#ifndef KWE_MATERIALDESIGNER_HPP
#define KWE_MATERIALDESIGNER_HPP

#include <Kit/ApplicationState.hpp>
#include <Kit/ImguiImpl.hpp>
#include <Kit/Renderer.hpp>

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "EditorComponent.hpp"
#include "EditorCamera.hpp"
#include "PaintTool.hpp"
#include "SculptTool.hpp"
#include "PropTool.hpp"
#include "BakeTool.hpp"

namespace kit
{
  class Light;
  typedef std::shared_ptr<Light> LightPtr;
  typedef std::weak_ptr<Light> LightWPtr;
  
  class EditorTerrain;
  typedef std::shared_ptr<EditorTerrain> EditorTerrainPtr;
}

namespace kwe
{
  class Application;
  typedef std::shared_ptr<kwe::Application> ApplicationPtr;

  typedef std::array<glm::vec4, 2> PickResult;
  
  class WorldEditorState : public kit::ApplicationState, public std::enable_shared_from_this<WorldEditorState>
  {
  public:

    typedef std::shared_ptr<WorldEditorState> Ptr;

    WorldEditorState();

    static Ptr create();

    void notifyDrag();

    kit::UISystem & getUISystem();
    kit::EditorTerrainPtr getTerrain();
    kit::Renderer::Payload::Ptr getRenderPayload();
    
    kwe::EditorTool* getActiveTool();

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

    void updatePickBuffer();
    PickResult readPickBuffer();
    
    void updateMaterials();
    void updateTextures();
    void updateBrushes();
    void updateMeshes();
    
    kit::UISystem::SelectionList & getBrushes();
    kit::UISystem::SelectionList & getMaterials();
    kit::UISystem::SelectionList & getTextures();
    kit::UISystem::SelectionList & getMeshes();
    
  private:
    void renderUI(double const & ms);

    // States
    glm::vec2                          m_lastMousePosition;
    glm::vec2                          m_mouseOffset;
    bool                                m_wasDragging;
    bool                                m_isDragging;

    // Components
    kwe::EditorCamera                   m_cameraComponent;
    std::vector<kwe::EditorComponent*>  m_components;

    // Tools (are also components though)
    kwe::PaintTool                      m_paintTool;
    kwe::SculptTool                     m_sculptTool;
    kwe::PropTool                       m_propTool;
    kwe::BakeTool                       m_bakeTool;
    std::vector<kwe::EditorTool*>       m_tools;
    kwe::EditorTool*                    m_activeTool;

    // Rendering & Picking
    kit::Renderer::Payload::Ptr         m_renderPayload;
    kit::PixelBufferPtr                 m_pickBuffer;
    PickResult                          m_pickResult;

    // Environment
    kit::LightPtr                       m_sunLight;
    kit::LightPtr                       m_envLight;

    // Terrain
    kit::EditorTerrainPtr               m_terrain;

    // UI
    kit::UISystem                       m_UISystem;
    
    // Resources!!! Maybe put under own component?
    kit::UISystem::SelectionList        m_materials;
    kit::UISystem::SelectionList        m_textures; 
    kit::UISystem::SelectionList        m_brushes;
    kit::UISystem::SelectionList        m_meshes;
  };
}

#endif
