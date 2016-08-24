#include "Kit/Font.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Types.hpp"
#include "Kit/Texture.hpp"

#include <string>
#include <locale>
#include <codecvt>


FT_Library kit::Font::m_ftLibrary = FT_Library();
uint32_t kit::Font::m_instanceCount = 0;

kit::Font::Ptr kit::Font::m_systemFont = nullptr;

// What characters to cache glyphs for. Any character not in this string will not be renderable using Kit
// Currently contains every visible character on a standard swedish qwerty-keyboard
const std::wstring glyphData = L" –ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖabcdefghijklmnopqrstuvwxyzåäö0123456789§½¶!¡\"@#£¤$%€&¥/{([)]=}?\\+`´±¨~^'´*-_.:·,;¸µ€<>|";

kit::Font::GlyphMap::GlyphMap(kit::Font::WPtr wfont, float size)
{
  
  kit::Font::Ptr font = wfont.lock();
  if(!font)
  {
    KIT_THROW("Invalid font passed");
  }
  
  FT_Set_Pixel_Sizes(font->getFTFace(), 0, (FT_UInt)size);
  
  // First iterate through all the characters to get the max possible size
  glm::uvec2 maxsize(0,0);
  for(wchar_t const & currChar : glyphData)
  {
    if(FT_Load_Char(font->getFTFace(), currChar, FT_LOAD_RENDER) != 0) {
      KIT_THROW("Could not load character from font");
    }
    
    FT_GlyphSlot g = font->getFTFace()->glyph;
    
    if(maxsize.x < g->bitmap.width)
    {
      maxsize.x = g->bitmap.width;
    }
    
    if(maxsize.y < g->bitmap.rows)
    {
      maxsize.y = g->bitmap.rows;
    }
  }
  uint32_t cellsize = (glm::max)(maxsize.x, maxsize.y);
  
  // Get the square root of the number of glyphs, to calculate the size of a square grid needed to take all glyphs
  uint32_t gridsize = (uint32_t)glm::ceil(glm::sqrt(float(glyphData.size())));
  
  // Calculate the total grid size in pixels
  float gridsizepx = glm::ceil(float(gridsize) * float(cellsize));
  
  // Create a texture to hold our glyphs, and initialize it to be fully black
  this->m_texture = kit::Texture::create2D(glm::uvec2((uint32_t)gridsizepx, (uint32_t)gridsizepx), kit::Texture::R8, Texture::ClampToEdge, Texture::Nearest, Texture::Nearest);
  this->m_texture->bind();
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  // AMD DSA texture bug no.2
  std::vector<GLubyte> data(uint32_t(gridsizepx*gridsizepx), 0);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)gridsizepx, (GLsizei)gridsizepx, GL_RED, GL_UNSIGNED_BYTE, &data[0]);
  
  //std::vector<GLubyte> data(uint32_t(gridsizepx*gridsizepx), 0);
  //glTextureSubImage2D(this->m_texture->getHandle(), 0, 0, 0, (GLsizei)gridsizepx, (GLsizei)gridsizepx, GL_RED, GL_UNSIGNED_BYTE, &data[0]);
  
  glm::vec2 currTexPos(0.0f, 0.0f);
  currTexPos.y = gridsizepx - cellsize;
  for(wchar_t const & currChar : glyphData)
  {
    if(FT_Load_Char(font->getFTFace(), currChar, FT_LOAD_RENDER) != 0) {
      KIT_THROW("Could not load character from font");
    }
    
    FT_GlyphSlot g = font->getFTFace()->glyph;
    
    kit::Font::Glyph adder;
    adder.m_size.x = g->bitmap.width;
    adder.m_size.y = g->bitmap.rows;
    adder.m_placement.x = g->bitmap_left;
    adder.m_placement.y = g->bitmap_top;
    adder.m_advance.x = float(g->advance.x >> 6);
    adder.m_advance.y = float(g->advance.y >> 6);
    

    if (!(g->bitmap.width == 0 && g->bitmap.rows == 0))
    {
      adder.m_uv.x = (currTexPos.x / gridsizepx);
      adder.m_uv.y = (currTexPos.y / gridsizepx);
      adder.m_uv.z = ((currTexPos.x + float(adder.m_size.x)) / gridsizepx);
      adder.m_uv.w = ((currTexPos.y + float(adder.m_size.y)) / gridsizepx);

      // We need to flip the data for OpenGL
      std::vector<GLubyte> glyphdata(adder.m_size.x * adder.m_size.y, 0);
      for (uint32_t x = 0; x < adder.m_size.x; x++)
      {
        for (uint32_t y = 0; y < adder.m_size.y; y++)
        {
	  glyphdata[(adder.m_size.x * y) + x] = g->bitmap.buffer[(adder.m_size.x * y) + x ];
	}
      }
      // Upload the data to GPU
      
      // Non-DSA
      glTexSubImage2D(GL_TEXTURE_2D, 0, uint32_t(currTexPos.x), uint32_t(currTexPos.y), adder.m_size.x, adder.m_size.y, GL_RED, GL_UNSIGNED_BYTE, &glyphdata[0]);
      
      // DSA bug no.2 -- AMD Linux still bugged
      //glTextureSubImage2D(this->m_texture->getHandle(), 0, uint32_t(currTexPos.x), uint32_t(currTexPos.y), adder.m_size.x, adder.m_size.y, GL_RED, GL_UNSIGNED_BYTE, &glyphdata[0]);
      //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      //glPixelStorei(GL_PACK_ALIGNMENT, 4);

      currTexPos.x += cellsize;
      if (currTexPos.x >= gridsizepx)
      {
        currTexPos.x = 0;
        currTexPos.y -= cellsize;
      }
    }
    else
    {
      adder.m_uv.x = 0;
      adder.m_uv.y = 0;
      adder.m_uv.z = 0;
      adder.m_uv.w = 0;
    }

    this->m_glyphIndex[currChar] = adder;

  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  
  // AMD DSA texture bug no.1 (reported to AMD, repro here: https://gist.github.com/haikarainen/97959adfe4e3ca10968a)
  this->m_texture->setMinFilteringMode(kit::Texture::Nearest);
  this->m_texture->setMagFilteringMode(kit::Texture::Nearest);

  this->m_lineHeight = float(font->getFTFace()->height) / 64.0f;
  //this->m_height = font->getFTFace()->size->metrics.height / 64.0f;
  this->m_height = (float)this->m_glyphIndex['X'].m_size.y;
  
}

const float & kit::Font::GlyphMap::getLineHeight()
{
  return this->m_lineHeight; 
}

const float & kit::Font::GlyphMap::getHeight()
{
  return this->m_height; 
}

float kit::Font::GlyphMap::getLineAdvance()
{
  return this->m_lineHeight;// + this->m_height;
}

kit::Font::Glyph const & kit::Font::GlyphMap::getGlyph(wchar_t character)
{
  if(this->m_glyphIndex.find(character) == this->m_glyphIndex.end())
  {
    return this->m_glyphIndex[wchar_t('?')];
  }
  
  return this->m_glyphIndex[character];
}

kit::Texture::WPtr kit::Font::GlyphMap::getTexture()
{
  return this->m_texture;
}

kit::Font::Ptr kit::Font::load(const std::string& filename, char const * dataDirectory)
{
  kit::Font::Ptr returner = std::make_shared<kit::Font>();

  std::string path = std::string(dataDirectory + std::string("/fonts/") + filename);
  std::cout << "Loading font from file " << path << std::endl;
  
  if(FT_New_Face(kit::Font::m_ftLibrary, path.c_str(), 0, &returner->m_ftFace) != 0)
  {
    KIT_THROW("Failed to load font from file");
  }
  
  if(FT_Select_Charmap(returner->m_ftFace,  FT_ENCODING_UNICODE))
  {
    KIT_THROW("Failed to set encoding");
  }
    
  
  returner->m_ftLoaded = true;
  return returner;
}

void kit::Font::precacheSize(float size)
{
  if(this->m_glyphCache.size() == 0 || this->m_glyphCache.find(size) == this->m_glyphCache.end())
  {
    this->m_glyphCache[size] = new kit::Font::GlyphMap(this->shared_from_this(), size);
  }
}

kit::Font::GlyphMap * kit::Font::getGlyphMap(float size)
{
  this->precacheSize(size);
  return this->m_glyphCache[size];
}

kit::Font::Font()
{
  this->m_instanceCount++;
  
  this->m_ftLoaded = false;
  
  if(this->m_instanceCount == 1)
  {
    if(FT_Init_FreeType(&kit::Font::m_ftLibrary)) {
      KIT_THROW("Could not initialize Freetype");
    }
  }
}

kit::Font::~Font()
{
  this->m_instanceCount--;
  
  if(this->m_ftLoaded)
  {
    if(FT_Done_Face(this->m_ftFace))
    {
      KIT_ERR("Failed to release font");
    }
  }
  
  for (auto &currCacheEntry : this->m_glyphCache)
  {
    delete currCacheEntry.second;
  }

  if(this->m_instanceCount == 0)
  {
    if(FT_Done_FreeType(kit::Font::m_ftLibrary)) {
      KIT_ERR("Could not destroy Freetype");
    }
  }
}

FT_Face & kit::Font::getFTFace()
{
  return m_ftFace;
}

kit::Font::Ptr kit::Font::getSystemFont()
{
  if(kit::Font::m_systemFont == nullptr)
  {
    kit::Font::m_systemFont = kit::Font::load("Inconsolata.otf", KIT_STATIC_DATA);
  }
  return kit::Font::m_systemFont;
}