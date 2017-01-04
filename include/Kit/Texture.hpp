#ifndef KIT_TEXTURE_HEADER
#define KIT_TEXTURE_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <unordered_map>
#include <memory>
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
        Texture1D = GLK_TEXTURE_1D,
        Texture1DArray = GLK_TEXTURE_1D_ARRAY,
        TextureBuffer = GLK_TEXTURE_BUFFER,
        Texture2D = GLK_TEXTURE_2D,
        Texture2DMultisample = GLK_TEXTURE_2D_MULTISAMPLE,
        Texture2DArray = GLK_TEXTURE_2D_ARRAY,
        Texture2DMultiSampleArray =  GLK_TEXTURE_2D_MULTISAMPLE_ARRAY,
        Texture3D = GLK_TEXTURE_3D,
        Cubemap = GLK_TEXTURE_CUBE_MAP,
        CubemapArray = GLK_TEXTURE_CUBE_MAP_ARRAY
      };

      ///
      /// \brief An enum that represents different texture filtering modes, mapped directly to OpenGL constants
      ///
      enum FilteringMode
      {
        Nearest = GLK_NEAREST,
        Linear = GLK_LINEAR,
        NearestMipmapNearest = GLK_NEAREST_MIPMAP_NEAREST,
        LinearMipmapNearest = GLK_LINEAR_MIPMAP_NEAREST,
        NearestMipmapLinear = GLK_NEAREST_MIPMAP_LINEAR,
        LinearMipmapLinear = GLK_LINEAR_MIPMAP_LINEAR
      };

      ///
      /// \brief An enum that represents different texture sampling axes, mapped directly to OpenGL constants (with the exception of EdgeSamplingAxis:: All)
      ///
      enum EdgeSamplingAxis
      {
        All,
        S = GLK_TEXTURE_WRAP_S,
        T = GLK_TEXTURE_WRAP_T,
        R = GLK_TEXTURE_WRAP_R
      };

      ///
      /// \brief An enum that represents different edge sampling modes, mapped directly to OpenGL constants
      ///
      enum EdgeSamplingMode
      {
        Repeat = GLK_REPEAT,
        MirroredRepeat = GLK_MIRRORED_REPEAT,
        ClampToEdge = GLK_CLAMP_TO_EDGE
      };

      ///
      /// \brief An enum that represents different texture formats, mapped directly to OpenGL constants
      ///
      enum Format
      {
        Red = GLK_RED,
        RG = GLK_RG,
        RGB = GLK_RGB,
        RGBA = GLK_RGBA,
        BGR = GLK_BGR,
        BGRA = GLK_BGRA,
        DepthComponent = GLK_DEPTH_COMPONENT,
        StencilIndex = GLK_STENCIL_INDEX
      };

      ///
      /// \brief An enum that represents different internal texture formats, mapped directly to OpenGL constants
      ///
      enum InternalFormat 
      {
        R8 = GLK_R8,
        R8SNorm = GLK_R8_SNORM,
        R16 = GLK_R16,
        R16SNorm = GLK_R16_SNORM,
        RG8 = GLK_RG8,
        RG8SNorm = GLK_RG8_SNORM,
        RG16 = GLK_RG16,
        RG16SNorm = GLK_RG16_SNORM,
        R3G3B2 = GLK_R3_G3_B2,
        RGB4 = GLK_RGB4,
        RGB5 = GLK_RGB5,
        RGB8 = GLK_RGB8,
        RGB8SNorm = GLK_RGB8_SNORM,
        RGB10 = GLK_RGB10,
        RGB12 = GLK_RGB12,
        RGB16SNorm = GLK_RGB16_SNORM,
        RGBA2 = GLK_RGBA2,
        RGBA4 = GLK_RGBA4,
        RGB5A1 = GLK_RGB5_A1,
        RGBA8 = GLK_RGBA8,
        RGBA8SNorm = GLK_RGBA8_SNORM,
        RGB10A2 = GLK_RGB10_A2,
        RGB10A2UI = GLK_RGB10_A2UI,
        RGBA12 = GLK_RGBA12,
        RGBA16 = GLK_RGBA16,
        SRGB8 = GLK_SRGB8,
        SRGB8Alpha8 = GLK_SRGB8_ALPHA8,
        R16F = GLK_R16F,
        RG16F = GLK_RG16F,
        RGB16F = GLK_RGB16F,
        RGBA16F = GLK_RGBA16F,
        R32F = GLK_R32F,
        RG32F = GLK_RG32F,
        RGB32F = GLK_RGB32F,
        RGBA32F = GLK_RGBA32F,
        R11FG11FB10F = GLK_R11F_G11F_B10F,
        RGB9E5 = GLK_RGB9_E5,
        R8I = GLK_R8I,
        R8UI = GLK_R8I,
        R16I = GLK_R16I,
        R16UI = GLK_R16UI,
        R32I = GLK_R32I,
        R32UI = GLK_R32UI,
        RG8I = GLK_RG8I,
        RG8UI = GLK_RG8UI,
        RG16I = GLK_RG16I,
        RG16UI = GLK_RG16UI,
        RG32I = GLK_RG32I,
        RG32UI = GLK_RG32UI,
        RGB8I = GLK_RGB8I,
        RGB8UI = GLK_RGB8UI,
        RGB16I = GLK_RGB16I,
        RGB16UI = GLK_RGB16UI,
        RGB32I = GLK_RGB32I,
        RGB32UI = GLK_RGB32UI,
        RGBA8I = GLK_RGBA8I,
        RGBA8UI = GLK_RGBA8UI,
        RGBA16I = GLK_RGBA16I,
        RGBA16UI = GLK_RGBA16UI,
        RGBA32I = GLK_RGBA32I,
        RGBA32UI = GLK_RGBA32UI,
        DepthComponent32F = GLK_DEPTH_COMPONENT32F, 
        DepthComponent24 = GLK_DEPTH_COMPONENT24, 
        DepthComponent16 = GLK_DEPTH_COMPONENT16, 
        Depth32FStencil8 = GLK_DEPTH32F_STENCIL8, 
        Depth24Stencil8 = GLK_DEPTH24_STENCIL8, 
        StencilIndex8 = GLK_STENCIL_INDEX8
      };

      ///
      /// \brief An enum that represents different texture datatypes, mapped directly to OpenGL constants
      ///
      enum DataType
      {
        Byte = GLK_BYTE,
        UnsignedByte = GLK_UNSIGNED_BYTE,
        Short = GLK_SHORT,
        UnsignedShort = GLK_UNSIGNED_SHORT,
        Int = GLK_INT,
        UnsignedInt = GLK_UNSIGNED_INT,
        Float = GLK_FLOAT,
        UnsignedByte332 = GLK_UNSIGNED_BYTE_3_3_2,
        UnsignedByte233Rev = GLK_UNSIGNED_BYTE_2_3_3_REV,
        UnsignedShort565 = GLK_UNSIGNED_SHORT_5_6_5,
        UnsignedShort565Rev = GLK_UNSIGNED_SHORT_5_6_5_REV,
        UnsignedShort4444 = GLK_UNSIGNED_SHORT_4_4_4_4,
        UnsignedShort4444Rev = GLK_UNSIGNED_SHORT_4_4_4_4_REV,
        UnsignedShort5551 = GLK_UNSIGNED_SHORT_5_5_5_1,
        UnsignedShort1555Rev = GLK_UNSIGNED_SHORT_1_5_5_5_REV,
        UnsignedInt8888 = GLK_UNSIGNED_INT_8_8_8_8,
        UnsignedInt8888Rev = GLK_UNSIGNED_INT_8_8_8_8_REV,
        UnsignedInt1010102 = GLK_UNSIGNED_INT_10_10_10_2,
        UnsignedInt2101010Rev = GLK_UNSIGNED_INT_2_10_10_10_REV
      };

      /// 
      /// \brief Creates a 2D texture
      ///
      /// \param resolution Resolution of the new texture
      /// \param format The internal format of the new texture
      ///
      Texture(glm::uvec2 resolution, InternalFormat format = RGBA8, uint8_t levels = 0);

      /// 
      /// \brief Creates a 2D texture and loads its content from a file
      ///
      /// \param filename Path to the source file, relative to the working directory.
      /// \param format The internal format of the new texture
      ///
      Texture(const std::string& filename, InternalFormat format = RGBA8, uint8_t levels = 0, Type t = Type::Texture2D);
      
      
      ///
      /// \brief Destructor
      ///
      ~Texture();

      /// 
      /// \brief Creates a 2D texture fitting for a shadowmap
      /// \param resolution Resolution of the new texture
      ///
      /// \returns A shared pointer pointing to the newly created texture
      ///
      static kit::Texture * createShadowmap(glm::uvec2 resolution);
      
      // ---- Resource-managed constructors

      /// 
      /// \brief Loads a texture from a file. This is probably want you want to use for regular 2D texture loading.
      ///
      /// \param name Path to the source file, relative to ./data/textures/
      /// \param srgb true if texture is sRGB-encoded
      ///
      /// \returns A shared pointer pointing to the newly created texture
      ///
      static std::shared_ptr<kit::Texture> load(const std::string& name, bool srgb = true);


      // ---- Operations

      ///
      /// \brief Generates mipmaps for the current texture, based on its current contents
      ///
      void generateMipmap();

      ///
      /// \brief Calculates the mip levels of this texture
      ///
      uint32_t calculateMipLevels();

      ///
      /// \brief Get a single pixel from this texture, stored as a vec4 (slow)
      /// \param position The position of the pixel to get. position.z is ignored on 2D textures
      /// \returns A vec4 containing the pixel information
      ///
      glm::vec4 getPixelFloat(glm::vec3 position);

      ///
      /// \brief Get a single pixel from this texture, stored as a uvec4 (slow)
      /// \param position The position of the pixel to get. position.z is ignored on 2D textures
      /// \returns An uvec4 containing the pixel information
      ///
      glm::uvec4 getPixelUint(glm::vec3 position);


      // -- For saving images
      ///
      /// \brief Save texture to file as a tga image.
      /// \param filename Path to output file, relative to working directory
      /// \returns true on success, false on failure
      ///
      bool saveToFile(const std::string& filename);

      ///
      /// \brief Bind this texture
      ///
      void bind();

      ///
      /// \brief Unbind a certain texture type
      /// \param t The type of texture to unbind
      ///
      static void unbind(Type t);


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
      /// \throws kit::Exception If axis is kit::Texture::EdgeSamplingAxis::All
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
      /// \param l The level to set it to. Has to be at least 1.
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
      uint32_t getHandle();

      ///
      /// \brief Gets the filename of this texture, if applicable
      /// \returns the filename for this texture
      ///
      std::string getFilename();

      
      // Helper utility
    private:

      Texture(Type t);
      
      std::string         m_filename = "";

      uint32_t            m_glHandle = 0;
      Type                m_type = Type::Texture2D;

      InternalFormat      m_internalFormat = InternalFormat::RGBA8;
      FilteringMode       m_minFilteringMode = FilteringMode::Linear;
      FilteringMode       m_magFilteringMode = FilteringMode::Linear;
      EdgeSamplingMode    m_edgeSamplingModeS = EdgeSamplingMode::Repeat;
      EdgeSamplingMode    m_edgeSamplingModeT = EdgeSamplingMode::Repeat;
      EdgeSamplingMode    m_edgeSamplingModeR = EdgeSamplingMode::Repeat;
      glm::uvec3          m_resolution = glm::uvec3(0, 0, 0);
      uint32_t            m_arraySize = 0;
      float               m_anisotropicLevel = 1.0f;

      static std::unordered_map<std::string, std::weak_ptr<kit::Texture>> m_cache;
  };

}

#endif // KIT_TEXTURE_HEADER
