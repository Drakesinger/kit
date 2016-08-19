#ifndef KWE_PROPENGINE_HPP
#define KWE_PROPENGINE_HPP

#include "EditorTool.hpp"
#include <Kit/EditorTerrain.hpp>
#include <Kit/ImguiImpl.hpp>

namespace kit
{
  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;
  
  class Model;
  typedef std::shared_ptr<Model> ModelPtr;
}

namespace kwe 
{

  struct Prop 
  {
    kit::ModelPtr m_displayModel;

    glm::vec3     m_position;
    bool          m_alignment;
    float         m_yRotation;
    float         m_heightOffset;
    float         m_scale;

    std::string   m_meshName;
  };
  
  class PropTool : public EditorTool
  {
    public:
      PropTool();
      ~PropTool();

      virtual void allocate(WorldEditorState * ref) override;
      
      virtual void handleEvent(const double & mstime, const kit::WindowEvent& evt) override;
      virtual void update(double const & ms, glm::vec2 const & mouseOffset) override;
      virtual void render() override;

      virtual void renderUI(const double & mstime) override;

      virtual void onActive() override;
      virtual void onInactive() override;
      virtual kit::TexturePtr getIcon() override;
      
      void brushesUpdated() override;
      void texturesUpdated() override;
      void materialsUpdated() override;
      void meshesUpdated() override;
      
      void updateSelectedMesh();

    private:
      // Settings
      float m_propHeightOffset;
      float m_propYRotation;
      float m_propScale;
      bool  m_propAlignment;
      float m_propAlignmentValue;
      glm::vec3 m_propPosition;
      

      // States
      bool m_active;
      
      std::string   m_selectedMesh;
      kit::ModelPtr m_displayModel;
      
      std::vector<Prop*> m_props;
      
      kit::UISystem::SelectionIterator    m_selectedProp;
      
  };
}

#endif