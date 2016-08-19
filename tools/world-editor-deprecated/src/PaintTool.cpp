#include "PaintTool.hpp"
#include "WorldEditorState.hpp"
#include "Application.hpp"

#include <Kit/Texture.hpp>
#include <Kit/Window.hpp>
#include <Kit/Material.hpp>
#include <Kit/ImguiImpl.hpp>
#include <Kit/imgui/imgui.h>

kwe::PaintTool::PaintTool()
{
}

kwe::PaintTool::~PaintTool()
{
}

void kwe::PaintTool::allocate(kwe::WorldEditorState* editorRef)
{
  kwe::EditorComponent::allocate(editorRef);
  
  this->m_currentBrush = kit::Texture::load("editor/brushes/brush_soft.tga");
  this->m_flow = 0.05f;
  this->m_choke = 10.0f;
  this->m_brushSize = 1.0 / 16.0f;
  this->m_brushStrength = 1.0f;
  this->m_operation = kit::EditorTerrain::Add;
  this->m_layer = 0;
  this->m_isPainting = false;
  this->m_lastUv.x = 0.0f;
  this->m_lastUv.y = 0.0f;
  this->m_active = false;
}

void kwe::PaintTool::handleEvent(const double & mstime, const kit::WindowEvent& evt)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();

  if(!this->m_active)
  {
    return;
  }

  // Lambda to apply paint at a specific mouse position with a specific op
  auto applyPaint = [this, win, editorRef](glm::vec2 mousePosition)
  {
    editorRef->updatePickBuffer();
    auto pickResult = editorRef->readPickBuffer();

    editorRef->getTerrain()->paintMaterialMask(this->m_layer, this->m_currentBrush, glm::vec2(pickResult[1].x - this->m_brushSize/2.0, pickResult[1].y- this->m_brushSize/2.0), glm::vec2(this->m_brushSize, this->m_brushSize), this->m_operation, this->m_brushStrength);
  };

  if(!uiSystem.usesMouse() && !uiSystem.usesKeyboard())
  {
    // Keyboard shortcuts for layer switching
    if(evt.type == kit::WindowEvent::KeyPressed)
    {
      if(evt.keyboard.key == kit::Num1)
      {
        this->m_layer = 0;
      }

      if(evt.keyboard.key == kit::Num2)
      {
        this->m_layer = 1;
      }

      if(evt.keyboard.key == kit::Num3)
      {
        this->m_layer = 2;
      }

      if(evt.keyboard.key == kit::Num4)
      {
        this->m_layer = 3;
      }
    }

    // Shortcut to change brush size with alt+mousewheel
    if(evt.type == kit::WindowEvent::MouseScrolled)
    {
      if(win->isKeyDown(kit::LeftAlt))
      {
        this->m_brushSize+= evt.mouse.scrollOffset.y* 0.005f;
        this->m_brushSize = glm::clamp(this->m_brushSize, 0.005f, 0.5f);
      }
    }

    // On mouseclick, initiate painting
    if(evt.type == kit::WindowEvent::MouseButtonPressed && !win->isMouseButtonDown(kit::MiddleMouse))
    {
      bool doPaint = false;
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
        doPaint = true;
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
        doPaint = true;
      }

      if(doPaint)
      {
        this->m_isPainting = true;
        this->m_lastUv = win->getMousePosition();
        applyPaint(win->getMousePosition());
      }
    }

    // On mouse release, stop painting!
    if(evt.type == kit::WindowEvent::MouseButtonReleased && (evt.mouse.button == kit::LeftMouse || evt.mouse.button == kit::RightMouse))
    {
      this->m_isPainting = false;
    }

    // On mousemove + mouse press, paint
    if(evt.type == kit::WindowEvent::MouseMoved && !win->isMouseButtonDown(kit::MiddleMouse))
    {
      if(this->m_isPainting)
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
            applyPaint(currp);
            currf = (glm::max)(0.0f, currf - paintStep);
          }

          this->m_lastUv = newMouse;
        }
      }
    }
  }
  
}

void kwe::PaintTool::update(double const & ms, glm::vec2 const & mouseOffset)
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

void kwe::PaintTool::render()
{
}

void kwe::PaintTool::renderUI(const double & mstime)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();

  if(!this->m_active)
  {
    return;
  }
  
  if(ImGui::Begin("Brush settings##paint"))
  {

    auto newBrush = kit::UISystem::select("Select brush", this->m_selectedBrush, this->m_editorReference->getBrushes());
    if(newBrush != this->m_selectedBrush)
    {
      this->m_selectedBrush = newBrush;
      this->setBrush((*this->m_selectedBrush).second);
    }

    ImGui::SliderFloat("Brush Size", &this->m_brushSize, 0.005f, 0.5f);
    ImGui::SliderFloat("Paint strength", &this->m_brushStrength, 0.001f, 10.0f);
    ImGui::SliderFloat("Paint flow", &this->m_flow, 0.001f, 1.0f, "%.5f", 3.0f);
    ImGui::SliderFloat("Paint choke", &this->m_choke, 1.0f, 100.0f);
    ImGui::SliderInt("Paint layer", &this->m_layer, 0, 7);
  }
  ImGui::End();

  if (ImGui::Begin("Terrain Layers"), 0, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)
  {
    bool doRecompile = false;

    int numLayers = editorRef->getTerrain()->getNumLayers();
    if (ImGui::SliderInt("Num. layers##numlayers", &numLayers, 1, 8))
    {
      editorRef->getTerrain()->setNumLayers(numLayers);
      doRecompile = true;
    }

    for (int i = 0; i < editorRef->getTerrain()->getNumLayers(); i++)
    {
      ImGui::Text(std::string(std::string("Layer #") + std::to_string(i)).c_str());
      auto s0 = kit::UISystem::select("Material##m" + std::to_string(i), this->m_selectedMaterials[i], editorRef->getMaterials());
      if (s0 != this->m_selectedMaterials[i])
      {
        this->m_selectedMaterials[i] = s0;
        editorRef->getTerrain()->getLayerInfo(i).material = kit::Material::load((*s0).first + ".material");
        doRecompile = true;
      }
    }

    if (doRecompile)
    {
      editorRef->getTerrain()->updateGpuProgram();
      editorRef->getTerrain()->invalidateMaterials();
    }
  }
  ImGui::End();
}

kit::Texture::Ptr kwe::PaintTool::getIcon()
{
  return kit::Texture::load("editor/icon-paint.png");
}

void kwe::PaintTool::onActive()
{
  this->m_active = true;
}

void kwe::PaintTool::onInactive()
{
  auto editorRef = this->m_editorReference;
  editorRef->getTerrain()->setDecalBrush(nullptr);
  this->m_active = false;
}

kit::Texture::Ptr kwe::PaintTool::getBrush()
{
  return this->m_currentBrush;
}

void kwe::PaintTool::setBrush(kit::Texture::Ptr b)
{
  this->m_currentBrush = b; // TODO: Rename to activebrush to distinguish it from brush selections
}

void kwe::PaintTool::brushesUpdated()
{
  // Get a reference to the available brushes
  kit::UISystem::SelectionList & brushes = this->m_editorReference->getBrushes();
  
  // Set the selected brush to a sane default (first available brush)
  this->m_selectedBrush = brushes.begin();
  
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

void kwe::PaintTool::materialsUpdated()
{
  // Find the terrains materials in the list
  for (int i = 0; i < 8; i++)
  {
    this->m_selectedMaterials[i] = this->m_editorReference->getMaterials().end();
  }

  for (auto i = this->m_editorReference->getMaterials().begin(); i != this->m_editorReference->getMaterials().end(); i++)
  {
    for (int x = 0; x < this->m_editorReference->getTerrain()->getNumLayers(); x++)
    {
      if (this->m_editorReference->getTerrain()->getLayerInfo(x).material != nullptr)
      {
        if (((*i).first + std::string(".material")) == this->m_editorReference->getTerrain()->getLayerInfo(x).material->getName())
        {
          this->m_selectedMaterials[x] = i;
        }
      }
    }
  }
}

void kwe::PaintTool::texturesUpdated()
{

}

void kwe::PaintTool::meshesUpdated()
{

}