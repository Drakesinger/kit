#ifndef KWE_EDITORTOOL_HPP
#define KWE_EDITORTOOL_HPP

#include "EditorComponent.hpp"

namespace kit 
{
  class Texture;
  typedef std::shared_ptr<Texture> TexturePtr;
}

namespace kwe 
{
  class EditorTool : public kwe::EditorComponent
  {
    public:
      virtual kit::TexturePtr getIcon()=0;
      virtual void onActive()=0;
      virtual void onInactive()=0;
  };
  
}

#endif 