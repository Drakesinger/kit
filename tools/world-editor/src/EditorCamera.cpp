#include "EditorCamera.hpp"
#include <WorldEditorState.hpp>
#include <Kit/Application.hpp>
#include <Kit/EditorTerrain.hpp>
#include <Kit/Camera.hpp>
#include <Kit/Window.hpp>

#include <glm/gtc/quaternion.hpp>
#include <Kit/Model.hpp>

kwe::EditorCamera::EditorCamera()
{

}

void kwe::EditorCamera::allocate(kwe::WorldEditorState* ref) 
{
  kwe::EditorComponent::allocate(ref);
  
  auto appRef = ref->getApplication();
  auto win = appRef->getWindow();
  auto fbSize = win->getFramebufferSize();
  
  this->m_camera = kit::Camera::create(45.0f, (float)fbSize.x / (float)fbSize.y, glm::vec2(0.1f, 1000.0f));
  this->m_camera->setWhitepoint(1.0f);
  this->m_horizontalAngle = 0.0f;
  this->m_verticalAngle = -45.0f;
  this->m_origin = glm::vec3(0.0f, 0.0f, 0.0f);
  this->m_targetOriginHeight = 0.0f;
  this->m_distance = 10.0f;
  this->m_targetDistance = 10.0f;
}


kwe::EditorCamera::~EditorCamera()
{
}

void kwe::EditorCamera::handleEvent(const double & mstime, const kit::WindowEvent& evt)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();
  
  if (!uiSystem.usesMouse())
  {
    if(evt.type == kit::WindowEvent::MouseScrolled && !win->isKeyDown(kit::LeftAlt) && !win->isKeyDown(kit::LeftControl) && !win->isKeyDown(kit::LeftShift))
    {
      this->m_targetDistance -= evt.mouse.scrollOffset.y * (this->m_targetDistance*0.2f);
    }
  }
}

void kwe::EditorCamera::update(double const & ms, const glm::vec2 & mouseOffset)
{
  auto editorRef = this->m_editorReference;
  auto & uiSystem = editorRef->getUISystem();
  auto appRef = editorRef->getApplication();
  auto win = appRef->getWindow();
  
  if (!uiSystem.usesMouse())
  {
    if(win->isMouseButtonDown(kit::MiddleMouse))
    {
      if(!win->isKeyDown(kit::Key::LeftShift))
      {
        this->m_verticalAngle   += mouseOffset.y * 0.1f;
        this->m_horizontalAngle += mouseOffset.x * 0.1f;
      }
      else
      {
        // Horizontal camera rotation
        glm::quat rot = glm::rotate(glm::quat(), glm::radians(this->m_horizontalAngle), glm::vec3(0.0, 1.0, 0.0));
        
        this->m_origin += rot * glm::vec3(1.0, 0.0, 0.0) * (mouseOffset.x * (this->m_distance * 0.1f) * 0.01f);
        this->m_origin += rot * glm::vec3(0.0, 0.0, 1.0) * (mouseOffset.y * (this->m_distance * 0.1f) * 0.01f);

      }
      
      this->updateOrigin();
      
      editorRef->notifyDrag();
    }
  }
  
  this->updateCamera(ms);
}

void kwe::EditorCamera::render()
{
}

void kwe::EditorCamera::renderUI(const double & mstime)
{
}

kit::Camera::Ptr kwe::EditorCamera::getCamera()
{
  return this->m_camera;
}

void kwe::EditorCamera::updateOrigin()
{
  // Terrain resolution
  auto terrain = this->m_editorReference->getTerrain();
  auto res = terrain->getResolution();
  float xzScale = terrain->getXZScale();
  
  glm::vec2 hsizef = ((glm::vec2(res.x, res.y) * xzScale) / 2.0f);

  if(this->m_origin.x >= hsizef.x - 0.001f - xzScale)
  {
    this->m_origin.x = hsizef.x - 0.001f - xzScale;
  }

  if(this->m_origin.z >= hsizef.y - 0.001f  - xzScale)
  {
    this->m_origin.z = hsizef.y - 0.001f  - xzScale;
  }

  if(this->m_origin.x <= -hsizef.x + 0.001f)
  {
    this->m_origin.x = -hsizef.x + 0.001f;
  }

  if(this->m_origin.z <= -hsizef.y + 0.001f)
  {
    this->m_origin.z = -hsizef.y + 0.001f;
  }

  float x = this->m_origin.x;
  float z = this->m_origin.z;

  glm::vec3 sample = this->m_editorReference->getTerrain()->sampleBilinear(x, z);


  //std::cout << "x "<< x << " z " << z << " h " << sample.x << std::endl;
  
  //this->m_originMarker->setEuler(glm::vec3(sample.y-90.0f, 0.0f, sample.z));
  this->m_targetOriginHeight = sample.x; // Set to terrain height 
}

void kwe::EditorCamera::updateCamera(double const & ms)
{
  if (this->m_targetDistance < 0.25f)
  {
    this->m_targetDistance = 0.25f;
  }
  if(this->m_targetDistance > 80.0f)
  {
    this->m_targetDistance = 80.0f;
  }
  
  this->m_distance = glm::mix(this->m_distance, this->m_targetDistance, 0.025f * ms);

  if (this->m_verticalAngle > 0.0f)
  {
    this->m_verticalAngle = 0.0f;
  }
  if (this->m_verticalAngle < -95.0f)
  {
    this->m_verticalAngle = -95.0f;
  }
  
  this->m_origin.y = glm::mix(this->m_origin.y, this->m_targetOriginHeight, 0.025f * ms);
  //this->m_originMarker->setPosition(glm::vec3(this->m_origin.x, this->m_origin.y, this->m_origin.z));

  glm::quat rot = glm::rotate(glm::quat(), glm::radians(this->m_horizontalAngle), glm::vec3(0.0, 1.0, 0.0));
  rot = glm::rotate(rot, glm::radians(this->m_verticalAngle), glm::vec3(1.0, 0.0, 0.0));
  this->m_camera->setRotation(rot);
  this->m_camera->setPosition(this->m_origin - (this->m_camera->getForward() * this->m_distance));
}

glm::vec3 kwe::EditorCamera::getOrigin()
{
  return this->m_origin;
}

float kwe::EditorCamera::getDistance()
{
  return this->m_distance;
}

void kwe::EditorCamera::brushesUpdated()
{

}

void kwe::EditorCamera::materialsUpdated()
{

}

void kwe::EditorCamera::texturesUpdated()
{

}

void kwe::EditorCamera::meshesUpdated()
{

}


