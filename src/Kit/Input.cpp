#include "Kit/Input.hpp"
#include <GLFW/glfw3.h>

std::array<kit::Controller, 16> kit::Controller::m_controllerCache;

kit::Controller::Controller()
{
  this->m_id = 0;
  this->m_name = "__invalid__";
  this->m_connected = false;
}

std::array<kit::Controller, 16> & kit::Controller::getControllers()
{
  return kit::Controller::m_controllerCache;
}

void kit::Controller::refreshControllers()
{
  kit::GLFWSingleton glfwSingleton;

  // For each possible controller
  for (uint32_t i = 0; i < 16; i++)
  {
    kit::Controller & currController = kit::Controller::m_controllerCache[i];
    currController.m_id = i;

    currController.m_axisStates.clear();
    currController.m_buttonStates.clear();

    // If controller is connected
    if (currController.m_connected = (glfwJoystickPresent(i) == GL_TRUE))
    {
      // Refresh name
      const char * currName = glfwGetJoystickName(i);
      if (currName != nullptr)
      {
        currController.m_name = currName;
      }


      // Refresh axes and buttons
      int32_t axisCount;
      const float * axes = glfwGetJoystickAxes(i, &axisCount);
      if (axes != nullptr)
      {
        currController.m_axisStates.resize(axisCount, 0.0f);
        for (int a = 0; a < axisCount; a++)
        {
          currController.m_axisStates[a] = axes[a];
        }
      }

      int32_t buttonCount;
      const unsigned char * buttons = glfwGetJoystickButtons(i, &buttonCount);
      if (buttons != nullptr)
      {
        currController.m_buttonStates.resize(buttonCount, false);
        for (int b = 0; b < buttonCount; b++)
        {
          currController.m_buttonStates[b] = buttons[b]; // TODO: Figure this out
        }
      }

      std::cout << "Found controller \"" << currName << "\" with " << buttonCount << " buttons and " << axisCount << " axes." << std::endl;

    }
    
  }
}

void kit::Controller::refreshStates()
{
  // If controller is connected
  if (this->m_connected = (glfwJoystickPresent(this->m_id) == GL_TRUE))
  {
    // Refresh axes and buttons
    int32_t axisCount;
    const float * axes = glfwGetJoystickAxes(this->m_id, &axisCount);
    if (axes != nullptr)
    {
      this->m_axisStates.resize(axisCount, 0.0f);
      for (int a = 0; a < axisCount; a++)
      {
        this->m_axisStates[a] = axes[a];
      }
    }

    int32_t buttonCount;
    const unsigned char * buttons = glfwGetJoystickButtons(this->m_id, &buttonCount);
    if (buttons != nullptr)
    {
      this->m_buttonStates.resize(buttonCount, false);
      for (int b = 0; b < buttonCount; b++)
      {
        this->m_buttonStates[b] = buttons[b]; // TODO
      }
    }
  }
}

bool kit::Controller::isConnected()
{
  return this->m_connected;
}

uint32_t const & kit::Controller::getId()
{
  return this->m_id;
}

std::string const & kit::Controller::getName()
{
  return this->m_name;
}

bool kit::Controller::isButtonDown(uint32_t button_id)
{
  return this->m_buttonStates[button_id];
}

float kit::Controller::getAxis(uint32_t axis_id)
{
  if (axis_id >= this->m_axisStates.size())
  {
    return 0.0f;
  }

  return this->m_axisStates.at(axis_id);
}

uint32_t kit::Controller::getNumAxis()
{
  return this->m_axisStates.size();
}