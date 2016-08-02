#ifndef KIT_PIXELSHADER_HEADER
#define KIT_PIXELSHADER_HEADER

#include "Kit/Export.hpp"
#include "Kit/GL.hpp"

#include <string>
#include <memory>

namespace kit{

  class KITAPI PixelShader{
    public:
      typedef std::shared_ptr<kit::PixelShader> Ptr;

      ~PixelShader();

      static kit::PixelShader::Ptr create();

      bool sourceFromFile(std::string filename);
      void sourceFromString(std::string s);
      void clearSource();

      bool compile();

      GLuint getHandle();

      PixelShader();
    
    private:
      kit::GL m_glSingleton;
      GLuint      m_glHandle;
      std::string m_source;
  };

}

#endif // KIT_PIXELSHADER_HEADER
