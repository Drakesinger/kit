#ifdef KIT_DLL
  #ifdef __MINGW32__
    #define KITAPI 
    #define KITEXPORT 
  #elif _WIN32
    #ifdef KIT_IS_BUILD
      #define KITAPI __declspec(dllexport)
      #define KITEXPORT __declspec(dllexport)
    #else
      #define KITAPI __declspec(dllimport)
      #define KITEXPORT 
    #endif
  #elif __unix__
    #define KITAPI 
    #define KITEXPORT 
  #else
    #define KITAPI
    #define KITEXPORT 
  #endif
#else
  #define KITAPI
  #define KITEXPORT 
#endif