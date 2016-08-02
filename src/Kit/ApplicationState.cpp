#include "Kit/ApplicationState.hpp"

kit::ApplicationState::~ApplicationState()
{
  
}

void kit::ApplicationState::registerApplication(kit::Application* appref)
{
  this->m_application = appref;
}

void kit::ApplicationState::onConsoleActive()
{

}

void kit::ApplicationState::onConsoleInactive()
{

}

kit::Application* kit::ApplicationState::getApplication()
{
  return this->m_application;
}