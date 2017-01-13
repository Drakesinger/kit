/* This example shows rather advanced dynamic materials 
 * Please read and understand the Renderer example first!
 * Also, this whole file is pretty messy, it needs to be cleaned up
 * and split into different classes.
 * 
 */

#include <Kit/Window.hpp>
#include <Kit/Renderer.hpp>
#include <Kit/Quad.hpp>

#include <Kit/Light.hpp>  // Lights..
#include <Kit/Camera.hpp> // Camera..
#include <Kit/Model.hpp>  // Action!
#include <Kit/Skybox.hpp>
#include <Kit/Mesh.hpp>
#include <Kit/Material.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <Kit/Skeleton.hpp>

int main(int argc, char *argv[])
{
  // Create a window
  kit::Window::Args winArgs;
  winArgs.mode = kit::Window::Windowed;
  winArgs.resizable = true;
  winArgs.title = "Dynamic materials example";
  winArgs.resolution = glm::uvec2(1280, 720);

  auto win = new kit::Window(winArgs);
  win->setMouseVirtual(true);
  
  // Create our renderer, set it's resolution to fit the windows resolution
  auto renderer = new kit::Renderer(glm::uvec2(1280, 720));

  // Create a render payload, anything we add to this will get rendered in our renderer
  auto payload = new kit::RenderPayload();
  renderer->registerPayload(payload);

  // Create a fullscreen quad, so we can blit what the renderer produce to the screen
  auto screenQuad = new kit::Quad();

  
  // Create a camera. fov of 72 degrees, aspect ratio according to resolution, and a short cliprange
  // Position the camera 2 meters backwards (Z- is forward)
  auto camera = new kit::Camera(72.0f, 1280.0f / 720.0f, glm::vec2(0.1f, 100.0f));
  camera->setPosition(glm::vec3(0.0f, 0.0f, 7.0f));

  // Create an environment light so that we can see what we render (IBL = Imagebased light)
  auto light = new kit::Light(kit::Light::IBL);
  light->setEnvironment("fortpoint");
  light->setColor(glm::vec3(1.0f, 1.0f, 1.0f) * 3.0f);
  payload->addLight(light);

  // Create a "weapon" model
  auto scanner = new kit::Model("ScannerMesh.mesh", "ScannerMesh.skeleton");
  scanner->getSkeleton()->setAnimation("Scanner|Idle");
  scanner->getSkeleton()->play(true);
  payload->addRenderable(scanner);
  
  // Create a sphere array
  std::vector<glm::mat4> instances;
  instances.push_back(glm::mat4());
  
  for(int x = -4; x < 4; x++)
  {
    for(int y = -4; y < 4; y++)
    {
      instances.push_back(glm::translate(glm::mat4(), glm::vec3(float(x)*2.0f, 0.0f, float(y)*2.0f)));
    }
  }
  
  auto sphere = new kit::Model("Sphere.mesh");
  sphere->rotateX(90.0f); // Rotate it 90 degrees (the sphere model has an ugly UV seam)
  sphere->setInstancing(true, instances);
  payload->addRenderable(sphere); // Dont forget to add the sphere to the payload

  // Create a discrete skybox, base it on the environment light
  auto skybox = new kit::Skybox("fortpoint");
  skybox->setStrength(0.5f);
  renderer->setSkybox(skybox);

  // Set some parameters for our renderer
  renderer->setActiveCamera(camera);                               // <- The active camera, so it knows what camera to use
  renderer->setSceneFringe(true);                                  // <- Enable scene fringe
  renderer->setColorCorrection(true);                              // <- Enable color correction 
  renderer->setCCLookupTable("example.tga");                      // <- Load LUT for the cc
  renderer->setBloomDirtMask(kit::Texture::load("lensdirt.tga"));  // <- Set a dirtmask for our bloom
  renderer->setBloomDirtMaskMultiplier(3.0f);
  renderer->setGPUMetrics(false);

  // Create a timer to keep track of frametimes, and some variables for our sphere spinning
  kit::Timer timer;
  double rotationSpeed = 0.005;

  // Keep track of mouse movement
  glm::vec2 prevMousePosition;
  float horizontalAngle = 0.0f;
  float verticalAngle = 0.0f;
  
  // Create a smaller quad to debugrender our minicamera
  auto miniQuad = new kit::Quad(glm::vec2(0.0f, 0.0f), glm::vec2(0.2f * 0.80822, 0.2f));
  auto miniRenderer = new kit::Renderer(glm::uvec2(uint32_t(128.0f * 0.80822), 128));
  miniRenderer->registerPayload(payload);
  auto miniCamera = new kit::Camera(90.0f, 0.80822f, glm::vec2(0.1f, 100.0f));
  miniRenderer->setActiveCamera(miniCamera);
  miniRenderer->setSkybox(skybox);
  miniRenderer->setSRGBEnabled(false);
  miniRenderer->setBloom(false);
  miniRenderer->setFXAA(false);
  miniRenderer->setGPUMetrics(false);
  miniRenderer->setShadows(false);
  
  // Create a mainloop that will run while the window is open
  while(win->isOpen())
  {
    // An event is dispatched everytime something related to the window happens
    // On every frame, we need to fetch any pending window event and handle them
    kit::WindowEvent evt;
    while(win->fetchEvent(evt))
    {
      // If the user pressed escape, close the window
      if(evt.type == kit::WindowEvent::KeyPressed && evt.keyboard.key == kit::Escape)
      {
        win->close();
      }

      // If the framebuffer has been resized, we need to update the renderer and the camera
      if(evt.type == kit::WindowEvent::FramebufferResized)
      {
        if(evt.resized.newSize.x > 0 && evt.resized.newSize.y > 0)
        {
          renderer->setResolution(evt.resized.newSize);
          camera->setAspectRatio(float(evt.resized.newSize.x) / float(evt.resized.newSize.y));
        }
      }
      
      if(evt.type == kit::WindowEvent::MouseButtonPressed)
      {
        scanner->getSkeleton()->playAnimation("Scanner|Attack", [scanner]()->void{
          // What to run when animation finishes?
          scanner->getSkeleton()->setAnimation("Scanner|Idle");
          scanner->getSkeleton()->play(true);
        });
      }
    }

    // Get the current frametime
    double frametime = timer.restart().asMilliseconds();

    win->setMouseVirtual(win->isKeyDown(kit::LeftShift));

    
    // Update mouse movement
    glm::vec2 currMousePosition = win->getMousePosition();
    glm::vec2 mouseDelta = currMousePosition - prevMousePosition;
    prevMousePosition = currMousePosition;
    
    if(win->isKeyDown(kit::LeftShift))
    {
      horizontalAngle -= mouseDelta.x * 0.1f;
      verticalAngle -= mouseDelta.y * 0.1f;
      verticalAngle = glm::clamp(verticalAngle, -89.0f, 89.0f);
      
      glm::quat cameraRotation = glm::rotate(glm::quat(), glm::radians(horizontalAngle), glm::vec3(0.0f, 1.0f, 0.0f));
      cameraRotation = glm::rotate(cameraRotation, glm::radians(verticalAngle), glm::vec3(1.0f, 0.0f, 0.0f));
      
      camera->setRotation(cameraRotation);   
    }
    // Update keyboard movement
    if(win->isKeyDown(kit::W))
    {
      camera->translate(camera->getWorldForward() * float(frametime) * 0.001f);
    }
    
    if(win->isKeyDown(kit::S))
    {
      camera->translate(-camera->getWorldForward() * float(frametime) * 0.001f);
    }
    
    if(win->isKeyDown(kit::D))
    {
      camera->translate(camera->getWorldRight() * float(frametime) * 0.001f);
    }
    
    if(win->isKeyDown(kit::A))
    {
      camera->translate(-camera->getWorldRight() * float(frametime) * 0.001f);
    }
    
    
    // Update scanner animation 
    scanner->setPosition(camera->getWorldPosition());
    scanner->setRotation(camera->getWorldRotation());
    scanner->update(frametime);
    
    // Rotate the spheres in our array
    for(auto & currInstance : instances)
    {
      currInstance = glm::rotate(currInstance, glm::radians(float(frametime) * float(rotationSpeed)), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    sphere->setInstancing(true, instances);

    
    // Render to our mini renderer
    miniCamera->setPosition(scanner->getBoneWorldPosition("ScannerBone"));
    miniCamera->setRotation(scanner->getBoneWorldRotation("ScannerBone"));
    miniRenderer->renderFrame();
    miniQuad->setTexture(miniRenderer->getBuffer());
    scanner->getMesh()->getSubmeshEntry("submesh-1")->m_material->setDynamicEO(true);
    scanner->getMesh()->getSubmeshEntry("submesh-1")->m_material->setEmissiveMap(miniRenderer->getBuffer());
    scanner->getMesh()->getSubmeshEntry("submesh-1")->m_material->setEmissiveStrength(2.0f);
    scanner->getMesh()->getSubmeshEntry("submesh-1")->m_material->setEmissiveColor(glm::vec3(0.6f, 0.9f, 1.0f));
    
    
    // Produce one frame using our renderer
    renderer->renderFrame();

    // Set our screenQuad texture to the freshly rendered frame
    screenQuad->setTexture(renderer->getBuffer());

    // Clear the window
    win->clear();

    // Render our screenquad (with the rendered scene)
    screenQuad->render();
    miniQuad->render();
    
    // Flip the buffers so that what we just rendered is visible on the window
    win->display();
  }

  delete miniCamera;
  delete miniRenderer;
  delete miniQuad;
  
  delete skybox;
  delete sphere;
  delete scanner;
  delete light;
  delete camera;
  delete screenQuad;
  delete payload;
  delete renderer;
  delete win;
  
}
