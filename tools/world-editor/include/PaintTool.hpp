#ifndef KWE_PAINTENGINE_HPP
#define KWE_PAINTENGINE_HPP

#include "EditorTool.hpp"
#include <Kit/EditorTerrain.hpp>
#include <Kit/ImguiImpl.hpp>

namespace kit
{
  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;
}

namespace kwe 
{

  class PaintTool : public EditorTool
  {
    public:
      PaintTool();
      ~PaintTool();

      virtual void allocate(WorldEditorState * ref) override;
      
      virtual void handleEvent(const double & mstime, const kit::WindowEvent& evt) override;
      virtual void update(double const & ms, glm::vec2 const & mouseOffset) override;
      virtual void render() override;

      virtual void renderUI(const double & mstime) override;

      virtual void onActive() override;
      virtual void onInactive() override;
      virtual kit::TexturePtr getIcon() override;
      
      kit::TexturePtr getBrush();
      void setBrush(kit::TexturePtr);
      
      void brushesUpdated() override;
      void texturesUpdated() override;
      void materialsUpdated() override;
      void meshesUpdated() override;
      
    private:
      // Settings
      bool m_active;
      kit::EditorTerrain::PaintOperation  m_operation;
      int             m_layer;
      float           m_flow;
      float           m_choke;
      float           m_brushSize;
      float           m_brushStrength;
      kit::TexturePtr m_currentBrush;
      kit::UISystem::SelectionIterator    m_selectedBrush;
      kit::UISystem::SelectionIterator    m_selectedMaterials[8];

      // States
      bool            m_isPainting;
      glm::vec2       m_lastUv;

  };
}

#endif