#ifndef KIT_SHADER_HEADER
#define KIT_SHADER_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <string>
#include <memory>

namespace kit{

  ///
  /// \brief An OpenGL shader
  ///
  class KITAPI Shader
  {
    public:
      
      ///
      /// \brief Different types of shaders. Directly mapped to OpenGL constants.
      ///
      enum class Type : uint32_t
      {
        Vertex = GLK_VERTEX_SHADER,
        Fragment = GLK_FRAGMENT_SHADER,
        Geometry = GLK_GEOMETRY_SHADER,
        TessControl = GLK_TESS_CONTROL_SHADER,
        TessEvaluation = GLK_TESS_EVALUATION_SHADER,
        Compute = GLK_COMPUTE_SHADER
      };
      
      typedef std::shared_ptr<kit::Shader> Ptr;

      ///
      /// \brief Constructor (FOR INTERNAL USE ONLY)
      /// 
      /// You should NEVER instance this as usual. ALWAYS use smart pointers (std::shared_ptr), and create them explicitly using the `create` methods!
      ///
      Shader(Shader::Type type);

      ///
      /// \brief Destructor
      ///
      ~Shader();

      /// 
      /// \brief Creates an empty shader object
      ///
      /// \param type The shader type to create
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \returns A shared pointer pointing to the newly created shader
      ///
      static kit::Shader::Ptr create(Shader::Type type);

      ///
      /// \brief Attempt to source from a file
      /// \returns true on success, false on failure
      /// \param filename Path to source file, relative to working directory
      ///
      bool sourceFromFile(std::string const & filename);

      ///
      /// \brief Source directly from string
      /// \param s The sourcecode
      ///
      void sourceFromString(std::string const & s);

      ///
      /// \brief Clears the sourcecode
      ///
      void clearSource();

      ///
      /// \brief Attempts to compile the source
      /// \returns true on success, false on failure
      ///
      bool compile();

      ///
      /// \brief Retrieve the internal OpenGL name for this shader object
      /// \returns the internal OpenGL name
      ///
      uint32_t getHandle();

    private:
      uint32_t            m_glHandle;
      std::string       m_source;
      Shader::Type      m_type;
  };

}

#endif // KIT_SHADER_HEADER
