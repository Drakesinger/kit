/*
 * This example shows some simple window usage.
 * 
 * It will open a window, and render a text in the middle of the screen.
 * It will exit if the window is closed or if the user presses escape.
 * 
 */

#include <Kit/Window.hpp>
#include <Kit/Text.hpp>
#include <Kit/Font.hpp>

int main(int argc, char *argv[])
{
  // Create a window
  auto win = kit::Window::create("Foo", kit::Window::Windowed, glm::uvec2(1280, 720));

  // Create our text
  auto txt = kit::Text::create(kit::Font::getSystemFont(), 14.0f, L"Hello, world!");

  // Place our text in the exact middle of the screen
  txt->setPosition(glm::vec2(1280.0f / 2.0f, 720.0f / 2.0f));
  txt->setAlignment(kit::Text::Centered, kit::Text::Bottom);

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
    }

    // Clear the window
    win->clear();

    // Render the text. We need to pass the screen resolution so that it can keep track of its position
    txt->render(glm::ivec2(1280, 720));

    // Flip the buffers so that what we just rendered is visible on the window
    win->display();
  }
}