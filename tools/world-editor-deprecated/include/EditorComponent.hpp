#ifndef KWE_EDITORCOMPONENT_HPP
#define KWE_EDITORCOMPONENT_HPP

#include <Kit/WindowEvent.hpp>

namespace kwe 
{

  class WorldEditorState;

  class EditorComponent
  {
    public:
      EditorComponent();
      virtual ~EditorComponent();
      
      virtual void allocate(WorldEditorState* editorRef);

      virtual void update(double const & ms, glm::vec2 const & mouseOffset) = 0;
      virtual void render() = 0;
      virtual void handleEvent(const double & mstime, const kit::WindowEvent& evt) = 0;
      virtual void renderUI(const double & mstime) = 0;

      virtual void brushesUpdated() = 0;
      virtual void texturesUpdated() = 0;
      virtual void materialsUpdated() = 0;
      virtual void meshesUpdated() = 0;
      
    protected:
      WorldEditorState *m_editorReference;
  };

}

#endif