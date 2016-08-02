#include "Application.hpp"
#include "WorldEditorState.hpp"
//#include "vld.h"

int main(int argc, char* argv[])
{
  try 
  {
    std::cout << "Initializing world editor.." << std::endl;
    kwe::Application app;
    
    std::cout << "Running.." << std::endl;
    app.run(kwe::WorldEditorState::create());
  }
  catch(kit::Exception & e)
  {
    std::cout << "Exception caught: " << e.what() << std::endl << "In file \"" << e.file() << "\" on line " << e.line() << " (in method " << e.method() << ")" << std::endl << std::endl;
  }

#ifdef _WIN32
  std::cin.get();
#endif

  return 0;
}