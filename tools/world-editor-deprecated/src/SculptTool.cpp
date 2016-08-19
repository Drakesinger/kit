#include "SculptTool.hpp"
#include "WorldEditorState.hpp"
#include "Application.hpp"

#include <Kit/Texture.hpp>
#include <Kit/Window.hpp>
#include <Kit/ImguiImpl.hpp>
#include <Kit/imgui/imgui.h>

kwe::SculptTool::SculptTool()
{
}

kwe::SculptTool::~SculptTool()
{
}

void kwe::SculptTool::allocate(kwe::WorldEditorState* editorRef)
{
  kwe::EditorComponent::allocate(editorRef);
  

  this->m_currentBrush = kit::Texture::load("editor/brushes/brush_soft.tga");
  this->m_flow = 0.05f;
  this->m_choke = 10.0f;
  this->m_brushSize = 1.0 / 16.0f;
  this->m_brushStrength = 0.01f;
  this->m_operation = kit::EditorTerrain::Add;
  this->m_isSculpting = false;
  this->m_lastUv.x = 0.0f;
  this->m_lastUv.y = 0.0f;
  this->m_active = false;
}

void kwe::SculptTool::handleEvent(const double & mstime, const kit::WindowEvent& evt)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();

  if(!this->m_active)
  {
    return;
  }

  // Lambda to apply sculpt at a specific mouse position with a specific op
  auto applySculpt = [this, win, editorRef](glm::vec2 mousePosition)
  {
    editorRef->updatePickBuffer();
    auto pickResult = editorRef->readPickBuffer();

    editorRef->getTerrain()->paintHeightmap(this->m_currentBrush, glm::vec2(pickResult[1].x - this->m_brushSize/2.0, pickResult[1].y- this->m_brushSize/2.0), glm::vec2(this->m_brushSize, this->m_brushSize), this->m_operation, this->m_brushStrength);
  };

  if(!uiSystem.usesMouse() && !uiSystem.usesKeyboard())
  {

    // Shortcut to change brush size with alt+mousewheel
    if(evt.type == kit::WindowEvent::MouseScrolled)
    {
      if(win->isKeyDown(kit::LeftAlt))
      {
        this->m_brushSize+= evt.mouse.scrollOffset.y* 0.005f;
        this->m_brushSize = glm::clamp(this->m_brushSize, 0.005f, 0.5f);
      }
    }

    // On mouseclick, initiate sculpting
    if(evt.type == kit::WindowEvent::MouseButtonPressed && !win->isMouseButtonDown(kit::MiddleMouse))
    {
      bool doSculpt = false;
      if(evt.mouse.button == kit::LeftMouse)
      {
        if(evt.mouse.modifiers && kit::LeftShift)
        {
          this->m_operation = kit::EditorTerrain::Set;
        }
        else
        {
          this->m_operation = kit::EditorTerrain::Add;
        }
        doSculpt = true;
      }
      
      if(evt.mouse.button == kit::RightMouse)
      {
        if(evt.mouse.modifiers && kit::LeftShift)
        {
          this->m_operation = kit::EditorTerrain::Smooth;
        }
        else
        {
          this->m_operation = kit::EditorTerrain::Subtract;
        }
        doSculpt = true;
      }

      if(doSculpt)
      {
        this->m_isSculpting = true;
        this->m_lastUv = win->getMousePosition();
        applySculpt(win->getMousePosition());
      }
    }

    // On mouse release, stop painting!
    if(evt.type == kit::WindowEvent::MouseButtonReleased && (evt.mouse.button == kit::LeftMouse || evt.mouse.button == kit::RightMouse))
    {
      this->m_isSculpting = false;
    }

    // On mousemove + mouse press, paint
    if(evt.type == kit::WindowEvent::MouseMoved && !win->isMouseButtonDown(kit::MiddleMouse))
    {
      if(this->m_isSculpting)
      {
        glm::vec2 newMouse = evt.mouse.newPosition;
        auto oldMouse = this->m_lastUv;
        glm::vec2 paintDir = glm::normalize(newMouse - oldMouse);
        float paintDistance = glm::distance(newMouse, oldMouse);

        if(paintDistance > this->m_choke)
        {
          float paintStep = 1.0f / this->m_flow;

          float currf = paintDistance;
          while(currf > 0.0f)
          {
            glm::vec2 currp =  oldMouse + (paintDir * currf);
            applySculpt(currp);
            currf = (glm::max)(0.0f, currf - paintStep);
          }

          this->m_lastUv = newMouse;
        }
      }
    }
  }
  
}

void kwe::SculptTool::update(double const & ms, glm::vec2 const & mouseOffset)
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
    editorRef->updatePickBuffer();
    auto pickResult = editorRef->readPickBuffer();
    
    editorRef->getTerrain()->setDecalBrush(this->m_currentBrush, glm::vec2(pickResult[1].x - this->m_brushSize/2.0, pickResult[1].y- this->m_brushSize/2.0), glm::vec2(this->m_brushSize, this->m_brushSize));
  }
}

void kwe::SculptTool::render()
{
}

void kwe::SculptTool::renderUI(const double & mstime)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();

  if(!this->m_active)
  {
    return;
  }
  
  if(ImGui::Begin("Brush settings##sculpt"))
  {
    auto newBrush = kit::UISystem::select("Select sculptbrush", this->m_selectedBrush, this->m_editorReference->getBrushes());
    if(newBrush != this->m_selectedBrush)
    {
      this->m_selectedBrush = newBrush;
      this->setBrush((*this->m_selectedBrush).second);
    }

    ImGui::SliderFloat("Brush Size##bss", &this->m_brushSize, 0.005f, 0.5f);
    ImGui::SliderFloat("Paint strength##bps", &this->m_brushStrength, 0.01f, 1.0f, "%.5f", 3.0);
    ImGui::SliderFloat("Paint flow##bpf", &this->m_flow, 0.001f, 1.0f, "%.5f", 3.0f);
    ImGui::SliderFloat("Paint choke##bpc", &this->m_choke, 1.0f, 100.0f);
  }
  ImGui::End();
  
}

kit::Texture::Ptr kwe::SculptTool::getIcon()
{
  return kit::Texture::load("editor/icon-sculpt.png");
}

void kwe::SculptTool::onActive()
{
  this->m_active = true;
}

void kwe::SculptTool::onInactive()
{
  auto editorRef = this->m_editorReference;
  editorRef->getTerrain()->setDecalBrush(nullptr);
  this->m_active = false;
}

kit::Texture::Ptr kwe::SculptTool::getBrush()
{
  return this->m_currentBrush;
}

void kwe::SculptTool::setBrush(kit::Texture::Ptr b)
{
  this->m_currentBrush = b;
}

void kwe::SculptTool::brushesUpdated()
{
  // Get a reference to the available brushes
  kit::UISystem::SelectionList & brushes = this->m_editorReference->getBrushes();
  
  // Set the selected brush to a sane default (first available brush)
  this->m_selectedBrush = brushes.end();
  

  // Iterate through the available brushes
  for(auto i = brushes.begin(); i != brushes.end(); i++)
  {
    // If the filename of the current brush selection matches the active brush, set it as our selected brush
    if( (*i).first == this->getBrush()->getFilename())
    {
      this->m_selectedBrush = i;
      this->setBrush((*this->m_selectedBrush).second);
      break;
    }
  }
}

void kwe::SculptTool::materialsUpdated()
{

}

void kwe::SculptTool::texturesUpdated()
{

}

void kwe::SculptTool::meshesUpdated()
{

}