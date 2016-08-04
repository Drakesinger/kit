#ifndef KIT_VERTEXSHADER_HEADER
#define KIT_VERTEXSHADER_HEADER

#include "Kit/Export.hpp"
#include "Kit/GL.hpp"

#include <string>
#include <memory>

namespace kit{

  ///
  /// \brief A vertex shader
  ///
  class KITAPI VertexShader
  {
    public:
      typedef std::shared_ptr<kit::VertexShader> Ptr;

      ///
      /// \brief Constructor (FOR INTERNAL USE ONLY)
      /// 
      /// You should NEVER instance this as usual. ALWAYS use smart pointers (std::shared_ptr), and create them explicitly using the `create` methods!
      ///
      VertexShader();

      ///
      /// \brief Destructor
      ///
      ~VertexShader();

      /// 
      /// \brief Creates an empty vertex shader
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \returns A shared pointer pointing to the newly created vertex shader
      ///
      static kit::VertexShader::Ptr create();

      ///
      /// \brief Attempt to source from a file
      /// \returns true on success, false on failure
      /// \param filename Path to source file, relative to working directory
      ///
      bool sourceFromFile(std::string filename);

      ///
      /// \brief Source directly from string
      /// \param s The sourcecode
      ///
      void sourceFromString(std::string s);

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
      /// \brief Retrieve the internal OpenGL name for this vertexshader
      /// \returns the internal OpenGL name
      ///
      GLuint getHandle();

    private:
      kit::GL           m_glSingleton;
      GLuint            m_glHandle;
      std::string       m_source;
  };

}

#endif // KIT_VERTEXSHADER_HEADER
