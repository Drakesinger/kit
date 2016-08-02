#ifndef KIT_IMGUIIMPL_HPP
#define KIT_IMGUIIMPL_HPP

#include "Kit/WindowEvent.hpp"
#include "Kit/GL.hpp"

#include <memory>

struct ImDrawData;

namespace kit
{
  class Window;
  typedef std::weak_ptr<Window> WindowWPtr;

  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;
  
  class KITAPI UISystem
  {
    public:
      UISystem();
      ~UISystem();
      
      typedef std::vector<std::pair<std::string, kit::TexturePtr>> SelectionList;
      typedef SelectionList::iterator SelectionIterator;

      static void setWindow(kit::WindowWPtr);

      static void handleEvent(kit::WindowEvent const & evt);
      static void prepareFrame(double const & ms);
      static void render();

      static bool usesMouse();
      static bool usesKeyboard();
      static bool usesTextInput();

      static kit::TexturePtr selectTexture(std::string name, kit::TexturePtr currentTexture, bool srgb = true, bool reload = false, std::string prefix = "");
      
      static SelectionIterator select(std::string title, SelectionIterator currIndex, SelectionList & list);
      
    private:
      kit::GL m_glSingleton;
      static void invalidateDeviceObjects();
      static bool createDeviceObjects();
      static bool createFontsTexture();
      
      static void allocateShared();
      static void releaseShared();
      static void __imgui_renderDrawLists(ImDrawData* draw_data);
      static const char* __imgui_getClipboardText();
      static void __imgui_setClipboardText(const char* text);

      static uint32_t m_instanceCount;
      static kit::WindowWPtr m_window;
      static bool m_usesKeyboard;
      static bool m_usesMouse;
      static bool m_usesTextInput;

      static bool __imgui_mousePressed[3];
      static float __imgui_mouseWheel;
      static GLuint __imgui_fontTexture;
      static int __imgui_shaderHandle;
      static int __imgui_vertHandle;
      static int __imgui_fragHandle;
      static int __imgui_attribLocationTex;
      static int __imgui_attribLocationProjMtx;
      static int __imgui_attribLocationPosition;
      static int __imgui_attribLocationUV;
      static int __imgui_attribLocationColor;
      static unsigned int __imgui_vboHandle;
      static unsigned int __imgui_vaoHandle;
      static unsigned int __imgui_elementsHandle;
  };
};

#endif
