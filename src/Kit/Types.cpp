#include "Kit/Types.hpp"
//#include "vld.h"

#include <cstring> // memcmp

#ifdef _WIN32
	#include <winsock2.h> // ntohl/htonl
	#include <Windows.h> // Sleep, dir iterations
#elif __unix
	#include <arpa/inet.h> // ntohl/htonl
    #include <dirent.h>   // opendir/readdir
    #include <sys/types.h> // DIR
    #include <unistd.h>  // usleep
    
    // For directory stat and creation
    #include <sys/stat.h>

#endif

#ifdef _WIN32

#else

#endif

#include <fstream>
#include <algorithm>
#include <random>
#include <glm/gtc/quaternion.hpp>

bool kit::isDirectory(const std::string& directory)
{
  std::cout << "Checking if directory " << directory << std::endl;
#ifdef __unix
  struct stat buff;
  int r = stat(directory.c_str(), &buff);

  if(r != 0)
  {
    KIT_ERR("Failed to check if directory");
    return false;
  }
  
  return S_ISDIR(buff.st_mode);
#elif _WIN32
  DWORD dwAttrib = GetFileAttributes(directory.c_str());

  if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
  {
    return true;
  }
  else
  {
    return false;
  }
#endif
}

bool kit::createDirectory(const std::string& directory)
{
  std::cout << "Creating directory" << directory << std::endl;
#ifdef __unix
  int r = mkdir(directory.c_str(), 0777);
  if(r != 0 && !(errno == EEXIST && kit::isDirectory(directory)))
  {
    KIT_ERR("Failed to create directory");
    return false;
  }
  
  return true;
#elif _WIN32
  // Check if directory before creating it, because Win32 API is inconvenient
  if (kit::isDirectory(directory))
  {
    return true;
  }
  
  if (CreateDirectory(directory.c_str(), NULL) != 0)
  {
    return true;
  }
  else{
    KIT_ERR("Failed to create directory");
    return false;
  }
#endif
}

std::vector<const char*> kit::toCharArray(std::vector<std::string> const & input)
{
  std::vector<const char*> returner;
  for (auto & currString : input)
  {
    returner.push_back(currString.c_str());
  }

  return returner;
}

bool kit::FileInfo::operator<(const FileInfo& rhs) const
{
  // Sort directories before files
  if(this->type != rhs.type)
  {
    return (this->type == Directory ? true : false);
  }
  
  // Sort alphabetically
  return this->filename < rhs.filename;
  
}


std::vector<kit::FileInfo> kit::listFilesystemEntries(const std::string& path, bool include_files, bool include_dirs)
{
  std::cout << "Reading directory " << path << std::endl;
#ifdef _WIN32
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  std::string spec = path + "/*";
  std::vector<kit::FileInfo> returner;
  hFind = FindFirstFile(spec.c_str(), &ffd);
  if (hFind == INVALID_HANDLE_VALUE) {
    KIT_ERR("Warning: Tried to iterate invalid path");
    return returner;
  }

  do
  {
    if (ffd.cFileName != std::string(".") && ffd.cFileName != std::string("..")) 
    {
      kit::FileInfo creator;
      creator.filename = ffd.cFileName;
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        creator.type = kit::FileInfo::Directory;
      }
      else
      {
        creator.type = kit::FileInfo::File;
      }
      creator.path = path;

      if ((creator.type == kit::FileInfo::Directory && include_dirs) || creator.type == kit::FileInfo::File && include_files)
      {
        returner.push_back(creator);
      }
    }
  }
  while (FindNextFile(hFind, &ffd) != 0);

  if (GetLastError() != ERROR_NO_MORE_FILES) {
    KIT_ERR("Warning: Got error when iterating path");
    FindClose(hFind);
    return returner;
  }

  FindClose(hFind);
  hFind = INVALID_HANDLE_VALUE;

  return returner;

#elif __unix__
  std::vector<kit::FileInfo> returner;
  DIR *dp;
  struct dirent *dirp;
  if((dp  = opendir(path.c_str())) == NULL) 
  {
    KIT_ERR("Warning: Tried to iterate invalid path");
    return returner;
  }

  while ((dirp = readdir(dp)) != NULL)
  {
    kit::FileInfo creator;
    
    creator.path = path;
    
    if(dirp->d_type == DT_REG)
    {
      creator.type = kit::FileInfo::File;
    }
    else if(dirp->d_type == DT_DIR)
    {
      creator.type = kit::FileInfo::Directory;
    }
    else
    {
      continue;
    }
    
    if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
    {
      continue;
    }
    
    creator.filename = dirp->d_name;
    
    if ((creator.type == kit::FileInfo::Directory && include_dirs) || (creator.type == kit::FileInfo::File && include_files))
    {
      returner.push_back(creator);
    }
  }
  
  closedir(dp);
  
  std::sort(returner.begin(), returner.end());
  
  return returner;
#endif
}

glm::quat kit::rotationTo(glm::vec3 from, glm::vec3 to, glm::vec3 fallback)
{
  glm::quat q;
  
  glm::vec3 v0 = glm::normalize(from);
  glm::vec3 v1 = glm::normalize(to);
  
  float d = glm::dot(v0, v1);
  if(d >= 1.0f)
  {
    return glm::quat();
  }
  
  if(d < (1e-6f - 1.0f))
  {
    if(fallback != glm::vec3(0.0, 0.0, 0.0))
    {
      q = glm::angleAxis(glm::radians(glm::pi<float>()), glm::normalize(fallback));
    }
    else
    {
      glm::vec3 axis = glm::cross(from, glm::vec3(1.0, 0.0, 0.0));
      if(glm::length(axis) == 0.0f)
      {
        axis = glm::cross(from, glm::vec3(0.0, 1.0, 0.0));
      }
      axis = glm::normalize(axis);
      q = glm::angleAxis(glm::radians(glm::pi<float>()), axis);
    }
  }
  else
  {
    float s = glm::sqrt(1.0f + d)*2.0f;
    float invs = 1.0f / s;
    glm::vec3 c = glm::cross(v0, v1);
    q.x = c.x * invs;
    q.y = c.y * invs;
    q.z = c.z * invs;
    q.w = s * 0.5f;
    q = glm::normalize(q);
  }
  
  return q;
}

void kit::sleepMs(uint32_t ms)
{
  #ifdef _WIN32
    //Sleep(ms);
  #else
    usleep(ms*1000);  /* sleep for 100 milliSeconds */
  #endif
}

int kit::randomInt(int min, int max)
{
  static std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(min, max);
  int dice_roll = distribution(generator);
  return dice_roll;
}

float kit::randomFloat(float min, float max)
{
  static std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(min, max);
  float dice_roll = distribution(generator);
  return dice_roll;
}


// String manipulators

std::string kit::trimLeft(const std::string& source)
{
  std::string str = source;
  size_t startpos = str.find_first_not_of(" \f\n\r\t\v");
  if( std::string::npos != startpos )
  {
      str = str.substr( startpos );
  }
  return str;
}

std::string kit::trimRight(const std::string& source)
{
  std::string str = source;
  size_t endpos = str.find_last_not_of(" \f\n\r\t\v");
  if( std::string::npos != endpos )
  {
      str = str.substr( 0, endpos+1 );
  }
  return str;
}

std::string kit::trim(const std::string& source)
{
  return kit::trimLeft(kit::trimRight(source));
}

std::wstring kit::stringToWide(const std::string& s)
{
  std::wstring ws;
  ws.assign(s.begin(), s.end());
  return ws;
}

std::string kit::wideToString(std::wstring ws)
{
  std::string s;
  s.assign(ws.begin(), ws.end());
  return s;
}

bool kit::isWhitespace(char const & curr)
{
  return (curr == ' ' || curr == '\f' || curr == '\n' || curr == '\r' || curr == '\t' || curr == '\v');
}

std::vector<std::string> kit::splitString(const std::string& source)
{
  // Create a vector (list) with words to return
  std::vector<std::string> returner;

  // If the length of the source string is 0, return the empty list
  if (source.size() < 1)
  {
    return returner;
  }

  // Setup some variables we will need when we parse the string
  unsigned int currPosition = 0; // Current position in string
  std::string currArg = "";      // Current argument (we fill this with characters, and copy it on splits, then we reset it)
  bool isQuoted = false;         // If the current character is inside quotation marks

                                 // Iterate the source buffer, character by character
  while (currPosition < source.size())
  {
    // Create a copy of the character at the current position in the string
    char currChar = source[currPosition];

    // If the current character is a quotationmark
    if (currChar == '"')
    {
      // Flip the isQuoted-bool, so it's the opposite of what it was
      isQuoted = !isQuoted;

      // If currArg is longer than 0 characters, add a copy to the list and reset currArg to "" (to prepare for the next argument)
      if (currArg.size() > 0)
      {
        returner.push_back(currArg);
        currArg = "";
      }
    }
    // Otherwise, if the current character isn't a quotation mark
    else
    {
      // If the current character is inside a quotation mark, add it to the current argument
      if (isQuoted)
      {
        currArg.push_back(currChar);
      }
      // Otherwise,
      else
      {
        // If the current character is a # (while not in  quotes), add the current argument to the list and return the list
        // (we are done here, since a # outside of quotes tells us the rest of the line is a comment)
        if (currChar == '#')
        {
          if (currArg.size() > 0)
          {
            returner.push_back(currArg);
          }

          return returner;
        }

        // If the current character is whitespace, add the current argument to the list and reset currArg
        if (kit::isWhitespace(currChar))
        {
          if (currArg.size() > 0)
          {
            returner.push_back(currArg);
            currArg = "";
          }
        }
        else
          // Otherwise add the current character to the current argument
        {
          currArg.push_back(currChar);
        }
      }
    }

    // Step forward in the source string
    currPosition++;
  }

  // We have now iterated through our source string
  // Add the remaining argument to our list and then return it
  if (currArg.size() > 0)
  {
    returner.push_back(currArg);
  }

  return returner;
}

std::vector<std::string> kit::splitString(const std::string& source, std::vector<char> delimiters)
{
  std::vector<std::string> returner;
  if (source.size() < 1)
  {
    return returner;
  }

  unsigned int currPosition = 0;
  std::string currstr = "";
  while (currPosition < source.size())
  {
    char curr = source[currPosition];

    if (std::find(delimiters.begin(), delimiters.end(), curr) != delimiters.end())
    {
      if (currstr.size() > 0)
      {
        returner.push_back(currstr);
        currstr = "";
      }
    }
    else
    {
      currstr.push_back(curr);
    }

    currPosition++;
  }
  if (currstr.size() > 0)
  {
    returner.push_back(currstr);
  }

  return returner;
}

std::string kit::toLower(const std::string& in)
{
  std::string out = in;
  std::transform(out.begin(), out.end(), out.begin(), ::tolower);
  return out;
}

bool kit::stringContains(const std::string& needle, const std::string& haystack)
{
  return (std::strstr((char*)haystack.c_str(), needle.c_str()) != nullptr);
}


// Animators

double kit::fadeInOut(double msTime, double delay, double fadetime, double staytime)
{
  double msFadeIn = delay;
  double msVisible = delay + fadetime;
  double msFadeOut = delay + fadetime + staytime;
  double msInvisible = delay + fadetime + staytime + fadetime;

  if (msTime >= msFadeIn && msTime < msVisible)
  {
    return 1.0 - ((msVisible - msTime) / (msVisible - msFadeIn));
  }
  else if (msTime >= msVisible && msTime < msFadeOut)
  {
    return 1.0;
  }
  else if (msTime >= msFadeOut && msTime < msInvisible)
  {
    return ((msInvisible - msTime) / (msInvisible - msFadeOut));
  }

  return 0.0;
}

double kit::fadeIn(double msTime, double delay, double fadetime)
{
  double msFadeIn = delay;
  double msVisible = delay + fadetime;

  if (msTime >= msFadeIn && msTime < msVisible)
  {
    return 1.0 - ((msVisible - msTime) / (msVisible - msFadeIn));
  }
  else if (msTime >= msVisible)
  {
    return 1.0;
  }

  return 0.0;
}

double kit::pulsate(double ms)
{
  return (sin(ms*0.001) + 1.0) / 2.0;
}


// Converters

kit::Converter8::Converter8()
{
  this->asUint8 = 0;
}

kit::Converter8::Converter8(char b)
{
  this->asByte = b;
}

kit::Converter8::Converter8(uint8_t ui)
{
  this->asUint8 = ui;
}

kit::Converter8::Converter8(int8_t i)
{
  this->asInt8 = i;
}

kit::Converter16::Converter16()
{
  this->asUint16 = 0;
}

kit::Converter16::Converter16(char * b)
{
  memcpy(this->asBytes, b, 2);
}

kit::Converter16::Converter16(uint16_t ui)
{
  this->asUint16 = ui;
}

kit::Converter16::Converter16(int16_t i)
{
  this->asInt16 = i;
}

void kit::Converter16::networkEncode()
{
  this->asUint16 = kit::netEnc16(this->asUint16);
}

void kit::Converter16::networkDecode()
{
  this->asUint16 = kit::netDec16(this->asUint16);
}

kit::Converter32::Converter32()
{
  this->asUint32 = 0;
}

kit::Converter32::Converter32(float f)
{
  this->asFloat = f;
}

kit::Converter32::Converter32(char * b)
{
  memcpy(this->asBytes, b, 4);
}

kit::Converter32::Converter32(uint32_t ui)
{
  this->asUint32 = ui;
}

kit::Converter32::Converter32(int32_t i)
{
  this->asInt32 = i;
}

void kit::Converter32::networkEncode()
{
  this->asUint32 = kit::netEnc32(this->asUint32);
}

void kit::Converter32::networkDecode()
{
  this->asUint32 = kit::netDec32(this->asUint32);
}

kit::Converter64::Converter64()
{
  this->asUint64 = 0;
}

kit::Converter64::Converter64(double d)
{
  this->asDouble = d;
}

kit::Converter64::Converter64(char * b)
{
  memcpy(this->asBytes, b, 8);
}

kit::Converter64::Converter64(uint64_t ui)
{
  this->asUint64 = ui;
}

kit::Converter64::Converter64(int64_t i)
{
  this->asInt64 = i;
}

void kit::Converter64::networkEncode()
{
  this->asUint64 = kit::netEnc64(this->asUint64);
}

void kit::Converter64::networkDecode()
{
  this->asUint64 = kit::netDec64(this->asUint64);
}


/*

Geometry layout

vertex
{
  float3 position     12 bytes
  float2 uv           8 bytes
  float3 normal       12 bytes
  float3 tangent      12 bytes
  uint4 boneid        16 bytes
  float4 boneweight   16 bytes
} 76 bytes


4 bytes       Signature                 KGEO
4 bytes       Index count     $ic       uint32
4 bytes       Vertex count    $vc       uint32
$ic*4 bytes   Indices                   uint32[$ic]
$vc*76 bytes  Vertices                  vertex[$vc]


Convex hull layout

vertex
{
  float3 position     12 bytes
} 12 bytes

4 bytes       Signature                 KCON
4 bytes       Vertex count    $vc       uint32
$vc*12 bytes  Vertices                  vertex[$vc]

*/

bool kit::Geometry::load(const std::string& filename)
{
  this->m_vertices.clear();
  this->m_indices.clear();
  
  std::ifstream s(filename.c_str(), std::ios::in | std::ios::binary);
  
  if(!s)
  {
    std::cout << "ERROR: couldn't open file \"" << filename.c_str() << "\" for reading" << std::endl;
    return false;
  }

  
  // Verify signature
  static std::vector<char> sig_ref = {'K', 'G', 'E', 'O'};
  std::vector<char> sig_read = kit::readBytes(s, 4);

  if(sig_ref != sig_read)
  {
    std::cout << "ERROR: bad signature" << std::endl;
    s.close();
    return false;
  }
  
  // Load the data

  // ... Index count
  uint32_t numIndices = kit::readUint32(s);
  
  // ... Vertex count
  uint32_t numVertices = kit::readUint32(s);
  
  // ... Indices
  for(uint32_t i = 0; i < numIndices; i++)
  {
    this->m_indices.push_back(kit::readUint32(s));
  }
  
  // ... Vertices
  for(uint32_t i = 0; i < numVertices; i++)
  {
    kit::Vertex currVertex;
    currVertex.m_position = kit::readVec3(s);
    currVertex.m_texCoords = kit::readVec2(s);
    currVertex.m_normal = kit::readVec3(s);
    currVertex.m_tangent = kit::readVec3(s);
    currVertex.m_boneIDs = kit::readVec4i(s);
    currVertex.m_boneWeights = kit::readVec4(s);
    
    this->m_vertices.push_back(currVertex);
  }
  
  // All done, close handle and return true!
  s.close();
  
  return true;
}

bool kit::Geometry::save(const std::string& filename)
{
  // Open file
  std::ofstream s(filename.c_str(), std::ios::out | std::ios::binary);
  if(!s)
  {
    return false;
  }

  // Write header
  kit::writeBytes(s, { 'K', 'G', 'E', 'O' });
  kit::writeUint32(s, (uint32_t)this->m_indices.size());
  kit::writeUint32(s, (uint32_t)this->m_vertices.size());

  // Write indices
  for (auto & currIndex : this->m_indices)
  {
    kit::writeUint32(s, currIndex);
  }

  // Write vertices
  for (auto & currVertex : this->m_vertices)
  {
    kit::writeVec3(s, currVertex.m_position);
    kit::writeVec2(s, currVertex.m_texCoords);
    kit::writeVec3(s, currVertex.m_normal);
    kit::writeVec3(s, currVertex.m_tangent);
    kit::writeVec4i(s, currVertex.m_boneIDs);
    kit::writeVec4(s, currVertex.m_boneWeights);
  }

  // All done, close handle and return true
  s.close();

  return true;
}

kit::Vertex::Vertex()
{
  this->m_boneIDs = glm::ivec4(0, 0, 0, 0);
  this->m_boneWeights = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  this->m_normal = glm::vec3(0.0f, 0.0f, 0.0f);
  this->m_position = glm::vec3(0.0f, 0.0f, 0.0f);
  this->m_tangent = glm::vec3(0.0f, 0.0f, 0.0f);
  this->m_texCoords = glm::vec2(0.0f, 0.0f);
}

glm::vec4 kit::srgbEnc(glm::vec4 color)
{
  glm::vec3 invec = glm::vec3(color.x, color.y, color.z);
  invec = glm::pow(invec, glm::vec3(2.2f));
  return glm::vec4(invec.x, invec.y, invec.z, color.w);
}

glm::vec3 kit::srgbEnc(glm::vec3 color)
{
  return glm::pow(color, glm::vec3(2.2f));
}

glm::vec4 kit::srgbDec(glm::vec4 color)
{
  glm::vec3 invec = glm::vec3(color.x, color.y, color.z);
  invec = glm::pow(invec, glm::vec3(1.0f / 2.2f));
  return glm::vec4(invec.x, invec.y, invec.z, color.w);
}

glm::vec3 kit::srgbDec(glm::vec3 color)
{
  return glm::pow(color, glm::vec3(1.0f/2.2f));
}


// Read/write

int64_t kit::readInt64(std::istream & s)
{
  kit::Converter64 c;
  s.read(&c.asBytes[0], 8);
  c.networkDecode();
  return c.asInt64;
}

void kit::writeInt64(std::ostream & s, int64_t v)
{
  kit::Converter64 c(v);
  c.networkEncode();
  s.write(&c.asBytes[0], 8);
}

int32_t kit::readInt32(std::istream & s)
{
  kit::Converter32 c;
  s.read(&c.asBytes[0], 4);
  c.networkDecode();
  return c.asInt32;
}

void kit::writeInt32(std::ostream & s, int32_t v)
{
  kit::Converter32 c(v);
  c.networkEncode();
  s.write(&c.asBytes[0], 4);
}

int16_t kit::readInt16(std::istream & s)
{
  kit::Converter16 c;
  s.read(&c.asBytes[0], 2);
  c.networkDecode();
  return c.asInt16;
}

void kit::writeInt16(std::ostream & s, int16_t v)
{
  kit::Converter16 c(v);
  c.networkEncode();
  s.write(&c.asBytes[0], 2);
}

int8_t kit::readInt8(std::istream & s)
{
  kit::Converter8 c;
  s.read(&c.asByte, 1);
  return c.asInt8;
}

void kit::writeInt8(std::ostream & s, int8_t v)
{
  kit::Converter8 c(v);
  s.write(&c.asByte, 1);
}

uint64_t kit::readUint64(std::istream & s)
{
  kit::Converter64 c;
  s.read(&c.asBytes[0], 8);
  c.networkDecode();
  return c.asUint64;
}

void kit::writeUint64(std::ostream & s, uint64_t v)
{
  kit::Converter64 c(v);
  c.networkEncode();
  s.write(&c.asBytes[0], 8);
}

uint32_t kit::readUint32(std::istream & s)
{
  kit::Converter32 c;
  s.read(&c.asBytes[0], 4);
  c.networkDecode();
  return c.asUint32;
}

void kit::writeUint32(std::ostream & s, uint32_t v)
{
  kit::Converter32 c(v);
  c.networkEncode();
  s.write(&c.asBytes[0], 4);
}

uint16_t kit::readUint16(std::istream & s)
{
  kit::Converter16 c;
  s.read(&c.asBytes[0], 2);
  c.networkDecode();
  return c.asUint16;
}

void kit::writeUint16(std::ostream & s, uint16_t v)
{
  kit::Converter16 c(v);
  c.networkEncode();
  s.write(&c.asBytes[0], 2);
}

uint8_t kit::readUint8(std::istream & s)
{
  kit::Converter8 c;
  s.read(&c.asByte, 1);
  return c.asUint8;
}

void kit::writeUint8(std::ostream & s, uint8_t v)
{
  kit::Converter8 c(v);
  s.write(&c.asByte, 1);
}

float kit::readFloat(std::istream & s)
{
  kit::Converter32 c;
  s.read(&c.asBytes[0], 4);
  c.networkDecode();
  return c.asFloat;
}

void kit::writeFloat(std::ostream & s, float v)
{
  kit::Converter32 c(v);
  c.networkEncode();
  s.write(&c.asBytes[0], 4);
}

double kit::readDouble(std::istream & s)
{
  kit::Converter64 c;
  s.read(&c.asBytes[0], 8);
  c.networkDecode();
  return c.asDouble;
}

void kit::writeDouble(std::ostream & s, double v)
{
  kit::Converter64 c(v);
  c.networkEncode();
  s.write(&c.asBytes[0], 8);
}

glm::vec2 kit::readVec2(std::istream & s)
{
  glm::vec2 v;
  for (int x = 0; x < 2; x++)
  {
    v[x] = readFloat(s);
  }

  return v;
}

void kit::writeVec2(std::ostream & s, glm::vec2 v)
{
  for (int x = 0; x < 2; x++)
  {
    kit::writeFloat(s, v[x]);
  }
}

glm::vec3 kit::readVec3(std::istream & s)
{
  glm::vec3 v;
  for (int x = 0; x < 3; x++)
  {
    v[x] = readFloat(s);
  }

  return v;
}

void kit::writeVec3(std::ostream & s, glm::vec3 v)
{
  for (int x = 0; x < 3; x++)
  {
    kit::writeFloat(s, v[x]);
  }
}

glm::vec4 kit::readVec4(std::istream & s)
{
  glm::vec4 v;
  for (int x = 0; x < 4; x++)
  {
    v[x] = readFloat(s);
  }

  return v;
}

glm::vec4 kit::readVec4i(std::istream & s)
{
  glm::ivec4 v;
  for (int x = 0; x < 4; x++)
  {
    v[x] = readUint32(s);
  }

  return v;
}

void kit::writeVec4(std::ostream & s, glm::vec4 v)
{
  for (int x = 0; x < 4; x++)
  {
    kit::writeFloat(s, v[x]);
  }
}

void kit::writeVec4i(std::ostream & s, glm::ivec4 v)
{
  for (int x = 0; x < 4; x++)
  {
    kit::writeUint32(s, v[x]);
  }
}

glm::quat kit::readQuat(std::istream & s)
{
  glm::quat v;
  for (int x = 0; x < 4; x++)
  {
    v[x] = readFloat(s);
  }

  return v;
}

void kit::writeQuat(std::ostream & s, glm::quat v)
{
  for (int x = 0; x < 4; x++)
  {
    kit::writeFloat(s, v[x]);
  }
}

glm::mat4 kit::readMat4(std::istream & s)
{
  glm::mat4 v;
  for (int x = 0; x < 4; x++)
  {
    for (int y = 0; y < 4; y++)
    {
      v[x][y] = readFloat(s);
    }
  }
  return v;
}

void kit::writeMat4(std::ostream & s, glm::mat4 v)
{
  for (int x = 0; x < 4; x++)
  {
    for (int y = 0; y < 4; y++)
    {
      kit::writeFloat(s, v[x][y]);
    }
  }
}

glm::mat3 kit::readMat3(std::istream & s)
{
  glm::mat3 v;
  for (int x = 0; x < 3; x++)
  {
    for (int y = 0; y < 3; y++)
    {
      v[x][y] = readFloat(s);
    }
  }
  return v;
}

void kit::writeMat3(std::ostream & s, glm::mat3 v)
{
  for (int x = 0; x < 3; x++)
  {
    for (int y = 0; y < 3; y++)
    {
      kit::writeFloat(s, v[x][y]);
    }
  }
}

std::string kit::readString(std::istream & s)
{
  std::string v = "";
  char r;
  s.get(r);

  while (r != '\0')
  {
    v.push_back(r);
    s.get(r);
  }

  return v;
}

void kit::writeString(std::ostream & s, const std::string& v)
{
  s.write(v.c_str(), v.size());
  s.put('\0');
}

std::vector<char> kit::readBytes(std::istream & s, uint32_t c)
{
  std::vector<char> r;
  char * b = new char[c];
  s.read(b, c);
  
  for (uint32_t i = 0; i < c; i++)
  {
    r.push_back(b[i]);
  }

  delete[] b;

  return r;
}

void kit::writeBytes(std::ostream & s, std::vector<char> v)
{
  s.write(&v[0], v.size());
}
