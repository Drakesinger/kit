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

#include <string>
#include <cstring>
#include <cstdint>

/*
 *  This file says types but it should really be types + utils (or separated to several different files)
 *  Enjoy.
 * 
 */

namespace kit{

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
  KITAPI void        writeString(std::ostream & s, std::string v);

  KITAPI std::vector<char> readBytes(std::istream & s, uint32_t c);
  KITAPI void        writeBytes(std::ostream & s, std::vector<char> v);

  
  // String manipulation functions
  KITAPI std::string toLower(std::string);
  KITAPI bool stringContains(std::string needle, std::string haystack);
  KITAPI bool isWhitespace(char const & curr);
  KITAPI std::vector<std::string>  splitString(std::string source);
  KITAPI std::vector<std::string>  splitString(std::string source, std::vector<char> delimiters);
  KITAPI std::string trimLeft(std::string source);
  KITAPI std::string trimRight(std::string source);
  KITAPI std::string trim(std::string source);
  KITAPI std::vector<const char*> toCharArray(std::vector<std::string> const & input);

  // Filesystem queries
  KITAPI bool isDirectory(std::string directory);
  KITAPI bool createDirectory(std::string directory);
  KITAPI std::vector<FileInfo> listFilesystemEntries(std::string directory, bool include_files = true, bool include_directories = true);

  // Math functions
  glm::quat rotationTo(glm::vec3 from, glm::vec3 to, glm::vec3 fallback = glm::vec3(0.0, 0.0, 0.0));
  
  // Randomness
  KITAPI int randomInt(int min, int max);
  KITAPI float randomFloat(float min, float max);
  
  // Ugliest shit you've ever seen
  KITAPI std::wstring stringToWide(std::string s);
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
    bool load(std::string filename);
    bool save(std::string filename);
    
    std::vector<kit::Vertex>            m_vertices;
    std::vector<uint32_t>            m_indices;
  };
  
}

#endif // KIT_TYPES_HEADER
