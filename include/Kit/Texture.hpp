#ifndef KIT_TEXTURE_HEADER
#define KIT_TEXTURE_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"

#include <memory>
#include <map>
#include <string>

namespace kit{

  ///
  /// \brief An OpenGL texture
  ///
  class KITAPI Texture
  {
    public:

      ///
      /// \brief An enum that represents different texture types, mapped directly to OpenGL constants
      ///
      enum Type
      {
        Texture1D = GL_TEXTURE_1D,
        Texture1DArray = GL_TEXTURE_1D_ARRAY,
        TextureBuffer = GL_TEXTURE_BUFFER,
        Texture2D = GL_TEXTURE_2D,
        Texture2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
        Texture2DArray = GL_TEXTURE_2D_ARRAY,
        Texture2DMultiSampleArray =  GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
        Texture3D = GL_TEXTURE_3D,
        Cubemap = GL_TEXTURE_CUBE_MAP,
        CubemapArray = GL_TEXTURE_CUBE_MAP_ARRAY
      };

      ///
      /// \brief An enum that represents different texture filtering modes, mapped directly to OpenGL constants
      ///
      enum FilteringMode
      {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
      };

      ///
      /// \brief An enum that represents different texture sampling axes, mapped directly to OpenGL constants (with the exception of EdgeSamplingAxis:: All)
      ///
      enum EdgeSamplingAxis
      {
        All,
        S = GL_TEXTURE_WRAP_S,
        T = GL_TEXTURE_WRAP_T,
        R = GL_TEXTURE_WRAP_R
      };

      ///
      /// \brief An enum that represents different edge sampling modes, mapped directly to OpenGL constants
      ///
      enum EdgeSamplingMode
      {
        Repeat = GL_REPEAT,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE
      };

      ///
      /// \brief An enum that represents different texture formats, mapped directly to OpenGL constants
      ///
      enum Format
      {
        Red = GL_RED,
        RG = GL_RG,
        RGB = GL_RGB,
        RGBA = GL_RGBA,
        BGR = GL_BGR,
        BGRA = GL_BGRA,
        DepthComponent = GL_DEPTH_COMPONENT,
        StencilIndex = GL_STENCIL_INDEX
      };

      ///
      /// \brief An enum that represents different internal texture formats, mapped directly to OpenGL constants
      ///
      enum InternalFormat 
      {
        R8 = GL_R8,
        R8SNorm = GL_R8_SNORM,
        R16 = GL_R16,
        R16SNorm = GL_R16_SNORM,
        RG8 = GL_RG8,
        RG8SNorm = GL_RG8_SNORM,
        RG16 = GL_RG16,
        RG16SNorm = GL_RG16_SNORM,
        R3G3B2 = GL_R3_G3_B2,
        RGB4 = GL_RGB4,
        RGB5 = GL_RGB5,
        RGB8 = GL_RGB8,
        RGB8SNorm = GL_RGB8_SNORM,
        RGB10 = GL_RGB10,
        RGB12 = GL_RGB12,
        RGB16SNorm = GL_RGB16_SNORM,
        RGBA2 = GL_RGBA2,
        RGBA4 = GL_RGBA4,
        RGB5A1 = GL_RGB5_A1,
        RGBA8 = GL_RGBA8,
        RGBA8SNorm = GL_RGBA8_SNORM,
        RGB10A2 = GL_RGB10_A2,
        RGB10A2UI = GL_RGB10_A2UI,
        RGBA12 = GL_RGBA12,
        RGBA16 = GL_RGBA16,
        SRGB8 = GL_SRGB8,
        SRGB8Alpha8 = GL_SRGB8_ALPHA8,
        R16F = GL_R16F,
        RG16F = GL_RG16F,
        RGB16F = GL_RGB16F,
        RGBA16F = GL_RGBA16F,
        R32F = GL_R32F,
        RG32F = GL_RG32F,
        RGB32F = GL_RGB32F,
        RGBA32F = GL_RGBA32F,
        R11FG11FB10F = GL_R11F_G11F_B10F,
        RGB9E5 = GL_RGB9_E5,
        R8I = GL_R8I,
        R8UI = GL_R8I,
        R16I = GL_R16I,
        R16UI = GL_R16UI,
        R32I = GL_R32I,
        R32UI = GL_R32UI,
        RG8I = GL_RG8I,
        RG8UI = GL_RG8UI,
        RG16I = GL_RG16I,
        RG16UI = GL_RG16UI,
        RG32I = GL_RG32I,
        RG32UI = GL_RG32UI,
        RGB8I = GL_RGB8I,
        RGB8UI = GL_RGB8UI,
        RGB16I = GL_RGB16I,
        RGB16UI = GL_RGB16UI,
        RGB32I = GL_RGB32I,
        RGB32UI = GL_RGB32UI,
        RGBA8I = GL_RGBA8I,
        RGBA8UI = GL_RGBA8UI,
        RGBA16I = GL_RGBA16I,
        RGBA16UI = GL_RGBA16UI,
        RGBA32I = GL_RGBA32I,
        RGBA32UI = GL_RGBA32UI,
        DepthComponent32F = GL_DEPTH_COMPONENT32F, 
        DepthComponent24 = GL_DEPTH_COMPONENT24, 
        DepthComponent16 = GL_DEPTH_COMPONENT16, 
        Depth32FStencil8 = GL_DEPTH32F_STENCIL8, 
        Depth24Stencil8 = GL_DEPTH24_STENCIL8, 
        StencilIndex8 = GL_STENCIL_INDEX8
      };

      ///
      /// \brief An enum that represents different texture datatypes, mapped directly to OpenGL constants
      ///
      enum DataType
      {
        Byte = GL_BYTE,
        UnsignedByte = GL_UNSIGNED_BYTE,
        Short = GL_SHORT,
        UnsignedShort = GL_UNSIGNED_SHORT,
        Int = GL_INT,
        UnsignedInt = GL_UNSIGNED_INT,
        Float = GL_FLOAT,
        UnsignedByte332 = GL_UNSIGNED_BYTE_3_3_2,
        UnsignedByte233Rev = GL_UNSIGNED_BYTE_2_3_3_REV,
        UnsignedShort565 = GL_UNSIGNED_SHORT_5_6_5,
        UnsignedShort565Rev = GL_UNSIGNED_SHORT_5_6_5_REV,
        UnsignedShort4444 = GL_UNSIGNED_SHORT_4_4_4_4,
        UnsignedShort4444Rev = GL_UNSIGNED_SHORT_4_4_4_4_REV,
        UnsignedShort5551 = GL_UNSIGNED_SHORT_5_5_5_1,
        UnsignedShort1555Rev = GL_UNSIGNED_SHORT_1_5_5_5_REV,
        UnsignedInt8888 = GL_UNSIGNED_INT_8_8_8_8,
        UnsignedInt8888Rev = GL_UNSIGNED_INT_8_8_8_8_REV,
        UnsignedInt1010102 = GL_UNSIGNED_INT_10_10_10_2,
        UnsignedInt2101010Rev = GL_UNSIGNED_INT_2_10_10_10_REV
      };

      typedef std::shared_ptr<kit::Texture> Ptr;
      typedef std::weak_ptr<kit::Texture> WPtr;

      ///
      /// \brief Constructor (FOR INTERNAL USE ONLY)
      /// 
      /// \param t The texture type to initialize this texture with
      ///
      /// You should NEVER instance this as usual. ALWAYS use smart pointers (std::shared_ptr), and create them explicitly using the `create` methods!
      ///
      Texture(Type t);

      ///
      /// \brief Destructor
      ///
      ~Texture();


      // ---- Primary constructors

      /// 
      /// \brief Creates an empty 2D texture
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param resolution Resolution of the new texture
      /// \param format The internal format of the new texture
      /// \param edgemode The edge sampling mode of the new texture
      /// \param minfilter The minification filtering mode of the new texture.
      /// \param magfilter The magnification filtering mode of the new texture. Valid paramters are Nearest and Linear.
      ///
      /// \returns A shared pointer pointing to the newly created texture
      ///
      static kit::Texture::Ptr create2D(glm::uvec2 resolution, InternalFormat format, EdgeSamplingMode edgemode = ClampToEdge, FilteringMode minfilter = Linear, FilteringMode magfilter = Linear);


      // ---- Convenience constructors

      /// 
      /// \brief Creates a 2D texture fitting for a shadowmap
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param resolution Resolution of the new texture
      ///
      /// \returns A shared pointer pointing to the newly created texture
      ///
      static kit::Texture::Ptr createShadowmap(glm::uvec2 resolution);

      /// 
      /// \brief Creates a 2D texture and loads its content from a file
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param filename Path to the source file, relative to the working directory.
      /// \param format The internal format of the new texture
      /// \param edgemode The edge sampling mode of the new texture
      /// \param minfilter The minification filtering mode of the new texture.
      /// \param magfilter The magnification filtering mode of the new texture. Valid paramters are Nearest and Linear.
      ///
      /// \returns A shared pointer pointing to the newly created texture
      ///
      static kit::Texture::Ptr create2DFromFile(const std::string& filename, InternalFormat format = RGBA8, EdgeSamplingMode edgemode = ClampToEdge, FilteringMode minfilter = Linear, FilteringMode magfilter = Linear);

      /// 
      /// \brief Creates a 3D texture and loads its content from a 2D image file. It does this by evenly splicing it on the height. It has to perfectly cubical.
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param filename Path to the source file, relative to the working directory.
      /// \param format The internal format of the new texture
      /// \param edgemode The edge sampling mode of the new texture
      /// \param minfilter The minification filtering mode of the new texture.
      /// \param magfilter The magnification filtering mode of the new texture. Valid paramters are Nearest and Linear.
      ///
      /// \returns A shared pointer pointing to the newly created texture
      ///
      static kit::Texture::Ptr create3DFromFile(const std::string& filename, InternalFormat format = RGBA8, EdgeSamplingMode edgemode = ClampToEdge, FilteringMode minfilter = Linear, FilteringMode magfilter = Linear);


      // ---- Resource-managed constructors

      /// 
      /// \brief Loads a texture from a file. This is probably want you want to use for regular 2D texture loading.
      ///
      /// You should only use the static `create` methods to create instances. Avoid instancing this class yourself!
      ///
      /// \param name Path to the source file, relative to ./data/textures/
      /// \param srgb true if texture is sRGB-encoded
      ///
      /// \returns A shared pointer pointing to the newly created texture
      ///
      static kit::Texture::Ptr load(const std::string& name, bool srgb = true);


      // ---- Operations

      ///
      /// \brief Generates mipmaps for the current texture, based on its current contents
      ///
      void          generateMipmap();

      ///
      /// \brief Calculates the mip levels of this texture
      ///
      uint32_t      calculateMipLevels();

      ///
      /// \brief Get a single pixel from this texture, stored as a vec4 (slow)
      /// \param position The position of the pixel to get. position.z is ignored on 2D textures
      /// \returns A vec4 containing the pixel information
      ///
      glm::vec4     getPixelFloat(glm::vec3 position);

      ///
      /// \brief Get a single pixel from this texture, stored as a uvec4 (slow)
      /// \param position The position of the pixel to get. position.z is ignored on 2D textures
      /// \returns An uvec4 containing the pixel information
      ///
      glm::uvec4    getPixelUint(glm::vec3 position);


      // -- For saving images
      ///
      /// \brief Save texture to file as a tga image.
      /// \param filename Path to output file, relative to working directory
      /// \returns true on success, false on failure
      ///
      bool          saveToFile(const std::string& filename);

      ///
      /// \brief Bind this texture
      ///
      void          bind();

      ///
      /// \brief Unbind a certain texture type
      /// \param t The type of texture to unbind
      ///
      static void   unbind(Type t);

      ///
      /// \brief Flushes the resource managed cache. Cache will be empty but textures might still be used elsewhere.
      ///
      static void   flushCache();

      ///
      /// \brief Returns a list of textures in ./data/textures/prefix. The list is cached and has to be updated using the reload parameter.
      /// \param prefix adds a prefix to the path before iterating the directory
      /// \param reload set to true to force reloading the cache
      /// \returns A list of filenames
      ///
      static std::vector<std::string> getAvailableTextures(const std::string& prefix = "", bool reload = false);


      // ---- Properties
      // Mutable

      ///
      /// \brief Gets the minification filtering mode for this texture
      /// \returns The minification filtering mode for this texture
      ///
      kit::Texture::FilteringMode getMinFilteringMode();

      ///
      /// \brief Sets the minification filtering mode for this texture
      /// \param mode The new minification filtering mode for this texture
      ///
      void setMinFilteringMode(kit::Texture::FilteringMode mode);

      ///
      /// \brief Gets the magnification filtering mode for this texture
      /// \returns The magnification filtering mode for this texture
      ///
      kit::Texture::FilteringMode getMagFilteringMode();

      ///
      /// \brief Sets the magnification filtering mode for this texture
      /// \param mode The new magnification filtering mode for this texture
      ///
      void setMagFilteringMode(kit::Texture::FilteringMode mode);

      ///
      /// \brief Gets the edge sampling mode on a specified axis on this texture
      /// \param axis The axis to get the sampling mode from, can not be All
      /// \returns The edge sampling mode for a specified axis on this texture
      ///
      kit::Texture::EdgeSamplingMode getEdgeSamplingMode(EdgeSamplingAxis axis);

      ///
      /// \brief Sets the edge sampling mode for a specified axis on this texture
      /// \param mode The new edge sampling mode for the specified axis on this texture
      /// \param axis The axis to set on. Use All to set it on all axes
      ///
      void setEdgeSamplingMode(kit::Texture::EdgeSamplingMode mode, kit::Texture::EdgeSamplingAxis axis = kit::Texture::All);

      ///
      /// \brief Set the anisotropic level for this texture
      /// \param The level to set it to. Has to be at least 1.
      ///
      void setAnisotropicLevel(float l);

      ///
      /// \brief Get the anisotropic level for this texture
      /// \returns the anisotropic level for this texture
      ///
      float getAnisotropicLevel();

      // Immutable
      ///
      /// \brief Get the resolution for this texture
      /// \returns the resolution for this texture
      ///
      glm::uvec3 getResolution();

      ///
      /// \brief Get the array size for this texture
      /// \returns the array size for this texture
      ///
      uint32_t getArraySize();

      ///
      /// \brief Get the internal format for this texture
      /// \returns the internal format for this texture
      ///
      InternalFormat getInternalFormat();

      ///
      /// \brief Get the internal handle for this texture
      /// \returns the internal handle for this texture
      ///
      GLuint getHandle();

      ///
      /// \brief Gets the filename of this texture, if applicable
      /// \returns the filename for this texture
      ///
      std::string getFilename();

    private:
      kit::GL             m_glSingleton;

      std::string         m_filename;

      GLuint              m_glHandle;
      Type                m_type;

      InternalFormat      m_internalFormat;
      FilteringMode       m_minFilteringMode;
      FilteringMode       m_magFilteringMode;
      EdgeSamplingMode    m_edgeSamplingModeS;
      EdgeSamplingMode    m_edgeSamplingModeT;
      EdgeSamplingMode    m_edgeSamplingModeR;
      glm::uvec3          m_resolution;
      uint32_t            m_arraySize;
      float               m_anisotropicLevel;

      static std::map<std::string, kit::Texture::Ptr> m_cachedTextures;
  };

}

#endif // KIT_TEXTURE_HEADER
