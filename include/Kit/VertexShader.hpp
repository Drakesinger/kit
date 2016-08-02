#ifndef KIT_VERTEXSHADER_HEADER
#define KIT_VERTEXSHADER_HEADER

#include "Kit/Export.hpp"
#include "Kit/GL.hpp"

#include <string>
#include <memory>

namespace kit{

  class KITAPI  VertexShader{
    public:
      typedef std::shared_ptr<kit::VertexShader> Ptr;

      ~VertexShader();

      static kit::VertexShader::Ptr create();

      bool sourceFromFile(std::string filename);
      void sourceFromString(std::string s);
      void clearSource();

      bool compile();

      GLuint getHandle();
      VertexShader();
  
    private:
      kit::GL m_glSingleton;
      GLuint      m_glHandle;
      std::string m_source;
  };

}

#endif // KIT_VERTEXSHADER_HEADER
