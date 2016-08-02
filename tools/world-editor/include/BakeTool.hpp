#ifndef KWE_BAKETOOL_HPP
#define KWE_BAKETOOL_HPP

#include "EditorTool.hpp"
#include <Kit/EditorTerrain.hpp>

namespace kit
{
  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;
}

namespace kwe 
{

  class BakeTool : public EditorTool
  {
    public:
      BakeTool();
      ~BakeTool();

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

    private:
      // Settings
      bool m_active;
      
      char m_terrainName[256];
  };
}

#endif