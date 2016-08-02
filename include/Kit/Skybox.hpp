#ifndef KIT_SKYBOX_HPP
#define KIT_SKYBOX_HPP

#include "Kit/Types.hpp"
#include "Kit/GL.hpp"

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

      kit::GL m_glSingleton;
      
      static void allocateShared();
      static void releaseShared();
      static uint32_t m_instanceCount;
      static kit::ProgramPtr m_program;
      static kit::ProgramPtr m_programNoTexture;

      static GLuint m_glVertexArray;
      static GLuint m_glVertexIndices;
      static GLuint m_glVertexBuffer;
      
      glm::vec3        m_color;
      float             m_strength;
      kit::CubemapPtr m_texture;
  };

}

#endif