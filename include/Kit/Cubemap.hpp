#pragma once

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <map>
#include <string>

namespace kit{

  class KITAPI Cubemap{
  public:
    
    enum FilteringMode
    {
      None,
      Bilinear,
      Trilinear
    };

    enum EdgeSamplingMode
    {
      Repeat,
      RepeatMirrored,
      Clamp,
      ClampMirrored
    };

    enum ComponentCount
    {
      RGB,
      RGBA
    };

    enum Side 
    {
      Begin = 0,
      PositiveX = 0,
      NegativeX = 1,
      PositiveY = 2,
      NegativeY = 3,
      PositiveZ = 4,
      NegativeZ = 5,
      End = 5,
      Count = 6
    };

    ~Cubemap();

    static kit::Cubemap * load(const std::string& zpos, const std::string& zneg, const std::string& xpos, const std::string& xneg, const std::string& ypos, const std::string& yneg);

    static kit::Cubemap * loadIrradianceMap(const std::string& name);
    static kit::Cubemap * loadRadianceMap(const std::string& name);
    static kit::Cubemap * loadSkybox(const std::string& name);

    static kit::Cubemap * createDepthmap(glm::uvec2 resolution);

    void bind();
    static void unbind();

    kit::Cubemap::FilteringMode getFilteringMode();
    void setFilteringMode(kit::Cubemap::FilteringMode mode);

    kit::Cubemap::EdgeSamplingMode getEdgeSamplingMode();
    void setEdgeSamplingMode(kit::Cubemap::EdgeSamplingMode mode);

    uint32_t getHandle();

    Cubemap();
    Cubemap(uint32_t handle);
    
  private:
    uint32_t	                       m_glHandle = 0;
    kit::Cubemap::FilteringMode    m_filteringMode = FilteringMode::Trilinear;
    kit::Cubemap::EdgeSamplingMode m_edgeSamplingMode = EdgeSamplingMode::Repeat;
    glm::uvec2                   m_resolution = glm::uvec2(0, 0);
  };

}
