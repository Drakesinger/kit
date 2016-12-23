#pragma once

#include "Kit/Export.hpp"

#ifdef _WIN32
  #include <ft2build.h>
#elif __unix__
  #include <ft2build.h>
//#include <freetype2/ft2build.h>
#endif
#include FT_FREETYPE_H

#include <glm/glm.hpp>

#include <memory>
#include <map>

namespace kit 
{
  class Texture;
  
  class KITAPI Font
  {
    public:
      
      struct KITAPI Glyph
      {
        glm::uvec2 m_size;     // Size of this glyph in pixels
        glm::ivec2 m_placement;// Placement position of this glyph, relative to cursor, in pixels
        glm::vec2   m_advance;  // Cursor advancement, in pixels
        glm::vec4   m_uv;       // UV Coordinates for this glyph in the cachemap. XY is topleft, ZW is bottomright
      };

      class KITAPI GlyphMap
      {
        public:
          GlyphMap(kit::Font * font, float size);
          ~GlyphMap();
          kit::Font::Glyph const & getGlyph(wchar_t character);
          kit::Texture * getTexture();
          const float & getLineHeight();
          const float & getHeight();
          
          float getLineAdvance();
          
        private:
          kit::Texture * m_texture = nullptr;
          std::map<wchar_t, kit::Font::Glyph> m_glyphIndex;
          float m_lineHeight;
          float m_height;
      };

      Font(const std::string& filename, char const * dataDirectory = "./data/");

      void precacheSize(float size);
      
      GlyphMap *getGlyphMap(float size);

      FT_Face & getFTFace();
      
      Font();
      ~Font();
      
      static kit::Font * getSystemFont();
      
    private:
      std::map<float, GlyphMap*> m_glyphCache; ///< Index of glyphmaps of different sizes
      
      static uint32_t m_instanceCount;
      
      static FT_Library m_ftLibrary;
      FT_Face m_ftFace; ///< Freetype Face instance
      bool m_ftLoaded;
      
      static kit::Font * m_systemFont;
  };
}
