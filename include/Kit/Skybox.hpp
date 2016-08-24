#ifndef KIT_SKYBOX_HPP
#define KIT_SKYBOX_HPP

#include "Kit/Types.hpp"


#include <memory>
#include <map>

namespace kit
{

  class Cubemap;
  typedef std::shared_ptr<Cubemap> CubemapPtr;

  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class Renderer;
  typedef std::shared_ptr<Renderer> RendererPtr;

  class KITAPI Skybox
  {
    public:
      
      typedef std::shared_ptr<Skybox> Ptr;
      
      ~Skybox();

      void render(kit::RendererPtr renderer);
      void renderGeometry();
      static kit::Skybox::Ptr create( kit::CubemapPtr cubemaptexture = nullptr );
      
      kit::CubemapPtr getTexture(); 
      void setTexture(kit::CubemapPtr cubemaptexture);
      Skybox(kit::CubemapPtr cubemaptexture);

      void setStrength(float);
      void setColor(glm::vec3 color);

      glm::vec3 getColor();
      float getStrength();

    private:

      
      static void allocateShared();
      static void releaseShared();
      static uint32_t m_instanceCount;
      static kit::ProgramPtr m_program;
      static kit::ProgramPtr m_programNoTexture;

      static uint32_t m_glVertexArray;
      static uint32_t m_glVertexIndices;
      static uint32_t m_glVertexBuffer;
      
      glm::vec3        m_color;
      float             m_strength;
      kit::CubemapPtr m_texture;
  };

}

#endif