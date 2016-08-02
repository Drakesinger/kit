#ifndef KIT_SCRIPTLIB_HPP
#define KIT_SCRIPTLIB_HPP

#include "Kit/Export.hpp"

#include <chaiscript/chaiscript.hpp>

namespace kit 
{

  template<typename T>
  KITEXPORT bool __script_enum_opequals(const T & lhs, const T & rhs)
  {
    return (lhs == rhs);
  }
  
  chaiscript::ModulePtr KITAPI  getScriptLibrary();
  
  class KITAPI Scriptable
  {
    public:
      Scriptable();
      virtual ~Scriptable();
      
      chaiscript::ChaiScript & getScriptEngine();

    protected:
      chaiscript::ChaiScript m_scriptEngine;
    
  };
}

#endif