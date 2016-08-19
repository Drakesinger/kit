#ifndef KMD_APPLICATION_HPP
#define KMD_APPLICATION_HPP

#include <Kit/Application.hpp>
#include <Kit/Renderer.hpp>

#include <memory>

namespace kit
{
  class Quad;
  typedef std::shared_ptr<Quad> QuadPtr;
  class Renderer;
  typedef std::shared_ptr<Renderer> RendererPtr;
}

namespace kwe
{
  class Application : public kit::Application
  {
    public:
      Application();
      ~Application();
            
      kit::RendererPtr getRenderer();
      
      virtual void onResize(glm::uvec2 newsize);
      virtual void onInitialize();
      virtual void onRender();
      
    private:
      kit::RendererPtr m_renderer;
      kit::QuadPtr m_fullscreenQuad;
  };
}

#endif