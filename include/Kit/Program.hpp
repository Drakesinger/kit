#ifndef KIT_PROGRAM_HEADER
#define KIT_PROGRAM_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"
#include "Kit/Shader.hpp"

#include <map>
#include <memory>
#include <glm/glm.hpp>

namespace kit {

  class Cubemap;
  typedef std::weak_ptr<Cubemap> CubemapWPtr;

  class Texture;
  typedef std::weak_ptr<Texture> TextureWPtr;
  
  ///
  /// \brief An OpenGL Program
  ///
  class KITAPI Program {
    public:
      typedef std::shared_ptr<kit::Program> Ptr;

      typedef std::vector<std::string > const & SourceList;
      
      ///
      /// \brief Constructor (FOR INTERNAL USE ONLY)
      /// 
      /// You should NEVER instance this as usual. ALWAYS use smart pointers (std::shared_ptr), and create them explicitly using the `create` methods!
      ///
      Program();

      ///
      /// \brief Destructor
      ///
      ~Program();

      /// 
      /// \brief Creates an empty proram
      ///
      /// You should only use the static `create` and `load` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \returns A shared pointer pointing to the newly created program
      ///
      static kit::Program::Ptr create();

      /// 
      /// \brief Creates, loads, compiles and links a program directly from lists of sourcefiles.
      ///
      /// You should only use the static `create` and `load` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param v A list of filepaths to compute sources, relative to ./data/shaders/
      ///
      /// \returns A shared pointer pointing to the newly created program
      ///
      static kit::Program::Ptr load(SourceList c, char const * dataDirectory = "./data/");
      
      /// 
      /// \brief Creates, loads, compiles and links a program directly from lists of sourcefiles.
      ///
      /// You should only use the static `create` and `load` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param v A list of filepaths to vertex sources, relative to ./data/shaders/
      /// \param f A list of filepaths to fragment sources, relative to ./data/shaders/
      ///
      /// \returns A shared pointer pointing to the newly created program
      ///
      static kit::Program::Ptr load(SourceList v, SourceList f, char const * dataDirectory = "./data/");
    
      /// 
      /// \brief Creates, loads, compiles and links a program directly from lists of sourcefiles.
      ///
      /// You should only use the static `create` and `load` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param v A list of filepaths to vertex sources, relative to ./data/shaders/
      /// \param g A list of filepaths to the geometry sources, relative to ./data/shaders/
      /// \param f A list of filepaths to fragment sources, relative to ./data/shaders/
      ///
      /// \returns A shared pointer pointing to the newly created program
      ///
      static kit::Program::Ptr load(SourceList v, SourceList g, SourceList f, char const * dataDirectory = "./data/");
      
      /// 
      /// \brief Creates, loads, compiles and links a program directly from lists of sourcefiles.
      ///
      /// You should only use the static `create` and `load` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param v A list of filepaths to vertex sources, relative to ./data/shaders/
      /// \param tc A list of filepaths to the tessellation control sources, relative to ./data/shaders/
      /// \param te A list of filepaths to the tesselation evaluation sources, relative to ./data/shaders/
      /// \param f A list of filepaths to fragment sources, relative to ./data/shaders/
      ///
      /// \returns A shared pointer pointing to the newly created program
      ///
      static kit::Program::Ptr load(SourceList v, SourceList tc, SourceList te, SourceList f, char const * dataDirectory = "./data/");
      
      
      /// 
      /// \brief Creates, loads, compiles and links a program directly from lists of sourcefiles.
      ///
      /// You should only use the static `create` and `load` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param v A list of filepaths to vertex sources, relative to ./data/shaders/
      /// \param tc A list of filepaths to the tessellation control sources, relative to ./data/shaders/
      /// \param te A list of filepaths to the tesselation evaluation sources, relative to ./data/shaders/
      /// \param g A list of filepaths to the geometry sources, relative to ./data/shaders/
      /// \param f A list of filepaths to fragment sources, relative to ./data/shaders/
      ///
      /// \returns A shared pointer pointing to the newly created program
      ///
      static kit::Program::Ptr load(SourceList v, SourceList tc, SourceList te, SourceList g, SourceList f, char const * dataDirectory = "./data/");
      
      ///
      /// \brief Attaches a compiled shader object to this program
      /// \param s The shader object to attach
      ///
      void attachShader(kit::Shader::Ptr s);
    
      ///
      /// \brief Detaches a compiled shader object from this program
      /// \param s The shader object to detach
      ///
      void detachShader(kit::Shader::Ptr s);
    
      ///
      /// \brief Attempts to link together the attached shaderobjects, making this program valid on success.
      /// \returns true on success, false on failure.
      ///
      bool link();

      ///
      /// \brief Tells OpenGL to use this program
      ///
      void use();
      
      ///
      /// \brief Tells OpenGL to unbind any program and use the fixed pipeline
      ///
      static void useFixed();

      ///
      /// \brief Retrieve the internal OpenGL name for this program object
      /// \returns The internal OpenGL name for this program object
      ///
      GLuint          getHandle();

      ///
      /// \brief Query OpenGL for the max. number of allowed texture units
      /// \returns The max. number of allowed texture units
      ///
      static int32_t getMaxTextureUnits();

      ///
      /// \brief Sets a Kit texture as a uniform
      /// \param name The name of the uniform to set 
      /// \param texture The Kit texture to set as uniform
      ///
      void setUniformTexture(const std::string& name, kit::TextureWPtr texture);
      
      ///
      /// \brief Sets a Kit cubemap as a uniform
      /// \param name The name of the uniform to set 
      /// \param cubemap The Kit cubemap to set as uniform
      ///
      void setUniformCubemap(const std::string& name, kit::CubemapWPtr cubemap);

      ///
      /// \brief Sets a float as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniform1f(const std::string& name, float value);

      ///
      /// \brief Sets a double as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniform1d(const std::string& name, double value);

      /// \brief Sets an integer as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniform1i(const std::string& name, int32_t value);

      /// \brief Sets an unsigned integer as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniform1ui(const std::string& name, uint32_t value);

      /// \brief Sets a vec2 as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniform2f(const std::string& name, const glm::vec2 & value);

      /// \brief Sets a vec3 as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniform3f(const std::string& name, const glm::vec3 & value);

      /// \brief Sets a vector of vec3s as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniform3fv(const std::string& name, const std::vector<glm::vec3> & value);

      /// \brief Sets a vec4 as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniform4f(const std::string& name, const glm::vec4 & value);

      /// \brief Sets a mat3 as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniformMat3(const std::string& name, const glm::mat3 & value);

      /// \brief Sets a mat4 as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniformMat4(const std::string& name, const glm::mat4 & value);

      /// \brief Sets a vector of mat4s as a uniform
      /// \param name The name of the uniform to set 
      /// \param value The value to set as uniform
      ///
      void setUniformMat4v(const std::string& name, const std::vector<glm::mat4> & value);

      /// \brief Retrieve the uniform location for a given uniform
      /// \param name The name of the uniform
      /// \returns The uniform location
      ///
      uint32_t getUniformLocation(const std::string& name);

      /// \brief For internal use
      void prepareTextures();

    private:
      static void addShaders(kit::Program::Ptr program, kit::Shader::Type type, std::vector<std::string> const & sources, std::vector<kit::Shader::Ptr> & outShaders, char const * dataDirectory = "./data/");
      
      std::string                               m_fileIdentifier;
      kit::GL                                   m_glSingleton;
      GLuint			                m_glHandle;
      std::map<std::string, uint32_t>           m_locationCache;
      std::map<uint32_t, kit::TextureWPtr>      m_textures;
      std::map<uint32_t, kit::CubemapWPtr>      m_cubemaps;
  };

}

#endif // KIT_PROGRAM_HEADER
