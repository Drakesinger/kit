#ifndef KIT_CUBEMAP_HEADER
#define KIT_CUBEMAP_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"

#include <memory>
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

      typedef std::shared_ptr<Cubemap> Ptr;
      typedef std::weak_ptr<Cubemap> WPtr;
      
			~Cubemap();
      
      static kit::Cubemap::Ptr load(const std::string& zpos, const std::string& zneg, const std::string& xpos, const std::string& xneg, const std::string& ypos, const std::string& yneg);
			
      static kit::Cubemap::Ptr loadIrradianceMap(const std::string& name);
      static kit::Cubemap::Ptr loadRadianceMap(const std::string& name);
      static kit::Cubemap::Ptr loadSkybox(const std::string& name);
      
      static kit::Cubemap::Ptr createDepthmap(glm::uvec2 resolution);
      
			void bind();
      static void unbind();

			kit::Cubemap::FilteringMode getFilteringMode();
			void setFilteringMode(kit::Cubemap::FilteringMode mode);
      
      kit::Cubemap::EdgeSamplingMode getEdgeSamplingMode();
      void setEdgeSamplingMode(kit::Cubemap::EdgeSamplingMode mode);
      
			/*float GetAnisotropicLevel();
			void SetAnisotropicLevel(float level);
      static float GetMaxAnisotropicLevel();*/
      
			GLuint getHandle();

      Cubemap();
      Cubemap(GLuint handle);
		private:
      kit::GL m_glSingleton;
      
			GLuint	                       m_glHandle;
			kit::Cubemap::FilteringMode    m_filteringMode;
      kit::Cubemap::EdgeSamplingMode m_edgeSamplingMode;
      glm::uvec2                   m_resolution;
      //float                          m_AnisotropicLevel;
	};

}

#endif // KIT_CUBEMAP_HEADER
