#ifndef KIT_WATER_HPP
#define KIT_WATER_HPP

#include "Kit/Renderable.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"
#include <memory>

namespace kit 
{
  class Camera;
  typedef std::shared_ptr<Camera> CameraPtr;

  class Program;
  typedef std::shared_ptr<Program> ProgramPtr;

  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;

  class Cubemap;
  typedef std::shared_ptr<Cubemap> CubemapPtr;

  class Renderer;
  typedef std::shared_ptr<Renderer> RendererPtr;

  class Water : public kit::Renderable
  {
    public:
      typedef std::shared_ptr<Water> Ptr;

      static Ptr create(glm::uvec2 resolution);

      Water(glm::uvec2 resolution);
      virtual ~Water() override;

      virtual void renderForward(kit::RendererPtr) override;

      virtual void update(double const & ms);

      virtual int32_t getRenderPriority() override;
      virtual bool requestAccumulationCopy() override;

      virtual void setRadianceMap(kit::CubemapPtr);
      virtual void setEnvironmentStrength(glm::vec3);

      virtual void setSunDirection(glm::vec3);
      virtual void setSunColor(glm::vec3);

    private:


      GLuint m_glVertexBuffer; // VBO to hold vertex data
      GLuint m_glIndexBuffer; // VBO to hold index data
      GLuint m_glVao; // VAO for our VBO and program
      kit::ProgramPtr m_program;

      kit::TexturePtr m_brdf;

      kit::TexturePtr m_heightmapA;
      kit::TexturePtr m_normalmapA;

      kit::TexturePtr m_heightmapB;
      kit::TexturePtr m_normalmapB;

      kit::CubemapPtr m_radianceMap;
      glm::vec3 m_environmentStrength;

      glm::vec3 m_sunColor;
      glm::vec3 m_sunDirection;

      glm::uvec2 m_resolution;
      uint32_t m_indexCount;
  };
}

#endif
