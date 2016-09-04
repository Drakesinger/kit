#include "../include/Application.hpp"
#include "../include/MaterialDesignerState.hpp"
//#include "vld.h"

int main(int argc, char* argv[])
{
  try 
  {
    // 

    std::cout << "Initializing material designer.." << std::endl;
    kmd::Application app;
    
    std::cout << "Running.." << std::endl;
    app.run(kmd::MaterialDesignerState::create());
  }
  catch(kit::Exception & e)
  {
    std::cout << "Exception caught: " << e.what() << std::endl << "In file \"" << e.file() << "\" on line " << e.line() << " (in method " << e.method() << ")" << std::endl << std::endl;
  }
  
#ifdef _WIN32
  std::cout << "Press any key to continue..." << std::endl;
  std::cin.get();
#endif

  return 0;
}