#pragma once

#include "Kit/Renderable.hpp"
#include "Kit/Types.hpp"

#include <memory>

namespace kit 
{
  class DoubleBuffer;
  
  class Camera;

  class Program;
  

  class Texture;
  

  class Cubemap;
  
  class Quad;

  class Renderer;
  

  class Water : public kit::Renderable
  {
    public:

      Water();
      virtual ~Water() override;

      virtual void renderForward(kit::Renderer*) override;

      virtual void update(double const & ms);

      virtual int32_t getRenderPriority() override;
      virtual bool requestAccumulationCopy() override;
      virtual bool requestPositionBuffer() override;

      virtual void setSunDirection(glm::vec3);
      virtual void setSunColor(glm::vec3);

    private:


      uint32_t m_glVertexBuffer = 0; // VBO to hold vertex data
      uint32_t m_glIndexBuffer = 0; // VBO to hold index data
      uint32_t m_glVao = 0; // VAO for our VBO and program
      
      // Program for surface
      kit::Program * m_program  = nullptr;
      
      // Program for surface, from below
      kit::Program * m_belowProgram  = nullptr;

      std::shared_ptr<kit::Texture> m_heightmapA = nullptr;
      std::shared_ptr<kit::Texture> m_normalmapA = nullptr;

      std::shared_ptr<kit::Texture> m_heightmapB = nullptr;
      std::shared_ptr<kit::Texture> m_normalmapB = nullptr;
      
      // Program for fullscreen effects when below water
      kit::Program * m_underwaterProgram = nullptr;
      kit::Quad * m_underwaterQuad = nullptr;
      std::shared_ptr<kit::Texture> m_vignette = nullptr;
      
      glm::vec3 m_sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
      glm::vec3 m_sunDirection = glm::vec3(1.0f, 1.0f, 1.0f);

      uint32_t m_indexCount = 0;
  };
}

