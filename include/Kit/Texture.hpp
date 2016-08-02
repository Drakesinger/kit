#ifndef KIT_TEXTURE_HEADER
#define KIT_TEXTURE_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/GL.hpp"

#include <memory>
#include <map>
#include <string>

namespace kit{

  class KITAPI Texture{
    public:

      enum Type
      {
        Texture1D = GL_TEXTURE_1D,
        Texture1DArray = GL_TEXTURE_1D_ARRAY,
        //TODO: TextureBuffer = GL_TEXTURE_BUFFER
        Texture2D = GL_TEXTURE_2D,
        Texture2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
        Texture2DArray = GL_TEXTURE_2D_ARRAY,
        //TODO: GL_TEXTURE_2D_MULTISAMPLE_ARRAY
        Texture3D = GL_TEXTURE_3D,
        Cubemap = GL_TEXTURE_CUBE_MAP,
        CubemapArray = GL_TEXTURE_CUBE_MAP_ARRAY
        
      };

      enum FilteringMode
      {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
      };

      enum EdgeSamplingAxis
      {
        All,
        S = GL_TEXTURE_WRAP_S,
        T = GL_TEXTURE_WRAP_T,
        R = GL_TEXTURE_WRAP_R
      };

      enum EdgeSamplingMode
      {
        Repeat = GL_REPEAT,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE
      };

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
      
      enum Face 
      {
        FaceBegin = 0,
        PositiveX = 0,
        NegativeX = 1,
        PositiveY = 2,
        NegativeY = 3,
        PositiveZ = 4,
        NegativeZ = 5,
        FaceEnd = 6
      };

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
      typedef std::array<std::string, 6> FaceList;
      
      Texture(Type t);
      Texture(GLuint handle);
      ~Texture();

      // ---- Primary constructors
      static kit::Texture::Ptr reference(GLuint handle);
      static kit::Texture::Ptr create2D(glm::uvec2 resolution, InternalFormat format, EdgeSamplingMode edgemode = ClampToEdge, FilteringMode minfilter = Linear, FilteringMode magfilter = Linear);
      static kit::Texture::Ptr create2DMultisample(glm::uvec2 resolution, uint32_t samples);
      static kit::Texture::Ptr createCubemap(glm::uvec2 faceresolution, InternalFormat format, FilteringMode minfilter = LinearMipmapLinear, FilteringMode magfilter = Linear);
      
      // ---- Convenience constructors
      static kit::Texture::Ptr createShadowmap(glm::uvec2 resolution);
      static kit::Texture::Ptr create2DFromFile(std::string filename, InternalFormat format = RGBA8, EdgeSamplingMode edgemode = ClampToEdge, FilteringMode minfilter = Linear, FilteringMode magfilter = Linear);
      static kit::Texture::Ptr createRaw(std::string filename, glm::uvec2 resolution);
      static kit::Texture::Ptr create3DFromFile(std::string filename, InternalFormat format = RGBA8, EdgeSamplingMode edgemode = ClampToEdge, FilteringMode minfilter = Linear, FilteringMode magfilter = Linear);
      //TODO: static kit::Texture::Ptr create3DFromFile(std::string filename, InternalFormat format = RGBA8, EdgeSamplingMode edgemode = Repeat, FilteringMode minfilter = LinearMipmapLinear, FilteringMode magfilter = Linear);
      static kit::Texture::Ptr createCubemapFromFiles(FaceList filenames, InternalFormat format = RGBA8, FilteringMode minfilter = LinearMipmapLinear, FilteringMode magfilter = Linear);
      static kit::Texture::Ptr loadContribMap(std::string name);
      
      // ---- Resource-managed constructors
      static kit::Texture::Ptr load(std::string name, bool srgb = true);
      static kit::Texture::Ptr loadRadianceMap(std::string name);
      static kit::Texture::Ptr loadIrradianceMap(std::string name);
      static kit::Texture::Ptr loadSkybox(std::string name);
      
      // ---- Operations
      bool          updatePixelsFromFile(std::string filename, Face cubeface = PositiveX);
      bool          updatePixelsFromFiles(FaceList filenames);
      void          generateMipmap();
      uint32_t   calculateMipLevels();
      glm::vec4     getPixelFloat(glm::vec3 position);
      glm::uvec4    getPixelUint(glm::vec3 position);
      
      // -- For saving images
      bool          saveToFile(std::string filename);

      void          bind();
      static void   unbind(Type t);
      static void   flushCache();
      
      static std::vector<std::string> getAvailableTextures(std::string prefix = "", bool reload = false);

      // ---- Properties
      // Mutable
      kit::Texture::FilteringMode getMinFilteringMode();
      void setMinFilteringMode(kit::Texture::FilteringMode mode);
      
      kit::Texture::FilteringMode getMagFilteringMode();
      void setMagFilteringMode(kit::Texture::FilteringMode mode);

      kit::Texture::EdgeSamplingMode getEdgeSamplingMode(EdgeSamplingAxis axis);
      void setEdgeSamplingMode(kit::Texture::EdgeSamplingMode mode, kit::Texture::EdgeSamplingAxis axis = kit::Texture::All);
      
      void setAnisotropicLevel(float l);
      float getAnisotropicLevel();
      
      // Immutable
      glm::uvec3 getResolution();
      uint32_t getArraySize();
      InternalFormat getInternalFormat();
      GLuint getHandle();
      
      std::string getFilename();
      
    private:
      kit::GL             m_glSingleton;
      
      std::string         m_filename;
      
      GLuint              m_glHandle;
      bool                m_isReference;
      Type                m_type;

      bool                m_isMultisampled;
      uint32_t         m_numSamples;
      InternalFormat      m_internalFormat;
      FilteringMode       m_minFilteringMode;
      FilteringMode       m_magFilteringMode;
      EdgeSamplingMode    m_edgeSamplingModeS;
      EdgeSamplingMode    m_edgeSamplingModeT;
      EdgeSamplingMode    m_edgeSamplingModeR;
      glm::uvec3          m_resolution;
      uint32_t         m_arraySize;
      float               m_anisotropicLevel;
      
      static std::map<std::string, kit::Texture::Ptr> m_cachedTextures;
      static std::map<std::string, kit::Texture::Ptr> m_cachedCubemaps;
  };

}

#endif // KIT_TEXTURE_HEADER
