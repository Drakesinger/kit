#include "MaterialDesignerState.hpp"
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

#include <iostream>
#include <glm/glm.hpp>

kmd::MaterialDesignerState::Ptr kmd::MaterialDesignerState::create()
{
  auto returner = std::make_shared<kmd::MaterialDesignerState>();
  return returner;
}

/* --- Default overrides --- */

void kmd::MaterialDesignerState::allocate()
{
  std::string env = "yokohama";

  auto fbSize = this->m_application->getWindow()->getFramebufferSize();
  glm::uvec2 ufbSize(fbSize.x, fbSize.y);
  
  std::cout << "Framebuffer size is " << fbSize.x << "x" << fbSize.y << "(ratio is " << ((float)fbSize.x / (float)fbSize.y) << ")" << std::endl; 
  
  this->m_camera = kit::Camera::create(45.0f, (float)fbSize.x / (float)fbSize.y, glm::vec2(0.01f, 100.0f));
  this->m_camera->setWhitepoint(1.0f);
  this->m_horizontalAngle = 180.0f;
  this->m_verticalAngle = -30.0f;
  this->m_origin = glm::vec3(0.0f, 0.4f, 0.0f);
  this->m_distance = 3.0f;

  this->m_lastMousePosition = glm::vec2(0.0, 0.0);
  
  this->m_renderPayload = kit::Renderer::Payload::create();

  this->m_envLight = kit::Light::create(kit::Light::IBL);
  this->m_envLight->setEnvironment(env);
  this->m_envLight->setColor(glm::vec3(1.0, 1.0, 1.0)*1.0f);
  this->m_renderPayload->addLight(this->m_envLight);

  this->m_sun = kit::Light::create(kit::Light::Directional, glm::uvec2(4096, 4096));
  this->m_sun->setMaxShadowDistance(40.0f);
  this->m_sun->setEuler(glm::vec3(-22.0f, -40.0f, 0.0f));
  this->m_sun->setColor(glm::vec3(1.0, 1.0, 1.0f)*1.0f);
  this->m_renderPayload->addLight(this->m_sun);

  this->m_skybox = kit::Skybox::create(kit::Cubemap::loadRadianceMap(env));

  this->m_gridfloor = kit::GridFloor::create();
  this->m_gridfloor->setShadowCaster(false);
  this->m_renderPayload->addRenderable(this->m_gridfloor);

  this->m_UISystem.setWindow(this->m_application->getWindow());

  this->m_pickBuffer = kit::PixelBuffer::create(ufbSize, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16UI), kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA32F) }, kit::PixelBuffer::AttachmentInfo(kit::Texture::DepthComponent24));
  this->m_pickProgram = kit::Program::load({ "editor/pick.vert" }, { "editor/pick.frag" });
  this->m_fullscreenQuad = kit::Quad::create();

  kit::Model::Ptr test = kit::Model::create("Sphere.mesh");
  this->m_models.push_back(test);
  this->m_renderPayload->addRenderable(test);

  for (auto & currLight : this->m_lights)
  {
    this->m_renderPayload->addLight(currLight);
  }

  this->m_gizmoProgram = kit::Program::load({ "editor/gizmo.vert" }, { "editor/gizmo.frag" });
  this->m_gizmoTranslateX = kit::Submesh::load("TranslateX.X.geometry");
  this->m_gizmoTranslateY = kit::Submesh::load("TranslateY.Y.geometry");
  this->m_gizmoTranslateZ = kit::Submesh::load("TranslateZ.Z.geometry");
  this->m_gizmoRotateX = kit::Submesh::load("RotateX.X.geometry");
  this->m_gizmoRotateY = kit::Submesh::load("RotateY.Y.geometry");
  this->m_gizmoRotateZ = kit::Submesh::load("RotateZ.Z.geometry");
  this->m_gizmoScaleX = kit::Submesh::load("ScaleX.X.geometry");
  this->m_gizmoScaleY = kit::Submesh::load("ScaleY.Y.geometry");
  this->m_gizmoScaleZ = kit::Submesh::load("ScaleZ.Z.geometry");
  this->m_selectedMode = None;
  this->m_transformMode = Translate;
  this->m_isTransforming = false;
  this->m_transformAxis = X;
  this->m_transformSpace = Global;
  this->m_inputMode = Blender;
  
  this->generateMaterialThumbs();
}

void kmd::MaterialDesignerState::release()
{

}

void kmd::MaterialDesignerState::handleEvent(const double& mstime, const kit::WindowEvent& evt)
{
  this->m_UISystem.handleEvent(evt);

  if (!this->m_UISystem.usesMouse() && !this->m_UISystem.usesKeyboard())
  {
    if (evt.type == kit::WindowEvent::KeyPressed && !this->m_isTransforming)
    {
      if (evt.keyboard.key == kit::T) this->m_transformMode = Translate;
      if (evt.keyboard.key == kit::R) this->m_transformMode = Rotate;
      if (evt.keyboard.key == kit::S) this->m_transformMode = Scale;
      if (evt.keyboard.key == kit::G) this->m_transformSpace = Global;
      if (evt.keyboard.key == kit::L) this->m_transformSpace = Local;
      if (evt.keyboard.key == kit::V) this->m_transformSpace = View;
    }

    if (evt.type == kit::WindowEvent::MouseScrolled)
    {
      this->m_distance -= float(evt.mouse.scrollOffset.y) * 0.1f;
    }

    bool lmb = evt.mouse.button == kit::MouseButton::LeftMouse;
    bool mmb = evt.mouse.button == kit::MouseButton::MiddleMouse;
    bool rmb = evt.mouse.button == kit::MouseButton::RightMouse;

    bool pressed = evt.type == kit::WindowEvent::MouseButtonPressed;
    bool released = evt.type == kit::WindowEvent::MouseButtonReleased;

    bool ctrl = this->m_application->getWindow()->isKeyDown(kit::LeftControl);
    bool shift = this->m_application->getWindow()->isKeyDown(kit::LeftShift);
    bool alt = this->m_application->getWindow()->isKeyDown(kit::LeftAlt);

    bool nomods = !ctrl && !shift;

    // On a mousepress/release (exclude stuff dedicated to camera navigation)
    if ((released || pressed) && (lmb || rmb) && !alt)
    {
      // When you press left mouse,  activate transform if 1-9, select the picked object if 12, or deselect if 0
      if (lmb && released && this->m_isTransforming)
      {
        this->m_application->getWindow()->setMouseVirtual(false);
        this->m_isTransforming = false;
      }

      // Render scene to pick buffer and pick at mouse cursor
      this->renderPickBuffer();
      glm::vec2 mousePosition = this->m_application->getWindow()->getMousePosition();
      glm::ivec2 winSize = this->m_application->getWindow()->getFramebufferSize();
      if (mousePosition.x >= winSize.x) mousePosition.x = winSize.x - 1.0f;
      if (mousePosition.y >= winSize.y) mousePosition.y = winSize.y - 1.0f;
      if (mousePosition.x < 0) mousePosition.x = 0.0f;
      if (mousePosition.y < 0) mousePosition.y = 0.0f;

      glm::vec3 pickPosition(mousePosition.x / float(winSize.x), mousePosition.y / float(winSize.y), 0.0f);
      glm::uvec4 result = this->m_pickBuffer->getColorAttachment(0)->getPixelUint(pickPosition);
      glm::vec4 resultextra = this->m_pickBuffer->getColorAttachment(1)->getPixelFloat(pickPosition);

      // When you press right mouse, select the picked material
      if (rmb && pressed && nomods && !this->m_isTransforming)
      {
        if (result.x == 12)
        {
          auto it = this->m_models[result.y]->getMesh()->getSubmeshEntries().begin();
          for (int i = 0; i != result.z; i++, it++);

          this->selectMaterial(it->second.m_material);
        }
      }
      else if (lmb && pressed && !alt && !this->m_isTransforming)
      {
        if (result.x == 0) // Empty space
        {
          this->deselect();
        }
        else if (result.x == 1) // translate X
        {
          this->m_transformAxis = X;
          this->m_isTransforming = true;
          this->m_transformMode = Translate;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 2) // translate Y
        {
          this->m_transformAxis = Y;
          this->m_isTransforming = true;
          this->m_transformMode = Translate;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 3) // translate Z
        {
          this->m_transformAxis = Z;
          this->m_isTransforming = true;
          this->m_transformMode = Translate;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 4) // rotate X
        {
          this->m_transformAxis = X;
          this->m_isTransforming = true;
          this->m_transformMode = Rotate;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 5) // rotate Y
        {
          this->m_transformAxis = Y;
          this->m_isTransforming = true;
          this->m_transformMode = Rotate;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 6) // rotate Z
        {
          this->m_transformAxis = Z;
          this->m_isTransforming = true;
          this->m_transformMode = Rotate;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 7) // scale X
        {
          this->m_transformAxis = X;
          this->m_isTransforming = true;
          this->m_transformMode = Scale;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 8) // scale Y
        {
          this->m_transformAxis = Y;
          this->m_isTransforming = true;
          this->m_transformMode = Scale;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 9) // scale Z
        {
          this->m_transformAxis = Z;
          this->m_isTransforming = true;
          this->m_transformMode = Scale;
          this->m_application->getWindow()->setMouseVirtual(true);
        }
        else if (result.x == 12) // Model
        {
          this->selectModel(this->m_models[result.y]);
        }
      }
    }
  }
}

void kmd::MaterialDesignerState::update(const double& ms)
{

  for (auto & currModel : this->m_models)
  {
    currModel->update(ms);
  }

  this->prepareUI(ms);

  glm::vec2 currMousePosition = this->m_application->getWindow()->getMousePosition();
  glm::vec2 mouseOffset = this->m_lastMousePosition - currMousePosition;
  this->m_lastMousePosition = currMousePosition;

  glm::ivec2 resolution = this->m_application->getWindow()->getFramebufferSize();

  if (!this->m_UISystem.usesMouse())
  {
    static bool isDragging = false;
    static bool wasDragging = false;
    isDragging = false;

    // Get input states
    bool lmb = this->m_application->getWindow()->isMouseButtonDown(kit::LeftMouse);
    bool rmb = this->m_application->getWindow()->isMouseButtonDown(kit::RightMouse);
    bool mmb = this->m_application->getWindow()->isMouseButtonDown(kit::MiddleMouse);
    bool ctrl = this->m_application->getWindow()->isKeyDown(kit::LeftControl);
    bool shift = this->m_application->getWindow()->isKeyDown(kit::LeftShift);
    bool alt = this->m_application->getWindow()->isKeyDown(kit::LeftAlt);

    if (this->m_isTransforming)
    {
      bool x = this->m_transformAxis == X;
      bool y = this->m_transformAxis == Y;
      bool z = this->m_transformAxis == Z;
      bool translate = this->m_transformMode == Translate;
      bool rotate = this->m_transformMode == Rotate;
      bool scale = this->m_transformMode == Scale;
      bool model = this->m_selectedMode == Model;
      bool light = this->m_selectedMode == Light;

      float xspeed = 0.001f * (mouseOffset.x);
      float yspeed = 0.001f * (mouseOffset.y);

      if (model)
      {
        auto currModel = this->m_selectedModel.lock();
        if (currModel)
        {
          glm::vec3 rightVec = glm::vec3(glm::vec4(this->m_camera->getRotationMatrix() * glm::vec4(currModel->getRight(), 0.0f)));
          glm::vec3 upVec = glm::vec3(glm::vec4(this->m_camera->getRotationMatrix() * glm::vec4(currModel->getUp(), 0.0f)));
          glm::vec3 frontVec = glm::vec3(glm::vec4(this->m_camera->getRotationMatrix() * glm::vec4(currModel->getForward(), 0.0f)));

          float xr = -glm::sign(rightVec.x) * xspeed;
          float xu = glm::sign(rightVec.y) * yspeed;
          float xx = xr + xu;

          float yr = -glm::sign(upVec.x) * xspeed;
          float yu = glm::sign(upVec.y) * yspeed;
          float yy = yr + yu;

          float zr = -glm::sign(frontVec.x) * xspeed;
          float zu = glm::sign(frontVec.y) * yspeed;
          float zz = zr + zu;

          float xy = xspeed + yspeed;

          if (x && rotate) currModel->rotateX(xy*20.0f);
          if (y && rotate) currModel->rotateY(xy*20.0f);
          if (z && rotate) currModel->rotateZ(xy*20.0f);
          if (scale && shift)
          {
            currModel->scale(glm::vec3(-xy, -xy, -xy));
          }
          else
          {
            if (x && scale) currModel->scale(glm::vec3(1.0f, 0.0f, 0.0f) * xx);
            if (y && scale) currModel->scale(glm::vec3(0.0f, 1.0f, 0.0f) * yy);
            if (z && scale) currModel->scale(glm::vec3(0.0f, 0.0f, -1.0f) * zz);
          }

          if (this->m_transformSpace == Local)
          {
            if (x && translate) currModel->translate(currModel->getRight() * xx);
            if (y && translate) currModel->translate(currModel->getUp() * yy);
            if (z && translate) currModel->translate(currModel->getForward() * zz);
          }
          else if (this->m_transformSpace == Global)
          {
            if (x && translate) currModel->translate(glm::vec3(1.0f, 0.0f, 0.0f) * xx);
            if (y && translate) currModel->translate(glm::vec3(0.0f, 1.0f, 0.0f) * yy);
            if (z && translate) currModel->translate(glm::vec3(0.0f, 0.0f, -1.0f) * zz);
          }
          else if (this->m_transformSpace == View)
          {
            if (x && translate) currModel->translate(this->m_camera->getRight() * xspeed);
            if (y && translate) currModel->translate(this->m_camera->getUp() * yspeed);
            if (z && translate) currModel->translate(this->m_camera->getForward() * xy);
          }
        }
      }
      else if (light)
      {
        auto currLight = this->m_selectedLight.lock();
        if (currLight)
        {
          glm::vec3 rightVec = glm::vec3(glm::vec4(this->m_camera->getRotationMatrix() * glm::vec4(currLight->getRight(), 0.0f)));
          glm::vec3 upVec = glm::vec3(glm::vec4(this->m_camera->getRotationMatrix() * glm::vec4(currLight->getUp(), 0.0f)));
          glm::vec3 frontVec = glm::vec3(glm::vec4(this->m_camera->getRotationMatrix() * glm::vec4(currLight->getForward(), 0.0f)));

          float xr = -glm::sign(rightVec.x) * xspeed;
          float xu = glm::sign(rightVec.y) * yspeed;
          float xx = xr + xu;

          float yr = -glm::sign(upVec.x) * xspeed;
          float yu = glm::sign(upVec.y) * yspeed;
          float yy = yr + yu;

          float zr = -glm::sign(frontVec.x) * xspeed;
          float zu = glm::sign(frontVec.y) * yspeed;
          float zz = zr + zu;

          float xy = xspeed + yspeed;

          if (this->m_transformSpace == Local)
          {
            if (x && translate) currLight->translate(currLight->getRight() * xx);
            if (y && translate) currLight->translate(currLight->getUp() * yy);
            if (z && translate) currLight->translate(currLight->getForward() * zz);
          }
          else if (this->m_transformSpace == Global)
          {
            if (x && translate) currLight->translate(glm::vec3(1.0f, 0.0f, 0.0f) * xx);
            if (y && translate) currLight->translate(glm::vec3(0.0f, 1.0f, 0.0f) * yy);
            if (z && translate) currLight->translate(glm::vec3(0.0f, 0.0f, -1.0f) * zz);
          }
          else if (this->m_transformSpace == View)
          {
            if (x && translate) currLight->translate(this->m_camera->getRight() * xspeed);
            if (y && translate) currLight->translate(this->m_camera->getUp() * yspeed);
            if (z && translate) currLight->translate(this->m_camera->getForward() * xy);
          }
        }
      }

    }
    else
    {
      // Define lambdas for tumble, track and dolly actions
      auto tumble = [this, mouseOffset]()
      {
        this->m_horizontalAngle += mouseOffset.x * 0.5f;
        this->m_verticalAngle += mouseOffset.y * 0.5f;
        isDragging = true;
      };

      auto track = [this, mouseOffset]()
      {
        this->m_origin += this->m_camera->getRight() * mouseOffset.x * 0.005f;
        this->m_origin -= this->m_camera->getUp() * mouseOffset.y * 0.005f;
        isDragging = true;
      };

      auto dolly = [this, mouseOffset]()
      {
        this->m_distance += -mouseOffset.y*0.005f;
        isDragging = true;
      };

      // Switch to route the different input combinations into actions, based on input mode
      // Free to use for other stuff: Any combination of L, R, Ctrl and Shift.
      switch (this->m_inputMode)
      {
      case Maya:
        if (alt)
        {
          if (lmb && mmb) dolly();
          else if (lmb) tumble();
          else if (mmb) track();
          else if (rmb) dolly();
        }
        break;
      case Max:
        if (mmb)
        {
          if (ctrl && alt) dolly();
          else if (alt) tumble();
          else track();
        }
        break;
      case Blender:
        if (mmb)
        {
          if (ctrl) dolly();
          else if (shift) track();
          else tumble();
        }
        break;
      };

      if (isDragging && !wasDragging)
      {
        this->m_application->getWindow()->setMouseVirtual(true);
      }
      else if (wasDragging && !isDragging)
      {
        this->m_application->getWindow()->setMouseVirtual(false);
      }
      wasDragging = isDragging;
    }
  }
  this->updateCamera(ms);
}

void kmd::MaterialDesignerState::render()
{
  this->m_application->getWindow()->bind();

  // Render gizmo
  this->renderGizmo();

  // Render UI
  this->m_UISystem.render();
}

void kmd::MaterialDesignerState::onConsoleActive()
{
  kit::ApplicationState::onConsoleActive();
}

void kmd::MaterialDesignerState::onConsoleInactive()
{
  kit::ApplicationState::onConsoleInactive();
}

void kmd::MaterialDesignerState::onActive()
{
  std::cout << "OA Setting aspect ratio to " << ((float)this->m_application->getWindow()->getFramebufferSize().x / (float)this->m_application->getWindow()->getFramebufferSize().y) << std::endl;
  this->m_camera->setAspectRatio((float)this->m_application->getWindow()->getFramebufferSize().x / (float)this->m_application->getWindow()->getFramebufferSize().y);
  dynamic_cast<kmd::Application*>(this->m_application)->getRenderer()->registerPayload(this->m_renderPayload);
  dynamic_cast<kmd::Application*>(this->m_application)->getRenderer()->setBloomDirtMask(kit::Texture::load("lensdirt.tga"));
  dynamic_cast<kmd::Application*>(this->m_application)->getRenderer()->setActiveCamera(this->m_camera);
  dynamic_cast<kmd::Application*>(this->m_application)->getRenderer()->setSkybox(this->m_skybox);
}

void kmd::MaterialDesignerState::onInactive()
{
  dynamic_cast<kmd::Application*>(this->m_application)->getRenderer()->unregisterPayload(this->m_renderPayload);
}

void kmd::MaterialDesignerState::onResize(glm::uvec2 size)
{
  if(size.x > 0 && size.y > 0)
  {
    std::cout << "OR Setting aspect ratio to " << ((float)size.x / (float)size.y) << std::endl;
    
    this->m_camera->setAspectRatio((float)size.x / (float)size.y);
    this->m_pickBuffer = kit::PixelBuffer::create(size, { kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA16UI), kit::PixelBuffer::AttachmentInfo(kit::Texture::RGBA32F) }, kit::PixelBuffer::AttachmentInfo(kit::Texture::DepthComponent24));
  }
}

/* --- Camera --- */
void kmd::MaterialDesignerState::updateCamera(double const & ms)
{
  if (this->m_distance < 0.001f)
  {
    this->m_distance = 0.001f;
  }

  if (this->m_verticalAngle > 89.0f)
  {
    this->m_verticalAngle = 89.0f;
  }
  if (this->m_verticalAngle < -89.0f)
  {
    this->m_verticalAngle = -89.0f;
  }

  glm::quat rot = glm::rotate(glm::quat(), glm::radians(this->m_horizontalAngle), glm::vec3(0.0, 1.0, 0.0));
  rot = glm::rotate(rot, glm::radians(this->m_verticalAngle), glm::vec3(1.0, 0.0, 0.0));
  this->m_camera->setRotation(rot);
  this->m_camera->setPosition(this->m_origin - (this->m_camera->getForward() * this->m_distance));
}

/* --- Editor states --- */

void kmd::MaterialDesignerState::selectMaterial(kit::Material::Ptr material)
{
  this->m_selectedMaterial = material;
}

void kmd::MaterialDesignerState::selectModel(kit::Model::Ptr model)
{
  if (model)
  {
    std::cout << "Selected model " << model << std::endl;
    this->m_selectedModel = model;
    this->m_selectedMode = Model;
  }
  else
  {
    std::cout << "Selected null model, ignoring.." << std::endl;
    this->m_selectedModel = kit::ModelWPtr();
    this->m_selectedMode = None;
  }
}

void kmd::MaterialDesignerState::selectLight(kit::Light::Ptr light)
{
  if (light)
  {
    std::cout << "Selected light " << light << std::endl;
    this->m_selectedLight = light;
    this->m_selectedMode = Light;
  }
  else
  {
    std::cout << "Selected null light, ignoring.." << std::endl;
    this->m_selectedLight = kit::LightWPtr();
    this->m_selectedMode = None;
  }
}

void kmd::MaterialDesignerState::deselect()
{
  std::cout << "Deselecting" << std::endl;
  this->m_selectedLight = kit::LightWPtr();
  this->m_selectedModel = kit::ModelWPtr();
  this->m_selectedMode = None;
}

/* --- Picking and rendering -- */
void kmd::MaterialDesignerState::renderGizmo()
{
  if (this->m_selectedMode == None)
  {
    return;
  }

  kit::GL::depthMask(GL_TRUE);
  kit::GL::enable(GL_DEPTH_TEST);
  kit::GL::disable(GL_BLEND);
  kit::GL::enable(GL_CULL_FACE);
  kit::GL::cullFace(GL_BACK);
  this->m_application->getWindow()->bind();

  this->m_gizmoProgram->use();
  this->m_gizmoProgram->setUniform1f("uniform_scaleCoeff", 2.0f * 48.0f / float(this->m_application->getWindow()->getFramebufferSize().x));

  glm::mat4 viewMatrix = this->m_camera->getViewMatrix();
  glm::mat4 projectionMatrix = this->m_camera->getProjectionMatrix();

  if (this->m_selectedMode == Model)
  {
    auto currModel = this->m_selectedModel.lock();
    if (!currModel)
    {
      this->m_selectedMode = None;
      return;
    }
    
    //glm::mat4 modelMatrix = unlockedModel->getTransformMatrix();
    glm::mat4 modelMatrix;
    if (this->m_transformMode == Translate)
    {
      if (this->m_transformSpace == Global)
      {
        modelMatrix = glm::translate(currModel->getPosition());
      }
      else if (this->m_transformSpace == Local)
      {
        glm::mat4 rotationMatrix = currModel->getRotationMatrix();
        glm::mat4 positionMatrix = glm::translate(currModel->getPosition());
        modelMatrix = positionMatrix * rotationMatrix;
      }
      else if (this->m_transformSpace == View)
      {
        glm::mat4 rotationMatrix = glm::inverse(this->m_camera->getRotationMatrix());
        glm::mat4 positionMatrix = glm::translate(currModel->getPosition());
        modelMatrix = positionMatrix * rotationMatrix;
      }
    }
    else
    {
      glm::mat4 rotationMatrix = currModel->getRotationMatrix();
      glm::mat4 positionMatrix = glm::translate(currModel->getPosition());
      modelMatrix = positionMatrix * rotationMatrix;
    }
    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));

    this->m_gizmoProgram->setUniformMat4("uniform_mvpMatrix", mvpMatrix);
    this->m_gizmoProgram->setUniformMat4("uniform_normalMatrix", normalMatrix);

    glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    glm::vec4 blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    glm::vec4 purple = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    bool shift = this->m_application->getWindow()->isKeyDown(kit::LeftShift);

    // Models should have all possible transformations
    if (this->m_transformMode == Translate)
    {
      this->m_gizmoProgram->setUniform4f("uniform_color", red);
      if (!this->m_isTransforming || this->m_transformAxis == X)  this->m_gizmoTranslateX->renderGeometry();
      this->m_gizmoProgram->setUniform4f("uniform_color", green);
      if (!this->m_isTransforming || this->m_transformAxis == Y) this->m_gizmoTranslateY->renderGeometry();
      this->m_gizmoProgram->setUniform4f("uniform_color", blue);
      if (!this->m_isTransforming || this->m_transformAxis == Z) this->m_gizmoTranslateZ->renderGeometry();
    }
    else if (this->m_transformMode == Rotate)
    {
      this->m_gizmoProgram->setUniform4f("uniform_color", red);
      if (!this->m_isTransforming || this->m_transformAxis == X) this->m_gizmoRotateX->renderGeometry();
      this->m_gizmoProgram->setUniform4f("uniform_color", green);
      if (!this->m_isTransforming || this->m_transformAxis == Y) this->m_gizmoRotateY->renderGeometry();
      this->m_gizmoProgram->setUniform4f("uniform_color", blue);
      if (!this->m_isTransforming || this->m_transformAxis == Z) this->m_gizmoRotateZ->renderGeometry();
    }
    else if (this->m_transformMode == Scale)
    {
      this->m_gizmoProgram->setUniform4f("uniform_color", (shift ? purple : red));
      if (!this->m_isTransforming || this->m_transformAxis == X || shift) this->m_gizmoScaleX->renderGeometry();
      this->m_gizmoProgram->setUniform4f("uniform_color", (shift ? purple : green));
      if (!this->m_isTransforming || this->m_transformAxis == Y || shift) this->m_gizmoScaleY->renderGeometry();
      this->m_gizmoProgram->setUniform4f("uniform_color", (shift ? purple : blue));
      if (!this->m_isTransforming || this->m_transformAxis == Z || shift) this->m_gizmoScaleZ->renderGeometry();
    }
  }
  else if (this->m_selectedMode == Light)
  {
    auto currLight = this->m_selectedLight.lock();
    if (!currLight)
    {
      this->m_selectedMode = None;
      return;
    }

    //glm::mat4 modelMatrix = unlockedModel->getTransformMatrix();
    glm::mat4 modelMatrix;
    if (this->m_transformMode == Translate)
    {
      if (this->m_transformSpace == Global)
      {
        modelMatrix = glm::translate(currLight->getPosition());
      }
      else if (this->m_transformSpace == Local)
      {
        glm::mat4 rotationMatrix = currLight->getRotationMatrix();
        glm::mat4 positionMatrix = glm::translate(currLight->getPosition());
        modelMatrix = positionMatrix * rotationMatrix;
      }
      else if (this->m_transformSpace == View)
      {
        glm::mat4 rotationMatrix = glm::inverse(this->m_camera->getRotationMatrix());
        glm::mat4 positionMatrix = glm::translate(currLight->getPosition());
        modelMatrix = positionMatrix * rotationMatrix;
      }
    }
    else
    {
      glm::mat4 rotationMatrix = currLight->getRotationMatrix();
      glm::mat4 positionMatrix = glm::translate(currLight->getPosition());
      modelMatrix = positionMatrix * rotationMatrix;
    }
    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));

    this->m_gizmoProgram->setUniformMat4("uniform_mvpMatrix", mvpMatrix);
    this->m_gizmoProgram->setUniformMat4("uniform_normalMatrix", normalMatrix);

    glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    glm::vec4 blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    glm::vec4 purple = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    bool shift = this->m_application->getWindow()->isKeyDown(kit::LeftShift);

    bool point = currLight->getType() == kit::Light::Point;

    // Lights should have translation. Only directional and spotlights should have X and Y rotation. Only spotlights should have XY and Z scaling. Only point lights should have uniform scale
    if (point && this->m_transformMode == Translate)
    {
      this->m_gizmoProgram->setUniform4f("uniform_color", red);
      if (!this->m_isTransforming || this->m_transformAxis == X)  this->m_gizmoTranslateX->renderGeometry();
      this->m_gizmoProgram->setUniform4f("uniform_color", green);
      if (!this->m_isTransforming || this->m_transformAxis == Y) this->m_gizmoTranslateY->renderGeometry();
      this->m_gizmoProgram->setUniform4f("uniform_color", blue);
      if (!this->m_isTransforming || this->m_transformAxis == Z) this->m_gizmoTranslateZ->renderGeometry();
    }
    if (point && this->m_transformMode == Scale)
    {
      this->m_gizmoProgram->setUniform4f("uniform_color", purple);
      this->m_gizmoScaleX->renderGeometry();
      this->m_gizmoScaleY->renderGeometry();
      this->m_gizmoScaleZ->renderGeometry();
    }
  }
  else if (this->m_selectedMode == Terrain)
  {
  }

}

void kmd::MaterialDesignerState::renderPickBuffer()
{
  // Render pick buffer
  //kit::GL::disable(GL_FRAMEBUFFER_SRGB);
  kit::GL::disable(GL_BLEND);

  kit::GL::depthMask(GL_TRUE);
  kit::GL::enable(GL_DEPTH_TEST);
  kit::GL::disable(GL_CULL_FACE);
  this->m_pickBuffer->clearDepth(1.0f);
  this->m_pickBuffer->clearAttachment(0, glm::uvec4(0, 0, 0, 0));
  this->m_pickBuffer->clearAttachment(1, glm::vec4(0.0, 0.0, 0.0, 0.0));
  this->m_pickProgram->use();

  glm::mat4 viewMatrix = this->m_camera->getViewMatrix();
  glm::mat4 projectionMatrix = this->m_camera->getProjectionMatrix();

  // Render models
  kit::GL::depthMask(GL_TRUE);
  kit::GL::enable(GL_DEPTH_TEST);
  kit::GL::disable(GL_CULL_FACE);
  uint32_t modelId = 0;
  this->m_pickProgram->setUniform1ui("uniform_targetId", 12);
  this->m_pickProgram->setUniform1i("uniform_doScale", 0);
  for (auto & currModel : this->m_models)
  {
    glm::mat4 modelMatrix = currModel ->getTransformMatrix();
    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

    this->m_pickProgram->setUniform1ui("uniform_modelId", modelId);
    this->m_pickProgram->setUniformMat4("uniform_mvpMatrix", mvpMatrix);
    uint32_t submeshId = 0;
    for (auto & currSubmesh : currModel->getMesh()->getSubmeshEntries())
    {
      this->m_pickProgram->setUniform1ui("uniform_submeshId", submeshId);
      currSubmesh.second.m_submesh->renderGeometry();
      submeshId++;
    }

    modelId++;
  }

  // Render gizmo
  kit::GL::depthMask(GL_FALSE);
  kit::GL::disable(GL_DEPTH_TEST);
  kit::GL::enable(GL_CULL_FACE);
  kit::GL::cullFace(GL_BACK);
  this->m_pickProgram->setUniform1i("uniform_doScale", 1);
  this->m_pickProgram->setUniform1f("uniform_scaleCoeff", 2.0f * 48.0f / float(this->m_application->getWindow()->getFramebufferSize().x));
  if (this->m_selectedMode == Model)
  {
    auto unlockedModel = this->m_selectedModel.lock();
    if (unlockedModel)
    {
      // Get model ID
      uint32_t cmodelId = 0;
      for (auto & currModel : this->m_models)
      {
        if (currModel == unlockedModel)
        {
          break;
        }
        else
        {
          cmodelId++;
        }
      }

      glm::mat4 modelMatrix;
      if (this->m_transformMode == Translate)
      {
        if (this->m_transformSpace == Global)
        {
          modelMatrix = glm::translate(unlockedModel->getPosition());
        }
        else if (this->m_transformSpace == Local)
        {
          glm::mat4 rotationMatrix = unlockedModel->getRotationMatrix();
          glm::mat4 positionMatrix = glm::translate(unlockedModel->getPosition());
          modelMatrix = positionMatrix * rotationMatrix;
        }
        else if (this->m_transformSpace == View)
        {
          glm::mat4 rotationMatrix = glm::inverse(this->m_camera->getRotationMatrix());
          glm::mat4 positionMatrix = glm::translate(unlockedModel->getPosition());
          modelMatrix = positionMatrix * rotationMatrix;
        }
      }
      else
      {
        glm::mat4 rotationMatrix = unlockedModel->getRotationMatrix();
        glm::mat4 positionMatrix = glm::translate(unlockedModel->getPosition());
        modelMatrix = positionMatrix * rotationMatrix;
      }

      glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

      this->m_pickProgram->setUniformMat4("uniform_mvpMatrix", mvpMatrix);
      this->m_pickProgram->setUniform1ui("uniform_transformableType", 12);
      this->m_pickProgram->setUniform1ui("uniform_transformableId", cmodelId);
      
      // Models should have all possible transformations
      if (this->m_transformMode == Translate)
      {
        this->m_pickProgram->setUniform1ui("uniform_targetId", 1);
        this->m_gizmoTranslateX->renderGeometry();
        this->m_pickProgram->setUniform1ui("uniform_targetId", 2);
        this->m_gizmoTranslateY->renderGeometry();
        this->m_pickProgram->setUniform1ui("uniform_targetId", 3);
        this->m_gizmoTranslateZ->renderGeometry();
      }
      else if (this->m_transformMode == Rotate)
      {
        this->m_pickProgram->setUniform1ui("uniform_targetId", 4);
        this->m_gizmoRotateX->renderGeometry();
        this->m_pickProgram->setUniform1ui("uniform_targetId", 5);
        this->m_gizmoRotateY->renderGeometry();
        this->m_pickProgram->setUniform1ui("uniform_targetId", 6);
        this->m_gizmoRotateZ->renderGeometry();
      }
      else if (this->m_transformMode == Scale)
      {
        this->m_pickProgram->setUniform1ui("uniform_targetId", 7);
        this->m_gizmoScaleX->renderGeometry();
        this->m_pickProgram->setUniform1ui("uniform_targetId", 8);
        this->m_gizmoScaleY->renderGeometry();
        this->m_pickProgram->setUniform1ui("uniform_targetId", 9);
        this->m_gizmoScaleZ->renderGeometry();
      }
    }
  }
  else if (this->m_selectedMode == Light)
  {
    auto unlockedLight = this->m_selectedLight.lock();
    if (!unlockedLight)
    {
      this->m_selectedMode = None;
      return;
    }

    // Get light ID
    uint32_t clightId = 0;
    for (auto & currLight : this->m_lights)
    {
      if (currLight == unlockedLight)
      {
        break;
      }
      else
      {
        clightId++;
      }
    }


    //glm::mat4 modelMatrix = unlockedModel->getTransformMatrix();
    glm::mat4 modelMatrix;
    if (this->m_transformMode == Translate)
    {
      if (this->m_transformSpace == Global)
      {
        modelMatrix = glm::translate(unlockedLight->getPosition());
      }
      else if (this->m_transformSpace == Local)
      {
        glm::mat4 rotationMatrix = unlockedLight->getRotationMatrix();
        glm::mat4 positionMatrix = glm::translate(unlockedLight->getPosition());
        modelMatrix = positionMatrix * rotationMatrix;
      }
      else if (this->m_transformSpace == View)
      {
        glm::mat4 rotationMatrix = glm::inverse(this->m_camera->getRotationMatrix());
        glm::mat4 positionMatrix = glm::translate(unlockedLight->getPosition());
        modelMatrix = positionMatrix * rotationMatrix;
      }
    }
    else
    {
      glm::mat4 rotationMatrix = unlockedLight->getRotationMatrix();
      glm::mat4 positionMatrix = glm::translate(unlockedLight->getPosition());
      modelMatrix = positionMatrix * rotationMatrix;
    }
    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;

    this->m_pickProgram->setUniformMat4("uniform_mvpMatrix", mvpMatrix);
    this->m_pickProgram->setUniform1ui("uniform_transformableType", 14);
    this->m_pickProgram->setUniform1ui("uniform_transformableId", clightId);


    glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    glm::vec4 blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    glm::vec4 purple = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    bool shift = this->m_application->getWindow()->isKeyDown(kit::LeftShift);

    bool point = unlockedLight->getType() == kit::Light::Point;

    // Lights should have translation. Only directional and spotlights should have X and Y rotation. Only spotlights should have XY and Z scaling. Only point lights should have uniform scale
    if (point && this->m_transformMode == Translate)
    {
      this->m_pickProgram->setUniform1ui("uniform_targetId", 1);
      this->m_gizmoTranslateX->renderGeometry();
      this->m_pickProgram->setUniform1ui("uniform_targetId", 2);
      this->m_gizmoTranslateY->renderGeometry();
      this->m_pickProgram->setUniform1ui("uniform_targetId", 3);
      this->m_gizmoTranslateZ->renderGeometry();
    }
    if (point && this->m_transformMode == Scale)
    {
      this->m_pickProgram->setUniform1ui("uniform_targetId", 11);
      this->m_gizmoScaleX->renderGeometry();
      this->m_gizmoScaleY->renderGeometry();
      this->m_gizmoScaleZ->renderGeometry();
    }
  }


  this->m_application->getWindow()->bind();
  kit::Program::useFixed();
  //kit::GL::enable(GL_FRAMEBUFFER_SRGB);
}

/* --- User interface --- */
kit::Texture::Ptr selectTexture(std::string name, kit::Texture::Ptr currentTexture, bool srgb = true, bool reload = false)
{
  kit::Texture::Ptr newTexture = currentTexture;
  static std::vector<std::string> textures = kit::Texture::getAvailableTextures();


  ImGui::BeginGroup();
  {
    // Set the map texture if available
    if (currentTexture)
    {
      ImGui::BeginGroup();
      ImTextureID texId = (ImTextureID)currentTexture->getHandle();
      if (ImGui::ImageButton(texId, ImVec2(92.0f, 92.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f)))
      {
        ImGui::OpenPopup(name.c_str());
      }
      ImGui::Text(currentTexture->getFilename().c_str());
      ImGui::EndGroup();
    }
    else
    {
      if (ImGui::Button( std::string( std::string("No texture..##notexture") + name ).c_str() ))
      {
        ImGui::OpenPopup(name.c_str());
      }
    }
    // Show the popup
    if (ImGui::BeginPopup(name.c_str()))
    {
      // Header text
      ImGui::Text("Set texture..");

      // Text to unset texture
      if (ImGui::Selectable(std::string(std::string("Unset texture..##unsettexture") + name).c_str()))
      {
        newTexture = nullptr;
      }
      ImGui::Separator();
      for (int i = 0; i < textures.size(); i++)
      {
        if (ImGui::Selectable(textures[i].c_str()))
        {
          newTexture = kit::Texture::load(textures[i], srgb);
        }
      }
      ImGui::EndPopup();
    }
  }
  ImGui::EndGroup();

  return newTexture;

}

void kmd::MaterialDesignerState::prepareUI(double const & ms)
{
  this->m_UISystem.prepareFrame(ms);
 
  this->prepareMaterialProperties(ms);
  this->prepareRenderProperties(ms);
  this->prepareEnvironmentProperties(ms);
  this->prepareMainMenu(ms);

  //ImGui::Image((ImTextureID)kit::Font::getSystemFont()->getGlyphMap(24.0)->getTexture().lock()->getHandle(), ImVec2(256, 256));
  ImGui::Image((ImTextureID)((kmd::Application*)this->m_application)->getRenderer()->getGeometryBuffer()->getColorAttachment(2)->getHandle(), ImVec2(720, 480), ImVec2(0, 1), ImVec2(1, 0));
  //ImGui::Image((ImTextureID)this->m_pickBuffer->getColorAttachment(1)->getHandle(), ImVec2(256, 256));
}

void kmd::MaterialDesignerState::prepareMainMenu(double const & ms)
{
  static std::vector<std::string> meshes;
  static bool meshesloaded = false;
  if (!meshesloaded)
  {
    meshes.clear();
    for (auto & currEntry : kit::listFilesystemEntries("./data/meshes/", true, false))
    {
      meshes.push_back(currEntry.filename);
    }
    meshesloaded = true;
  }

  bool doLoadMesh = false;
  static std::string meshLoader = "";

  
  static std::vector<std::string> materials;
  static bool materialsloaded = false;
  if (!materialsloaded)
  {
    materials.clear();
    for (auto & currEntry : kit::listFilesystemEntries("./data/materials/", true, false))
    {
      materials.push_back(currEntry.filename);
    }
    materialsloaded = true;
  }

  bool doLoadMaterial = false;
  static std::string materialLoader = "";
  
  bool doQuit = false;
  bool doNew = false;
  bool doNewConfirm = false;
  

  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("New material"))
      {
        doNew = true;
      }

      if (ImGui::BeginMenu("Load mesh"))
      {
        for (auto & currMesh : meshes)
        {
          if (ImGui::MenuItem(currMesh.c_str()))
          {
            doLoadMesh = true;
            meshLoader = currMesh;
          }
        }
        ImGui::EndMenu();
      }
      
      if (ImGui::BeginMenu("Load material"))
      {
        for (auto & currMaterial : materials)
        {
          if (ImGui::MenuItem(currMaterial.c_str()))
          {
            doLoadMaterial = true;
            materialLoader = currMaterial;
          }
        }
        ImGui::EndMenu();
      }
      
      ImGui::Separator();
      if (ImGui::MenuItem("Exit##menuexit"))
      {
        doQuit = true;
      }
      ImGui::EndMenu();
    }
    
    if (ImGui::BeginMenu("Settings##menusettings"))
    {
      if (ImGui::BeginMenu("Set input mode##menusettingsinputmode"))
      {
        bool mset = (this->m_inputMode == Maya);
        if (ImGui::MenuItem("Maya##menuinputmodemaya", "", &mset))
        {
          this->m_inputMode = Maya;
        }
        bool dset = (this->m_inputMode == Max);
        if (ImGui::MenuItem("3ds Max##menuinputmodemax", "", &dset))
        {
          this->m_inputMode = Max;
        }
        bool bset = (this->m_inputMode == Blender);
        if (ImGui::MenuItem("Blender##menuinputmodeblender", "", &bset))
        {
          this->m_inputMode = Blender;
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if (doQuit)
  {
    ImGui::OpenPopup("Are you sure?##exitconfirm");
  }

  if (doNew)
  {
    ImGui::OpenPopup("Specify new material name##createnewmaterial");
  }

  if (ImGui::BeginPopupModal("Specify new material name##createnewmaterial"))
  {
    static char name[256];
    ImGui::InputText("New material name", name, 256);
    if (ImGui::Button("Cancel"))
    {
      ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Create"))
    {
      for (auto & currModel : this->m_models)
      {
        this->m_renderPayload->removeRenderable(currModel);
      }
      this->m_models.clear();
      kit::Material::clearCache();
      auto mAdd = kit::Mesh::create();
      mAdd->addSubmeshEntry("Sphere", kit::Submesh::load("Sphere.geometry"), kit::Material::load(name));
      auto adder = kit::Model::create(mAdd);
      this->m_models.push_back(adder);
      this->m_renderPayload->addRenderable(adder);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (ImGui::BeginPopupModal("Are you sure?##exitconfirm"))
  {
    ImGui::Text("Are you sure you want to exit? Any unsaved changes will be lost!");
    ImGui::Separator();
    if (ImGui::Button("Yes##exityes"))
    {
      this->m_application->quit();
    }
    ImGui::SameLine();
    if (ImGui::Button("No##exitno"))
    {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  if (doLoadMesh && this->m_models.size() > 0)
  {
    ImGui::OpenPopup("Are you sure?##meshconfirm");
  }

  if (doLoadMesh && this->m_models.size() == 0)
  {
    for (auto & currModel : this->m_models)
    {
      this->m_renderPayload->removeRenderable(currModel);
    }
    this->m_models.clear();
    kit::Material::clearCache();
    auto adder = kit::Model::create(meshLoader);
    this->m_models.push_back(adder);
    this->m_renderPayload->addRenderable(adder);
  }

  if (ImGui::BeginPopupModal("Are you sure?##meshconfirm"))
  {
    ImGui::Text("Are you sure you want to load a new mesh? Any unsaved changes will be lost!");
    ImGui::Separator();
    if (ImGui::Button("Yes##meshyes"))
    {
      for (auto & currModel : this->m_models)
      {
        this->m_renderPayload->removeRenderable(currModel);
      }
      this->m_models.clear();
      kit::Material::clearCache();
      auto adder = kit::Model::create(meshLoader);
      this->m_models.push_back(adder);
      this->m_renderPayload->addRenderable(adder);
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("No##meshno"))
    {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  
  if(doLoadMaterial)
  {
    for (auto & currModel : this->m_models)
    {
      this->m_renderPayload->removeRenderable(currModel);
    }
    this->m_models.clear();
    kit::Material::clearCache();
    auto mAdd = kit::Mesh::create();
    mAdd->addSubmeshEntry("Sphere", kit::Submesh::load("Sphere.geometry"), kit::Material::load(materialLoader));
    auto adder = kit::Model::create(mAdd);
    this->m_models.push_back(adder);
    this->m_renderPayload->addRenderable(adder);
  }
}

void kmd::MaterialDesignerState::prepareRenderProperties(double const & ms)
{

  auto winSize = this->m_application->getWindow()->getFramebufferSize();

  static bool winOpened = true;


  static std::string selectedCCTable = "identity.tga";
  static std::vector<std::string> ccTables;
  static bool ccTablesLoaded = false;
  if (!ccTablesLoaded)
  {
    ccTables.clear();
    for (auto & currEntry : kit::listFilesystemEntries("./data/luts/", true, false))
    {
      ccTables.push_back(currEntry.filename);
    }
    ccTablesLoaded = true;
  }

  if (ImGui::Begin("Render Properties", &winOpened))
  {
    float maxwidth = winSize.x;
    ImVec2 minsize = ImVec2(320.0f, 256.0f);
    auto size = ImGui::GetWindowSize();
    auto pos = ImGui::GetWindowPos();
    ImVec2 newpos = pos;
    ImVec2 newsize = size;

    // Make sure the window follows the min/max size
    if (newsize.x > maxwidth) newsize.x = maxwidth;
    if (newsize.x < minsize.x) newsize.x = minsize.x;
    if (newsize.y < minsize.y) newsize.y = minsize.y;

    // Make sure the window isnt outside the OS window
    if (newpos.x < 0) newpos.x = 0;
    if (newpos.y < 0) newpos.y = 0;
    while (newpos.x + newsize.x >= winSize.x) newpos.x -= 1;
    while (newpos.y + newsize.y >= winSize.y) newpos.y -= 1;

    // Make sure window size isnt bigger than the OS window
    if (newsize.x > winSize.x) newsize.x = winSize.x;
    if (newsize.y > winSize.y) newsize.y = winSize.y;

    // If any of the above checks had a correction, update the window size and position accordingly
    if (newpos.x != pos.x || newpos.y != pos.y) ImGui::SetWindowPos(newpos);
    if (newsize.x != size.x || newsize.y != size.y) ImGui::SetWindowSize(newsize);

    static bool showGrid = true;
    if (ImGui::Checkbox("Show grid", &showGrid))
    {
      if (showGrid)
      {
        this->m_renderPayload->addRenderable(this->m_gridfloor);
      }
      else
      {
        this->m_renderPayload->removeRenderable(this->m_gridfloor);
      }
    }


    if (ImGui::CollapsingHeader("Camera##cameraproperties"))
    {
      float cnear = this->m_camera->getClipRange().x;
      float cfar = this->m_camera->getClipRange().y;
      bool cchange = false;

      if (ImGui::SliderFloat("Nearclip##cameranear", &cnear, 0.0f, 100.0f, "%.5f", 10.0f))
      {
        cchange = true;
      }

      if (ImGui::SliderFloat("Farclip##cameranear", &cfar, 0.0f, 1000.0f, "%.5f", 10.0f))
      {
        cchange = true;
      }

      if (cchange)
      {
        this->m_camera->setClipRange(glm::vec2(cnear, cfar));
      }

      float fov = this->m_camera->getFov();
      if (ImGui::SliderFloat("Field of view##camera-fov", &fov, 1.0, 180.0, "%.3f", 1.0f))
      {
        this->m_camera->setFov(fov);
      }

      float whitepoint = this->m_camera->getWhitepoint();
      if (ImGui::SliderFloat("Whitepoint##camera-whitepoint", &whitepoint, 0.0, 100.0, "%.3f", 10.0f))
      {
        this->m_camera->setWhitepoint(whitepoint);
      }

      float exposure = this->m_camera->getExposure();
      if (ImGui::SliderFloat("Exposure##camera-exposure", &exposure, 0.0, 100.0, "%.3f", 10.0f))
      {
        this->m_camera->setExposure(exposure);
      }
    }

    if (ImGui::CollapsingHeader("Renderer##rendererproperties"))
    {
      auto renderer = dynamic_cast<kmd::Application*>(this->m_application)->getRenderer();

      float resolution = renderer->getInternalResolution();
      if (ImGui::SliderFloat("Resolution scale##renderer-resolution", &resolution, 0.5, 4.0))
      {
        std::cout << "MD setting internal res to " << resolution << std::endl;
        renderer->setInternalResolution(resolution);
      }

      bool bloom = renderer->getBloom();
      if (ImGui::Checkbox("Bloom##renderer-bloom", &bloom))
      {
        renderer->setBloom(bloom);
      }

      float bloomtreshold = renderer->getBloomTresholdBias();
      if (ImGui::SliderFloat("Bloom treshold##renderer-bloom", &bloomtreshold, -1.0, 1.0))
      {
        renderer->setBloomTresholdBias(bloomtreshold);
      }

      bool fxaa = renderer->getFXAA();
      if (ImGui::Checkbox("FXAA##renderer-fxaa", &fxaa))
      {
        renderer->setFXAA(fxaa);
      }

      bool shadows = renderer->getShadows();
      if (ImGui::Checkbox("Shadows##renderer-shadows", &shadows))
      {
        renderer->setShadows(shadows);
      }

      bool fringe = renderer->getSceneFringe();
      if (ImGui::Checkbox("Scene Fringe##renderer-fringe", &fringe))
      {
        renderer->setSceneFringe(fringe);
      }

      float fringescale = renderer->getSceneFringeScale();
      if (ImGui::SliderFloat("Fringe Scale##renderer-fringescale", &fringescale, 0.0, 3.0))
      {
        renderer->setSceneFringeScale(fringescale);
      }

      float fringeexp = renderer->getSceneFringeExponential();
      if (ImGui::SliderFloat("Fringe Exp.##renderer-fringeexp", &fringeexp, 0.0, 3.0))
      {
        renderer->setSceneFringeExponential(fringeexp);
      }

      bool gpumetrics = renderer->getGPUMetrics();
      if (ImGui::Checkbox("Debug metrics##renderer-debug", &gpumetrics))
      {
        renderer->setGPUMetrics(gpumetrics);
      }



      ImGui::Text("Color correction");

      bool ccenabled = renderer->getColorCorrection();
      if (ImGui::Checkbox("Enabled##renderer-ccenabled", &ccenabled))
      {
        renderer->setColorCorrection(ccenabled);
      }

      if (ImGui::Button("CC lookup table.."))
      {
        ImGui::OpenPopup("##newcclut");
      }

      // Show the popup
      if (ImGui::BeginPopup("##newcclut"))
      {
        // Header text
        ImGui::Text("Set CC lookup table..");
        ImGui::Separator();
        for (int i = 0; i < ccTables.size(); i++)
        {
          if (ImGui::Selectable(ccTables[i].c_str()))
          {
            selectedCCTable = ccTables[i];
            renderer->setCCLookupTable(kit::Texture::create3DFromFile(std::string("./data/luts/") + selectedCCTable));
          }
        }
        ImGui::EndPopup();
      }

      ImGui::SameLine();

      ImGui::Text(selectedCCTable.c_str());

    }

  }
  ImGui::End();

}

void kmd::MaterialDesignerState::prepareEnvironmentProperties(double const & ms)
{
  auto winSize = this->m_application->getWindow()->getFramebufferSize();

  static bool winOpened = true;

  static std::string selectedEnvMap = "vindelalvend";
  static std::vector<std::string> envmaps;
  static bool envmapsLoaded = false;
  if (!envmapsLoaded)
  {
    envmaps.clear();
    for (auto & currEntry : kit::listFilesystemEntries("./data/env/", false, true))
    {
      envmaps.push_back(currEntry.filename);
    }
    envmapsLoaded = true;
  }

  if (ImGui::Begin("Environment Properties", &winOpened))
  {

    static bool showSkybox = (this->m_skybox->getTexture() != nullptr);
    static float backgroundColor[3] = { this->m_skybox->getColor().x,  this->m_skybox->getColor().y,  this->m_skybox->getColor().z };
    static float backgroundStrength = this->m_skybox->getStrength();

    if (ImGui::CollapsingHeader("Sky##skyheader"))
    {
      if (ImGui::Checkbox("Set color from IBL##colorfromIBL", &showSkybox))
      {
        if (showSkybox)
        {
          this->m_skybox->setTexture(kit::Cubemap::loadRadianceMap(selectedEnvMap));
        }
        else
        {
          this->m_skybox->setTexture(nullptr);
        }
      }

      if (showSkybox)
      {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25);
      }
      if (ImGui::ColorEdit3("Color##skycolor", backgroundColor))
      {
        glm::vec3 bgcolor(backgroundColor[0], backgroundColor[1], backgroundColor[2]);
        this->m_skybox->setColor(bgcolor);
      }
      if (showSkybox)
      {
        ImGui::PopStyleVar();
      }

      if (ImGui::SliderFloat("Strength##skystrength", &backgroundStrength, 0.0f, 30.0f, "%.3f", 3.0f))
      {
        this->m_skybox->setStrength(backgroundStrength);
      }
    }

    if (ImGui::CollapsingHeader("Image-based Lighting##environmentheader"))
    {
      ImGui::Text("Environment map");

      if (ImGui::Button("Select.."))
      {
        ImGui::OpenPopup("##newenvmap");
      }

      // Show the popup
      if (ImGui::BeginPopup("##newenvmap"))
      {
        // Header text
        ImGui::Text("Set environment map..");
        ImGui::Separator();
        for (int i = 0; i < envmaps.size(); i++)
        {
          if (ImGui::Selectable(envmaps[i].c_str()))
          {
            selectedEnvMap = envmaps[i];
            this->m_envLight->setEnvironment(selectedEnvMap);
            if (showSkybox)
            {
              this->m_skybox->setTexture(this->m_envLight->getReflectionMap());
            }
          }
        }
        ImGui::EndPopup();
      }

      ImGui::SameLine();

      ImGui::Text(selectedEnvMap.c_str());

      // Color
      bool changeColor = false;

      static float currColor[3] = { 1.0f, 1.0f, 1.0f };
      if (ImGui::ColorEdit3("Color##envlightcolor", currColor))
      {
        changeColor = true;
      }

      static float currStrength = 1.0f;
      if (ImGui::SliderFloat("Strength##envlightstrength", &currStrength, 0.0f, 30.0f, "%.3f", 3.0f))
      {
        changeColor = true;
      }

      if (changeColor)
      {
        this->m_envLight->setColor(glm::vec3(currColor[0], currColor[1], currColor[2]) * currStrength);
      }
    }

    if (ImGui::CollapsingHeader("Directional sun"))
    {
      // Angle
      bool changeAngle = false;

      static float currAngleX = glm::radians(-45.0f);
      if (ImGui::SliderAngle("Tilt degrees##suntilt", &currAngleX))
      {
        changeAngle = true;
      }

      static float currAngleY = glm::radians(-45.0f);
      if (ImGui::SliderAngle("Pan degrees##sunpan", &currAngleY))
      {
        changeAngle = true;
      }

      if (changeAngle)
      {
        this->m_sun->setEuler(glm::vec3(glm::degrees(currAngleX), glm::degrees(currAngleY), 0.0f));
      }

      // Color
      bool changeColor = false;

      static float currColor[3] = { 1.0f, 1.0f, 1.0f };
      if (ImGui::ColorEdit3("Color##suncolor", currColor))
      {
        changeColor = true;
      }

      static float currStrength = 0.5f;
      if (ImGui::SliderFloat("Strength##sunstrength", &currStrength, 0.0f, 30.0f, "%.3f", 3.0f))
      {
        changeColor = true;
      }

      if (changeColor)
      {
        this->m_sun->setColor(glm::vec3(currColor[0], currColor[1], currColor[2]) * currStrength);
      }

      float dist = this->m_sun->getMaxShadowDistance();
      if (ImGui::SliderFloat("Shadow distance##sunshadowdist", &dist, 0.0f, 200.0f, "%.3f", 2.0f))
      {
        this->m_sun->setMaxShadowDistance(dist);
      }
    }

    // BEING HERE
    if (ImGui::CollapsingHeader("Custom lights"))
    {
      if (ImGui::Button("Add light"))
      {
        kit::Light::Ptr adder = kit::Light::create(kit::Light::Point);
        this->m_lights.push_back(adder);
        this->m_renderPayload->addLight(adder);
      }

      for (auto &currLight : this->m_lights)
      {
        std::stringstream ss;
        ss << "Select##selectlight" << currLight;

        std::stringstream ssc;
        ssc << "Color##lightcolor" << currLight;

        std::stringstream ssr;
        ssr << "Radius##lightradius" << currLight;

        if (ImGui::Button(ss.str().c_str()))
        {
          this->selectLight(currLight);
        }

        float currVal[3] = {currLight->getColor().x, currLight->getColor().y, currLight->getColor().z};
        if (ImGui::SliderFloat3(ssc.str().c_str(), &currVal[0], 0.0f, 20.0f))
        {
          currLight->setColor(glm::vec3(currVal[0], currVal[1], currVal[2]));
        }

        float radius = currLight->getRadius();
        if (ImGui::SliderFloat(ssr.str().c_str(), &radius, 0.01f, 100.0f))
        {
          currLight->setRadius(radius);
        }
      }
    }

  }
  ImGui::End();
}

void kmd::MaterialDesignerState::prepareMaterialProperties(double const & ms)
{
  auto winSize = this->m_application->getWindow()->getFramebufferSize();

  std::map<std::string, kit::Material::Ptr> cachedMaterials = kit::Material::getCacheList();

  // Return if we have no materials
  if (cachedMaterials.size() == 0)
  {
    return;
  }

  auto currMaterialUnlock = this->m_selectedMaterial.lock();

  if (!currMaterialUnlock)
  {
    this->m_selectedMaterial = cachedMaterials.begin()->second;
    currMaterialUnlock = this->m_selectedMaterial.lock();
  }

  // Create a material-list
  static int materialListIndex = 0;
  std::vector<const char*> materialListContent;

  // Fill the material-list and find the currently selected index
  int finder = 0;
  for (auto & currMaterial : cachedMaterials)
  {
    materialListContent.push_back(currMaterial.first.c_str());

    if (currMaterial.second == currMaterialUnlock)
    {
      materialListIndex = finder;
    }

    finder++;
  }

  // Safety check
  if (materialListIndex >= materialListContent.size())
  {
    materialListIndex = 0;
  }

  // Create a materials window
  static bool matWinOpened = true;
  if (ImGui::Begin("Material Properties", &matWinOpened/*,*/ /*ImGuiWindowFlags_NoSavedSettings*/ /*|ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove*/))
  {
    float maxwidth = winSize.x;
    ImVec2 minsize = ImVec2(256.0f, 384.0f);
    auto size = ImGui::GetWindowSize();
    auto pos = ImGui::GetWindowPos();
    ImVec2 newpos = pos;
    ImVec2 newsize = size;

    // Make sure the window follows the min/max size
    if (newsize.x > maxwidth) newsize.x = maxwidth;
    if (newsize.x < minsize.x) newsize.x = minsize.x;
    if (newsize.y < minsize.y) newsize.y = minsize.y;
    
    // Make sure the window isnt outside the OS window
    if (newpos.x < 0) newpos.x = 0;
    if (newpos.y < 0) newpos.y = 0;
    while (newpos.x + newsize.x >= winSize.x) newpos.x -= 1;
    while (newpos.y + newsize.y >= winSize.y) newpos.y -= 1;

    // Make sure window size isnt bigger than the OS window
    if (newsize.x > winSize.x) newsize.x = winSize.x;
    if (newsize.y > winSize.y) newsize.y = winSize.y;
    
    // If any of the above checks had a correction, update the window size and position accordingly
    if (newpos.x != pos.x || newpos.y != pos.y) ImGui::SetWindowPos(newpos);
    if (newsize.x != size.x || newsize.y != size.y) ImGui::SetWindowSize(newsize);
        
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
    ImGui::Text("Available Materials");
    if (ImGui::ListBox("##loaded-materials", &materialListIndex, &materialListContent[0], (int)materialListContent.size()))
    {
      this->selectMaterial(kit::Material::load(materialListContent[materialListIndex]));
    }
    ImGui::PopItemWidth();
    
    if (currMaterialUnlock)
    {
      bool materialChanged = false;
      if (ImGui::Button("Save##savematerial"))
      {
        ImGui::OpenPopup("Save material##savematerialpopup");
      }
      if (ImGui::BeginPopupModal("Save material##savematerialpopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
      {
        if (currMaterialUnlock->save(materialListContent[materialListIndex]))
        {
          ImGui::Text("Material was successfully saved!");
        }
        else
        {
          ImGui::Text("ERROR: Could not save material to file!");
        }

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }

      ImGui::SameLine();

      if (ImGui::Button("Reset##resetmaterial"))
      {
        ImGui::OpenPopup("Are you sure?##resetsurepopup");
      }

      bool openWasReset = false;
      if (ImGui::BeginPopupModal("Are you sure?##resetsurepopup"))
      {
        ImGui::Text(std::string(std::string("Reload ") + std::string(materialListContent[materialListIndex]) + std::string(" from disk?")).c_str());

        ImGui::Separator();

        if (ImGui::Button("Yes, reload!"))
        {
          this->m_selectedMaterial = kit::Material::load(materialListContent[materialListIndex], true);
          materialChanged = true;
          ImGui::CloseCurrentPopup();
          openWasReset = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("No."))
        {
          ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
      }

      if (openWasReset)
      {
        ImGui::OpenPopup("Material was reset##materialwasreset");
      }

      if (ImGui::BeginPopupModal("Material was reset##materialwasreset"))
      {
        ImGui::Text("Material was successfully reloaded from disk!");

        ImGui::Separator();

        if (ImGui::Button("OK"))
        {
          ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
      }

      ImGui::Separator();

      // Material properties below
      {
        //GEneral stuff 
        if (ImGui::CollapsingHeader("General##generalheader"))
        {
          bool dw = currMaterialUnlock->getDepthWrite();
          if (ImGui::Checkbox("Depthwrite##matdepthwrite", &dw))
          {
            currMaterialUnlock->setDepthWrite(dw);
          }

          bool dr = currMaterialUnlock->getDepthRead();
          if (ImGui::Checkbox("Depthread##matdepthread", &dr))
          {
            currMaterialUnlock->setDepthRead(dr);
          }

        }

        // Albedo
        if (ImGui::CollapsingHeader("Albedo##albedoheader"))
        {
          // Texture selection
          kit::Texture::Ptr currTexture = currMaterialUnlock->getAlbedoMap();
          kit::Texture::Ptr newTexture = selectTexture("albedotextureselect", currTexture);
          if (newTexture != currTexture)
          {
            currMaterialUnlock->setAlbedoMap(newTexture);
            materialChanged = true;
          }

          //ImGui::SameLine();

          // Albedo value
          ImGui::BeginGroup();
          {
            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 80.0f);
            glm::vec3 currAlbedo = currMaterialUnlock->getAlbedo();
            float currColor[3] = { currAlbedo.x, currAlbedo.y, currAlbedo.z };
            if (ImGui::ColorEdit3("Color##albedocolor", currColor))
            {
              currAlbedo = glm::vec3(currColor[0], currColor[1], currColor[2]);
              currMaterialUnlock->setAlbedo(currAlbedo);
              materialChanged = true;
            }
            ImGui::PopItemWidth();
          }
          ImGui::EndGroup();
        }

        // Ambient Occlusion
        if (ImGui::CollapsingHeader("Ambient Occlusion##occlusionheader"))
        {
          bool hasMap = currMaterialUnlock->getOcclusionMap() != nullptr;
          // Texture selection
          kit::Texture::Ptr currTexture = currMaterialUnlock->getOcclusionMap();
          kit::Texture::Ptr newTexture = selectTexture("occlusiontextureselect", currTexture, false);
          if (newTexture != currTexture)
          {
            currMaterialUnlock->setOcclusionMap(newTexture);
            materialChanged = true;
          }

          //ImGui::SameLine();


          ImGui::BeginGroup();
          ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 80.0f);
          if (hasMap)
          {
            // Occlusion value
            float currOcclusion = currMaterialUnlock->getOcclusionGamma();
            if (ImGui::SliderFloat("Gamma##occlusiongamma", &currOcclusion, 0.0f, 10.0f, "%.3f", 2.0f))
            {
              currMaterialUnlock->setOcclusionGamma(currOcclusion);
              materialChanged = true;
            }

            // Input
            glm::vec2 currOcclusionInput = currMaterialUnlock->getOcclusionInput();
            if (ImGui::SliderFloat("Input Min.##occlusioninputmin", &currOcclusionInput.x, 0.0f, currOcclusionInput.y - 0.001f))
            {
              currMaterialUnlock->setOcclusionInput(currOcclusionInput);
              materialChanged = true;
            }

            if (ImGui::SliderFloat("Input Max.##occlusioninputmax", &currOcclusionInput.y, currOcclusionInput.x + 0.001f, 1.0f))
            {
              currMaterialUnlock->setOcclusionInput(currOcclusionInput);
              materialChanged = true;
            }

            // Output
            glm::vec2 currOcclusionOutput = currMaterialUnlock->getOcclusionOutput();
            if (ImGui::SliderFloat("Output Min.##occlusionoutputmin", &currOcclusionOutput.x, 0.0f, currOcclusionOutput.y - 0.001f))
            {
              currMaterialUnlock->setOcclusionOutput(currOcclusionOutput);
              materialChanged = true;
            }

            if (ImGui::SliderFloat("Output Max.##occlusionoutputmax", &currOcclusionOutput.y, currOcclusionOutput.x + 0.001f, 1.0f))
            {
              currMaterialUnlock->setOcclusionOutput(currOcclusionOutput);
              materialChanged = true;
            }
          }
          ImGui::EndGroup();
          ImGui::PopItemWidth();
        }

        // Emissive
        if (ImGui::CollapsingHeader("Emissive##emissiveheader"))
        {
          // Texture selection
          kit::Texture::Ptr currTexture = currMaterialUnlock->getEmissiveMap();
          kit::Texture::Ptr newTexture = selectTexture("emissivetextureselect", currTexture);
          if (newTexture != currTexture)
          {
            currMaterialUnlock->setEmissiveMap(newTexture);
            materialChanged = true;
          }

          //ImGui::SameLine();

          // Emissive values
          ImGui::BeginGroup();
          {
            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 80.0f);

            glm::vec3 currEmissive = currMaterialUnlock->getEmissiveColor();
            float currColor[3] = { currEmissive.x, currEmissive.y, currEmissive.z };
            if (ImGui::ColorEdit3("Color##coloremissive", currColor))
            {
              currEmissive = glm::vec3(currColor[0], currColor[1], currColor[2]);
              currMaterialUnlock->setEmissiveColor(currEmissive);
              materialChanged = true;
            }

            float currStrength = currMaterialUnlock->getEmissiveStrength();
            if (ImGui::SliderFloat("Strength##emissivestrength", &currStrength, 0.0f, 100.0f, "%.3f", 10.0f))
            {
              currMaterialUnlock->setEmissiveStrength(currStrength);
              materialChanged = true;
            }

            ImGui::PopItemWidth();
          }
          ImGui::EndGroup();
        }

        // Normal
        if (ImGui::CollapsingHeader("Normal##normalheader"))
        {
          // Texture selection
          kit::Texture::Ptr currTexture = currMaterialUnlock->getNormalMap();
          kit::Texture::Ptr newTexture = selectTexture("normaltextureselect", currTexture, false);
          if (newTexture != currTexture)
          {
            currMaterialUnlock->setNormalMap(newTexture);
            materialChanged = true;
          }

          if (currTexture)
          {
            //ImGui::SameLine();

            // Normal values
            ImGui::BeginGroup();
            {
              ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 80.0f);
              float currStrength = currMaterialUnlock->getNormalStrength();
              if (ImGui::SliderFloat("Strength##normalstrength", &currStrength, -2.0, 2.0))
              {
                currMaterialUnlock->setNormalStrength(currStrength);
                materialChanged = true;
              }
              ImGui::PopItemWidth();
            }
            ImGui::EndGroup();
          }
        }

        // Roughness
        if (ImGui::CollapsingHeader("Roughness##roughnessheader"))
        {
          bool hasMap = currMaterialUnlock->getRoughnessMap() != nullptr;

          // Texture selection
          kit::Texture::Ptr currTexture = currMaterialUnlock->getRoughnessMap();
          kit::Texture::Ptr newTexture = selectTexture("roughnesstextureselect", currTexture, false);
          if (newTexture != currTexture)
          {
            currMaterialUnlock->setRoughnessMap(newTexture);
            materialChanged = true;
          }

          //ImGui::SameLine();

          ImGui::BeginGroup();
          ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 80.0f);
          // Roughness value
          float currRoughness = currMaterialUnlock->getRoughness();
          if (ImGui::SliderFloat((hasMap ? "Gamma##roughness" : "Roughness##roughness"), &currRoughness, 0.0f, (hasMap ? 10.0f : 1.0f), "%.3f", (hasMap ? 2.0f : 1.0f)))
          {
            currMaterialUnlock->setRoughness(currRoughness);
            materialChanged = true;
          }

          if (hasMap)
          {
            // Input
            glm::vec2 currRoughnessInput = currMaterialUnlock->getRoughnessInput();
            if (ImGui::SliderFloat("Input Min.##roughnessinputmin", &currRoughnessInput.x, 0.0f, currRoughnessInput.y - 0.001f))
            {
              currMaterialUnlock->setRoughnessInput(currRoughnessInput);
              materialChanged = true;
            }

            if (ImGui::SliderFloat("Input Max.##roughnessinputmax", &currRoughnessInput.y, currRoughnessInput.x + 0.001f, 1.0f))
            {
              currMaterialUnlock->setRoughnessInput(currRoughnessInput);
              materialChanged = true;
            }

            // Output
            glm::vec2 currRoughnessOutput = currMaterialUnlock->getRoughnessOutput();
            if (ImGui::SliderFloat("Output Min.##roughnessoutputmin", &currRoughnessOutput.x, 0.0f, currRoughnessOutput.y - 0.001f))
            {
              currMaterialUnlock->setRoughnessOutput(currRoughnessOutput);
              materialChanged = true;
            }

            if (ImGui::SliderFloat("Output Max.##roughnessoutputmax", &currRoughnessOutput.y, currRoughnessOutput.x + 0.001f, 1.0f))
            {
              currMaterialUnlock->setRoughnessOutput(currRoughnessOutput);
              materialChanged = true;
            }
          }
          ImGui::EndGroup();
          ImGui::PopItemWidth();
        }

        // Metalness
        if (ImGui::CollapsingHeader("Metalness##metalnessheader"))
        {
          bool hasMap = currMaterialUnlock->getMetalnessMap() != nullptr;

          // Texture selection
          kit::Texture::Ptr currTexture = currMaterialUnlock->getMetalnessMap();
          kit::Texture::Ptr newTexture = selectTexture("metalnesstextureselect", currTexture, false);
          if (newTexture != currTexture)
          {
            currMaterialUnlock->setMetalnessMap(newTexture);
            materialChanged = true;
          }

          //ImGui::SameLine();

          ImGui::BeginGroup();
          ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 80.0f);
          // Metalness
          float currMetalness = currMaterialUnlock->getMetalness();
          if (ImGui::SliderFloat((hasMap ? "Gamma##metalness" : "Metalness##metalness"), &currMetalness, 0.0f, (hasMap ? 10.0f : 1.0f), "%.3f", (hasMap ? 2.0f : 1.0f)))
          {
            currMaterialUnlock->setMetalness(currMetalness);
            materialChanged = true;
          }

          if (hasMap)
          {
            // Input
            glm::vec2 currMetalnessInput = currMaterialUnlock->getMetalnessInput();
            if (ImGui::SliderFloat("Input Min.##metalnessinputmin", &currMetalnessInput.x, 0.0f, currMetalnessInput.y - 0.001f))
            {
              currMaterialUnlock->setMetalnessInput(currMetalnessInput);
              materialChanged = true;
            }

            if (ImGui::SliderFloat("Input Max.##metalnessinputmax", &currMetalnessInput.y, currMetalnessInput.x + 0.001f, 1.0f))
            {
              currMaterialUnlock->setMetalnessInput(currMetalnessInput);
              materialChanged = true;
            }

            // Output
            glm::vec2 currMetalnessOutput = currMaterialUnlock->getMetalnessOutput();
            if (ImGui::SliderFloat("Output Min.##metalnessoutputmin", &currMetalnessOutput.x, 0.0f, currMetalnessOutput.y - 0.001f))
            {
              currMaterialUnlock->setMetalnessOutput(currMetalnessOutput);
              materialChanged = true;
            }

            if (ImGui::SliderFloat("Output Max.##metalnessoutputmax", &currMetalnessOutput.y, currMetalnessOutput.x + 0.001f, 1.0f))
            {
              currMaterialUnlock->setMetalnessOutput(currMetalnessOutput);
              materialChanged = true;
            }
          }
          ImGui::EndGroup();
          ImGui::PopItemWidth();
        }
      }

      // Update material if it changed, otherwise we will get flickering
      if (materialChanged)
      {
        currMaterialUnlock->assertCache();
      }
    }
  }
  ImGui::End();

}

void kmd::MaterialDesignerState::generateMaterialThumbs()
{
  std::cout << "Generathing thumbs!" << std::endl;
  auto materialFiles = kit::listFilesystemEntries("./data/materials", true, false);
  auto thumbsFiles = kit::listFilesystemEntries("./data/materials/thumbs", true, false);
  
  kit::Renderer::Ptr thumbRenderer = kit::Renderer::create(glm::uvec2(256, 256));
  thumbRenderer->setFXAA(false);
  
  kit::Renderer::Payload::Ptr renderPayload = kit::Renderer::Payload::create();
  thumbRenderer->registerPayload(renderPayload);
  
  auto envLight = kit::Light::create(kit::Light::IBL);
  envLight->setIrradianceMap(kit::Cubemap::loadIrradianceMap("meadow"));
  envLight->setRadianceMap(kit::Cubemap::loadRadianceMap("meadow"));
  renderPayload->addLight(envLight);
  
  auto sunLight = kit::Light::create(kit::Light::Directional);
  sunLight->setEuler(glm::vec3(-65.0f, -65.0f, 0.0f));
  sunLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
  renderPayload->addLight(sunLight);
  
  auto spotLight = kit::Light::create(kit::Light::Spot);
  spotLight->setRadius(20.0f);
  spotLight->setConeAngle(45.0f, 65.0f);
  spotLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f) * 100.0f);
  spotLight->setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
  renderPayload->addLight(spotLight);
  
  kit::Skybox::Ptr skybox = kit::Skybox::create();
  skybox->setColor(glm::vec3(0.8f, 0.9f, 1.0f));
  skybox->setStrength(0.5f);
  thumbRenderer->setSkybox(skybox);
 
  const float cameraDistance = 2.5f;
  kit::Camera::Ptr camera = kit::Camera::create(60.0f, 1.0, glm::vec2(0.1f, 1000.0f));
  camera->setWhitepoint(1.0f);
  camera->setEuler(glm::vec3(-20.0f, 0.0f, 0.0f));
  camera->setPosition(camera->getForward() * -cameraDistance);

  thumbRenderer->setActiveCamera(camera);
  
  for(kit::FileInfo & currMaterial : materialFiles)
  {
    std::string materialName = currMaterial.filename.substr(0, currMaterial.filename.size()-9);
    std::string thumbName = materialName + ".tga";
   
    std::cout << "Generating thumb for " << materialName << std::endl;
    
    bool found = false;
    for(kit::FileInfo & currThumb : thumbsFiles)
    {
      if(currThumb.filename == thumbName)
      {
        found = true;
        break;
      }
    }
    
    if(!found)
    {
      kit::Material::Ptr currMaterial = kit::Material::load(materialName + ".material");
      kit::Mesh::Ptr sphereMesh = kit::Mesh::create();
      sphereMesh->addSubmeshEntry("0", kit::Submesh::load("Sphere.geometry"), currMaterial);
      kit::Model::Ptr sphere = kit::Model::create(sphereMesh);
      sphere->setEuler(glm::vec3(90.0f, 0.0f, 0.0f));
      
      renderPayload->addRenderable(sphere);
      thumbRenderer->renderFrame();
      thumbRenderer->getBuffer()->saveToFile("./data/materials/thumbs/" + thumbName);
      renderPayload->removeRenderable(sphere);
      
      kit::Material::clearCache();
      kit::Texture::flushCache();
    }
  }
}
