#ifndef KIT_PIXELSHADER_HEADER
#define KIT_PIXELSHADER_HEADER

#include "Kit/Export.hpp"
#include "Kit/GL.hpp"

#include <string>
#include <memory>

namespace kit{

  ///
  /// \brief A pixel (fragment) shader
  ///
  class KITAPI PixelShader
  {
    public:
      typedef std::shared_ptr<kit::PixelShader> Ptr;

      ///
      /// \brief Constructor (FOR INTERNAL USE ONLY)
      /// 
      /// You should NEVER instance this as usual. ALWAYS use smart pointers (std::shared_ptr), and create them explicitly using the `create` methods!
      ///
      PixelShader();

      ///
      /// \brief Destructor
      ///
      ~PixelShader();

      /// 
      /// \brief Creates an empty pixel shader
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \returns A shared pointer pointing to the newly created pixel shader
      ///
      static kit::PixelShader::Ptr create();

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
      /// \brief Retrieve the internal OpenGL name for this pixelshader
      /// \returns the internal OpenGL name
      ///
      GLuint getHandle();

    private:
      kit::GL           m_glSingleton;
      GLuint            m_glHandle;
      std::string       m_source;
  };

}

#endif // KIT_PIXELSHADER_HEADER
