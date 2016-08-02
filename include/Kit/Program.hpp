#ifndef KIT_PROGRAM_HEADER
#define KIT_PROGRAM_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"

#include <map>
#include <memory>
#include <glm/glm.hpp>

namespace kit {

  class Cubemap;
  typedef std::weak_ptr<Cubemap> CubemapWPtr;

  class Texture;
  typedef std::weak_ptr<Texture> TextureWPtr;

  class KITAPI Program {
    public:
      typedef std::shared_ptr<kit::Program> Ptr;
  
      ~Program();

      static kit::Program::Ptr create();
      static kit::Program::Ptr load(std::vector<std::string> vertexSources, std::vector<std::string> fragmentSources);
    
      template <typename T>
      void attachShader(T & s) {
        kit::GL::attachShader(this->m_glHandle, s->getHandle());
        this->m_fileIdentifier += "dynamic;";
      }
    
      template <typename T>
      void detachShader(T & s) {
        kit::GL::detachShader(this->m_glHandle, s->getHandle());
      }
    
      bool link();

      void use();
      static void useFixed();

      GLuint          getHandle();

      static int32_t getMaxTextureUnits();

      void setUniformTexture(const std::string& name, kit::TextureWPtr texture);
      void setUniformCubemap(const std::string& name, kit::CubemapWPtr cubemap);

      void setUniform1f(const std::string& name, float value);
      void setUniform1d(const std::string& name, double value);
      void setUniform1i(const std::string& name, int value);
      void setUniform1ui(const std::string& name, uint32_t value);

      void setUniform2f(const std::string& name, const glm::vec2& vec);

      void setUniform3f(const std::string& name, const glm::vec3& vec);

      void setUniform3fv(const std::string& name, const std::vector<glm::vec3>& vectors);

      void setUniform4f(const std::string& name, const glm::vec4& vec);

      void setUniformMat3(const std::string& name, const glm::mat3& matrix);
      void setUniformMat4(const std::string& name, const glm::mat4& matrix);
      void setUniformMat4v(const std::string& name, const std::vector<glm::mat4>& matrices);

      uint32_t getUniformLocation(const std::string& name);
      void prepareTextures();

      Program();

    private:
    
      std::string m_fileIdentifier;
      kit::GL m_glSingleton;
      GLuint			m_glHandle;
      std::map<std::string, uint32_t>      m_locationCache;
      std::map<uint32_t, kit::TextureWPtr>   m_textures;
      std::map<uint32_t, kit::CubemapWPtr>   m_cubemaps;
  };

}

#endif // KIT_PROGRAM_HEADER
