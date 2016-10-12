/*
 * This example shows some simple renderer usage.
 * 
 * It will open a window, and render a spinning sphere in the middle of the screen.
 * It will exit if the window is closed or if the user presses escape.
 * 
 * Usually you would inherit your application code from the kit::Application
 * and kit::ApplicationState classes but this is fine for an example!
 * 
 */

#include <Kit/Window.hpp>
#include <Kit/Renderer.hpp>
#include <Kit/Quad.hpp>

#include <Kit/Light.hpp>  // Lights..
#include <Kit/Camera.hpp> // Camera..
#include <Kit/Model.hpp>  // Action!
#include <Kit/Skybox.hpp>

int main(int argc, char **argv)
{
  // Create a window
  kit::Window::Args winArgs;
  winArgs.mode = kit::Window::Windowed;
  winArgs.resizable = true;
  winArgs.title = std::string("Sphere example");
  winArgs.resolution = glm::uvec2(1280, 720);

  auto win = kit::Window::create(winArgs);

  // Create our renderer, set it's resolution to fit the windows resolution
  auto renderer = kit::Renderer::create(glm::uvec2(1280, 720));

  // Create a render payload, anything we add to this will get rendered in our renderer
  auto payload = kit::RenderPayload::create();
  renderer->registerPayload(payload);

  // Create a fullscreen quad, so we can blit what the renderer produce to the screen
  auto screenQuad = kit::Quad::create();

  // Create a camera. fov of 72 degrees, aspect ratio according to resolution, and a short cliprange
  // Position the camera 2 meters backwards (Z- is forward)
  auto camera = kit::Camera::create(72.0f, 1280.0f / 720.0f, glm::vec2(0.1f, 100.0f));
  camera->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));

  // Create an environment light so that we can see what we render (IBL = Imagebased light)
  auto light = kit::Light::create(kit::Light::IBL);
  light->setEnvironment("fortpoint");
  light->setColor(glm::vec3(1.0f, 1.0f, 1.0f) * 3.0f);
  payload->addLight(light);

  // Create a sphere model
  auto sphere = kit::Model::create("Sphere.mesh");
  sphere->rotateX(90.0f); // Rotate it 90 degrees (the sphere model has an ugly UV seam)
  payload->addRenderable(sphere); // Dont forget to add the sphere to the payload

  // Create a discrete skybox, base it on the environment light
  auto skybox = kit::Skybox::create(light->getIrradianceMap());
  skybox->setStrength(0.5f);
  renderer->setSkybox(skybox);

  // Set some parameters for our renderer
  renderer->setActiveCamera(camera);                               // <- The active camera, so it knows what camera to use
  renderer->setSceneFringe(true);                                  // <- Enable scene fringe
  renderer->setColorCorrection(true);                              // <- Enable color correction 
  renderer->loadCCLookupTable("example.tga");                      // <- Load LUT for the cc
  renderer->setBloomDirtMask(kit::Texture::load("lensdirt.tga"));  // <- Set a dirtmask for our bloom
  renderer->setBloomDirtMaskMultiplier(3.0f);

  // Create a timer to keep track of frametimes, and some variables for our sphere spinning
  kit::Timer timer;
  double rotationSpeed = 0.005;

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
    }

    // Get the current frametime and rotate our sphere based on it
    double frametime = timer.restart().asMilliseconds();
    sphere->rotateZ(frametime * rotationSpeed);

    // Produce one frame using our renderer
    renderer->renderFrame();

    // Set our screenQuad texture to the freshly rendered frame
    screenQuad->setTexture(renderer->getBuffer());

    // Clear the window
    win->clear();

    // Render our screenquad (with the rendered scene)
    screenQuad->render();

    // Flip the buffers so that what we just rendered is visible on the window
    win->display();
  }
}
