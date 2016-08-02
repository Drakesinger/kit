#include "WorldEditorState.hpp"
#include "Application.hpp"

#include <Kit/Application.hpp>
#include <Kit/Camera.hpp>
#include <Kit/Window.hpp>
#include <Kit/Light.hpp>
#include <Kit/Skybox.hpp>
#include <Kit/GridFloor.hpp>
#include <Kit/Texture.hpp>
#include <Kit/PixelBuffer.hpp>
#include <Kit/Quad.hpp>
#include <Kit/Material.hpp>
#include <Kit/Model.hpp>
#include <Kit/Program.hpp>
#include <Kit/Mesh.hpp>
#include <Kit/Submesh.hpp>

#include <Kit/imgui/imgui.h>
#include <Kit/Font.hpp>
#include <Kit/EditorTerrain.hpp>

#include <iostream>
#include <glm/glm.hpp>

kwe::WorldEditorState::WorldEditorState()
{
}

kwe::WorldEditorState::Ptr kwe::WorldEditorState::create()
{
  auto returner = std::make_shared<kwe::WorldEditorState>();
  return returner;
}

/* --- Default overrides --- */

void kwe::WorldEditorState::allocate()
{
  auto fbSize = this->m_application->getWindow()->getFramebufferSize();
  glm::uvec2 ufbSize(fbSize.x, fbSize.y);
  std::string env = "meadow";

  // States
  this->m_wasDragging = false;
  this->m_isDragging = false;
  this->m_lastMousePosition = glm::vec2(0.0f, 0.0f);
  this->m_mouseOffset = glm::vec2(0.0f, 0.0f);
  
  // Rendering
  this->m_renderPayload = kit::Renderer::Payload::create();
  //this->m_renderPayload->addRenderable(kit::GridFloor::create());

  // Environment
  this->m_envLight = kit::Light::create(kit::Light::IBL);
  this->m_envLight->setIrradianceMap(kit::Cubemap::loadIrradianceMap(env));
  this->m_envLight->setRadianceMap(kit::Cubemap::loadRadianceMap(env));
  this->m_envLight->setColor(glm::vec3(1.0, 1.0, 1.0)*1.0f);
  this->m_renderPayload->addLight(this->m_envLight);

  this->m_sunLight = kit::Light::create(kit::Light::Directional, glm::uvec2(4096, 4096));
  this->m_sunLight->setMaxShadowDistance(10.0f);
  this->m_sunLight->setEuler(glm::vec3(-25.0f, 0.2f, 0.0f));
  this->m_sunLight->setColor(glm::vec3(1.0, 1.0, 1.0f)*1.0f); 
  this->m_renderPayload->addLight(this->m_sunLight);

  // Terrain
  this->m_terrain = kit::EditorTerrain::create("unnamed", glm::uvec2(128, 128), 0.25f, 32.0f);
  this->m_pickBuffer = kit::PixelBuffer::create(ufbSize, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA32F),  kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA32F)}, kit::PixelBuffer::AttachmentInfo(kit::Texture::DepthComponent24));
  this->m_pickResult = {glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};
  this->m_renderPayload->addRenderable(this->m_terrain);

  // UI system
  this->m_UISystem.setWindow(this->m_application->getWindow());
  
  // Component system
  this->m_cameraComponent.allocate(this);
  this->m_paintTool.allocate(this);
  this->m_sculptTool.allocate(this);
  this->m_propTool.allocate(this);
  this->m_bakeTool.allocate(this);
  this->m_components.push_back(&this->m_cameraComponent);
  this->m_components.push_back(&this->m_paintTool);
  this->m_components.push_back(&this->m_sculptTool);
  this->m_components.push_back(&this->m_propTool);
  this->m_components.push_back(&this->m_bakeTool);
  
  // Tool system
  this->m_tools.push_back(&this->m_paintTool);
  this->m_tools.push_back(&this->m_sculptTool);
  this->m_tools.push_back(&this->m_propTool);
  this->m_tools.push_back(&this->m_bakeTool);
  
  this->m_activeTool = &this->m_paintTool;
  this->m_paintTool.onActive();
  
  this->updateTextures();
  this->updateBrushes();
  this->updateMaterials();
  this->updateMeshes();
  
  
}

void  kwe::WorldEditorState::release()
{

}

void kwe::WorldEditorState::handleEvent(const double& mstime, const kit::WindowEvent& evt)
{
  this->m_UISystem.handleEvent(evt);

  if (!this->m_UISystem.usesMouse() && !this->m_UISystem.usesKeyboard())
  {
    for(auto currComponent : this->m_components)
    {
      currComponent->handleEvent(mstime, evt);
    }
  }
}

void kwe::WorldEditorState::update(const double& ms)
{
  // Begin internal update
  auto win = this->m_application->getWindow();
  auto resolution = win->getFramebufferSize();
  
  // Update mouse offset
  glm::vec2 currMousePosition = this->m_application->getWindow()->getMousePosition();
  this->m_mouseOffset = m_lastMousePosition - currMousePosition;
  this->m_lastMousePosition = currMousePosition;

  // Render UI
  this->renderUI(ms);

  // Initialize dragmode as false each frame
  this->m_isDragging = false;

  for(auto currComponent : this->m_components)
  {
    currComponent->update(ms, this->m_mouseOffset);
  }

  // Update shadow  distance 
  //glm::distance(this->m_cameraComponent.getOrigin(), this->m_cameraComponent.getCamera()->getPosition()); WHY I DO THIS I HAVE DISTANCE!!!!!
  this->m_sunLight->setMaxShadowDistance(this->m_cameraComponent.getDistance() + 5.0f);
  
  // End internal update
  if (this->m_isDragging && !this->m_wasDragging)
  {
    this->m_application->getWindow()->setMouseVirtual(true);
  }
  else if (this->m_wasDragging && !this->m_isDragging)
  {
    this->m_application->getWindow()->setMouseVirtual(false);
  }
  this->m_wasDragging = this->m_isDragging;
}

void kwe::WorldEditorState::render()
{
  this->m_application->getWindow()->bind();

  for(auto currComponent : this->m_components)
  {
    currComponent->render();
  }

  // Render UI
  this->m_UISystem.render();
}

void kwe::WorldEditorState::onConsoleActive()
{
  kit::ApplicationState::onConsoleActive();
}

void kwe::WorldEditorState::onConsoleInactive()
{
  kit::ApplicationState::onConsoleInactive();
}

void kwe::WorldEditorState::onActive()
{
  this->m_cameraComponent.getCamera()->setAspectRatio((float)this->m_application->getWindow()->getFramebufferSize().x / (float)this->m_application->getWindow()->getFramebufferSize().y);
  dynamic_cast<kwe::Application*>(this->m_application)->getRenderer()->registerPayload(this->m_renderPayload);
  dynamic_cast<kwe::Application*>(this->m_application)->getRenderer()->setBloomDirtMask(kit::Texture::load("lensdirt.tga"));
  dynamic_cast<kwe::Application*>(this->m_application)->getRenderer()->setActiveCamera(this->m_cameraComponent.getCamera());
}

void kwe::WorldEditorState::onInactive()
{
  dynamic_cast<kwe::Application*>(this->m_application)->getRenderer()->unregisterPayload(this->m_renderPayload);
}

void kwe::WorldEditorState::onResize(glm::uvec2 size)
{
  if(size.x > 0 && size.y > 0)
  { 
    this->m_cameraComponent.getCamera()->setAspectRatio((float)size.x / (float)size.y);
    this->m_pickBuffer = kit::PixelBuffer::create(size, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA32F),  kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA32F)}, kit::PixelBuffer::AttachmentInfo(kit::Texture::DepthComponent24));
  }
}

void kwe::WorldEditorState::renderUI(double const & ms)
{
  this->m_UISystem.prepareFrame(ms);

  if(ImGui::Begin("Toolbar", 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize ))
  {
    for(auto currTool : this->m_tools)
    {
      if(ImGui::ImageButton((ImTextureID)currTool->getIcon()->getHandle(), ImVec2(32.0f, 32.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), -1.0f, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, this->m_activeTool == currTool ? 1.0f : 0.1f )))
      {
        this->m_activeTool->onInactive();
        this->m_activeTool = currTool;
        this->m_activeTool->onActive();
      }

      ImGui::SameLine();
    }
  }
  ImGui::End();
    
  for(auto currComponent : this->m_components)
  {
    currComponent->renderUI(ms);
  }
  
  //ImGui::Image((ImTextureID)this->m_sunLight->getShadowBuffer()->getDepthAttachment()->getHandle(), ImVec2(128, 128), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
  //ImGui::Image((ImTextureID)this->m_terrain->getARCache()->getHandle(), ImVec2(128, 128));
  //ImGui::Image((ImTextureID)this->m_terrain->getHeightmap()->getHandle(), ImVec2(128, 128));
  //ImGui::Image((ImTextureID)this->m_terrain->getMaterialMask()->getColorAttachment(0)->getHandle(), ImVec2(128, 128));
  //ImGui::Image((ImTextureID)this->m_terrain->getMaterialMask()->getColorAttachment(1)->getHandle(), ImVec2(128, 128));

}

void kwe::WorldEditorState::notifyDrag()
{
  this->m_isDragging = true;
}

kit::UISystem & kwe::WorldEditorState::getUISystem()
{
  return this->m_UISystem;
}

kit::EditorTerrain::Ptr kwe::WorldEditorState::getTerrain()
{
  return this->m_terrain;
}

kit::Renderer::Payload::Ptr kwe::WorldEditorState::getRenderPayload()
{
  return this->m_renderPayload;
}

kwe::EditorTool* kwe::WorldEditorState::getActiveTool()
{
  return this->m_activeTool;
}

void kwe::WorldEditorState::updatePickBuffer()
{
  auto mousePosition = this->m_application->getWindow()->getMousePosition();
  
  this->m_pickBuffer->bind();
  this->m_pickBuffer->clearAttachment(1, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  this->m_pickBuffer->clearDepth(1.0f);

  this->m_terrain->renderPickbuffer(this->m_cameraComponent.getCamera());

  auto buffResolution = this->m_pickBuffer->getResolution();
  this->m_pickResult[0] = this->m_pickBuffer->readPixel(0, mousePosition.x, buffResolution.y - mousePosition.y);
  this->m_pickResult[1] = this->m_pickBuffer->readPixel(1, mousePosition.x, buffResolution.y - mousePosition.y);
  
  glm::vec3 worldPosition = glm::vec3(this->m_pickResult[0].x, this->m_pickResult[0].y, this->m_pickResult[0].z);
  auto viewMatrix = this->m_cameraComponent.getCamera()->getViewMatrix();
  auto modelMatrix = this->m_terrain->getTransformMatrix();
  auto modelViewMatrix = viewMatrix * modelMatrix;
  worldPosition = glm::vec3(glm::inverse(modelViewMatrix) * glm::vec4(worldPosition, 1.0f));
  this->m_pickResult[0].x = worldPosition.x;
  this->m_pickResult[0].y = worldPosition.y;
  this->m_pickResult[0].z = worldPosition.z;
  
}

kwe::PickResult kwe::WorldEditorState::readPickBuffer()
{
  return this->m_pickResult;
}

void kwe::WorldEditorState::updateMeshes()
{
  this->m_meshes.clear();
  auto meshFiles = kit::listFilesystemEntries("./data/meshes/", true, false);
  for (kit::FileInfo & currFile : meshFiles)
  {
    std::pair<std::string, kit::Texture::Ptr> adder;
    adder.first = currFile.filename;
    //adder.second = kit::Texture::create2DFromFile("./data/meshes/thumbs/" + adder.first + ".tga", kit::Texture::SRGB8Alpha8);
    adder.second = nullptr;
    this->m_meshes.push_back(adder);
  }

  for (kwe::EditorComponent *currComponent : this->m_components)
  {
    currComponent->meshesUpdated();
  }
}

void kwe::WorldEditorState::updateMaterials()
{
  this->m_materials.clear();
  auto materialFiles = kit::listFilesystemEntries("./data/materials/", true, false);
  for(kit::FileInfo & currFile : materialFiles)
  {
    std::pair<std::string, kit::Texture::Ptr> adder;
    adder.first = currFile.filename.substr(0, currFile.filename.size() - 9);
    adder.second = kit::Texture::create2DFromFile("./data/materials/thumbs/" + adder.first + ".tga", kit::Texture::SRGB8Alpha8);
    this->m_materials.push_back(adder);
  }
  
  for(kwe::EditorComponent *currComponent : this->m_components)
  {
    currComponent->materialsUpdated();
  }
}

void kwe::WorldEditorState::updateTextures()
{
  this->m_textures.clear();
  auto textureFiles = kit::listFilesystemEntries("./data/textures/", true, false);
  for(kit::FileInfo & currFile : textureFiles)
  {
    std::pair<std::string, kit::Texture::Ptr> adder;
    adder.first = currFile.filename;
    
    bool srgb = !kit::stringContains("normal", kit::toLower(currFile.filename).c_str());
    
    adder.second = kit::Texture::load(currFile.filename, srgb);
    this->m_textures.push_back(adder);
  }

  for(kwe::EditorComponent *currComponent : this->m_components)
  {
    currComponent->texturesUpdated();
  }
}

void kwe::WorldEditorState::updateBrushes()
{
  this->m_brushes.clear();
  auto  brushFiles = kit::listFilesystemEntries("./data/textures/editor/brushes", true, false);
  for(kit::FileInfo & currFile : brushFiles)
  {
    std::pair<std::string, kit::Texture::Ptr> adder;
    adder.first = std::string("editor/brushes/")+currFile.filename;
    adder.second = kit::Texture::load(adder.first);
    this->m_brushes.push_back(adder);
  }
  
  for(kwe::EditorComponent *currComponent : this->m_components)
  {
    currComponent->brushesUpdated();
  }
}


kit::UISystem::SelectionList & kwe::WorldEditorState::getBrushes()
{
  return this->m_brushes;
}

kit::UISystem::SelectionList & kwe::WorldEditorState::getMaterials()
{
  return this->m_materials;
}

kit::UISystem::SelectionList & kwe::WorldEditorState::getTextures()
{
  return this->m_textures;
}

kit::UISystem::SelectionList & kwe::WorldEditorState::getMeshes()
{
  return this->m_meshes;
}