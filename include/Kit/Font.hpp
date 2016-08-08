#ifndef KIT_FONT_HPP
#define KIT_FONT_HPP

#include "Kit/Export.hpp"
#include "Kit/GL.hpp"

#ifdef _WIN32
  #include <ft2build.h>
#elif __unix__
  #include <ft2build.h>
//#include <freetype2/ft2build.h>
#endif
#include FT_FREETYPE_H
#include <memory>
#include <map>

namespace kit 
{
  class Texture;
  typedef std::weak_ptr<Texture> TextureWPtr;
  typedef std::shared_ptr<Texture> TexturePtr;

  class KITAPI Font : public std::enable_shared_from_this<Font>
  {
    public:
      typedef std::shared_ptr<kit::Font> Ptr;
      typedef std::weak_ptr<kit::Font> WPtr;

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
          GlyphMap(kit::Font::WPtr font, float size);
          kit::Font::Glyph const & getGlyph(wchar_t character);
          kit::TextureWPtr getTexture();
          const float & getLineHeight();
          const float & getHeight();
          
          float getLineAdvance();
          
        private:
          kit::TexturePtr m_texture;
          std::map<wchar_t, kit::Font::Glyph> m_glyphIndex;
          float m_lineHeight;
          float m_height;
      };

      static kit::Font::Ptr load(const std::string& filename);

      void precacheSize(float size);
      
      GlyphMap *getGlyphMap(float size);

      FT_Face & getFTFace();
      
      Font();
      ~Font();
      
      static kit::Font::Ptr getSystemFont();
      
    private:
      kit::GL m_glSingleton;
      std::map<float, GlyphMap*> m_glyphCache; //< Index of glyphmaps of different sizes
      
      static uint32_t m_instanceCount;
      
      static FT_Library m_ftLibrary;
      FT_Face m_ftFace; //< Freetype Face instance
      bool m_ftLoaded;
      
      static kit::Font::Ptr m_systemFont;
  };
}

#endif