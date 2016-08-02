#ifndef KIT_APPLICATIONSTATE_HPP
#define KIT_APPLICATIONSTATE_HPP

#include "Kit/WindowEvent.hpp"
#include <memory>

namespace kit 
{

  class Application;
  
  class KITAPI ApplicationState 
  {
  public:
      
      typedef std::shared_ptr<ApplicationState> Ptr;
      
      virtual ~ApplicationState()=0;
      
      virtual void allocate()=0;      //< Allocates resources just after this state has been added to the stack
      virtual void release()=0;       //< Releases resources just before this state is removed from the stack
      
      virtual void onInactive()=0;    //< Called when something is pushed on top of this state, inactivating it
      virtual void onActive()=0;      //< Called when everything on top of this state has been popped, activating it
      
      virtual void onConsoleInactive(); //< Called when the console is inactivated/hidden
      virtual void onConsoleActive();   //< Called when the console is activated/shown

      virtual void onResize(glm::uvec2 newsize)=0; 

      virtual void handleEvent(double const &ms, kit::WindowEvent const & evt)=0;
      virtual void update(double const & ms)=0;
      virtual void render()=0;
      
      void registerApplication(kit::Application* appref);
      
      kit::Application * getApplication();
  protected:
      kit::Application* m_application;
  };

}

#endif 