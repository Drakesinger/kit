#include "PropTool.hpp"
#include <WorldEditorState.hpp>

#include <Kit/Texture.hpp>
#include <Kit/Model.hpp>
#include <Kit/Application.hpp>
#include <Kit/Window.hpp>

#include <Kit/imgui/imgui.h>

void kwe::PropTool::allocate(kwe::WorldEditorState* ref)
{
  kwe::EditorComponent::allocate(ref);
  
  this->m_selectedMesh = "Fern001.mesh";
  this->m_displayModel = kit::Model::create(this->m_selectedMesh);
  
  this->m_propAlignment = true;
  this->m_propAlignmentValue = 1.0f;
  this->m_propHeightOffset = 0.f;
  this->m_propYRotation = 0.f;
  this->m_propScale = 1.0f;
}

kwe::PropTool::PropTool()
{
  this->m_active = false;
}

kwe::PropTool::~PropTool()
{

}


void kwe::PropTool::update(const double& ms, const glm::vec2& mouseOffset)
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
    auto result = editorRef->readPickBuffer();
    glm::vec3 worldPosition = glm::vec3(result[0].x, result[0].y, result[0].z);   
    auto sample = editorRef->getTerrain()->sampleBilinear(worldPosition.x, worldPosition.z );
    worldPosition.y = sample.x;
    
    this->m_propPosition = worldPosition;
    
    this->m_displayModel->setPosition(worldPosition);

    if (this->m_propAlignment)
    {
      this->m_displayModel->setEuler(glm::vec3(sample.y * this->m_propAlignmentValue, this->m_propYRotation, sample.z * this->m_propAlignmentValue));
    }
    else
    {
      this->m_displayModel->setEuler(glm::vec3(0.0f, this->m_propYRotation, 0.0f));
    }
    this->m_displayModel->setScale(glm::vec3(this->m_propScale));
    this->m_displayModel->translate(this->m_displayModel->getUp() * this->m_propHeightOffset);
  }
}

void kwe::PropTool::handleEvent(const double& mstime, const kit::WindowEvent& evt)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();
  
  if (!uiSystem.usesMouse() && this->m_active)
  {
    auto ctrl = win->isKeyDown(kit::LeftControl);
    auto alt = win->isKeyDown(kit::LeftAlt);
    auto shift = win->isKeyDown(kit::LeftShift);

    if(evt.type == kit::WindowEvent::MouseScrolled)
    { 
      if (shift)
      {
        this->m_propAlignmentValue += evt.mouse.scrollOffset.y * 0.05f;
      }
      else if(ctrl && alt)
      {
        this->m_propScale += evt.mouse.scrollOffset.y * (this->m_propScale*0.1f);
      }
      else if(ctrl)
      {
        this->m_propHeightOffset += evt.mouse.scrollOffset.y * 0.05f;
      }
      else if(alt)
      {
        this->m_propYRotation += evt.mouse.scrollOffset.y * 360.0f/16.0f;
      }
    }
    
    if(evt.type == kit::WindowEvent::MouseButtonPressed)
    {
      if(evt.mouse.button == kit::LeftMouse)
      {
        kwe::Prop * adder = new kwe::Prop();
        adder->m_meshName = this->m_selectedMesh;
        adder->m_heightOffset = this->m_propHeightOffset;
        adder->m_yRotation = this->m_propYRotation;
        adder->m_alignment = this->m_propAlignment;
        adder->m_scale = this->m_propScale;
        adder->m_position = this->m_propPosition;
        
        adder->m_displayModel = kit::Model::create(adder->m_meshName);
        adder->m_displayModel->setPosition(this->m_displayModel->getPosition());
        adder->m_displayModel->setRotation(this->m_displayModel->getRotation());
        adder->m_displayModel->setScale(this->m_displayModel->getScale());
        
        editorRef->getRenderPayload()->addRenderable(adder->m_displayModel);
        
        this->m_props.push_back(adder);
      }
    }

    if (evt.type == kit::WindowEvent::KeyPressed)
    {
      if (shift && evt.keyboard.key == kit::S)
      {
        this->m_propAlignmentValue = !this->m_propAlignmentValue;
      }
    }
  }
}

void kwe::PropTool::render()
{

}

void kwe::PropTool::renderUI(const double& mstime)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();

  if (!this->m_active)
  {
    return;
  }

  if (ImGui::Begin("Prop placement##propplacement"))
  {
    auto newProp = kit::UISystem::select("Select prop", this->m_selectedProp, this->m_editorReference->getMeshes());
    if (newProp != this->m_selectedProp)
    {
      this->m_selectedProp = newProp;
      this->updateSelectedMesh();
    }

    ImGui::Checkbox("Align to surface", &this->m_propAlignment);
    ImGui::SliderFloat("Alignment", &this->m_propAlignmentValue, 0.0f, 1.0f);

    ImGui::SliderFloat("Y-offset##propyoffset", &this->m_propHeightOffset, -10.0f, 10.0f);

    float yRotRad = glm::radians(this->m_propYRotation);
    if (ImGui::SliderAngle("Y-rotation##propyrotation", &yRotRad))
    {
      this->m_propYRotation = yRotRad;
    }

    ImGui::SliderFloat("Scale##propscale", &this->m_propScale, 0.001f, 100.0f, "%.3f", 2.0f);
  }
  ImGui::End();
}

kit::Texture::Ptr kwe::PropTool::getIcon()
{
  return kit::Texture::load("editor/icon-props.png");
}

void kwe::PropTool::onActive()
{
  this->m_active = true;

  // Update prop transforms in case heightmap has changed!
  for (kwe::Prop* currProp : this->m_props)
  {
    auto sample = this->m_editorReference->getTerrain()->sampleBilinear(currProp->m_position.x, currProp->m_position.z );
    currProp->m_position.y = sample.x;
    
    currProp->m_displayModel->setPosition(currProp->m_position);    
    currProp->m_displayModel->setEuler(glm::vec3(sample.y, currProp->m_yRotation, sample.z));
    currProp->m_displayModel->setScale(glm::vec3(currProp->m_scale));
    currProp->m_displayModel->translate(currProp->m_displayModel->getUp() * currProp->m_heightOffset);
  }

  this->m_editorReference->getRenderPayload()->addRenderable(this->m_displayModel);
}

void kwe::PropTool::onInactive()
{
  this->m_active = false;
  this->m_editorReference->getRenderPayload()->removeRenderable(this->m_displayModel);
}

void kwe::PropTool::brushesUpdated()
{

}

void kwe::PropTool::materialsUpdated()
{

}

void kwe::PropTool::texturesUpdated()
{

}

void kwe::PropTool::meshesUpdated()
{
  // Get a reference to the available meshes
  kit::UISystem::SelectionList & meshes = this->m_editorReference->getMeshes();

  // Set the selected prop to a sane default (first available prop)
  this->m_selectedProp = meshes.begin();

  // Iterate through the available meshes
  for (auto i = meshes.begin(); i != meshes.end(); i++)
  {
    // If the filename of the current brush selection matches the active brush, set it as our selected brush
    if ((*i).first == this->m_selectedMesh)
    {
      this->m_selectedProp = i;
      this->updateSelectedMesh();
      break;
    }
  }
}

void kwe::PropTool::updateSelectedMesh()
{
  this->m_selectedMesh = this->m_selectedProp->first;
  this->m_editorReference->getRenderPayload()->removeRenderable(this->m_displayModel);
  this->m_displayModel = kit::Model::create(this->m_selectedMesh);
  this->m_editorReference->getRenderPayload()->addRenderable(this->m_displayModel);
}
