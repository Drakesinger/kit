#pragma once

#include "Kit/Types.hpp"

#include <map>

namespace kit
{

  class Cubemap;
  

  class Camera;
  

  class Program;
  

  class Renderer;
  

  class KITAPI Skybox
  {
    public:
      
      Skybox(std::string environment);
      Skybox(glm::vec3 color, float strength = 1.0f);
      ~Skybox();

      void render(kit::Renderer * renderer);
      void renderReflection(kit::Renderer * renderer, glm::mat4 const & rotationMatrix);
      void renderGeometry();
      
      kit::Cubemap * getTexture();

      void setStrength(float);
      void setColor(glm::vec3 color);

      glm::vec3 getColor();
      float getStrength();

    private:

      
      static void allocateShared();
      static void releaseShared();
      static uint32_t m_instanceCount;
      static kit::Program* m_program;
      static kit::Program* m_programNoTexture;

      static uint32_t m_glVertexArray;
      static uint32_t m_glVertexIndices;
      static uint32_t m_glVertexBuffer;
      
      glm::vec3        m_color;
      float             m_strength;
      kit::Cubemap* m_texture = nullptr;
  };

}
