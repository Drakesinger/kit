#include "BakeTool.hpp"
#include "WorldEditorState.hpp"
#include "Application.hpp"

#include <Kit/Texture.hpp>
#include <Kit/Window.hpp>
#include <Kit/ImguiImpl.hpp>
#include <Kit/imgui/imgui.h>

kwe::BakeTool::BakeTool()
{
}

kwe::BakeTool::~BakeTool()
{
}

void kwe::BakeTool::allocate(kwe::WorldEditorState* editorRef)
{
  kwe::EditorComponent::allocate(editorRef);

  for(int i = 0; i < 255; i++)
  {
    this->m_terrainName[i] = '\0';
  }
  
  strcpy(this->m_terrainName, "unnamed");
  this->m_active = false;
}

void kwe::BakeTool::handleEvent(const double & mstime, const kit::WindowEvent& evt)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();

  if(!this->m_active)
  {
    return;
  }
  
  if(!uiSystem.usesMouse() && !uiSystem.usesKeyboard())
  {

  }
  
}

void kwe::BakeTool::update(double const & ms, glm::vec2 const & mouseOffset)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();

  if(!this->m_active)
  {
    return;
  }
  
  if (!uiSystem.usesMouse() && !win->isMouseButtonDown(kit::MiddleMouse))
  {
  }
}

void kwe::BakeTool::render()
{
}

void kwe::BakeTool::renderUI(const double & mstime)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();

  if(!this->m_active)
  {
    return;
  }
  
  if(ImGui::Begin("Terrain settings##terrain-settings"))
  {
    // Current terrain
    ImGui::BeginGroup();
    {
      if(ImGui::InputText("Terrain name##curr-terrain-name", this->m_terrainName, 256))
      {
        this->m_editorReference->getTerrain()->setName(this->m_terrainName);
      }
      
      if(ImGui::Button("Save terrain##save-terrain"))
      {
        this->m_editorReference->getTerrain()->save();
      }

      if (ImGui::Button("Load terrain##load-terrain"))
      {
        //this->m_editorReference->getTerrain()->load();
      }
      
      ImGui::SameLine();
      
      if(ImGui::Button("Bake terrain##bake-terrain"))
      {
        this->m_editorReference->getTerrain()->bake();
      }
    }
    ImGui::EndGroup();
    
    // Create new 
    ImGui::BeginGroup();
    {
      ImGui::Text("Create new terrain");
      
      static int newResolution[2] = {80, 80};
      ImGui::SliderInt2("Resolution", newResolution, 4, 400);
      
      static float newXzScale = 0.25f;
      ImGui::SliderFloat("XZ-scale##new-xz-scale", &newXzScale, 0.1f, 2.0f);

      ImGui::Text("Terrain size: %fx%f meters", newResolution[0] * newXzScale, newResolution[1] * newXzScale);

      static float newYScale = 5.0f;
      ImGui::SliderFloat("Y-scale##new-y-scale", &newYScale, 1.0f, 30.0f);
      
      if(ImGui::Button("Create##create-terrain"))
      {
        ImGui::OpenPopup("confirm-create-terrain");
      }
      
      if(ImGui::BeginPopupModal("confirm-create-terrain"))
      {
        ImGui::Text("Are you sure? This will discard any unsaved terrain-data.");
        if(ImGui::Button("Yes##confirm-create-terrain-yes"))
        {
          this->m_editorReference->getTerrain()->reset(this->m_terrainName, glm::uvec2(newResolution[0], newResolution[1]), newXzScale, newYScale);
          ImGui::CloseCurrentPopup();
        }
        
        if(ImGui::Button("Cancel##confirm-create-terrain-cancel"))
        {
          ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
      }
    }
    ImGui::EndGroup();
    
    // Load
    ImGui::BeginGroup();
    
    ImGui::EndGroup();
    
  }
  ImGui::End();
  
}

kit::Texture::Ptr kwe::BakeTool::getIcon()
{
  return kit::Texture::load("editor/icon-bake.png");
}

void kwe::BakeTool::onActive()
{
  this->m_active = true;
}

void kwe::BakeTool::onInactive()
{
  auto editorRef = this->m_editorReference;
  editorRef->getTerrain()->setDecalBrush(nullptr);
  this->m_active = false;
}

void kwe::BakeTool::brushesUpdated()
{

}

void kwe::BakeTool::materialsUpdated()
{

}

void kwe::BakeTool::texturesUpdated()
{

}

void kwe::BakeTool::meshesUpdated()
{

}