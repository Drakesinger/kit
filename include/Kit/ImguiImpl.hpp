#pragma once

#include "Kit/WindowEvent.hpp"

struct ImDrawData;

namespace kit
{
  class Window;
  

  class Texture;
  
  class KITAPI UISystem
  {
    public:
      UISystem();
      ~UISystem();
      
      typedef std::vector<std::pair<std::string, kit::Texture*>> SelectionList;
      typedef SelectionList::iterator SelectionIterator;

      static void setWindow(kit::Window*);

      static void handleEvent(kit::WindowEvent const & evt);
      static void prepareFrame(double const & ms);
      static void render();

      static bool usesMouse();
      static bool usesKeyboard();
      static bool usesTextInput();

      static kit::Texture * selectTexture(const std::string& name, kit::Texture * currentTexture, bool srgb = true, bool reload = false, const std::string& prefix = "");
      
      static SelectionIterator select(const std::string& title, SelectionIterator currIndex, SelectionList & list);
      
    private:
      static void invalidateDeviceObjects();
      static bool createDeviceObjects();
      static bool createFontsTexture();
      
      static void allocateShared();
      static void releaseShared();
      static void imgui_renderDrawLists(ImDrawData* draw_data);
      static const char* imgui_getClipboardText();
      static void imgui_setClipboardText(const char* text);

      static uint32_t m_instanceCount;
      static kit::Window * m_window;
      static bool m_usesKeyboard;
      static bool m_usesMouse;
      static bool m_usesTextInput;

      static bool imgui_mousePressed[3];
      static float imgui_mouseWheel;
      static uint32_t imgui_fontTexture;
      static int imgui_shaderHandle;
      static int imgui_vertHandle;
      static int imgui_fragHandle;
      static int imgui_attribLocationTex;
      static int imgui_attribLocationProjMtx;
      static int imgui_attribLocationPosition;
      static int imgui_attribLocationUV;
      static int imgui_attribLocationColor;
      static unsigned int imgui_vboHandle;
      static unsigned int imgui_vaoHandle;
      static unsigned int imgui_elementsHandle;
  };
}
