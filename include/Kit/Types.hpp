#ifndef KIT_TYPES_HEADER
#define KIT_TYPES_HEADER

#include "Kit/Export.hpp"
#include "Kit/Exception.hpp"

#include <vector>
#include <map>
#include <list>
#include <glm/glm.hpp>

#ifdef _WIN32
  #include <winsock2.h> // ntohl/htonl
#elif __unix
  #include <arpa/inet.h> // ntohl/htonl
#endif

#ifndef KIT_BIG_ENDIAN
  #define KIT_BIG_ENDIAN (htonl(1)==1)
#endif
#define htonll(x) (KIT_BIG_ENDIAN ? (x) : ( (uint64_t) htonl ((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) (KIT_BIG_ENDIAN ? (x) : ( (uint64_t) ntohl ((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#ifndef KIT_STATIC_DATA
  #ifdef _WIN32
    #define KIT_STATIC_DATA "./static/"
  #elif __unix
    #define KIT_STATIC_DATA "/usr/share/kit/static/"
  #else
    #error Unsupported OS.
  #endif
#endif

#ifndef KIT_DATA
  #define KIT_DATA "./data/"
#endif

#ifndef KIT_EDITOR_DATA
  #define KIT_EDITOR_DATA "./data/editor/"
#endif 

// A bunch of header-only kit defines for opengl.
// This is so that we can use GL constants in header files, and still preserve GL interoperability

// Shaders
#define GLK_VERTEX_SHADER                           0x8B31
#define GLK_FRAGMENT_SHADER                         0x8B30
#define GLK_TESS_EVALUATION_SHADER                  0x8E87
#define GLK_TESS_CONTROL_SHADER                     0x8E88
#define GLK_GEOMETRY_SHADER                         0x8DD9
#define GLK_COMPUTE_SHADER                          0x91B9

// Textures
#define GLK_TEXTURE_1D 0x0DE0
#define GLK_TEXTURE_1D_ARRAY 0x8C18
#define GLK_TEXTURE_BUFFER 0x8C2A
#define GLK_TEXTURE_2D 0x0DE1
#define GLK_TEXTURE_2D_MULTISAMPLE 0x9100
#define GLK_TEXTURE_2D_ARRAY 0x8C1A
#define GLK_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9102
#define GLK_TEXTURE_3D 0x806F
#define GLK_TEXTURE_CUBE_MAP 0x8513
#define GLK_TEXTURE_CUBE_MAP_ARRAY 0x9009
#define GLK_NEAREST 0x2600
#define GLK_LINEAR 0x2601
#define GLK_NEAREST_MIPMAP_NEAREST 0x2700
#define GLK_LINEAR_MIPMAP_NEAREST 0x2701
#define GLK_NEAREST_MIPMAP_LINEAR 0x2702
#define GLK_LINEAR_MIPMAP_LINEAR 0x2703
#define GLK_TEXTURE_WRAP_S 0x2802
#define GLK_TEXTURE_WRAP_T 0x2803
#define GLK_TEXTURE_WRAP_R 0x8072
#define GLK_REPEAT 0x2901
#define GLK_MIRRORED_REPEAT 0x8370
#define GLK_CLAMP_TO_EDGE 0x812F
#define GLK_RED 0x1903
#define GLK_RG 0x8227
#define GLK_RGB 0x1907
#define GLK_RGBA 0x1908
#define GLK_BGR 0x80E0
#define GLK_BGRA 0x80E1
#define GLK_DEPTH_COMPONENT 0x1902
#define GLK_STENCIL_INDEX 0x1901
#define GLK_R8 0x8229
#define GLK_R8_SNORM 0x8F94
#define GLK_R16 0x822A
#define GLK_R16_SNORM 0x8F98
#define GLK_RG8 0x822B
#define GLK_RG8_SNORM 0x8F95
#define GLK_RG16 0x822C
#define GLK_RG16_SNORM 0x8F99
#define GLK_R3_G3_B2 0x2A10
#define GLK_RGB4 0x804F
#define GLK_RGB5 0x8050
#define GLK_RGB8 0x8051
#define GLK_RGB8_SNORM 0x8F96
#define GLK_RGB10 0x8052
#define GLK_RGB12 0x8053
#define GLK_RGB16_SNORM 0x8F9A
#define GLK_RGBA2 0x8055
#define GLK_RGBA4 0x8056
#define GLK_RGB5_A1 0x8057
#define GLK_RGBA8 0x8058
#define GLK_RGBA8_SNORM 0x8F97
#define GLK_RGB10_A2 0x8059
#define GLK_RGB10_A2UI 0x906F
#define GLK_RGBA12 0x805A
#define GLK_RGBA16 0x805B
#define GLK_SRGB8 0x8C41
#define GLK_SRGB8_ALPHA8 0x8C43
#define GLK_R16F 0x822D
#define GLK_RG16F 0x822F
#define GLK_RGB16F 0x881B
#define GLK_RGBA16F 0x881A
#define GLK_R32F 0x822E
#define GLK_RG32F 0x8230
#define GLK_RGB32F 0x8815
#define GLK_RGBA32F 0x8814
#define GLK_R11F_G11F_B10F 0x8C3A
#define GLK_RGB9_E5 0x8C3D
#define GLK_R8I 0x8231
#define GLK_R16I 0x8233
#define GLK_R16UI 0x8234
#define GLK_R32I 0x8235
#define GLK_R32UI 0x8236
#define GLK_RG8I 0x8237
#define GLK_RG8UI 0x8238
#define GLK_RG16I 0x8239
#define GLK_RG16UI 0x823A
#define GLK_RG32I 0x823B
#define GLK_RG32UI 0x823C
#define GLK_RGB8I 0x8D8F
#define GLK_RGB8UI 0x8D7D
#define GLK_RGB16I 0x8D89
#define GLK_RGB16UI 0x8D77
#define GLK_RGB32I 0x8D83
#define GLK_RGB32UI 0x8D71
#define GLK_RGBA8I 0x8D8E
#define GLK_RGBA8UI 0x8D7C
#define GLK_RGBA16I 0x8D88
#define GLK_RGBA16UI 0x8D76
#define GLK_RGBA32I 0x8D82
#define GLK_RGBA32UI 0x8D70
#define GLK_DEPTH_COMPONENT32F 0x8CAC
#define GLK_DEPTH_COMPONENT24 0x81A6
#define GLK_DEPTH_COMPONENT16 0x81A5
#define GLK_DEPTH32F_STENCIL8 0x8CAD
#define GLK_DEPTH24_STENCIL8 0x88F0
#define GLK_STENCIL_INDEX8 0x8D48
#define GLK_BYTE 0x1400
#define GLK_UNSIGNED_BYTE 0x1401
#define GLK_SHORT 0x1402
#define GLK_UNSIGNED_SHORT 0x1403
#define GLK_INT 0x1404
#define GLK_UNSIGNED_INT 0x1405
#define GLK_FLOAT 0x1406
#define GLK_UNSIGNED_BYTE_3_3_2 0x8032
#define GLK_UNSIGNED_BYTE_2_3_3_REV 0x8362
#define GLK_UNSIGNED_SHORT_5_6_5 0x8363
#define GLK_UNSIGNED_SHORT_5_6_5_REV 0x8364
#define GLK_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GLK_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GLK_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GLK_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GLK_UNSIGNED_INT_8_8_8_8 0x8035
#define GLK_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GLK_UNSIGNED_INT_10_10_10_2 0x8036
#define GLK_UNSIGNED_INT_2_10_10_10_REV 0x8368


#include <string>
#include <cstring>
#include <cstdint>

/*
 *  This file says types but it should really be types + utils (or separated to several different files)
 *  Enjoy.
 * 
 */

namespace kit{

  enum class DataSource : uint8_t
  {
    Static,
    Data,
    Editor
  };

  KITAPI std::string getDataDirectory(kit::DataSource source = kit::DataSource::Data);

  struct FileInfo
  {
    std::string path;
    std::string filename;
    enum Type
    {
      File,
      Directory
    } type;
    
    bool operator<(const FileInfo&) const;
  };
  
  // Template functions to convert types from/to network byte order
  // Passed parameter MUST be either 2, 4 or 8 bytes long
  template<typename T>
  T netEnc64(T & in)
  {
    uint64_t a = 0;
    T b;
    memcpy((void*)&a, (void*)&in, sizeof(uint64_t));
    a = htonll(a);
    memcpy((void*)&b, (void*)&a, sizeof(uint64_t));
    return b;
  }

  template<typename T>
  T netDec64(T & in)
  {
    uint64_t a = 0;
    T b;
    memcpy((void*)&a, (void*)&in, sizeof(uint64_t));
    a = ntohll(a);
    memcpy((void*)&b, (void*)&a, sizeof(uint64_t));
    return b;
  }

  template<typename T>
  T netEnc32(T& in)
  {
    uint32_t a = 0;
    T b;
    memcpy((void*)&a, (void*)&in, sizeof(uint32_t));
    a = htonl(a);
    memcpy((void*)&b, (void*)&a, sizeof(uint32_t));
    return b;
  }
  
  template<typename T>
  T netDec32(T& in)
  {
    uint32_t a = 0;
    T b;
    memcpy((void*)&a, (void*)&in, sizeof(uint32_t));
    a = ntohl(a);
    memcpy((void*)&b, (void*)&a, sizeof(uint32_t));
    return b;
  }
  
  template<typename T>
  T netEnc16(T& in)
  {
    uint16_t a = 0;
    T b;
    memcpy((void*)&a, (void*)&in, sizeof(uint16_t));
    a = htons(a);
    memcpy((void*)&b, (void*)&a, sizeof(uint16_t));
    return b;
  }
  
  template<typename T>
  T netDec16(T& in)
  {
    uint16_t a = 0;
    T b;
    memcpy((void*)&a, (void*)&in, sizeof(uint16_t));
    a = ntohs(a);
    memcpy((void*)&b, (void*)&a, sizeof(uint16_t));
    return b;
  }

  // Convenience functions to read/write binary data
  // Each of these functions will automatically convert data from/to network byte-order
  KITAPI int64_t  readInt64(std::istream & s);
  KITAPI void        writeInt64(std::ostream & s, int64_t v);

  KITAPI int32_t  readInt32(std::istream & s);
  KITAPI void        writeInt32(std::ostream & s, int32_t v);

  KITAPI int16_t  readInt16(std::istream & s);
  KITAPI void        writeInt16(std::ostream & s, int16_t v);

  KITAPI int8_t   readInt8(std::istream & s);
  KITAPI void        writeInt8(std::ostream & s, int8_t v);

  KITAPI uint64_t readUint64(std::istream & s);
  KITAPI void        writeUint64(std::ostream & s, uint64_t v);

  KITAPI uint32_t readUint32(std::istream & s);
  KITAPI void        writeUint32(std::ostream & s, uint32_t v);

  KITAPI uint16_t readUint16(std::istream & s);
  KITAPI void        writeUint16(std::ostream & s, uint16_t v);

  KITAPI uint8_t  readUint8(std::istream & s);
  KITAPI void        writeUint8(std::ostream & s, uint8_t v);

  KITAPI float       readFloat(std::istream & s);
  KITAPI void        writeFloat(std::ostream & s, float v);

  KITAPI double      readDouble(std::istream & s);
  KITAPI void        writeDouble(std::ostream & s, double v);

  KITAPI glm::vec2   readVec2(std::istream & s);
  KITAPI void        writeVec2(std::ostream & s, glm::vec2 v);

  KITAPI glm::vec3   readVec3(std::istream & s);
  KITAPI void        writeVec3(std::ostream & s, glm::vec3 v);

  KITAPI glm::vec4   readVec4(std::istream & s);
  KITAPI void        writeVec4(std::ostream & s, glm::vec4 v);

  KITAPI glm::vec4   readVec4i(std::istream & s);
  KITAPI void        writeVec4i(std::ostream & s, glm::ivec4 v);

  KITAPI glm::quat   readQuat(std::istream & s);
  KITAPI void        writeQuat(std::ostream & s, glm::quat v);

  KITAPI glm::mat4   readMat4(std::istream & s);
  KITAPI void        writeMat4(std::ostream & s, glm::mat4 v);

  KITAPI glm::mat3   readMat3(std::istream & s);
  KITAPI void        writeMat3(std::ostream & s, glm::mat3 v);

  KITAPI std::string readString(std::istream & s); // reads null-terminated string
  KITAPI void        writeString(std::ostream & s, const std::string& v);

  KITAPI std::vector<char> readBytes(std::istream & s, uint32_t c);
  KITAPI void        writeBytes(std::ostream & s, std::vector<char> v);

  
  // String manipulation functions
  KITAPI std::string toLower(const std::string&);
  KITAPI bool stringContains(const std::string& needle, const std::string& haystack);
  KITAPI bool isWhitespace(char const & curr);
  KITAPI std::vector<std::string>  splitString(const std::string& source);
  KITAPI std::vector<std::string>  splitString(const std::string& source, std::vector<char> delimiters);
  KITAPI std::string trimLeft(const std::string& source);
  KITAPI std::string trimRight(const std::string& source);
  KITAPI std::string trim(const std::string& source);
  KITAPI std::vector<const char*> toCharArray(std::vector<std::string> const & input);

  // Filesystem queries
  KITAPI bool isDirectory(const std::string& directory);
  KITAPI bool createDirectory(const std::string& directory);
  KITAPI std::vector<FileInfo> listFilesystemEntries(const std::string& directory, bool include_files = true, bool include_directories = true);

  // Math functions
  glm::quat rotationTo(glm::vec3 from, glm::vec3 to, glm::vec3 fallback = glm::vec3(0.0, 0.0, 0.0));
  
  // Randomness
  KITAPI int randomInt(int min, int max);
  KITAPI float randomFloat(float min, float max);
  
  // Ugliest shit you've ever seen
  KITAPI std::wstring stringToWide(const std::string& s);
  KITAPI std::string wideToString(std::wstring ws);

  // Animators
  KITAPI double fadeInOut(double msTime, double delay, double fadetime, double staytime);
  KITAPI double fadeIn(double msTime, double delay, double fadetime);
  KITAPI double pulsate(double ms);

  // Sleeper function
  KITAPI void sleepMs(uint32_t ms);

  // sRGB conversion functions
  KITAPI glm::vec4 srgbEnc(glm::vec4 color);
  KITAPI glm::vec3 srgbEnc(glm::vec3 color);

  KITAPI glm::vec4 srgbDec(glm::vec4 color);
  KITAPI glm::vec3 srgbDec(glm::vec3 color);

  // Converter utility to quickly convert between types based on raw binary data
  struct KITAPI Converter8
  {
    Converter8();
    Converter8(char v);
    Converter8(uint8_t v);
    Converter8(int8_t v);

    union
    {
      uint8_t asUint8;
      int8_t  asInt8;
      char   asByte;
    };
  };

  struct KITAPI Converter16
  {
    Converter16();
    Converter16(char v[2]);
    Converter16(uint16_t v);
    Converter16(int16_t v);

    void networkEncode();
    void networkDecode();

    union
    {
      uint16_t asUint16;
      int16_t  asInt16;
      char   asBytes[2];
    };
  };

  struct KITAPI Converter32
  {
    Converter32();
    Converter32(float v);
    Converter32(char v[4]);
    Converter32(uint32_t v);
    Converter32(int32_t v);

    void networkEncode();
    void networkDecode();

    union
    {
      float    asFloat;
      uint32_t asUint32;
      int32_t  asInt32;
      char     asBytes[4];
    };
  };

  struct KITAPI Converter64
  {
    Converter64();
    Converter64(char v[8]);
    Converter64(uint64_t v);
    Converter64(int64_t v);
    Converter64(double v);

    void networkEncode();
    void networkDecode();

    union
    {
      double      asDouble;
      uint64_t asUint64;
      int64_t  asInt64;
      char   asBytes[8];
    };
  };
  
  struct KITAPI  Vertex
  {
    Vertex();
    glm::vec3      m_position;       
    glm::vec2      m_texCoords;      
    glm::vec3      m_normal;         
    glm::vec3      m_tangent;        
    glm::ivec4     m_boneIDs;        
    glm::vec4      m_boneWeights;    
  };
  
  struct KITAPI  Geometry
  {
    bool load(const std::string& filename);
    bool save(const std::string& filename);
    
    std::vector<kit::Vertex>            m_vertices;
    std::vector<uint32_t>            m_indices;
  };
  
}

#endif // KIT_TYPES_HEADER
