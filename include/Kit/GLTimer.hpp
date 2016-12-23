#pragma once

#include "Kit/Export.hpp"
#include <cstdint>

namespace kit 
{
  class KITAPI GLTimer
  {
  public:
    
    GLTimer();
    ~GLTimer();
    
    void start();
    uint64_t end(); // Returns nanoseconds taken since start
    
  private:
    uint32_t m_glHandle;
  };
}
